/*
 * taskmanager.cpp
 * 
 * Task manager class implementation. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "taskmanager.hpp"
#include "tsource/taskfile.hpp"

#include "admin/adminport.hpp"

#include <ctype.h>
#include <fcntl.h>
#include <assert.h>


#ifndef TESTING
#define TESTING 1
/* This is for testing: before dequeuing a task from the todo/proc list, 
 * check if is actually on that list: */
#define TEST_TASK_LIST_MEMBERSHIP   1
#endif


#define UnsetNegMagic  (-29649)

#ifndef __LINE__
# warning No __LINE__ info provided. 
# define __LINE__ -1
#endif


#if TESTING
#warning TESTING switched on. 
#endif


// NOTE: 
// TaskManager contains a highly non-trivial state machine. 
// I expect that there are some bugs in it. 
// Lots of circumstances should be checked: (TODO, FIXME)
//   (e.g. What if schedule_quit and tid_ts_cwait is active, etc.)

/******************************************************************************/
// NOTE...
// dont_start_more_tasks -> no more tasks will be started. 
// schedule_quit -> may start more tasks but do not start NEW tasks. 
//     set to 2 if exit code shall be non-zero (i.e. 1)
// sched_kill_tasks -> send TERM and schedule KILL on all tasks in _schedule()
//     Value 1/2 for user interrupt / server error
// kill_tasks_and_quit_now -> necessary if connection to task source failed; 
//     always also set sched_kill_tasks. 
// schedule_quit_after: like schedule_quit but allows to process all 
//     tasks in the queue before quitting. 
// recovering -> uahh.. now it gets complicated. NOTE that recovering does 
//     not set any of the quit flags but is similar to them in many ways. 
//     Recovery is done when we re-start because the active task source 
//     got disconnected. 
/******************************************************************************/
// Task lists: 
//  - todo list: All task to be done. Also, scheduled_for_start is in 
//               todo list. ctsk->d.any() may NEVER be set. 
//  - proc list: All tasks currently being processed. 
//               ctsk->d.any() must be set. 
//  - done list: All tasks to be reported to the task source as done 
//               (with or without error). 
/******************************************************************************/


static volatile void __CheckAllocFailFailed()
{
	Error("%s.\n",cstrings.allocfail);
	abort();
}

static inline void _CheckAllocFail(int failed)
{
	if(failed)
	{  __CheckAllocFailFailed();  }
}


inline int cmpMAX(int a,int b)
{  return((a>b) ? a : b);  }


// This is called to tell the active task source to deliver 
// no more tasks to us. 
void TaskManager::_NoMoreNewTasksFromTSource()
{
	if(GetTaskSourceType()!=TST_Active)  return;
	
	if(told_tasksource_to_quit)  return;
	
	if(!told_want_no_more_tasks)
	{
		int rv=TSDisconnect(/*special=*/2);
		assert(rv==0);  // Everything else is an error somewhere, I think...
		told_want_no_more_tasks=1;
	}
}


void TaskManager::DontStartMoreTasks()
{
	// Note: We may enter this function also in case 
	//       dont_start_more_tasks is already set. In this case 
	//       we MUST NOT return immediately but go through tasklist.todo 
	//       again and re-queue tasks from todo to done queue if needed. 
	
	dont_start_more_tasks=1;
	if(GetTaskSourceType()==TST_Active)
	{
		// Tell active task source that we no longer take tasks. 
		// The task source then will recject tasks from the server. 
		_NoMoreNewTasksFromTSource();
	}
	
	_KillScheduledForStart();    // ALWAYS!
	
	// Tell interface (LDR client) to give back all todo tasks. 
	interface->ScheduleGiveBackTasks(/*may_keep=*/0);
	
	for(CompleteTask *_i=tasklist.todo.first(); _i; )
	{
		CompleteTask *i=_i;  _i=_i->next;
		
		assert(!i->d.any());   // Otherwise: belongs into proc list. 
		
		// If the next assert fails, then this probably means that there 
		// is a fresh task in todo queue which has not yet been set up 
		// using interface->DealWithNewTask(). This is a bug because if 
		// we put a TaskDone - task into done queue it is regarded as 
		// done completely but in fact it was not worked on yet. 
		assert(i->state!=CompleteTask::TaskDone);
		
		if(!i->ProcessedTask())  continue;
		// Here: *i was processed and is currently not processed but 
		// still in the todo list (i.e. *i rendered but not yet 
		// filtered). 
		assert(i->state==CompleteTask::ToBeFiltered);
		
		// In this case: Move task to done queue. 
		tasklist.todo.dequeue(i);  --tasklist.todo_nelem;
		tasklist.done.append(i);   ++tasklist.done_nelem;
	}
}


void TaskManager::HandleSuccessfulJob(CompleteTask *ctsk)
{
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return;  }
	
	// Great, everything went fine. 
	// Reset this counter: 
	jobs_failed_in_sequence=0;
	
	#if TEST_TASK_LIST_MEMBERSHIP
	// Test: see if task is actually in tasklist.proc: 
	assert(tasklist.proc.find(ctsk));
	#endif
	assert(!ctsk->d.any());  // Must already be NULL. 
	
	// Dump all the information to the user (if requested):
	// DO NOT CHANGE THE ORDER OF THE IF STATEMENTS. 
	if(ctsk->ft && ctsk->ftes.tes.GetTermReason()!=TTR_Unset)
	{
		_AccumulateTimeStat(DTFilter,&ctsk->ftes.tes);
		DumpTaskInfo(ctsk,NULL,DTSK_NowFiltered,VERBOSE_TDR);
	}
	else if(ctsk->rt && ctsk->rtes.tes.GetTermReason()!=TTR_Unset)
	{
		_AccumulateTimeStat(DTRender,&ctsk->rtes.tes);
		DumpTaskInfo(ctsk,NULL,DTSK_NowRendered,VERBOSE_TDR);
	}
	else assert(0);
	
	switch(ctsk->state)
	{
		case CompleteTask::TaskDone:
			// Good, task is done completely. 
			
			tasklist.proc.dequeue(ctsk);  --tasklist.proc_nelem;
			tasklist.done.append(ctsk);   ++tasklist.done_nelem;
			
			// This is definitely needed: 
			_CheckStartTasks();
			
			// See if we have to connect to the task source and schedule 
			// that if needed: 
			_CheckStartExchange();
			
			break;
		case CompleteTask::ToBeFiltered:
			// Okay, task was rendered and has to be filtered now
			
			// Re-queue into todo queue (at the BEGINNING): 
			tasklist.proc.dequeue(ctsk);  --tasklist.proc_nelem;
			tasklist.todo.insert(ctsk);   ++tasklist.todo_nelem;
			
			// In case dont_start_more_tasks is set we must make sure that we 
			// do not bring the state machine into a state where that was 
			// required: If we do not start more tasks, then a successful 
			// task which is now to be filtered cannot be filtered any more 
			// and so has to be put into the lasklist.done queue which is 
			// exactly what DontStartMoreTasks() does. That is why we only 
			// call DontStartMoreTasks() in case dont_start_more_tasks is set. 
			if(dont_start_more_tasks)  // CORRECT!!
			{  DontStartMoreTasks();  }
			
			_CheckStartTasks();
			
			break;
		case CompleteTask::ToBeRendered:  // fall through
		default:  assert(0);  break;
	}
}


// This is to be called when a task has really failed, NOT if you 
// want to re-try or do the second half of the work. 
// running_jobs: number of still running jobs; ONLY FOR emergency error handling. 
void TaskManager::HandleFailedTask(CompleteTask *ctsk,int running_jobs,
	TaskDriverType which_dtype)
{
	Verbose(DBGV,"HandleFailedTask([%d],%d,sfs=%s)\n",
		ctsk->frame_no,running_jobs,
		(ctsk==scheduled_for_start) ? "yes" : "no");
	
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return;  }
	
	// Error info already stored. 
	
	if(ctsk==scheduled_for_start)
	{
		assert(launching_special_flag);
		// Called on the stack of schedule() -> LaunchTask(). 
		// In this case, ctsk is still in tasklist.todo. 
		#if TEST_TASK_LIST_MEMBERSHIP
		// Test: see if task is actually in tasklist.proc: 
		assert(tasklist.todo.find(ctsk));
		#endif
		
		// Also, we must kill scheduled for start: 
		_KillScheduledForStart();
		
		// In any case, we have to put the task into done queue. 
		tasklist.todo.dequeue(ctsk);  --tasklist.todo_nelem;
		tasklist.done.append(ctsk);   ++tasklist.done_nelem;
	}
	else
	{
		#if TEST_TASK_LIST_MEMBERSHIP
		// Test: see if task is actually in tasklist.proc: 
		assert(tasklist.proc.find(ctsk));
		#endif
		
		tasklist.proc.dequeue(ctsk);  --tasklist.proc_nelem;
		tasklist.done.append(ctsk);   ++tasklist.done_nelem;
	}
	
	assert(!ctsk->d.any());   // Must already be NULL. 
	
	// Dump all the information to the user (if requested):
	TaskExecutionStatus *fail_tes=NULL;
	switch(which_dtype)
	{
		case DTFilter:
			_AccumulateTimeStat(DTFilter,&ctsk->ftes.tes);
			DumpTaskInfo(ctsk,NULL,DTSK_NowFiltered,VERBOSE_TDR);
			fail_tes=&ctsk->rtes.tes;
			break;
		case DTRender:
			_AccumulateTimeStat(DTRender,&ctsk->rtes.tes);
			DumpTaskInfo(ctsk,NULL,DTSK_NowRendered,VERBOSE_TDR);
			fail_tes=&ctsk->ftes.tes;
			break;
		default: assert(0);
	}
	
	if(kill_tasks_and_quit_now)
	{
		// No, we only kill 'em and quit. Nothing else. 
		if(!running_jobs)
		{
			Warning("No more running jobs, QUITTING NOW.\n");
			_ActuallyQuit(2);
		}
		return;  // necessary! ...
		// ... because it will be normal that the tasks have 
		//     edsetail==TaskDriver::EDFailed (thus not EDSuccess) 
		//     here. And we won't muck with errors now and we 
		//     do NOT (try to) connect to the task source. We 
		//     are here becuase of severe error (probably because 
		//     connecton to TaskSource failed). 
	}
	
	// Now, if we killed the task and it can be continued or 
	// it was killed due to user request, then do not count as 
	// failure. 
	bool count_as_failed=1;
	if(fail_tes->WasKilledByUs())
	{
		if( (which_dtype==DTRender && ctsk->IsPartlyRenderedTask()) || 
		    (which_dtype==DTFilter && ctsk->IsPartlyFilteredTask()) )
		{  count_as_failed=0;  }
		else
		{
			TaskTerminationReason ttr;
			ttr=(TaskTerminationReason)(fail_tes->rflags & TTR_JK_Mask);
			if(ttr==TTR_JK_UserIntr)
			{  count_as_failed=0;  }
		}
	}
	if(count_as_failed)
	{
		++jobs_failed_in_sequence;
		if(max_failed_in_sequence && 
		   jobs_failed_in_sequence>=max_failed_in_sequence)
		{
			if(!failed_in_sequence_reported)
			{
				Error("%d jobs failed in sequence. Giving up.\n",
					jobs_failed_in_sequence);
				failed_in_sequence_reported=1;
			}
			
			DontStartMoreTasks();
			
	// For active task sources, we probably do not want to 
	// schedule quit but recover instead. 
	// ---> tell task source to quit (special=1) and do recovery. 
	//      ^^^^^^ REALLY?! -- IMHO this should work as it is here: 
	
			if(!recovering)  // We might already be recovering (waiting for tasks to finish)
			{  _QuitOrBeginRecovery();  }
			ReSched(__LINE__);
		}
	}
	
	// This is definitely needed: 
	_CheckStartTasks();
	
	_CheckStartExchange();
	
	// Furthermore, we should probably send back the 
	// done (and failed) tasks first before getting 
	// new ones: 
	if(connected_to_tsource)
	{  ts_done_all_first=true;  }
}


void TaskManager::LaunchingTaskDone(CompleteTask *ctsk)
{
	// Currently, this is only called on success. 
	
	assert(ctsk->d.any());
	#if TEST_TASK_LIST_MEMBERSHIP
	// Test: see if task is actually in tasklist.proc: 
	assert(tasklist.proc.find(ctsk));
	#endif
	
	// See if we can start more tasks. (NEEDED.)
	_CheckStartTasks();

	// This IS needed, too. (Checked!) 
	_CheckStartExchange();
}


// Called for a task which was just obtained from the task source. 
// from_take_task: 1 -> TASTakeTask (active task source)
void TaskManager::_TakeFreshTask(CompleteTask *ctsk,int /*from_take_task*/)
{
	assert(ctsk);   // otherwise internal error
	
	// Put task into queue (at the END of the queue): 
	tasklist.todo.append(ctsk);  ++tasklist.todo_nelem;
	
	// NOTE: ni->ctsk->state = TaskDone here. Must set up 
	//       proper value now and set up TaskParams: 
	int if_rv=interface->DealWithNewTask(ctsk);
	// This called HandleFailedTask() on error. 
	
	// NOTE: IT IS IMPORTANT THAT we call interface->DealWithNewTask() 
	//       before [-> proper setup of task state] and that we first 
	//       queue the task in todo list (DontStartMoreTasks() will 
	//       put it into done list again). 
	if(dont_start_more_tasks || schedule_quit || schedule_quit_after)
	{
		if(if_rv)
		{  assert(tasklist.todo.is_empty());  }
		else
		{
			// Oops... We don't start any more new tasks. 
			// So, we can put this task right back into the done queue. 
			Warning("Fresh task [frame %d] immediately marked done unprocessed.\n",
				ctsk->frame_no);
			
			DontStartMoreTasks();
		}
		
		// Maybe we want to give it right back... 
		_CheckStartExchange();
		return;
	}
	
	// Great. Task queued; now see if we want to start a job. 
	_CheckStartTasks();
}


// running_jobs just needed to be able to pass it to 
// HandleFailedTask() if needed. 
void TaskManager::PutBackTask(CompleteTask *ctsk,int running_jobs)
{
	assert(ctsk);   // otherwise internal error
	
	// NOTE: ctsk is queued in tasklist.proc (YES!)
	//       although ctsk->d.any() is already 0. 
	#if TEST_TASK_LIST_MEMBERSHIP
	assert(tasklist.proc.find(ctsk));
	#endif
	
	// If someone puts back a task, then it may not have a 
	// task driver or LDR client attached. 
	assert(!ctsk->d.any());
	
	// NOTE: We MAY dequeue or re-queue the passed ctsk here 
	//       (the caller knows that and has removal-safe loop). 
	
	// In case the task failed (see LDRClient and _MarkTaskFailed()), 
	// ctsk->{rtes,ftes}->rflags are no longer TTR_Unset. In this case, 
	// LDRClient thinks that it would not be useful to put the task 
	// into todo list because it is very likely to fail again. 
	bool render_is_good=(!ctsk->rt || ctsk->rtes.tes.rflags==TTR_Unset || 
		ctsk->rtes.tes.IsCompleteSuccess());
	bool filter_is_good=(!ctsk->ft || ctsk->ftes.tes.rflags==TTR_Unset || 
		ctsk->ftes.tes.IsCompleteSuccess());
	Verbose(DBG,"PutBackTask: render=%s, filter=%s [frame %d] "
		"(squit=%d,%d; dsmt=%d)\n",
		render_is_good ? "good" : "bad",
		filter_is_good ? "good" : "bad",
		ctsk->frame_no,
		int(schedule_quit),int(schedule_quit_after),
		int(dont_start_more_tasks));
	if(render_is_good && filter_is_good)
	{
		if((!ctsk->rt || ctsk->rtes.tes.IsCompleteSuccess()) && 
		   (!ctsk->ft || ctsk->ftes.tes.IsCompleteSuccess()) )
		{
			// Wow. A completely successfully done task here?! 
			// That may not happen. 
			// (In case it does, we should probably put it in done queue.) 
			assert(0);
		}
		
		// Re-queue at BEGINNING of todo list. 
		tasklist.proc.dequeue(ctsk);  --tasklist.proc_nelem;
		tasklist.todo.insert(ctsk);   ++tasklist.todo_nelem;
		
		// See also the note on the call to DontStartMoreTasks() in 
		// HandleSuccessfulJob(). In case we no not start more tasks, 
		// we may not bring the state machine in a situation where it 
		// would have to. 
		if(dont_start_more_tasks)
		{  DontStartMoreTasks();  }
		
		// We put task into todo queue. 
		// In case quit is scheduled, call _CheckStartExchange() which 
		// will put back tasks from todo queue if not processed. 
		// THIS MAY RESULT IN MORE CONNECT-DISCONNECT CYCLES THAN NECESSARY. 
		//#warning fixme?! (We should, but how?)
		if(schedule_quit)
		{  _CheckStartExchange();  }
	}
	else
	{
		TaskDriverType failed_dtype=DTNone;
		// DO NOT CHANGE ORDER. I use DTFilter if render and filter 
		// marked as failed. 
		// The logic here relies on the fact that there is never a failed 
		// job in the todo queue. Because if rendering failed, we cannot 
		// filter any more and if filtering failed, we put it straight 
		// into done queue. 
		// The next line is essentially: 
		//if(ctsk->ft && ctsk->ftes.tes.rflags!=TTR_Unset && 
		//	!ctsk->ftes.tes.IsCompleteSuccess())
		if(!filter_is_good)
		{  failed_dtype=DTFilter;  }
		else if(!render_is_good)
		{  failed_dtype=DTRender;  }
		else assert(0);  // Should branch into if() above!!
		
		HandleFailedTask(ctsk,running_jobs,failed_dtype);
		
		// No need to call DontStartMoreTasks() like below because 
		// todo list was not touched. 
		// _CheckStartExchange() called by HandleFailedTask(). 
	}
	
}


void TaskManager::_DoHandleGiveBackTasks(int may_keep)
{
	// NOTE: special value: may_keep==0x7fffffff -> give back 
	//       all not yet processed tasks. 
	
	char giveback_str[24];
	if(may_keep==0x7fffffff)
	{  strcpy(giveback_str,"not processed");  }
	else
	{
		int will_give_back=tasklist.todo_nelem-may_keep;
		snprintf(giveback_str,24,"%d",will_give_back<0 ? 0 : will_give_back);
	}
	Verbose(TDI,"Giving back %s tasks:",giveback_str);
	
	int wanted_to_start=0;
	
	// Simply put tasks from todo to done queue: 
	for(int pass=0; pass<2; pass++)
	{
		if(pass==0 && may_keep!=0x7fffffff)
		{  Verbose(TDI," unprocessed:");  }
		else if(pass==1)
		{  Verbose(TDI," processed:");  }
		
		// pass=0: first, give back tasks which were not yet processed
		// pass=1: also give back tasks which were already processed. 
		// Traverse list backwards to give back newest tasks first. 
		for(CompleteTask *_i=tasklist.todo.last(); _i; )
		{
			if(may_keep!=0x7fffffff && tasklist.todo_nelem<=may_keep)
			{  goto breakdone;  }
			
			CompleteTask *i=_i;
			_i=_i->prev;
			
			assert(i->state!=CompleteTask::TaskDone);
			assert(!i->d.any());   // Otherwise: proc list. 
			
			if(i->ProcessedTask() && pass==0)  continue;
			
			if(i==scheduled_for_start)
			{  _KillScheduledForStart();  wanted_to_start=1;  }
			
			// Remove from todo queue and append to done queue: 
			tasklist.todo.dequeue(i);  --tasklist.todo_nelem;
			tasklist.done.append(i);   ++tasklist.done_nelem;
			
			Verbose(TDI," [%d]",i->frame_no);
		}
		
		if(may_keep==0x7fffffff)  break;
	}
	breakdone:;
	Verbose(TDI," (todo: %d, proc: %d, done: %d)\n",
		tasklist.todo_nelem,tasklist.proc_nelem,tasklist.done_nelem);
	assert(tasklist.todo_nelem<=may_keep);
	
	// Schedule giving them back: 
	// (Setting ts_done_all_first is enough to trigger the 
	// starting of an exchange.) 
	ts_done_all_first=1;
	_CheckStartExchange();
	
	if(wanted_to_start || fit_strategy_flag==FSF_Tight)
	{  _CheckStartTasks();  }
}


CompleteTask *TaskManager::FindTaskByTaskID(u_int32_t task_id)
{
	//#warning could speed up this loopup in some way?
	for(CompleteTask *ctsk=tasklist.proc.first(); ctsk; ctsk=ctsk->next)
	{
		if(ctsk->task_id==task_id)
		{  return(ctsk);  }
	}
	for(CompleteTask *ctsk=tasklist.todo.first(); ctsk; ctsk=ctsk->next)
	{
		if(ctsk->task_id==task_id)
		{  return(ctsk);  }
	}
	return(NULL);
}


int TaskManager::tsnotify(TSNotifyInfo *ni)
{
	Verbose(DBG,"--<tsnotify>--<%d>--\n",ni->action);
	
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	// Will be just verbose for success codes: 
	TSWriteError(ni);
	
	switch(ni->action)
	{
		case ANone:  assert(0);  break;
		case AConnect:
		{
			assert(pending_action==AConnect);
			assert(!connected_to_tsource);
			
			// Must stop connect re-try timer (if active): 
			UpdateTimer(tid_ts_cwait,-1,0);
			
			// Check error code: 
			if(ni->connstat==CSConnected)
			{
				// Successfully connected. 
				pending_action=ANone;
				connected_to_tsource=1;
				
				// Okay, now as we're connected we must get 
				// tasks and tell the source about done tasks. 
				tsgod_next_action=ANone;
				//ts_done_all_first=0; <-- Used to be reset here. 
				//                Now we do that when done queue is empty. 
				nth_call_to_get_task=0;
				_TS_GetOrDoneTask();
			}
			else if(ni->connstat==CSWorking)
			{
				// Working...
				// pending_action stays AConnect. 
				// simply, WAIT A LITTLE...
			}
			else
			{
				// Failed to connect to the task source. 
				pending_action=ANone;
				
				// Okay, there are two chioces: 
				//  1) quit immediately. (Not all that scheduling stuff 
				//     because
				//     a) doing more work is worthless if we cannot talk 
				//        to the task source
				//     b) we may not try to connect again (and maybe wait 
				//        for another connect() timeout)
				//  2) wait some time and try again. 
				long msec=TSConnectRetryMakesSense();
				assert(msec>=0);
				if(msec)
				{
					pending_action=AConnect;
					UpdateTimer(tid_ts_cwait,msec,20 | FDAT_AlignToShorter);
				}
				else
				{
					Error("Doing more work makes no sense. I'm upset.\n");
					DontStartMoreTasks();
					schedule_quit=2;
					sched_kill_tasks=2;  // server error
					kill_tasks_and_quit_now=1;
					ReSched(__LINE__);
				}
				return(0);
			}
		}  break;
		case AGetTask:
		{
			assert(pending_action==AGetTask);
			assert(connected_to_tsource);
			
			// Check error code: 
			if(ni->getstat==GTSGotTask)
			{
				// Successfully got task. 
				pending_action=ANone;
				
				_TakeFreshTask(ni->ctsk,0);
				
				// Go on talking to task source: 
				_TS_GetOrDoneTask();
			}
			else if(ni->getstat==GTSWorking)
			{
				// Working...
				// pending_action stays AGetTask. 
				// simply, WAIT A LITTLE...
			}
			else if(ni->getstat==GTSNoMoreTasks)
			{
				pending_action=ANone;
				
				// No more tasks. 
				// Okay, schedule quit when all available tasks are done: 
				if(!schedule_quit_after)
				{  schedule_quit_after=1;  }
				// Also, set the fit strategy flag: 
				fit_strategy_flag=FSF_Tight;
				ReSched(__LINE__);
			}
			else if(ni->getstat==GTSEnoughTasks)
			{
				pending_action=ANone;
				
				// So the task source thinks we have enough tasks. 
				// Let's see: 
				if(!tasklist.done.is_empty())
				{
					// Okay, first we report `done´ for all done tasks. 
					ts_done_all_first=1;
					_TS_GetOrDoneTask();
				}
				else if(tasklist.todo.is_empty() && tasklist.proc.is_empty())
				{
					// No, the task source is wrong. 
					Error("Task source thinks we have enough tasks but that's "
						"not true.\n");
					schedule_quit=2;
					ReSched(__LINE__);
				}
				else
				{
					// Okay, disconnect for now. 
					int rv=TSDisconnect();
					assert(!rv);   // ...otherwise internal error
					pending_action=ADisconnect;
					//#warning check if that results in too many connect - disconnect - connect ... calls. 
				}
			}
			else
			{
				pending_action=ANone;
				
				// Uha... some error happened. 
				// FIXME...
				// If this is the first call to TSGetTask(), then schedule 
				// a quit. Otherwise do all the DoneTask first. 
				if(nth_call_to_get_task==1)
				{
					Error("Failed task query was first one. Quitting.\n");
					schedule_quit_after=2;
					_NoMoreNewTasksFromTSource();  // This may or may not be necessary here...
					ReSched(__LINE__);
				}
				else
				{
					ts_done_all_first=1;
					_TS_GetOrDoneTask();
				}
			}
		}  break;
		case ADoneTask:
		{
			assert(pending_action==ADoneTask);
			assert(connected_to_tsource);
			
			// Check error code: 
			if(ni->donestat==DTSOkay)
			{
				pending_action=ANone;
				
				// Write this frame down in the last frame number history: 
				if(last_pend_done_frame_no>=0)  // non failed
				{
					if(lpf_hist_size>0)
					{
						last_proc_frames[lpf_hist_idx]=last_pend_done_frame_no;
						lpf_hist_idx=(lpf_hist_idx+1)%lpf_hist_size;
					}
					last_pend_done_frame_no=-1;
				}
				
				// Okay, go on talking to the task source: 
				_TS_GetOrDoneTask();
			}
			else if(ni->donestat==DTSWorking)
			{
				// Working...
				// pending_action stays ADoneTask. 
				// simply, WAIT A LITTLE...
			}
			else
			{
				// I hate that. Error when calling DoneTask(). 
				// Why did we do the work then, he?
				// FIXME: currently, no error can happen. 
				Error("Please implement me. ABORTING.\n");
				assert(0);
			}
		}  break;
		case ADisconnect:
		{
			assert(pending_action==ADisconnect);
			#if TESTING
			if(!connected_to_tsource && !told_tasksource_to_quit)
			{  assert(0);  }
			#endif
			
			if(ni->disconnstat==DSOkay)
			{
				// Successfully disconnected. 
				pending_action=ANone;
				connected_to_tsource=0;
				// Nothing else to do. Just wait. 
				assert(!told_tasksource_to_quit);
			}
			else if(ni->disconnstat==DSQuitOkay)
			{
				pending_action=ANone;
				connected_to_tsource=0;
				// We told the task source to quit. In this 
				// case we must go on quitting...
				assert(told_tasksource_to_quit);
				ReSched(__LINE__);
			}
			else if(ni->disconnstat==DSWorking)
			{
				// Working...
				// pending_action stays ADisconnect. 
				// simply, WAIT A LITTLE...
			}
			else if(ni->disconnstat==DSQuitting)
			{
				assert(told_tasksource_to_quit);
				// Wait a little...
			}
			else
			{
				// Failed to disconnect from the task source. 
				pending_action=ANone;
				
				// NOTE: CURRENTLY, WE WILL NEVER REACH HERE. 
				
				// This is bad. 
				Error("ABORTING.\n");
				abort();
				//#warning MISSING. 
				// FIXME: 
				// For NRP, we should treat the connection as dead & disconnected 
				// and simply go on. 
				// ALSO TELL THE TaskSource to set its state to NOT connected! 
			}
			
			if(!connected_to_tsource)
			{
				// Make sure we schedule one time if needed: 
				if(schedule_quit || schedule_quit_after || recovering)
				{  ReSched(__LINE__);  }
			}
		}  break;
		case AActive:
		{
			// Interface may not send us active commands when we told it 
			// to quit. 
			assert(!told_tasksource_to_quit);
			
			// Active task source only. 
			// We must neither be connected to the task source to receive 
			// this nor must we have sent a query. 
			if(ni->activestat==TASTakeTask)
			{
				_TakeFreshTask(ni->ctsk,1);
			}
			else if(ni->activestat==TASRecoveringBad || 
			        ni->activestat==TASRecoveringQuit)
			{
				// Bad. We lost connection to the server. 
				// This means, we must now start recovery and call 
				// GetTask() when done [which is normally invalid for 
				// active task sources]. 
				// Okay, then let's recover: 
				
				// This does recovery because task source type is active. 
				if(!recovering)
				{  _QuitOrBeginRecovery();  }
				
				ReSched(__LINE__);
			}
			else if(ni->activestat==TASSimpleCommand)
			{
				switch(ni->cccmd)
				{
					case CCC_Kill_UserInterrupt:
					case CCC_Kill_ServerError:
					{
						int nkilled=interface->TermAllJobs(
							(ni->cccmd==CCC_Kill_UserInterrupt) ? 
							TTR_JK_UserIntr : TTR_JK_ServerErr);
						
						// Maybe, this could be left away: 
						Verbose(TDI,"Sent SIGTERM to %d jobs.\n",nkilled);
						
						// Must also make sure that we do not start more tasks. 
						DontStartMoreTasks();
						
						return(0);  // return to TellTaskManagerToExecSimpleCCmd()
					}  break;
					case CCC_StopJobs:
					case CCC_ContJobs:
					{
						_HandleStopCont(/*action=*/
							(ni->cccmd==CCC_ContJobs) ? +1 : -1);
						
						// Now, as there is no LDR forwarder, stop/cont response 
						// is immediately: 
						assert(exec_stop_status==
							(ni->cccmd==CCC_ContJobs ? ESS_Running : ESS_Stopped));
						
						return(0);  // return to TellTaskManagerToExecSimpleCCmd()
					}  break;
					default:  assert(0);  // BUG, may not happen. 
				}
			}
			else if(ni->activestat==TASGiveBackTasks)
			{
				// This was all put into the following function: 
				_DoHandleGiveBackTasks(/*may_keep=*/ni->int_payload);
				
				return(0);   // return to TellTaskManagerToGiveBackTasks()
			}
			else assert(0);
		}  break;
		default:  assert(0);  break;
	}
	
	return(0);
}


int TaskManager::signotify(const SigInfo *si)
{
	if((si->info.si_signo==SIGINT || si->info.si_signo==SIGTERM) && 
	   abort_on_signal )
	{
		fprintf(stderr,"*** Caught fatal SIG%s. Aborting. ***\n",
			si->info.si_signo==SIGTERM ? "TERM" : "INT");
		abort();
	}
	
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	_ActOnSignal(si->info.si_signo,/*real_signal=*/1);
	return(0);
}

void TaskManager::_ActOnSignal(int signo,int real_signal)
{
	Verbose(DBG,"--<ActOnSignal>--<sig=%d,real=%s>--\n",
		signo,real_signal ? "yes" : "no");
	
	int do_sched_quit=0;
	int do_kill_tasks=0;
	
	if(signo==SIGINT)
	{
		if(!caught_sigint)
		{
			// This is the first SIGINT. 
			++caught_sigint;
			
			if(real_signal)
			{  Warning("Caught SIGINT. No more tasks will be started. "
				"(Next ^C is more brutal.)\n");  }
			
			// First, do not allow NEW tasks to be started. 
			// (Old tasks may be finished even if we have to start new 
			// tasks (i.e. a filter for the rendered image).)
			// --> DO NOT SET dont_start_more_tasks=1;
			
			do_sched_quit=1;
		}
		else if(caught_sigint==1)
		{
			// This is the second SIGINT. 
			++caught_sigint;
			
			if(real_signal)
			{  Warning("Caught second SIGINT. Killing tasks. "
				"(Next ^C will abort %s.)\n",prg_name);  }
			
			do_kill_tasks=1;
			
			abort_on_signal=1;
		}
	}
	else if(signo==SIGTERM)
	{
		if(real_signal)
		{  Warning("Caught SIGTERM. Killing tasks. "
			"(Next signal will abort %s.)\n",prg_name);  }
		
		do_sched_quit=1;
		do_kill_tasks=1;
		
		abort_on_signal=1;
	}
	else if(signo==SIGTSTP)     // Terminal stop 
	{  _HandleStopCont(-1);  }
	else if(signo==SIGCONT)
	{  _HandleStopCont(+1);  }
	else if(signo==SIGUSR1)
	{  _DumpInternalState();  }
	else if(signo==SIGUSR2)
	{  _DumpCompleteTaskList();  }
	
	if(do_sched_quit)
	{
		// Make sure we quit then...
		if(!schedule_quit)
		{  schedule_quit=1;  }  // 1 -> exit(0);
		
		if(scheduled_for_start)
		{
			// No NEW tasks...
			if(!scheduled_for_start->ProcessedTask())
			{  _KillScheduledForStart();  }
		}
		
		// Tell active task source that we do not want more tasks. 
		_NoMoreNewTasksFromTSource();
		
		if(!do_kill_tasks)
		{
			// Tell interface to give back all not processed 
			// tasks: special value 0x7fffffff. 
			// (Note: in case do_kill_tasks is set, we call 
			// DontStartMoreTasks() below which will call 
			// ScheduleGiveBackTasks(may_keep=0). 
			interface->ScheduleGiveBackTasks(/*may_keep=*/0x7fffffff);
		}
	}
	if(do_kill_tasks)
	{
		DontStartMoreTasks();  // -> calls _NoMoreNewTasksFromTSource();
		
		schedule_quit=2;  // 2 -> exit(1);
		sched_kill_tasks=1;   // user interrupt
	}
	if(do_sched_quit || do_kill_tasks)
	{  ReSched(__LINE__);  }  // -> quit now if nothing to do 
}

// Gets called by _HandleStopCont() and backcall from interface when done. 
// action: +1 -> cont; -1 -> stop
void TaskManager::ChangeExecStopStatus(ExecStopStatus new_ess)
{
	ExecStopStatus old_ess=exec_stop_status;
	exec_stop_status=new_ess;
	
	// Special case: This gets called for each continuing client 
	// while continuing. This is done to ensure we can give a 
	// continuing client a job if needed. 
	// So, if old and new status are ESS_Continuing, don't bother 
	// the user. 
	if(! (old_ess==ESS_Continuing && new_ess==ESS_Continuing) )
	{  Warning("%s: Tasks now %s (previously %s).%s",prg_name,
		ExecStopStatus_String(exec_stop_status),
		ExecStopStatus_String(old_ess),
		exec_stop_status==ESS_Stopped ? "" : "\n");  }
	
	switch(exec_stop_status)
	{
		case ESS_Continuing:  // fall through:
		case ESS_Running:
			_CheckStartExchange();
			_CheckStartTasks();
			ReSched(__LINE__);
			break;
		case ESS_Stopping:
			_KillScheduledForStart();
			break;
		case ESS_Stopped:
			// For active task source, we do NOT raise SIGSTOP because 
			// that would render the program useless. 
			if(GetTaskSourceType()!=TST_Active)
			{
				Warning(" [%s now STOPPED]\n",prg_name);
				raise(SIGSTOP);
			}
			else
			{  Warning(" [waiting idle...]\n");  }
			break;
		default:  assert(0);
	}
}

// Called by signal handler (or task source). 
// action: +1 -> cont; -1 -> stop
void TaskManager::_HandleStopCont(int action)
{
	assert(action!=0);
	// This is a problem. Processes may ignore SIGTSTP and simply run on, 
	// so we should use SIGSTOP. Other processes (shell scripts) need to 
	// interprete SIGTSTP, so we may NOT use SIGSTOP. 
	// We let the user decide by using SIGTSTP here and stop_sig_to_send 
	// in the driver layer. 
	int signo=(action<0) ? SIGTSTP : SIGCONT;
	
	// How it works: 
	// SIGTSTP -> status=ESS_Stopping -> send to clients 
	//         -> recv confirm from clients 
	//         -> when all clients stopped: ESS_Stopped. 
	// SIGCONT -> status=ESS_Continuing -> send to clients 
	//         -> recv confirm from clients 
	//         -> when all clients continued: ESS_Running
	// THUS, we should not start jobs in states ESS_Stopped and ESS_Stopping 
	// but we may do so in ESS_Running AND ESS_Continuing. 
	
	// In any case, we tell the interface. (Also on cases that we are 
	// already running and getting told to continued, etc.)
	int rv=interface->StopContTasks(signo);
	
	if(rv==0)  // done
	{
		ChangeExecStopStatus((action<0) ? ESS_Stopped : ESS_Running);
	}
	else if(rv==1)
	{
		// This happens for LDR. It says that stopping is in 
		// progress. Have to wait. 
		ChangeExecStopStatus((action<0) ? ESS_Stopping : ESS_Continuing);
	}
	else assert(0);
	
	//Warning("Ignoring SIG%s because already %s.\n",
	//	signo==SIGCONT ? "CONT" : "TSTP",
	//	exec_stopped ? "stopped" : "running");
}


void TaskManager::_schedule(TimerInfo *ti)
{
	assert(ti->tid==tid0);
	
	if(IsVerbose(DBG))
	{
		static HTime last(HTime::Curr);
		HTime delta=(*ti->current-last);
		Verbose(DBG,"--schedule--<%s>--\n",delta.PrintElapsed(1));
		last=*ti->current;
		
		// This is the anti-repetiton schedule watchdog: 
		// Note that you need to enable DBG verbosity to make this work. 
		if(!_actually_quit_step)
		{
			static int zero_reps=0;
			// NOTE: TimerInterval(ti->tid) is ALWAYS 0 here. 
			if(delta.Get(HTime::msec)>1)
			{  zero_reps=0;  }
			else if((++zero_reps)==8)
			{
				Verbose(DBG,"OOPS: TaskManager scheduler strangely busy.\n");
				_DumpInternalState();
			}
		}
	}
	
	// Read instructions near _ActuallyQuit() to understand what is 
	// going on. 
	if(_actually_quit_step)   // Initially 1. 
	{
		//Verbose(DBGV,"[%d]",_actually_quit_step);
		switch(_actually_quit_step)
		{
			case 1:  break;  // do nothing
			case 2:  _DestructCleanup();  break;  // do the destructor stuff
			case 3:  break;  // do nothing
			case 4:  // Quit now. Really this time. 
				fdmanager()->Quit(_actually_quit_status);
				break;
			default:  assert(0);
		}
		++_actually_quit_step;
		return;
	}
	
	// (This MUST be done here, NOT at the end of the function so 
	// that other functions can re-enable the timer -- and NOT 
	// before the _actually_quit stuff above.)
	UpdateTimer(tid0,-1,0);  // First, stop timer. 
	
	// Okay, let's see what we have to do...
	if(!tasksource())
	{
		assert(!recovering);
		
		// We're in the 0msec timer installed at the constuctor to get 
		// things running: 
		int rv=_StartProcessing();
		if(rv)
		{  _ActuallyQuit((rv==1) ? 0 : 1);  }
		else
		{
			// If rv==0, tasksource() MAY not be NULL here: 
			assert(tasksource());
		}
		return;
	}
	
	// This is high priority; if we do that 
	// then we do nothing else!
	if(sched_kill_tasks)
	{
		// Schedule TERM & KILL for all tasks. 
		int nkilled=interface->TermAllJobs(
			(sched_kill_tasks==1) ? TTR_JK_UserIntr : TTR_JK_ServerErr);
		
		// Maybe, this could be left away: 
		Verbose(TDI,"Sent SIGTERM to %d jobs/clients.%s",nkilled,
			kill_tasks_and_quit_now ? "" : "\n");
		
		// Okay, did that. 
		sched_kill_tasks=0;
		
		if(kill_tasks_and_quit_now)
		{
			if(nkilled==0)
			{
				Verbose(TDI," QUITTING\n");
				_ActuallyQuit(2);  // QUIT NOW. 
				return;
			}
			else
			{  Verbose(TDI," Waiting for jobs to quit.\n");  }
			_KillScheduledForStart();
		}
	}
	
	if(scheduled_for_start && load_permits_starting)
	{
		// Special flag for KillScheduledForStart: 
		launching_special_flag=1;
		
		// We have to start a job for a task. Okay, let's do it: 
		int rv=interface->LaunchTask(scheduled_for_start);
		// rv=-1/-2 issued by local driver which also calls 
		//    HandleFailedTask() so this is dealt with. 
		// rv=+1 issued by LDR driver in case there is no free 
		//    client (e.g. the free one unregistered between 
		//    GetTaskToStart() and here) or local if alloc failed. 
		//    Try again. 
		
		// NOTE: scheduled_for_start can be NULL here in case 
		//       DontStartMoreTasks() or _KillScheduledForStart() 
		//       was called by LaunchTask(). 
		
		if(rv)
		{  assert(!scheduled_for_start || !scheduled_for_start->d.any());  }
		else
		{  assert(scheduled_for_start && scheduled_for_start->d.any());  }
		
		_KillScheduledForStart();
		launching_special_flag=0;
		
		// See if we want to start more task(s): 
		_CheckStartTasks();
	}
	
	if(schedule_quit || schedule_quit_after || recovering)
	{
		// Quit is scheduled. So, we quit if everything is cleaned up: 
		// Recovering: similar...
		
		// The interface (in case of an LDR server) must disconnect 
		// from the clients if there are no more jobs to do. 
		// Let him do that. 
		if(tasklist.todo.is_empty() && tasklist.proc.is_empty() && 
		   !told_interface_to_quit )
		{
			interface->PleaseQuit();
			told_interface_to_quit=1;
			// The interface will call CheckStartNewJobs(-1) when done 
			// and we'll return to schedule(). 
			return;
		}
		
		if(!interface->AreThereJobsRunning() && 
		   tasklist.todo.is_empty() && 
		   tasklist.proc.is_empty() && 
		   tasklist.done.is_empty() && 
		   !connected_to_tsource && 
		   pending_action==ANone )
		{
			// Must tell task source to quit now (it not already done) 
			// and wait for it to finish: 
			if(!told_tasksource_to_quit)
			{
				int rv=TSDisconnect(/*special=*/1);
				assert(rv==0);   // else: internal error. 
				pending_action=ADisconnect;
				
				told_tasksource_to_quit=1;
				// The task source will notify us and we'll return to 
				// _schedule() here. 
				return;
			}
			
			// Okay, we may actually quit. 
			// For recovery, this means that recovery is now complete. 
			// (If no quit flag is set (only recovering), this will 
			// pass -1 as argument to _DoQuit() which is exactly what 
			// we want. 
			_DoQuit(cmpMAX(schedule_quit,schedule_quit_after)-1);
			return;
		}
		
		// We will start to put back all non-processed and 
		// all non-done tasks to source NOW: (If needed only, of couse.)
		//#warning These may cause trouble. Check that. 
		if(connected_to_tsource && pending_action==ANone)
		{  _TS_GetOrDoneTask();  }
		else if(!connected_to_tsource)
		{  _CheckStartExchange();  }
	}
}

inline void TaskManager::_KillScheduledForStart()
{
	if(!scheduled_for_start)  return;
	
	#if TEST_TASK_LIST_MEMBERSHIP
	// Test: see if task is actually in tasklist.todo: 
	assert(tasklist.todo.find(scheduled_for_start));
	#endif
	
	// This is called regularly after launching a task. 
	// In this case, scheduled_for_start->d.any() is set but the 
	// task is not yet removed from todo queue. 
	// MAYBE this is a dangerous race condition... 
	//       Should probably be fixed. But seems to work ATM. 
	if(scheduled_for_start->d.any())
	{
		// Bug trap: see where the flag is set. 
		assert(launching_special_flag);
		
		// NOTE: Some other code relies on tasklist.todo to 
		// have d.any()=false which also corresponds to the 
		// definition of the todo list... 
		// So, scheduled_for_start->d.any() may only be set if 
		// launching_special_flag is set. 
		
		// Must put task into proc queue: 
		tasklist.todo.dequeue(scheduled_for_start);  --tasklist.todo_nelem;
		tasklist.proc.append(scheduled_for_start);   ++tasklist.proc_nelem;
		
		launching_special_flag=0;
	}
	
	scheduled_for_start=NULL;
	if(!load_permits_starting)
	{  UpdateTimer(tid_load_poll,-1,0);  }
}


int TaskManager::timernotify(TimerInfo *ti)
{
	if(ti->tid==tid0)
	{  _schedule(ti);  }   // <-- Has own --<schedule>-- DBG message. 
	else if(ti->tid==tid_load_poll)
	{
		Verbose(DBGV,"--<TaskMan::timernotify>--<poll load>--\n");
		
		if(!scheduled_for_start)
		{
			// Why are we polling the load for no reason? 
			// (Maybe because scheduled_for_start was cancelled.)
			UpdateTimer(tid_load_poll,-1,0);
		}
		else
		{
			assert(!load_permits_starting);
			_DoCheckLoad();
			if(load_permits_starting)
			{
				UpdateTimer(tid_load_poll,-1,0);
				ReSched(__LINE__);
			}
		}
	}
	else if(ti->tid==tid_ts_cwait)
	{
		Verbose(DBGV,"--<TaskMan::timernotify>--<TS wait>--\n");
		
		// Seems that we shall re-try to connect. 
		UpdateTimer(tid_ts_cwait,-1,0);  // DISABLE TIMER. 
		if(!connected_to_tsource && pending_action==AConnect)
		{
			// Temporarily set pending action to ANone for _CheckStartExchange(): 
			pending_action=ANone;
			_CheckStartExchange();
		}
		else
		{  Error("OOPS: spurious tid_ts_cwait timer.\n");  }
	}
	else
	{  assert(0);  }
	
	return(0);
}


int TaskManager::timeoutnotify(TimeoutInfo *ti)
{
	Verbose(DBG,"--<TaskMan::timeoutnotify>--\n");
	
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	int act='a';
	if(ti->tid==exec_timeout_id)
	{
		Warning("Execution timeout. Task queue: todo=%d, proc=%d, done=%d\n",
			tasklist.todo_nelem,tasklist.proc_nelem,tasklist.done_nelem);
		act=exec_timeout_action;
	}
	else if(ti->tid==cyc_timeout_id)
	{
		HTime tmp;
		tmp.Set(cyc_idle_timeout,HTime::seconds);
		Warning("Execution cycle idle timeout. "
			"(Idle for %s; I'm bored.)\n",tmp.PrintElapsed(/*with_msec=*/0));
		act='i';  // SIGINT
	}
	else assert(0);
	Warning("  Timeout time: %s\n",ti->timeout->PrintTime(1));
	Warning("  Current time: %s\n",ti->current->PrintTime(1));
	
	if(act=='i' || act=='t')
	{  _ActOnSignal((act=='i') ? SIGINT : SIGTERM,/*real_signal=*/0);  }
	else
	{  Error("Aborting (timeout).\n");  abort();  }
	
	return(0);
}


// Enable/disable work cycle timeout. 
void TaskManager::_ControlWorkCycleTimeout(int install)
{
	if(install && cyc_idle_timeout>=0)
	{
		// Install cyc exec timeout: 
		HTime end_time(HTime::Curr);
		end_time.Add(cyc_idle_timeout,HTime::seconds);
		UpdateTimeout(cyc_timeout_id,end_time);
	}
	else if(!install)
	{  UpdateTimeout(cyc_timeout_id,HTime(HTime::Invalid));  }
}


static int _int_cmp(const void *a,const void *b)
{
	return(*(int*)a - *(int*)b);
}


// Used by _DoQuit() (maybe others). 
void TaskManager::_EmitCPUStats(const char *title,
	HTime *elapsed,
	ProcessManager::ProcTimeUsage *ptu_self,
	ProcessManager::ProcTimeUsage *ptu_chld)
{
	if(elapsed->GetD(HTime::seconds)<=0.2)  return;
	
	Verbose(TDI,"  %s:\n",title);
	
	double rv_cpu=100.0*(ptu_self->stime.GetD(HTime::seconds)+
		ptu_self->utime.GetD(HTime::seconds)) / elapsed->GetD(HTime::seconds);
	double ch_cpu=100.0*(ptu_chld->stime.GetD(HTime::seconds)+
		ptu_chld->utime.GetD(HTime::seconds)) / elapsed->GetD(HTime::seconds);
	
	char tmp[48];
	snprintf(tmp,48,"%s",ptu_self->utime.PrintElapsed());
	Verbose(TDI,"    RendView:   %4d.%02d%% CPU  (user: %s; sys: %s)\n",
		int(rv_cpu),int(100.0*rv_cpu+0.5)%100,
		tmp,ptu_self->stime.PrintElapsed());
	snprintf(tmp,48,"%s",ptu_chld->utime.PrintElapsed());
	Verbose(TDI,"    Local jobs: %4d.%02d%% CPU  (user: %s; sys: %s)\n",
		int(ch_cpu),int(100.0*ch_cpu+0.5)%100,
		tmp,ptu_chld->stime.PrintElapsed());
	snprintf(tmp,48,"%s",(ptu_chld->utime+ptu_self->utime).PrintElapsed());
	Verbose(TDI,"    Together:   %4d.%02d%% CPU  (user: %s; sys: %s)\n",
		int(ch_cpu+rv_cpu),int(100.0*(ch_cpu+rv_cpu)+0.5)%100,
		tmp,(ptu_chld->stime+ptu_self->stime).PrintElapsed());
}


void TaskManager::_EmitAccumulatedTime(HTime &elapsed)
{
	Verbose(TDI,"    Jobs:    Rendered: %8d  Filtered: %8d\n",
		acctime_render.njobs,acctime_filter.njobs);
	
	Verbose(TDI,"    User:    %18s",
		acctime_render.utime.PrintElapsed(/*with_msec=*/1));
	Verbose(TDI,"  %18s\n",
		acctime_filter.utime.PrintElapsed(/*with_msec=*/1));
	
	Verbose(TDI,"    System:  %18s",
		acctime_render.stime.PrintElapsed(/*with_msec=*/1));
	Verbose(TDI,"  %18s\n",
		acctime_filter.stime.PrintElapsed(/*with_msec=*/1));
	
	Verbose(TDI,"    Elapsed: %18s",
		acctime_render.elapsed.PrintElapsed(/*with_msec=*/1));
	Verbose(TDI,"  %18s\n",
		acctime_filter.elapsed.PrintElapsed(/*with_msec=*/1));
	
	double sec=elapsed.GetD(HTime::seconds);
	if(sec>0.01)
	{
		double rsum=( acctime_render.utime+acctime_render.stime 
			).GetD(HTime::seconds);
		double fsum=( acctime_filter.utime+acctime_filter.stime 
			).GetD(HTime::seconds);
		
		Verbose(TDI,"    Eff.CPU:            %6.0f%%             %6.0f%%\n",
			100.0*rsum/sec,
			100.0*fsum/sec);
	}
}


// YOU MUST call ReSched() after this function. 
// This is always called due to error and thus returns error. 
void TaskManager::_QuitOrBeginRecovery()
{
	assert(!recovering);
	
	// Of course, recovery is only done if the task source 
	// type is active. Else, we quit. 
	if(GetTaskSourceType()==TST_Active)
	{
		int ntodo=tasklist.todo_nelem;
		int nproc=tasklist.proc_nelem;
		int ndone=tasklist.done_nelem;
		int nrun=interface->Get_nrunning();
		VerboseSpecial("Beginning recovery. "
			"Tasks: todo=%d; proc=%d (%d); done=%d %s",
			ntodo,nproc,nrun,ndone,
			(ndone || nproc) ? "(bad)" : 
			(!ntodo && !ndone && !nproc) ? "(good)" : "(hrm...)");
		assert(nrun==nproc);
		
		DontStartMoreTasks();
		
		// Also, kill tasks currently running: 
		if(tasklist.proc_nelem || nrun>0)   // redundant if()
		{
			int nkilled=interface->TermAllJobs(
				(sched_kill_tasks==1) ? TTR_JK_UserIntr : TTR_JK_ServerErr);
			Verbose(TDI,"Sent SIGTERM to %d jobs/clients.%s",nkilled,
				kill_tasks_and_quit_now ? "" : "\n");
		}
		
		recovering=1;
		// DO NOT SET THESE
		// schedule_quit=1;  NO!
		// sched_kill_tasks=2;  NO! // server error
	}
	else
	{
		// Schedule quit
		//if(!schedule_quit)
		//{  schedule_quit=1;  }
		schedule_quit=2;
		
		// [This is probably not needed here:] 
		// Tell interface (LDR client) to give back all not yet 
		// processed tasks in todo queue (special value 0x7fffffff). 
		if(tasklist.todo_nelem || interface->Get_nrunning()>0)  // redundant if()
		{  interface->ScheduleGiveBackTasks(/*may_keep=*/0x7fffffff);  }
	}
}

		
// Simply call fdmanager()->Quit(status) and write info. 
// (Actually, _ActuallyQuit() is called...)
// This is also used for recovery. 
// NOTE: status=-1 for recovering without quit. 
void TaskManager::_DoQuit(int status)
{
	int doquit;
	if(status<0)
	{
		assert(recovering);
		doquit=0;
	}
	else
	{
		assert(schedule_quit || schedule_quit_after);
		doquit=1;
	}
	
	// Now, it's time to quickly tidy up the files: 
	int rv=TaskFileManager::DoTidyUp(
		doquit ? TaskFile::DelExit : TaskFile::DelCycle);
	assert(!rv);   // Currently, rv can only be 0. 
	
	if(doquit)
	{  VerboseSpecial("Now exiting with status=%s (%d)",
		status ? "failure" : "success",status);  }
	else
	{  VerboseSpecial("Completing recovery (work cycle count: %d).",
		work_cycle_count);  }
	
	if(interface)
	{  interface->WriteProcessingInfo(1,NULL);  }
	
	ProcessManager::ProcTimeUsage ptu_self_start,ptu_chld_start;
	ProcessManager::manager->GetTimeUsage(0,&ptu_self_start);
	ProcessManager::manager->GetTimeUsage(1,&ptu_chld_start);
	
	ProcessManager::ProcTimeUsage ptu_self_work,ptu_chld_work;
	if(work_cycle_count)
	{
		ptu_self_work.stime=ptu_self_start.stime-ptu_self_start_work.stime;
		ptu_self_work.utime=ptu_self_start.utime-ptu_self_start_work.utime;
		ptu_chld_work.stime=ptu_chld_start.stime-ptu_chld_start_work.stime;
		ptu_chld_work.utime=ptu_chld_start.utime-ptu_chld_start_work.utime;
	}
	
	HTime elapsed_start=ptu_self_start.uptime;  // elapsed since start
	HTime elapsed_work(HTime::Invalid);  // elapsed since work...
	if(work_cycle_count)
	{  elapsed_work=ptu_self_start.uptime-ptu_self_start_work.uptime;  }
	
	{
		HTime endtime_curr=starttime+elapsed_start;
		Verbose(TDI,"  %s at (local): %s\n",
			doquit ? "Exiting" : "Recovery",endtime_curr.PrintTime(1,1));
	}
	Verbose(TDI,"  Elapsed since start:  %s\n",elapsed_start.PrintElapsed());
	// The next line is the elapsed time since the first job of this 
	// recovery cycle (LDR: since connection to server). 
	if(!doquit)
	{  Verbose(TDI,"  Elapsed time working: %s\n",elapsed_work.PrintElapsed());  }
	
	int loadval=_GetLoadValue();
	Verbose(TDI,"  Load control: ");
	if(load_low_thresh<=0)
	{
		char tmp[16];
		if(loadval<0)
		{  strcpy(tmp,"??");  }
		else
		{  snprintf(tmp,16,"%.2f",double(loadval)/100.0);  }
		Verbose(TDI,"[disabled]; current load: %s\n",tmp); 
	}
	else
	{
		char max_tmp[32];
		if(max_load_measured<0)  *max_tmp='\0';
		else  snprintf(max_tmp,32,"; max: %.2f",double(max_load_measured)/100.0);
		Verbose(TDI,"%d times; curr: %.2f%s  (%.2f/%.2f)\n",
			load_control_stop_counter,double(loadval)/100.0,max_tmp,
			double(load_low_thresh)/100.0,double(load_high_thresh)/100.0);
	}
	
	if(lpf_hist_size)
	{
		Verbose(TDI,"  Last successfully done frames:");
		qsort(last_proc_frames,lpf_hist_size,sizeof(int),&_int_cmp);
		int nw=0;
		for(int i=0; i<lpf_hist_size; i++)
		{
			int l=last_proc_frames[i];
			if(l<0)  continue;
			Verbose(TDI," %d",l);  ++nw;
		}
		Verbose(TDI,"%s\n",nw ? "" : " [none]");
	}
	
	if(!doquit)
	{  _EmitCPUStats("CPU stats for last work cycle",&elapsed_work,
		&ptu_self_work,&ptu_chld_work);  }
	_EmitCPUStats(doquit ? "Overall CPU stats" : "Overall CPU stats (until now)",
		&elapsed_start,
		&ptu_self_start,&ptu_chld_start);
	
	if(doquit)
	{
		Verbose(TDI,"  Accumulated CPU stats (all jobs, since start):\n");
		// This writes the sum of user, sys and elapsed time, 
		// summed up over all jobs (especially all LDR clients). 
		// The Eff.CPU line displays the effective CPU percentage. 
		// This is, if 10 clients render 1 job for 2 seconds each and 
		// 2.1 seconds elapsed from start of the server till the end, 
		// the effective CPU is 10*2 / 2.1 = 952% - or "9.52 the speed 
		// of a single node" (as an estimate). 
		_EmitAccumulatedTime(elapsed_start);
	}
	
	#warning FIXME: More info to come (?)
	
	if(doquit)
	{  _ActuallyQuit(status);  }
	else
	{
		// Make sure state is correct. 
		int fail=1;
		do {
			if(scheduled_for_start)  break;
			if(!tasklist.todo.is_empty() || 
			   !tasklist.proc.is_empty() || 
			   !tasklist.done.is_empty())  break;
			if(tasklist.todo_nelem || 
			   tasklist.done_nelem || 
			   tasklist.done_nelem )  break;
			if(!recovering)  break;
			if(!dont_start_more_tasks)  break;  // Must be set by recovering. 
			if(schedule_quit || schedule_quit_after)  break;  // We may not be here. 
			if(!told_interface_to_quit)  break;
			if(!told_tasksource_to_quit)  break;
			if(kill_tasks_and_quit_now)  break;  // We may not be here. 
			if(sched_kill_tasks)  break;
			if(connected_to_tsource)  break;
#warning RECOVERY AND STOP/CONT MAKE TROUBLE!!!!!!!
if(exec_stop_status!=ESS_Running)  break;   // We are here and stopped??
			if(launching_special_flag)  break;  // <-- On failure: severe bug!
			if(last_pend_done_frame_no>=0)  break;  // Must be -1. 
			if(tsgod_next_action!=ANone || pending_action!=ANone)  break;
			// Maybe, I'll change the meaning of SIGINT for ldrclient. 
			// But for now, sigint must schedule some quit and thus 
			// we may not be here. 
			if(abort_on_signal || caught_sigint)  break;
			
			// Misc checks: 
			fail=2;
			if(!interface)  break;
			// Recovery may only be done with active task source: 
			if(GetTaskSourceType()!=TST_Active)  break;
			
			fail=0;
		} while(0);
		if(fail)
		{
			Error("Internal error: Illegal internal state at end of "
				"recovery (fail=%d).\n",fail);
			_DumpInternalState();
			assert(0);
		}
		
		// Tell interface: 
		interface->RecoveryDone();
		
		// Reset: 
		jobs_failed_in_sequence=0;
		failed_in_sequence_reported=0;
		dont_start_more_tasks=0;
		recovering=0;    // End it. 
		told_interface_to_quit=0;
		told_tasksource_to_quit=0;
		told_want_no_more_tasks=0;
		ts_done_all_first=0;
		nth_call_to_get_task=0;
		fit_strategy_flag=FSF_Run;
		
		// This is at least to force a sane value: 
		exec_stop_status=ESS_Running;
		
		// Reset little history: 
		lpf_hist_idx=0;
		if(lpf_hist_size>0)
		{
			for(int i=0; i<lpf_hist_size; i++)
			{  last_proc_frames[i]=-1;  }
		}
		
		// Probably, these shall be reset, too: 
		// (See check above, too)  [currently useless here]
		abort_on_signal=0;
		caught_sigint=0;
		
		Verbose(DBGV,"todo-thresh: low=%d, high=%d\n",
			interface->Get_todo_thresh_low(),
			interface->Get_todo_thresh_high());
		
		// Okay, start work cycle idle timeout if we have one: 
		_ControlWorkCycleTimeout(/*install=*/1);
		
		// Tell the task source: 
		// (It is an active task source as checked above,)
		// This is a special situation; we call TSGetTask for the 
		// active task source to inform it about the end of recovery. 
		int rv=TSGetTask();
		assert(rv==0);
		
		last_work_cycle_end_time.SetCurr();
		
		ReSched(__LINE__);
	}
}


// This function is to be used instead of fdmanager->Quit(). 
// This is done to give all the other objects some hlib cycles 
// (0 msec timer) to clean up correctly (e.g. becuause x->DeleteMe(); 
// is used rather than delete x;). This works by setting 
// _actually_quit_step=1 and then runnig the timer a couple of 
// times and then calling fdmanager->Quit(). 
// While _actually_quit_step is set, all functions must react like 
// dead (i.e. do nothing). 
void TaskManager::_ActuallyQuit(int status)
{
	assert(_actually_quit_step==0);  // HUGE bug otherwise
	_actually_quit_status=status;
	_actually_quit_step=1;
	ReSched(__LINE__);
}


// Called via timernotify() after everything has been set up and 
// as the first action in the main loop. Used to get the TaskSource 
// and begin actually doing something...
// Return value: 0 -> OK; -1 -> error; 1 -> simply quit
int TaskManager::_StartProcessing()
{
	assert(tasksource()==NULL);  // yes, must be NULL here. 
	assert(!interface);
	
	// Okay, check operation mode: 
	const char *opmode=opmode_name.str();
	if(!opmode)  opmode=prg_name;
	
	// *NOTE:* FIX HELP TEXT IN database.cpp TOO, 
	//         IF YOU CHANGE SOMETHING HERE. 
	
	RendViewOpMode opmode_num=GetRendViewOpMode(opmode);
	
	if(opmode_num==RVOM_None)
	{
		if(opmode_name.str())
		{  Error("Illegal operation mode \"%s\". Try --list-opmode.\n",
			opmode_name.str());  return(-1);  }
		else 
		// These are no longer supported (LDR forwarder). Gave up plans. 
		//if(!tsource_name && !tdinterface_name)
		{
			Warning("Nonstandard binary name. "
				"Defaulting to opmode \"rendview\".\n");
			opmode_num=RVOM_RendView;
		}
	}
	
	// These are no longer supported (LDR forwarder). Gave up plans. 
	#if 0
	// Set tsource and tdif name if not explicitly told otherwise by user: 
	if(!tsource_name)
	{  _CheckAllocFail(tsource_name.set(
		(opmode_num==RVOM_LDRClient) ? "LDR" : "local"));  }
	if(!tdinterface_name)
	{  _CheckAllocFail(tdinterface_name.set(
		(opmode_num==RVOM_LDRServer) ? "LDR" : "local"));  }
	
	const char *tdif_name=tdinterface_name.str();
	const char *ts_name=tsource_name.str();
	assert(tdif_name && ts_name);
	#else
	const char *tdif_name=(opmode_num==RVOM_LDRServer) ? "LDR" : "local";
	const char *ts_name=(opmode_num==RVOM_LDRClient) ? "LDR" : "local";
	// MAY NEVER USE ts and tdif = LDR. These plans were given up. 
	#endif
	
	Verbose(MiscInfo,"Choosing: %s task source; %s task driver (interface)\n",
		ts_name,tdif_name);
	
	// OKAY; SET UP TASK SOURCE: 
	TaskSourceFactory *tsf=component_db->FindSourceFactoryByName(ts_name);
	if(!tsf)
	{
		Error("Fatal: Task source \"%s\" not present. Giving up.\n",ts_name);
		return(-1);
	}
	
	// Let TaskSourceFactory do final init: 
	if(tsf->FinalInit())
	{  return(-1);  }  // Failed; message(s) written. 
	
	TaskSource *ts=tsf->Create();
	if(!ts)
	{
		Error("Failed to initialize %s task source.\n",ts_name);
		return(-1);
	}
	
	// NOW, SET UP TASK DRIVER INTERFACE: 
	TaskDriverInterfaceFactory *tdiff=
		component_db->FindDriverInterfaceFactoryByName(tdif_name);
	if(!tdiff)
	{
		Error("Fatal: Task driver interface \"%s\" not present. "
			"Giving up.\n",tdif_name);
		return(-1);
	}
	
	// Let TaskDriverInterfaceFactory do final init: 
	if(tdiff->FinalInit())
	{  return(-1);  }    // Failed; message(s) written. 
	
	interface=tdiff->Create();  // MUST be done after FinalInit(). 
	if(!interface)
	{
		Error("Failed to initialize %s task driver interface\n",tdif_name);
		return(-1);
	}
	
	// Finally, set up the admin port: 
	if(component_db->adminport()->FinalInit())
	{  return(-1);  }    // Failed; message(s) written. 
	
	// Write out useful verbose information: 
	char tmp[24];
	if(max_failed_in_sequence)  snprintf(tmp,24,"%d",max_failed_in_sequence);
	else  strcpy(tmp,"OFF");
	Verbose(TDI,"Ready to perform work: max-failed-in-seq=%s\n",tmp);
	
	Verbose(TDI,"  Execution timeout: ");
	if(exec_timeout_id && !TimeoutTime(exec_timeout_id)->IsInvalid())
	{
		HTime tmp(HTime::Curr);
		HTime delta=*TimeoutTime(exec_timeout_id)-tmp;
		Verbose(TDI,"%s (%s);  action: %s\n",
			TimeoutTime(exec_timeout_id)->PrintTime(1),
			delta.PrintElapsed(),
			exec_timeout_action=='i' ? "SIGINT" : 
				(exec_timeout_action=='t' ? "SIGTERM" : "abort"));
		tmp.Set(10,HTime::seconds);
		if(delta<tmp)
		{
			UpdateTimeout(exec_timeout_id,HTime(HTime::Invalid));
			Error("He... you will give me more than 10 seconds, will you?!\n");
			return(-1);
		}
	}
	else
	{  Verbose(TDI,"[none]\n");  }
	
	// Get load val for first time; see if it is >0: 
	int loadval=_GetLoadValue();
	
	Verbose(TDI,"  Load control: ");
	if(load_low_thresh<=0)
	{
		char tmp[16];
		if(loadval<0)
		{  strcpy(tmp,"??");  }
		else
		{  snprintf(tmp,16,"%.2f",double(loadval)/100.0);  }
		Verbose(TDI,"[disabled]; current load: %s\n",tmp);
		assert(load_poll_msec<0);
	}
	else
	{
		Verbose(TDI,"min: %.2f; max: %.2f; poll: %ld msec; curr: %.2f\n",
			double(load_low_thresh)/100.0,double(load_high_thresh)/100.0,
			load_poll_msec,double(loadval)/100.0);
		assert(load_high_thresh>=load_low_thresh);
		assert(load_poll_msec>0);  // we want it LARGER THAN 0
	}
	
	// Set some important vars: 
	connected_to_tsource=0;
	pending_action=ANone;
	UseTaskSource(ts);
	
	ProcessManager::ProcTimeUsage ptu;
	ProcessManager::manager->GetTimeUsage(-1,&ptu);
	starttime=ptu.starttime;
	last_work_cycle_end_time=starttime;  // initially...
	Verbose(TDI,"  Starting at (local): %s\n",starttime.PrintTime(1,1));
	HTime timetmp;
	//timetmp=*TimeoutTime(exec_timeout_id);
	//Verbose(TDI,"  Execution timeout: %s\n",
	//	timetmp.IsInvalid() ? "[none]" : timetmp.PrintTime(/*local=*/1,/*with_msec=*/0));
	if(cyc_idle_timeout<0)  timetmp.SetInvalid();
	else  timetmp.Set(cyc_idle_timeout,HTime::seconds);
	Verbose(TDI,"  Cycle timeout: %s\n",
		cyc_idle_timeout<0 ? "[none]" : timetmp.PrintElapsed(/*width_msec=*/0));
	
	// Tell the interface to really start things; this will call 
	// TaskManager::ReallyStartProcessing(). 
	interface->ReallyStartProcessing();
	return(0);
}

// Called by interface; the LDR interface calls this when the first client 
// was connected successfully, the local interface calls this immediately. 
void TaskManager::ReallyStartProcessing(int error_occured)
{
	assert(!_actually_quit_step);
	
	if(error_occured)
	{
		// Former solution was _ActuallyQuit(1) here. 
		// This is bad. We need to recover or schedule quit. 
if(GetTaskSourceType()==TST_Active)
{  Error("PLEASE CHECK ME!!! (calling _QuitOrBeginRecovery() for TST_Active)\n");  }
		_QuitOrBeginRecovery();
		ReSched(__LINE__);
		return;
	}
	
	interface->WriteProcessingInfo(0,
		(GetTaskSourceType()==TST_Active) ? "waiting for" : "beginning to");
	
	// Daemonize if user wants that. 
	if(do_daemonize)
	{
		char *str=NULL;
		int flags=0;
		switch(do_daemonize)
		{
			case 1:
				str="keeping streams open";
				break;
			case 2:
				str="closing stdin";
				flags=DAEMONIZE_CLOSE_IN;
				break;
			case 3:
			default:  // <-- should not happen
				str="closing streams";
				flags=DAEMONIZE_CLOSE;  // <-- _IN, _OUT, _ERR
				break;
		}
		
		Verbose(TDI,"Now daemonizing (%s) ...\n",str);
		int rv=Daemonize(flags);
		if(rv<0)
		{
			Error("Failed to daemonize: %s (rv=%d)\n",
				strerror(errno),rv);
			_ActuallyQuit(0);
			return;
		}
	}
	
	// Active task source (TST_Active) will call tsnotify(AActive). 
	switch(GetTaskSourceType())
	{
		case TST_Passive:
		{
			// Do start exchange with task source: 
			int rv=_CheckStartExchange();
			if(rv==1)
			{
				Warning("Erm... Nothing will be done. I'm bored.\n");
				_ActuallyQuit(0);
				return;
			}
		}  break;
		case TST_Active:
		{
			_ControlWorkCycleTimeout(/*install=*/1);
		}  break;
		default: assert(0);
	}
}


void TaskManager::PrintWorkCycleStart()
{
	assert(!recovering);
	assert(tasklist.todo_nelem==0 && tasklist.proc_nelem==0);  // bug trap...
	
	_ControlWorkCycleTimeout(/*install=*/0);
	
	// Save some info from work start: 
	ProcessManager::manager->GetTimeUsage(0,&ptu_self_start_work);
	ProcessManager::manager->GetTimeUsage(1,&ptu_chld_start_work);
	++work_cycle_count;
	
	VerboseSpecial("Beginning new work cycle (cycle count: %d)",
		work_cycle_count);
	HTime curr(HTime::Curr);
	Verbose(TDI,"  Current time (local): %s\n",curr.PrintTime());
	int loadval=_GetLoadValue();
	if(loadval>=0)
	{  Verbose(TDI,"  Current load: %.2f\n",double(loadval)/100.0);  }
	//Verbose(TDI,"  Was idle since (local): %s\n",
	//	last_work_cycle_end_time.PrintTime());
}


// Called by TaskDriverInterface becuase a TaskDriver unregisterd 
// or because new LDR clients are connected or others disconnect. 
void TaskManager::CheckStartNewJobs(int njobs_changed)
{
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return;  }
	
	if(njobs_changed==-1)
	{
		// Special case. Called by LDR interface when all clients were 
		// disconnected. 
		assert((schedule_quit || schedule_quit_after || recovering) && 
			   tasklist.todo.is_empty() && tasklist.proc.is_empty());
		
		ReSched(__LINE__);
		return;
	}
	
	if(told_interface_to_quit)
	{
		// We're no longer interested in new njobs values. 
		// Yes, the LDR clients are disconnected by the interface 
		// and when done, njobs_changed=-1. 
		return;
	}
	
	if(njobs_changed)
	{
		int njobs=Get_njobs();
		if(!njobs)
		{
			// First action: make sure we don't start a task now. 
			_KillScheduledForStart();
			
			// We cannot go on...?
			
			// This will put back all tasks to the source and quit then. 
			// This is an error unless there is nothing to do and we're 
			// quitting. 
			bool just_quitting = 
				tasklist.todo.is_empty() && tasklist.proc.is_empty() && 
				(schedule_quit || schedule_quit_after || recovering);
			if(!just_quitting)
			{
				//Error("Waaaaaaaaaaaaaaaaait....\n");
				// ##FIXME####
				// If after some delay (timeout), Get_njobs() is still 0, then schedule quit. 
				// [NOTE: This could be done. OTOH, if there is no client left 
				//        then the net is probably so bad that there is little point 
				//        in going on. After all, you can re-launch rendview -l-cont 
				//        to go on...]
				// Implementation would be simple; I fix that in case it is needed 
				// (e-mail me). There are more ugent things to be done ATM...
				#warning RACE: njobs may be 0 here because the first client disconnected \
					unexpectedly. In this case we should not quit but wait for the other \
					connections. 
				
				_NoMoreNewTasksFromTSource();
				schedule_quit=2;
			}
			ReSched(__LINE__);
			return;
		}
	}
	
	// See if we want to start more jobs: 
	_CheckStartTasks();
	
	if(!scheduled_for_start)  // _CheckStartTasks() did not schedule a start 
	{
		// Okay, quit handling: 
		if(schedule_quit_after && 
		   tasklist.todo.is_empty() && tasklist.done.is_empty())
		{
			if(schedule_quit<schedule_quit_after)
			{  schedule_quit=schedule_quit_after;  }
			ReSched(__LINE__);
		}
		if(recovering)
		{
			// Will this ever happen? - YES. For example when 
			// the server disconnects while there are jobs running 
			// and the job then terminates. 
			ReSched(__LINE__);
		}
	}
}


// Check if we can start tasks and schedule that if we can: 
void TaskManager::_CheckStartTasks()
{
	// See if we're allowed to start more tasks: 
	if(dont_start_more_tasks)  return;
	assert(!recovering);  // For recovering, dont_start_more_tasks must be set. 
	
	// In case we are stopped or stopping, we do not start more tasks: 
	if(exec_stop_status==ESS_Stopping || exec_stop_status==ESS_Stopped)  return;
	
	// See if there's already a task scheduled for start: 
	if(scheduled_for_start)  return;
	
	CompleteTask *startme=interface->GetTaskToStart(&tasklist,schedule_quit);
	
	if(!startme)  return;
	
	// NOTE: d.any() may NOT yet be set. It must be set when launching the task. 
	assert(!startme->d.any());
	
	// Okay, actually schedule task for starting: 
	_DoScheduleForStart(startme);
}

inline void TaskManager::_DoScheduleForStart(CompleteTask *startme)
{
	assert(!scheduled_for_start);
	scheduled_for_start=startme;
	_DoCheckLoad();
	if(load_permits_starting)
	{  ReSched(__LINE__);  }  // start schedule timer
	else
	{  _StartLoadPolling();  }
}


// Check if we should start talking to the task source again and schedule 
// that if we should. 
// Return value: 
//  0 -> Okay, connect request sent. 
//  1 -> nothing to do 
//  2 -> we're in stopped state; don't do anything 
int TaskManager::_CheckStartExchange()
{
	if(connected_to_tsource || pending_action==AConnect)
	{  return(0);  }
	
	if(kill_tasks_and_quit_now)
	{  return(-2);  }
	
	if(exec_stop_status==ESS_Stopping || exec_stop_status==ESS_Stopped)
	{  return(2);  }
	
	// (Note that we do NOT need the tid0 / _schedule() - stuff here, 
	// as the task source is guaranteed not to call tsnotify() 
	// immediately (i.e. on the stack now).) 
	
	// First, see what we CAN do: 
	//bool can_done=_TS_CanDo_DoneTask();
	//bool can_get=_TS_CanDo_GetTask(can_done);;
	
	// Note: recovering sets dont_start_more_tasks. 
	assert(!recovering || dont_start_more_tasks);
	
	int must_connect=0;
	if(dont_start_more_tasks || schedule_quit || ts_done_all_first)
	{
		// We will immediately contact the task source and put back 
		// the tasks that need to be put back...
		must_connect=_TS_CanDo_DoneTask() ? 1 : 0;
	}
	
	if(!must_connect)
	{
		if(tasklist.done_nelem>=interface->Get_done_thresh_high() ||
		   (!schedule_quit_after && !dont_start_more_tasks && !schedule_quit && 
			tasklist.todo_nelem<interface->Get_todo_thresh_low()) )   // NOT <= 
		{  must_connect=1;  }
	}
	
	if(!must_connect && 
	   tasklist.todo.is_empty() && 
	   tasklist.proc.is_empty() && 
	   !tasklist.done.is_empty() )
	{
		if(schedule_quit_after || dont_start_more_tasks)
		{  must_connect=1;  }
		else assert(!recovering);
		// NOTE: If recovering (dont_start_more_tasks is set), 
		//       _TS_CanDo_DoneTask() must return true if there are 
		//       tasks in the done queue because during recovery we 
		//       MUST report all tasks to the task source. 
	}
	
	// Now, let's see...
	if(must_connect)
	{
		// Yes, we should get some new tasks or we must connect due 
		// to some different reason. 
		int rv=TSConnect();
		// If this returns non-zero, then that is an interal error!
		assert(!rv);
		pending_action=AConnect;
		return(0);  // connect request sent
	}
	
	return(1);  // nothing to do
}


bool TaskManager::_TS_CanDo_DoneTask(CompleteTask **special_done)
{
	// No, in case we are stopped, we do not do anything...
	if(exec_stop_status==ESS_Stopping || exec_stop_status==ESS_Stopped)
	{  return(0);  }
	
	bool can_done=false;
	// schedule_quit -> put back tasks which were not yet processed. 
	// dont_start_more_tasks -> put back all non-processed tasks. 
	// And, we must deliver done tasks (if any). 
	// NOTE: during recovering, we can (and MUST) report all tasks as 
	//       done; recovering sets dont_start_more_tasks, so no extra 
	//       condition is needed. 
	assert(!recovering || dont_start_more_tasks);
	     if(!tasklist.done.is_empty())  can_done=true;
	else if(dont_start_more_tasks || schedule_quit)
	{
		// This checks if we want to give back tasks from todo list which 
		// were not yet processed. Hence, no need to check proc list. 
		for(CompleteTask *i=tasklist.todo.first(); i; i=i->next)
		{
			assert(i->state!=CompleteTask::TaskDone);
			assert(!i->d.any());   // Otherwise: proc list. 
			if( (dont_start_more_tasks /*&& !i->d.any()*/) || 
			    (schedule_quit && !i->ProcessedTask() ) )
			{
				if(special_done)  *special_done=i;
				can_done=true;
				break;
			}
		}
	}
	return(can_done);
}

bool TaskManager::_TS_CanDo_GetTask(bool can_done)
{
	if(GetTaskSourceType()==TST_Active)  return(false);
	bool can_get=true;
	// Note: recovering sets dont_start_more_tasks. 
	     if(dont_start_more_tasks)  can_get=false;
	else if(exec_stop_status==ESS_Stopping || 
	        exec_stop_status==ESS_Stopped)  can_get=false;
	else if(schedule_quit || schedule_quit_after)  can_get=false;
	else if(ts_done_all_first && can_done)  can_get=false;
	else if(tasklist.todo_nelem>=interface->Get_todo_thresh_high())
		can_get=false;
	return(can_get);
}


// Tell TaskSource about done task or request a new task. 
// Currently all that is done alternating until a limit is reached. 
void TaskManager::_TS_GetOrDoneTask()
{
	if(kill_tasks_and_quit_now)
	{
		Error("OOps: kill_tasks_and_quit_now set in _TS_GetOrDoneTask().\n");
		return;
	}
	
	assert(connected_to_tsource);
	assert(pending_action==ANone);
	
	CompleteTask *special_done=NULL;
	
	// First, see what we CAN do: 
	bool can_done=_TS_CanDo_DoneTask(&special_done);
	bool can_get=_TS_CanDo_GetTask(can_done);
	
	// Compute new value for tsgod_next_action out of current value: 
	TSAction old_val=tsgod_next_action;
	tsgod_next_action=ANone;
	switch(old_val)
	{
		case ANone:     // start with DoneTask()
		case AGetTask:  // want to go on with DoneTask()
			if(can_done)  tsgod_next_action=ADoneTask;
			else if(can_get)  tsgod_next_action=AGetTask;
			break;
		case ADoneTask:   // want to go on with GetTask()
			if(can_get)  tsgod_next_action=AGetTask;
			else if(can_done)  tsgod_next_action=ADoneTask;
			break;
		default:  assert(0);  break;
	}
	
	switch(tsgod_next_action)
	{
		case ANone:
		{
			// Nothing more to do. Disconnect. 
			int rv=TSDisconnect();
			assert(rv==0);  // everything else is internal error
			pending_action=ADisconnect;
			break;
		}
		case AGetTask:
		{
			int rv=TSGetTask();
			assert(rv==0);  // everything else is internal error
			pending_action=AGetTask;
			++nth_call_to_get_task;
			break;
		}
		case ADoneTask:
		{
			CompleteTask *ctsk=tasklist.done.popfirst();
			if(ctsk)
			{  --tasklist.done_nelem;  }
			else
			{
				// Obviously. as the done queue is empty, we gave back 
				// all the done tasks, so reset this flag: 
				ts_done_all_first=0;
				
				ctsk=special_done;
				// We may not dequeue and call TaskDone() if the task 
				// is currently processed. This should not happen, though. 
				assert(!ctsk->d.any());
				// However, the task could be scheduled for start: 
				if(scheduled_for_start==ctsk)
				{  _KillScheduledForStart();  }
				
				#if TEST_TASK_LIST_MEMBERSHIP
				assert(tasklist.todo.find(ctsk));  // NOT tasklist.proc
				#endif
				
				tasklist.todo.dequeue(ctsk);  --tasklist.todo_nelem;
			}
			assert(ctsk);  // otherwise tsgod_next_action=ADoneTask illegal
			
			// Dump all the information to the user (if requested):
			DumpTaskInfo(ctsk,NULL,DTSK_ReportBack,VERBOSE_TDR);
			
			// This is needed so that we know the frame number in tsnotify(). 
			if(ctsk->state==CompleteTask::TaskDone)
			{  last_pend_done_frame_no=ctsk->frame_no;  }
			
			int rv=TSDoneTask(ctsk);
			assert(rv==0);  // everything else is internal error
			pending_action=ADoneTask;
			break;
		}
		default:  assert(0);  break;
	}
}


int TaskManager::_GetLoadValue()
{
	int lv=::GetLoadValue();
	if(lv<0)
	{  _DisableLoadFeature();  }
	else if(lv>max_load_measured)
	{  max_load_measured=lv;  }
	return(lv);
}

void TaskManager::_DoCheckLoad()
{
	if(load_low_thresh<=0)  return;   // feature disabled
	
	int loadval=_GetLoadValue();
	if(loadval<0)  return;  // feature already disabled by _GetLoadValue()
	if(load_permits_starting)
	{
		if(loadval>=load_high_thresh)
		{
			load_permits_starting=false;
			Warning("Load control: jobs disabled (load: %.2f >=%.2f)\n",
				double(loadval)/100.0,double(load_high_thresh)/100.0);
			++load_control_stop_counter;
		}
	}
	else
	{
		if(loadval<load_low_thresh)
		{
			load_permits_starting=true;
			Warning("Load control: jobs enabled (load: %.2f <%.2f)\n",
				double(loadval)/100.0,double(load_low_thresh)/100.0);
		}
	}
}

void TaskManager::_StartLoadPolling()
{
	assert(load_poll_msec>0);  // we want it to be >0, NOT >=0. 
	assert(!load_permits_starting);  // or internal error
	UpdateTimer(tid_load_poll,load_poll_msec,
		10 | FDAT_FirstLater | FDAT_FirstEarlier);
}

void TaskManager::_DisableLoadFeature()
{
	// Disable load feature at any time. 
	
	if(load_low_thresh<=0)  return;   // already disabled
	
	Warning("Disabling load feature.\n");
	load_low_thresh=0;
	load_poll_msec=-1;
	bool ov=load_permits_starting;
	load_permits_starting=true;
	KillTimer(tid_load_poll);  tid_load_poll=NULL;
	if(!ov || scheduled_for_start)
	{  ReSched(__LINE__);  }
}


void TaskManager::DumpTaskInfo(CompleteTask *ctsk,const char *fmt_hdr,
	int dtsk_flag,int vlevel)
{
	if(!ctsk)  return;
	
	//if(!IsVerboseR(vlevel))  return;   <-- do it?
	
	if((dtsk_flag & dtsk_short_info) || 
	   (dtsk_flag & dtsk_long_info) )
	{
		if(fmt_hdr)
		{  VerboseSpecial("%s",fmt_hdr);  }
		else switch(dtsk_flag)
		{
			case DTSK_ReportBack:
				VerboseSpecial("Reporting task [frame %d] as done "
					"(%s processed).",
					ctsk->frame_no,ctsk->Completely_Partly_Not_Processed());
				break;
			case DTSK_ReportBackDone:
				VerboseSpecial("Task [frame %d] finally reported as done "
					"(%s processed).",
					ctsk->frame_no,ctsk->Completely_Partly_Not_Processed());
				break;
			case DTSK_Arrival:
				VerboseSpecial("Task [frame %d] just arrived.",ctsk->frame_no);
				break;
			case DTSK_InitialQueue:
				VerboseSpecial("Queuing task [frame %d] to be processed.",
					ctsk->frame_no);
				break;
			case DTSK_NowRendered:  // fall through
			case DTSK_NowFiltered:
			{
				const char *what="???";
				switch((dtsk_flag==DTSK_NowRendered) ? 
					ctsk->DidRenderTaskFail() : ctsk->DidFilterTaskFail())
				{
					case 0:  what="success";  break;
					case 1:  what="incomplete";  break;
					case 2:  what="failure";  break;
					case -1:  what="??? [BUG!!]";  break;  // may not happen here
					default:  assert(0);  // keep that
				}
				VerboseSpecial("%sering done for task [frame %d]: %s",
					dtsk_flag==DTSK_NowRendered ? "Rend" : "Filt",
					ctsk->frame_no,what);
			}  break;
			default: assert(0);
		}
	}
	if((dtsk_flag & dtsk_long_info))
	{
		int when=1;
		if(dtsk_flag & (DTSK_Arrival | DTSK_InitialQueue))
		{  when=0;  }
		ctsk->DumpTask(vlevel,when);
	}
}


void TaskManager::_AccumulateTimeStat(TaskDriverType dtype,
	TaskExecutionStatus *tes)
{
	AccumulatedTime *acctime=NULL;
	switch(dtype)
	{
		case DTRender:  acctime=&acctime_render;  break;
		case DTFilter:  acctime=&acctime_filter;  break;
		default: assert(0);
	}
	
	if(!tes->utime.IsInvalid() && !tes->stime.IsInvalid() && 
		!tes->starttime.IsInvalid() && !tes->endtime.IsInvalid())
	{
		acctime->utime+=tes->utime;
		acctime->stime+=tes->stime;
		acctime->elapsed+=(tes->endtime-tes->starttime); 
		++acctime->njobs;
	}
}


int TaskManager::admin_command(ADMCmd *cmd,RVAPGrowBuffer *dest)
{
	if(!strcmp(cmd->arg[0],"?"))
	{
		dest->printf(
			"hello...\n");
		return(0);
	}
	
	return(1);
}


int TaskManager::CheckParams()
{
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	int failed=0;
	
	// Parse dumptask_spec: 
	if(dumptask_spec.str())
	{
		int mode='\0';
		for(const char *str=dumptask_spec.str(); *str; str++)
		{
			int flag=0;
			switch(tolower(*str))
			{
				case '+':  // fall through
				case '-':  mode=*str;  break;
				case 'a':  flag=DTSK_Arrival;  break;
				case 'q':  flag=DTSK_InitialQueue;  break;
				case 'b':  flag=DTSK_ReportBack;  break;
				case 'd':  flag=DTSK_ReportBackDone;  break;
				case 'r':  flag=DTSK_NowRendered;  break;
				case 'f':  flag=DTSK_NowFiltered;  break;
				case 'z':  flag=DTSK_ALLFLAGS;  break;
				default:
					Error("Illegal -dumptask spec ('%c')\n",*str);
					++failed;  break;
			}
			if(!flag)  continue;
			if(mode=='\0')
			{
				dtsk_short_info=0;
				dtsk_long_info=0;
				mode='+';
			}  // <-- NO ELSE.
			if(mode=='+')
			{
				dtsk_short_info|=flag;
				if(isupper(*str))  dtsk_long_info|=flag;
			}
			else // mode='-'
			{
				if(islower(*str))  dtsk_short_info&=~flag;
				dtsk_long_info&=~flag;
			}
		}
	}
	
	// Check daemonize: 
	if(IsSet(daemon_pi)>1)
	{
		if(!daemon_spec.str())
		{  do_daemonize=2;  }
		else if(!strcmp(daemon_spec.str(),"yes"))
		{  do_daemonize=2;  }
		else if(!strcmp(daemon_spec.str(),"no"))
		{  do_daemonize=0;  }
		else if(!strcmp(daemon_spec.str(),"close"))
		{  do_daemonize=3;  }
		else if(!strcmp(daemon_spec.str(),"noclose"))
		{  do_daemonize=1;  }
		else
		{  Error("Invalid value \"%s\" for -daemon: "
			"use yes,no,close,noclose.\n",
			daemon_spec.str());  ++failed;  }
	}
	else
	{  do_daemonize=0;  }
	daemon_spec.deref();
	
	if(max_failed_in_sequence<0)
	{  max_failed_in_sequence=0;  }
	if(!max_failed_in_sequence)
	{  Verbose(MiscInfo,"Note that max-failed-in-seq feature is disabled.\n");  }
	
	if(load_low_thresh<=0)
	{
		if(load_low_thresh!=UnsetNegMagic)
		{
			Error("Illegal load-min value %d.\n",load_low_thresh);
			++failed;
		}
		// Feature switched off by default. 
		load_low_thresh=0;
		load_poll_msec=-1;
		assert(load_permits_starting);
		// Kill load poll timer; it's not needed. 
		KillTimer(tid_load_poll);  tid_load_poll=NULL;
	}
	else
	{
		// Feature turned on. 
		if(load_high_thresh<load_low_thresh)
		{
			int oldval=load_high_thresh;
			load_high_thresh=load_low_thresh;
			if(oldval!=UnsetNegMagic)
			{  Warning("Adjusted load-max to %d (was %d), as load-min is %d.\n",
				load_high_thresh,oldval,load_low_thresh);  }
		}
		if(load_poll_msec<=0)  // YES!
		{
			long oldval=load_poll_msec;
			load_poll_msec=300;
			if(oldval!=UnsetNegMagic)
			{  Warning("Adjusted load-poll-msec to %ld (was %ld).\n",
				load_poll_msec,oldval);  }
		}
	}
	
	if(exec_timeout_spec.str() && *(exec_timeout_spec.str())!='\0' && 
	   strcmp(exec_timeout_spec.str(),"none"))
	{
		HTime end_time;
		if(end_time.ReadTime(exec_timeout_spec.str()))
		{
			Error("Invalid timeout spec \"%s\".\n",exec_timeout_spec.str());
			++failed;
		}
		else
		{  UpdateTimeout(exec_timeout_id,end_time);  }
	}
	exec_timeout_spec.deref();
	
	if(exec_timeout_sig_spec.str() && *(exec_timeout_sig_spec.str())!='\0')
	{
		const char *str=exec_timeout_sig_spec.str();
		if(!strcasecmp(str,"int"))         exec_timeout_action='i';
		else if(!strcasecmp(str,"term"))   exec_timeout_action='t';
		else if(!strcasecmp(str,"abort"))  exec_timeout_action='a';
		else
		{  Error("Illegal timeout action (-etimeout-sig) spec \"%s\".\n",str);
			++failed;  }
	}
	exec_timeout_sig_spec.deref();
	
	if(_resched_interval<0)
	{  _resched_interval=-1;  }  // This does NOT MAKE ANY SENSE, but well...
	if(_resched_interval)
	{  Warning("Schedule interval set to %ld msec.\n",_resched_interval);  }
	
	if(cyc_idle_timeout<0)
	{  cyc_idle_timeout=-1;  }  // disabled;
	
	// Always open /dev/null: 
	dev_null_fd=open("/dev/null",O_RDONLY);
	// We must PollFD() the fd so that it gets properly closed on 
	// execution of other processes. 
	if(dev_null_fd<0 || PollFD(dev_null_fd))
	{
		Error("Failed to open /dev/null: %s\n",
			(dev_null_fd<0) ? strerror(errno) : cstrings.allocfail);
		++failed;
	}
	
	return(failed ? 1 : 0);
}


// Return value: 0 -> OK; 1 -> error
int TaskManager::_SetUpParams()
{
	if(SetSection(NULL))
	{  return(1);  }
	
	AddParam("opmode",
		"Operation mode; defaults to binary name; use --list-opmode to "
		"get possible values",
		&opmode_name);
	// This is not supported. Gave up plans for LDR forwarder. 
	/*AddParam("tsource",
		"task source spec; use --list-tsource to get possible values "
		"(use if you cannot use -opmode)",
		&tsource_name);*/
	/*AddParam("tdriver",
		"task driver spec; use --list-tdriver to get possible values "
		"(use if you cannot use -opmode)",
		&tdinterface_name);*/
	
	daemon_pi=ParameterConsumer::AddParam("daemon",PTOptPar,
		"Detach from terminal and go into background. "
		"Possible values: \"no\", \"yes\" (also close stdin; default if "
		"value left away), \"close\" (to also close stdin,out,err), "
		"\"noclose\" (daemon, do not close streams)",
		&daemon_spec,par::default_string_handler,/*flags=*/0);
	if(!daemon_pi)  ++add_failed;
	
	AddParam("max-failed-in-seq|mfis",
		"max number of jobs to fail in sequence until giving up "
		"(0 to disable [NOT recommended])",&max_failed_in_sequence);
	
	AddParam("etimeout",
		"execution timeout; will behave like catching -etimeout-sig when the "
		"timeout expires; use absolute time (\"[DD.MM.[YYYY]] HH:MM[:SS]\") "
		"or relative time (\"now + {DD | [[HH:]MM]:SS}\") or \"none\" to "
		"switch off",&exec_timeout_spec);
	AddParam("etimeout-sig",
		"see -etimeout; possible values: \"int\" (default), \"term\", "
		"\"abort\"",&exec_timeout_sig_spec);
	AddParam("cycitimeout",
		"run cycle idle timeout in seconds; max inactivity time after "
		"run cycle end (LDR client); will behave like catching SIGINT when "
		"it expires; -1 to disable",
		&cyc_idle_timeout);
	
	AddParam("load-max","do not start jobs if the system load multiplied "
		"with 100 is >= this value; instead wait unitl it is lower than "
		"load-min",&load_high_thresh);
	AddParam("load-min","resume to start jobs if the system load (multiplied "
		"with 100) is < than this value; 0 turns off load check feature",
		&load_low_thresh);
	AddParam("load-poll-msec","load value poll delay in msec",&load_poll_msec);
	
	AddParam("schedule-delay","task manager schedule delay in msec; mainly "
		"useful in debugging",&_resched_interval);
	
	AddParam("dumptask",
		"When to dump task info. Use +/-aqbdrfz, capital letters for complete "
		"dump, small ones for headline only (default: \"+QDarf-d\"):\n"
		"  \"a\" -> task arrival (LDR)       \"q\" -> task being queued in todo queue\n"
		"  \"b\" -> reporting task as done   \"d\" -> task was and given back/destroyed\n"
		"  \"r\" -> when rendering is done   \"f\" -> when filtering is done\n"
		"  \"+Z\" -> turn all on             \"-z\" -> turn all off",
		&dumptask_spec);
	
	return(add_failed ? 1 : 0);
}


void TaskManager::_DumpInternalState()
{
	fprintf(stderr,
		"TaskManager state:\n"
		"  scheduled_for_start:     %s\n"
		"  jobs_failed_in_sequence: %d\n"
		"  failed_in_sequence_rp't: %s\n"
		"  dont_start_more_tasks:   %d\n"
		"  caught_sigint:           %d\n"
		"  abort_on_signal:         %d\n"
		"  schedule_quit:           %d\n"
		"  schedule_quit_after:     %d\n"
		"  recovering:              %d\n"
		"  told_interface_to_quit:  %d\n"
		"  told_tasksource_to_quit: %d\n"
		"  told_want_no_more_tasks: %d\n"
		"  kill_tasks_and_quit_now: %d\n"
		"  sched_kill_tasks:        %d\n"
		"  connected_to_tsource:    %d\n"
		"  ts_done_all_first:       %d\n"
		"  load_permits_starting:   %d\n"
		"  exec_stop_status:        %s (%d)\n"
		"  launching_special_flag:  %d\n"
		"  last_pend_done_frame_no: %d\n"
		"  tsgod_next_action:       %d\n"
		"  pending_action:          %d   (last frame %d)\n"
		"  nth_call_to_get_task:    %d\n"
		"  resched (tid0):          %s\n"
		"  tid_ts_cwait:            %ld\n"
		"  tid_load_poll:           %ld\n"
		"  fit_strategy_flag:       %d\n"
		,scheduled_for_start ? 
		  (scheduled_for_start->state==CompleteTask::ToBeRendered ? 
		    "render" : "filter") : "none",
		jobs_failed_in_sequence,
		failed_in_sequence_reported ? "yes" : "no",
		dont_start_more_tasks ? 1 : 0,
		caught_sigint,
		abort_on_signal ? 1 : 0,
		schedule_quit,
		schedule_quit_after,
		recovering ? 1 : 0,
		told_interface_to_quit ? 1 : 0,
		told_tasksource_to_quit ? 1 : 0,
		told_want_no_more_tasks ? 1 : 0,
		kill_tasks_and_quit_now ? 1 : 0,
		sched_kill_tasks,
		connected_to_tsource ? 1 : 0,
		ts_done_all_first ? 1 : 0,
		load_permits_starting ? 1 : 0,
		ExecStopStatus_String(exec_stop_status),int(exec_stop_status),
		launching_special_flag ? 1 : 0,
		last_pend_done_frame_no,
		tsgod_next_action,
		pending_action,last_pend_done_frame_no,
		nth_call_to_get_task,
		TimerInterval(tid0)<0 ? "no" : "yes",
		TimerInterval(tid_ts_cwait),
		TimerInterval(tid_load_poll),
		fit_strategy_flag);
		
	
	fprintf(stderr,"  Tasks todo (%d):",tasklist.todo_nelem);
	for(CompleteTask *i=tasklist.todo.first(); i; i=i->next)
	{
		char st[3]={'-','-','\0'};
		if(i->rt)
		{  st[0] = (i->state==CompleteTask::ToBeRendered ? 'r' : 'R');  }
		if(i->ft)
		{  st[1] = (i->state==CompleteTask::ToBeFiltered ? 'f' : 'F');  }
		fprintf(stderr," [%d:%s]",i->frame_no,st);
		assert(!i->d.any());
	}
	fprintf(stderr,"\n");
	
	fprintf(stderr,"  Tasks proc (%d):",tasklist.proc_nelem);
	for(CompleteTask *i=tasklist.proc.first(); i; i=i->next)
	{
		char st[3]={'D','R','F'};
		assert(i->state>=0 && i->state<3);
		fprintf(stderr," [%d:%c]",i->frame_no,st[i->state]);
		assert(i->d.any());
	}
	fprintf(stderr,"\n");
	
	fprintf(stderr,"  Tasks done (%d):",tasklist.done_nelem);
	for(CompleteTask *i=tasklist.done.first(); i; i=i->next)
	{
		fprintf(stderr," [%d:%c%c]",i->frame_no,
			i->ProcessedTask() ? 'p' : '-',
			(i->state==CompleteTask::ToBeRendered ? 'r' : 
			  i->state==CompleteTask::ToBeFiltered ? 'f' : 
			   i->state==CompleteTask::TaskDone ? 'd' : '?'));
		assert(!i->d.any());
	}
	fprintf(stderr,"\n");
}

void TaskManager::_DoDumpTaskList(const char *name,int nelem,
	CompleteTask *first)
{
	Verbose(TDR,"--<TaskDump>--<%c%s:%d>----------------------"
		"----------------------\n",tolower(*name),name+1,nelem);
	for(CompleteTask *i=first; i; i=i->next)
	{
		Verbose(TDR,
			"%s task [frame %d] (task ID %d) (td=%p,ldrc=%p,shall=%c%c)\n",
			name,i->frame_no,i->task_id,i->d.td,i->d.ldrc,
			i->d.shall_render ? 'R' : '-',i->d.shall_filter ? 'F' : '-');
		i->DumpTask(VERBOSE_TDR,/*when=*/1);
	}
}

void TaskManager::_DumpCompleteTaskList()
{
	_DoDumpTaskList("Todo",tasklist.todo_nelem,tasklist.todo.first());
	_DoDumpTaskList("Proc",tasklist.proc_nelem,tasklist.proc.first());
	_DoDumpTaskList("Done",tasklist.done_nelem,tasklist.done.first());
	Verbose(TDR,"--<TaskDump>---------------------------------"
		"----------------------\n");
}


// Well... this has to be done: 
// (Used by task source for verbose messages.) 
int TaskManager::tsGetDebugInfo(TSDebugInfo *dest)
{
	dest->todo_queue=tasklist.todo_nelem;
	dest->proc_queue=tasklist.proc_nelem;
	dest->done_queue=tasklist.done_nelem;
	#if TESTING
	assert(tasklist.todo_nelem==tasklist.todo.count());
	assert(tasklist.proc_nelem==tasklist.proc.count());
	assert(tasklist.done_nelem==tasklist.done.count());
	#endif
	return(0);
}


TaskManager::TaskManager(ComponentDataBase *cdb,int *failflag) :
	FDBase(failflag),
	TimeoutBase(failflag),
	TaskSourceConsumer(failflag),
	par::ParameterConsumer_Overloaded(cdb->parmanager(),failflag),
	RendViewAdmin_CommandExecuter(cdb->adminport(),failflag),
	exec_timeout_spec(failflag),exec_timeout_sig_spec(failflag),
	starttime(),
	last_work_cycle_end_time(HTime::Invalid),
	tasklist(failflag),
	opmode_name(failflag),
	// These are no longer supported (LDR forwarder). Gave up plans. 
	/*tsource_name(failflag),tdinterface_name(failflag),*/
	dumptask_spec(failflag),
	daemon_spec(failflag),
	acctime_render(failflag),
	acctime_filter(failflag)
{
	component_db=cdb;
	component_db->_SetTaskManager(this);
	
	interface=NULL;
	
	// Initial values: 
	jobs_failed_in_sequence=0;
	max_failed_in_sequence=3;   // 0 -> switch off `failed in sequence´-feature
	
	dev_null_fd=-1;
	
	scheduled_for_start=NULL;
	dont_start_more_tasks=0;
	caught_sigint=0;
	abort_on_signal=0;
	schedule_quit=0;
	schedule_quit_after=0;
	_actually_quit_step=0;
	recovering=0;
	sched_kill_tasks=0;
	told_interface_to_quit=0;
	told_tasksource_to_quit=0;
	told_want_no_more_tasks=0;
	kill_tasks_and_quit_now=0;
	exec_stop_status=ESS_Running;
	launching_special_flag=0;
	fit_strategy_flag=FSF_Run;
	
	connected_to_tsource=0;
	pending_action=ANone;
	
	tsgod_next_action=ANone;
	ts_done_all_first=0;
	nth_call_to_get_task=0;
	
	load_low_thresh=UnsetNegMagic;
	load_high_thresh=UnsetNegMagic;
	load_poll_msec=UnsetNegMagic;
	load_permits_starting=true;  // important. 
	load_control_stop_counter=0;
	max_load_measured=-1;
	
	failed_in_sequence_reported=0;
	
	work_cycle_count=0;
	cyc_idle_timeout=-1;   // disabled
	
	_resched_interval=0;
	
	dtsk_short_info=(DTSK_Arrival | DTSK_NowRendered | DTSK_NowFiltered);
	dtsk_long_info=(DTSK_ReportBackDone | DTSK_InitialQueue);
	dtsk_short_info|=dtsk_long_info;
	
	exec_timeout_action='i';  // SIGINT
	
	do_daemonize=0;
	daemon_pi=NULL;
	
	int failed=0;
	
	lpf_hist_size=5;
	lpf_hist_idx=0;
	last_proc_frames=(int*)LMalloc(lpf_hist_size*sizeof(int));
	if(!last_proc_frames && lpf_hist_size)
	{  ++failed;  }
	else
	{
		for(int i=0; i<lpf_hist_size; i++)
		{  last_proc_frames[i]=-1;  }
	}
	last_pend_done_frame_no=-1;
	
	//if(FDBase::SetManager(1))
	//{  ++failed;  }
	
	// Install 0msec timer to start the stuff: 
	tid0=InstallTimer(0,0);
	// Install disabled timer for connect re-try: 
	tid_ts_cwait=InstallTimer(-1,0);
	// Install disabled timer for load polling; 
	// IT IS KILLED LATER IF NOT NEEDED. 
	tid_load_poll=InstallTimer(-1,0);
	if(!tid0 || !tid_ts_cwait || !tid_load_poll)
	{  ++failed;  }
	
	// Install timeout node: 
	exec_timeout_id=InstallTimeout(HTime(HTime::Invalid));
	cyc_timeout_id=InstallTimeout(HTime(HTime::Invalid));
	if(!exec_timeout_id || !cyc_timeout_id)  ++failed;
	
	if(_SetUpParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskMan");  }
}


void TaskManager::_DestructCleanup(int real_destructor)
{
	if(!real_destructor)
	{
		_KillScheduledForStart();
	}
	
	//if(!kill_tasks_and_quit_now)
	{
		const char *_oopsmsg="OOPS: Tasks left in %s queue.\n";
		if(!tasklist.todo.is_empty())
		{  Warning(_oopsmsg,"todo");  }
		while(!tasklist.todo.is_empty())
		{  delete tasklist.todo.popfirst();  --tasklist.todo_nelem;  }
		if(!tasklist.proc.is_empty())
		{  Warning(_oopsmsg,"proc");  }
		while(!tasklist.proc.is_empty())
		{  delete tasklist.proc.popfirst();  --tasklist.proc_nelem;  }
		if(!tasklist.done.is_empty())
		{  Warning(_oopsmsg,"done");  }
		while(!tasklist.done.is_empty())
		{  delete tasklist.done.popfirst();  --tasklist.done_nelem;  }
		assert(!tasklist.todo_nelem);
		assert(!tasklist.proc_nelem);
		assert(!tasklist.done_nelem);
	}
	
	TaskSource *ts=tasksource();
	UseTaskSource(NULL);
	DELETE(ts);
	
	DELETE(interface);
}

TaskManager::~TaskManager()
{
	CloseFD(dev_null_fd);
	
	_DestructCleanup(/*real_destructor=*/1);
	
	// Make sure there are no jobs and no tasks left: 
	assert(!scheduled_for_start);
	
	assert(!connected_to_tsource);
	assert(pending_action==ANone);
	
	assert(!tasksource());
	assert(!interface);
	
	if(last_proc_frames)  last_proc_frames=(int*)LFree(last_proc_frames);
}


TaskManager_TaskList::TaskManager_TaskList(int *failflag) : 
	todo(failflag),
	proc(failflag),
	done(failflag)
{
	todo_nelem=0;
	proc_nelem=0;
	done_nelem=0;
}

TaskManager_TaskList::~TaskManager_TaskList()
{
	assert(todo.is_empty());  assert(!todo_nelem);
	assert(proc.is_empty());  assert(!proc_nelem);
	assert(done.is_empty());  assert(!done_nelem);
}


void TaskManager::AccumulatedTime::Reset()
{
	utime=HTime::Null;
	stime=HTime::Null;
	elapsed=HTime::Null;
	
	njobs=0;
}

TaskManager::AccumulatedTime::AccumulatedTime(int * /*failflag*/) : 
	utime(HTime::Null),
	stime(HTime::Null),
	elapsed(HTime::Null)
{
	njobs=0;
}
