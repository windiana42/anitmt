/*
 * taskmanager.cpp
 * 
 * Task manager class implementation. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <fcntl.h>
#include <assert.h>

/* This is for testing: before dequeuing a task from the todo/proc list, 
 * check if is actually on that list: */
#define TEST_TASK_LIST_MEMBERSHIP   1

#define UnsetNegMagic  (-29649)

#ifndef __LINE__
# warning No __LINE__ info provided. 
# define __LINE__ -1
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
	Error("Allocation failure.\n");
	abort();
}

static inline void _CheckAllocFail(int failed)
{
	if(failed)
	{  __CheckAllocFailFailed();  }
}


// Returns 1 if the passed task is being processed or was processed 
// by us: 
inline bool TaskManager::_ProcessedTask(const CompleteTask *ctsk)
{
	return(ctsk->d.any() || 
	   (ctsk->state==CompleteTask::TaskDone) ||
	   (ctsk->state==CompleteTask::ToBeFiltered && ctsk->rt) );
}


// True, if we rendered the task: 
bool TaskManager::IsARenderedTask(const CompleteTask *ctsk)
{
	if(!ctsk->rt)  return(false);
	if(ctsk->state==CompleteTask::TaskDone || 
	   ctsk->state==CompleteTask::ToBeFiltered )
	{  return(true);  }
	if(ctsk->state==CompleteTask::ToBeRendered && ctsk->d.td)
	{  if(ctsk->d.td->GetFactory()->DType()==DTRender)  return(true);  }
	return(false);
}

bool TaskManager::IsAFilteredTask(const CompleteTask *ctsk)
{
	if(!ctsk->ft)  return(false);
	if(ctsk->state==CompleteTask::TaskDone)
	{  return(true);  }
	if(ctsk->state==CompleteTask::ToBeFiltered && ctsk->d.td)
	{  if(ctsk->d.td->GetFactory()->DType()==DTFilter)  return(true);  }
	return(false);
}

bool TaskManager::IsPartlyRenderedTask(const CompleteTask *ctsk)
{
	//if(ctsk->td && ctsk->td->GetFactory()->DType()==DTRender)  return(true);
	assert(!ctsk->d.any());
	if(ctsk->rtes.tes.status==TTR_JobTerm && 
	   (ctsk->rtes.tes.signal==JK_UserInterrupt || 
	    ctsk->rtes.tes.signal==JK_Timeout) )
	{  return(true);  }
	return(false);
}
bool TaskManager::IsPartlyFilteredTask(const CompleteTask *ctsk)
{
	//if(ctsk->td && ctsk->td->GetFactory()->DType()==DTFilter)  return(true);
	assert(!ctsk->d.any());
	if(ctsk->ftes.tes.status==TTR_JobTerm && 
	   (ctsk->ftes.tes.signal==JK_UserInterrupt || 
	    ctsk->ftes.tes.signal==JK_Timeout) )
	{  return(true);  }
	return(false);
}

inline int cmpMAX(int a,int b)
{  return((a>b) ? a : b);  }


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
Error("MISSING!!!!!!!!!!!! (taskmanager.cpp:%d)\n",__LINE__);
		// (...still able to deliver done tasks...???)
		//assert(0);
	}
	
	_KillScheduledForStart();    // ALWAYS!
	
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
		
		if(!_ProcessedTask(i))  continue;
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
			// Okay, task was rendered and has to be filtered now. 
			
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
			
			// [[Seems there is nothing more to do.]]
			//_CheckStartTasks();
			
			break;
		case CompleteTask::ToBeRendered:  // fall through
		default:  assert(0);  break;
	}
}


// running_jobs: number of still running jobs; ONLY FOR emergency error handling. 
void TaskManager::HandleFailedTask(CompleteTask *ctsk,int running_jobs)
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
		//     edsetail==TaskDriver::EDATerm (thus not EDSuccess) 
		//     here. And we won't muck with errors now and we 
		//     do NOT (try to) connect to the task source. We 
		//     are here becuase of severe error (probably because 
		//     connecton to TaskSource failed). 
	}
	
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
		
		if(GetTaskSourceType()==TST_Active)
		{
			// For active task sources, we probably do not want to 
			// schedule quit but recover instead. 
			fprintf(stderr,"HACK ME...********* DO RECOVERY INSTEAD OF QUIT.\n");
			//assert(0);
			schedule_quit=2;
		}
		else
		{
			schedule_quit=2;
		}
		
		ReSched(__LINE__);
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
	if(dont_start_more_tasks)
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


void TaskManager::PutBackTask(CompleteTask *ctsk)
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
	
	// We put task into todo queue. 
	// ###FIXME###
	// We should put it into done queue with error if that happens too often?
	
	// Re-queue at BEGINNING of todo list. 
	tasklist.proc.dequeue(ctsk);  --tasklist.proc_nelem;
	tasklist.todo.insert(ctsk);   ++tasklist.todo_nelem;
	
	// See also the note on the call to DontStartMoreTasks() in 
	// HandleSuccessfulJob(). In case we no not start more tasks, 
	// we may not bring the state machine in a situation where it 
	// would have to. 
	if(dont_start_more_tasks)
	{  DontStartMoreTasks();  }
}


CompleteTask *TaskManager::FindTaskByTaskID(u_int32_t task_id)
{
	#warning could speed up this loopup in some way?
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
				ts_done_all_first=0;
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
					pending_action=ADisconnect;
					int rv=TSDisconnect();
					assert(!rv);   // ...otherwise internal error
					#warning check if that results in too many connect - disconnect - connect ... calls. 
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
			assert(connected_to_tsource);
			
			if(ni->disconnstat==DSOkay)
			{
				// Successfully disconnected. 
				pending_action=ANone;
				connected_to_tsource=0;
				// Nothing else to do. Just wait. 
			}
			else if(ni->disconnstat==DSWorking)
			{
				// Working...
				// pending_action stays ADisconnect. 
				// simply, WAIT A LITTLE...
			}
			else
			{
				// Failed to disconnect from the task source. 
				pending_action=ANone;
				
				// This is bad. 
				Error("ABORTING.\n");
				abort();
				#warning MISSING. 
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
				recovering=1;
				// DO NOT SET THESE
				// schedule_quit=1;  NO!
				// sched_kill_tasks=2;  NO! // server error
				ReSched(__LINE__);
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
	else if(signo==SIGTSTP || signo==SIGCONT )
	{
		// Terminal stop and cont. Hmm... check state. 
		if( ( exec_stopped && signo==SIGCONT) || 
		    (!exec_stopped && signo==SIGTSTP) )
		{
			int signo=exec_stopped ? SIGCONT : SIGSTOP;
			interface->StopContTasks(signo);
			
			// Swap state: 
			exec_stopped=exec_stopped ? 0 : 1;
			// If we shall be stopped, then we actually stop. Note that this 
			// means that we also won't disconnect from the task source, 
			// check timers, etc. 
			if(exec_stopped)
			{  raise(SIGSTOP);  }
		}
		else
		{  Warning("Ignoring SIG%s because already %s.\n",
			signo==SIGCONT ? "CONT" : "TSTP",
			exec_stopped ? "stopped" : "running");  }
	}
	else if(signo==SIGUSR1)
	{  _DumpInternalState();  }
	
	if(do_sched_quit)
	{
		// Make sure we quit then...
		if(!schedule_quit)
		{  schedule_quit=1;  }  // 1 -> exit(0);
		
		if(scheduled_for_start)
		{
			// No NEW tasks...
			if(!_ProcessedTask(scheduled_for_start))
			{  _KillScheduledForStart();  }
		}
	}
	if(do_kill_tasks)
	{
		DontStartMoreTasks();
		
		schedule_quit=2;  // 2 -> exit(1);
		sched_kill_tasks=1;   // user interrupt
	}
	if(do_sched_quit || do_kill_tasks)
	{  ReSched(__LINE__);  }  // -> quit now if nothing to do 
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
			if(delta.Get(HTime::msec)>0)
			{  zero_reps=0;  }
			else if((++zero_reps)==5)
			{
				fprintf(stderr,"OOPS: TaskManager scheduler strangely busy.\n");
				_DumpInternalState();
			}
		}
	}
	
	// Read instructions near _ActuallyQuit() to understand what is 
	// going on. 
	if(_actually_quit_step)   // Initially 1. 
	{
		Verbose(DBGV,"[%d]",_actually_quit_step);
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
			(sched_kill_tasks==1) ? JK_UserInterrupt : JK_ServerError);
		
		Verbose(0,"Sent SIGTERM to %d jobs.%s",nkilled,
			kill_tasks_and_quit_now ? "" : "\n");
		
		// Okay, did that. 
		sched_kill_tasks=0;
		
		if(kill_tasks_and_quit_now)
		{
			if(nkilled==0)
			{
				Verbose(0," QUITTING\n");
				_ActuallyQuit(2);  // QUIT NOW. 
				return;
			}
			else
			{  Verbose(0," Waiting for jobs to quit.\n");  }
			_KillScheduledForStart();
		}
	}
	
	if(scheduled_for_start && load_permits_starting)
	{
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
			// The interface will call CheckStartNewJobs(-1) when done. 
		}
		
		if(!interface->AreThereJobsRunning() && 
		   tasklist.todo.is_empty() && 
		   tasklist.proc.is_empty() && 
		   tasklist.done.is_empty() && 
		   !connected_to_tsource && 
		   pending_action==ANone )
		{
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
		#warning These may cause trouble. Check that. 
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
	
	// Ugrmbl... Not sure about this: ###
	// We can get this case with the LDR task driver interface. 
	if(scheduled_for_start->d.any())
	{
		// Must put task into proc queue: 
		tasklist.todo.dequeue(scheduled_for_start);  --tasklist.todo_nelem;
		tasklist.proc.append(scheduled_for_start);   ++tasklist.proc_nelem;
	}
	
	scheduled_for_start=NULL;
	if(!load_permits_starting)
	{  UpdateTimer(tid_load_poll,-1,0);  }
}


int TaskManager::timernotify(TimerInfo *ti)
{
	if(ti->tid==tid0)
	{  _schedule(ti);  }
	else if(ti->tid==tid_load_poll)
	{
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
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	if(ti->tid==timeout_id)
	{  Warning("Execution timeout. Task queue: todo=%d, proc=%d, done=%d\n",
		tasklist.todo_nelem,tasklist.proc_nelem,tasklist.done_nelem);  }
	else if(ti->tid==cyc_timeout_id)
	{
		HTime tmp;
		tmp.Set(cyc_idle_timeout,HTime::seconds);
		Warning("Execution cycle idle timeout. "
			"(Idle for %s; I'm bored.)\n",tmp.PrintElapsed(/*with_msec=*/0));
	}
	else assert(0);
	Warning("  Timeout time: %s\n",ti->timeout->PrintTime(1));
	Warning("  Current time: %s\n",ti->current->PrintTime(1));
	
	#warning allow different actions. (also SIGTERM and abort)
	_ActOnSignal(SIGINT,/*real_signal=*/0);
	
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
	
	_EmitCPUStats(doquit ? "Overall CPU stats" : "Overall CPU stats (until now)",
		&elapsed_start,
		&ptu_self_start,&ptu_chld_start);
	if(!doquit)
	{  _EmitCPUStats("CPU stats for last work cycle",&elapsed_work,
		&ptu_self_work,&ptu_chld_work);  }
	
	#warning FIXME: more info to come
	
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
			if(kill_tasks_and_quit_now)  break;  // We may not be here. 
			if(sched_kill_tasks)  break;
			if(connected_to_tsource)  break;
			if(exec_stopped)  break;   // We are here and stopped??
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
		ts_done_all_first=0;
		nth_call_to_get_task=0;
		
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
		
fprintf(stderr,"todo-thresh: low=%d, high=%d [debug]\n",
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
	
	int opmode_num=-1;
	     if(!strcasecmp(opmode,"rendview"))   opmode_num=0;
	else if(!strcasecmp(opmode,"ldrclient"))  opmode_num=1;
	else if(!strcasecmp(opmode,"ldrserver"))  opmode_num=2;
	
	if(opmode_num<0)
	{
		if(opmode_name.str())
		{  Error("Illegal operation mode \"%s\". Try --list-opmode.\n",
			opmode_name.str());  return(-1);  }
		else if(!tsource_name && !tdinterface_name)
		{
			Warning("Nonstandard binary name. "
				"Defaulting to opmode \"rendview\".\n");
			opmode_num=0;
		}
	}
	
	// Set tsource and tdif name if not explicitly told otherwise by user: 
	if(!tsource_name)
	{  _CheckAllocFail(tsource_name.set((opmode_num==1) ? "LDR" : "local"));  }
	if(!tdinterface_name)
	{  _CheckAllocFail(tdinterface_name.set((opmode_num==2) ? "LDR" : "local"));  }
	
	
	const char *tdif_name=tdinterface_name.str();
	const char *ts_name=tsource_name.str();
	assert(tdif_name && ts_name);
	
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
	
	// Write out useful verbose information: 
	char tmp[24];
	if(max_failed_in_sequence)  snprintf(tmp,24,"%d",max_failed_in_sequence);
	else  strcpy(tmp,"OFF");
	Verbose(TDI,"Ready to perform work: max-failed-in-seq=%s\n",tmp);
	
	Verbose(TDI,"  Execution timeout: ");
	#warning could also print relative time...
	if(timeout_id && !TimeoutTime(timeout_id)->IsInvalid())
	{
		HTime tmp(HTime::Curr);
		HTime delta=*TimeoutTime(timeout_id)-tmp;
		Verbose(TDI,"%s (%s)\n",TimeoutTime(timeout_id)->PrintTime(1),
			delta.PrintElapsed());
		tmp.Set(10,HTime::seconds);
		if(delta<tmp)
		{
			UpdateTimeout(timeout_id,HTime(HTime::Invalid));
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
	Verbose(TDI,"  Starting at (local): %s\n",starttime.PrintTime(1,1));
	HTime timetmp;
	timetmp=*TimeoutTime(timeout_id);
	Verbose(TDI,"  Execution timeout: %s\n",
		timetmp.IsInvalid() ? "[none]" : timetmp.PrintTime(/*local=*/1,/*with_msec=*/0));
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
		_ActuallyQuit(1);
		return;
	}
	
	interface->WriteProcessingInfo(0,
		(GetTaskSourceType()==TST_Active) ? "waiting for" : "beginning to");
	
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
			_ControlWorkCycleTimeout(/*install=*/1);
			break;
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
#warning RACE: njobs may be 0 here because the first client disconnected \
unexpectedly. In this case we should not quit but wait for the other \
connections. 
			
			// This will put back all tasks to the source and quit then. 
			// This is an error unless there is nothing to do and we're 
			// quitting. 
			bool just_quitting = 
				tasklist.todo.is_empty() && tasklist.proc.is_empty() && 
				(schedule_quit || schedule_quit_after || recovering);
			if(!just_quitting)
			{  schedule_quit=2;  }
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
int TaskManager::_CheckStartExchange()
{
	if(connected_to_tsource || pending_action==AConnect)
	{  return(0);  }
	
	if(kill_tasks_and_quit_now)
	{  return(-2);  }
	
	// (Note that we do NOT need the tid0 / _schedule() - stuff here, 
	// as the task source is guaranteed not to call tsnotify() 
	// immediately (i.e. on the stack now).) 
	
	// First, see what we CAN do: 
	//bool can_done=_TS_CanDo_DoneTask();
	//bool can_get=_TS_CanDo_GetTask(can_done);;
	
	// Note: recovering sets dont_start_more_tasks. 
	assert(!recovering || dont_start_more_tasks);
	
	int must_connect=0;
	if(dont_start_more_tasks || schedule_quit)
	{
		// We will immediately contact the task source and put back 
		// the tasks that need to be put back...
		must_connect=_TS_CanDo_DoneTask() ? 1 : 0;
	}
	
	if(!must_connect)
	{
		if(tasklist.done_nelem>=interface->Get_done_thresh_high() ||
		   (!schedule_quit_after && !dont_start_more_tasks && 
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
			    (schedule_quit && !_ProcessedTask(i) ) )
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
			
			// Dump all the information to the user: 
			_PrintDoneInfo(ctsk);
			
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

// Special function used by _PrintDoneInfo(): 
void TaskManager::_PrintTaskExecStatus(CompleteTask::TES *ct_tes,
	const char *rnd_flt)
{
	TaskExecutionStatus *tes=&ct_tes->tes;
	
	VerboseSpecial("      Status: %s",tes->StatusString());
	if(tes->status==TTR_Unset)  return;
	Verbose(TDR,"      %sed by: %s\n",rnd_flt,
		ct_tes->processed_by.str() ? ct_tes->processed_by.str() : "[n/a]");
	Verbose(TDR,"      Started: %s\n",tes->starttime.PrintTime(1,1));
	Verbose(TDR,"      Done:    %s\n",tes->endtime.PrintTime(1,1));
	HTime duration=tes->endtime-tes->starttime;
	
	char cpu_tmp[24];
	double dur_sec=duration.GetD(HTime::seconds);
	double dur_pc=100.0*(tes->utime.GetD(HTime::seconds)+
		tes->stime.GetD(HTime::seconds))/dur_sec;
	if(dur_sec>=0.01 && dur_pc<10000.0)
	{  snprintf(cpu_tmp,24,"  (%.2f%% CPU)",dur_pc);  }
	else
	{  cpu_tmp[0]='\0';  }
	Verbose(TDR,"      Elapsed: %s%s\n",duration.PrintElapsed(),cpu_tmp);
	
	char tmp[48];
	snprintf(tmp,48,"%s",tes->utime.PrintElapsed());
	Verbose(TDR,"      Time in mode: user: %s; system: %s\n",
		tmp,tes->stime.PrintElapsed());
}

// Special function used by _PrintDoneInfo(): 
void TaskManager::_DoPrintTaskExecuted(TaskParams *tp,TaskStructBase *tsb,
	const char *binpath,bool was_processed)
{
	if(!was_processed)
	{  VerboseSpecial("    Executed: [not processed]");  return;  }
	
	char tmpA[32];
	if(tp)
	{
		if(tp->niceval==TaskParams::NoNice)  strcpy(tmpA,"(none)");
		else  snprintf(tmpA,32,"%d",tp->niceval);
	}
	else
	{  strcpy(tmpA,"??");  }
	
	long timeout=-1;
	char timeout_char='\0';
	if(tp && tp->timeout>0)
	{  timeout=tp->timeout;  timeout_char='L';  }
	if(tsb->timeout>0 && timeout>tsb->timeout)
	{  timeout=tsb->timeout;  timeout_char='T';  }  // Another timeout. Take shorter one. 
	
	char tmpB[32];
	if(timeout<=0)  strcpy(tmpB,"(none)");
	else
	{  snprintf(tmpB,32,"%ld sec (%s)",(timeout+500)/1000,
		timeout_char=='L' ? "loc" : "TS");  }
	
	Verbose(TDR,"    Executed: %s (nice value: %s; timeout: %s; tty: %s)\n",
		binpath,tmpA,tmpB,(tp && tp->call_setsid) ? "no" : "yes");
}

char *TaskManager::Completely_Partly_Not_Processed(const CompleteTask *ctsk,
	int *level)
{
	if(ctsk->state==CompleteTask::TaskDone)
	{
		if(level)  *level=2;
		return("completely");
	}
	if(_ProcessedTask(ctsk) ||         // e.g. rendered but but not filtered
	   IsPartlyRenderedTask(ctsk) ||
	   IsPartlyFilteredTask(ctsk) )    // interrupt
	{
		if(level)  *level=1;
		return("partly");
	}
	if(level)  *level=0;
	return("not");
}

void TaskManager::_PrintDoneInfo(CompleteTask *ctsk)
{
	VerboseSpecial("Reporting task [frame %d] as done (%s processed).",
		ctsk->frame_no,Completely_Partly_Not_Processed(ctsk));
	Verbose(TDR,"  Task state: %s\n",CompleteTask::StateString(ctsk->state));
	
	if(ctsk->rt)
	{
		Verbose(TDR,"  Render task: %s (%s driver)\n",
			ctsk->rt->rdesc->name.str(),ctsk->rt->rdesc->dfactory->DriverName());
		Verbose(TDR,"    Input file:  hd path: %s\n",
			ctsk->rt->infile ? ctsk->rt->infile->HDPath().str() : NULL);
		Verbose(TDR,"    Output file: hd path: %s; size %dx%d; format: %s (%d bpc)\n",
			ctsk->rt->outfile ? ctsk->rt->outfile->HDPath().str() : NULL,
			ctsk->rt->width,ctsk->rt->height,
			ctsk->rt->oformat ? ctsk->rt->oformat->name : NULL,
			ctsk->rt->oformat ? ctsk->rt->oformat->bits_p_rgb : 0);
		#warning more info?
		_DoPrintTaskExecuted(ctsk->rtp,ctsk->rt,ctsk->rt->rdesc->binpath.str(),
			IsARenderedTask(ctsk) || IsPartlyRenderedTask(ctsk));
		_PrintTaskExecStatus(&ctsk->rtes,"Render");
	}
	else
	{  Verbose(TDR,"  Render task: [none]\n");  }
	
	if(ctsk->ft)
	{
		Verbose(TDR,"  Filter task: %s (%s driver)\n",
			ctsk->ft->fdesc->name.str(),ctsk->ft->fdesc->dfactory->DriverName());
		Verbose(TDR,"    Input file:  hd path: %s\n",
			ctsk->ft->infile ? ctsk->ft->infile->HDPath().str() : NULL);
		Verbose(TDR,"    Output file: hd path: %s\n",
			ctsk->ft->outfile ? ctsk->ft->outfile->HDPath().str() : NULL);
		#warning more info?
		_DoPrintTaskExecuted(ctsk->ftp,ctsk->ft,ctsk->ft->fdesc->binpath.str(),
			IsAFilteredTask(ctsk) || IsPartlyFilteredTask(ctsk));
		_PrintTaskExecStatus(&ctsk->ftes,"Filter");
	}
	else
	{  Verbose(TDR,"  Filter task: [none]\n");  }
}


int TaskManager::CheckParams()
{
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return(0);  }
	
	int failed=0;
	
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
		{  UpdateTimeout(timeout_id,end_time);  }
	}
	exec_timeout_spec.deref();
	
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
			(dev_null_fd<0) ? strerror(errno) : "alloc failure");
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
		"Operation mode; shorthand for -tsource and -tdriver; defaults to "
		"binary name; use --list-opmode to get possible values",
		&opmode_name);
	AddParam("tsource",
		"task source spec; use --list-tsource to get possible values "
		"(use if you cannot use -opmode)",
		&tsource_name);
	AddParam("tdriver",
		"task driver spec; use --list-tdriver to get possible values "
		"(use if you cannot use -opmode)",
		&tdinterface_name);
	
	AddParam("max-failed-in-seq|mfis",
		"max number of jobs to fail in sequence until giving up "
		"(0 to disable [NOT recommended])",&max_failed_in_sequence);
	
	AddParam("etimeout",
		"execution timeout; will behave like catching SIGINT when the "
		"timeout expires; use absolute time (\"[DD.MM.[YYYY]] HH:MM[:SS]\") "
		"or relative time (\"now + {DD | [[HH:]MM]:SS}\") or \"none\" to "
		"switch off",&exec_timeout_spec);
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
	AddParam("load-poll-msec","load value poll delay",&load_poll_msec);
	
	AddParam("schedule-delay","task manager schedule delay in msec; mainly "
		"useful in debugging",&_resched_interval);
	
	#warning further params: delay_between_tasks, max_failed_jobs, \
		dont_fail_on_failed_jobs, \
		brutal_on_first_sigint, ignore_sigint, signal_never_abort
	
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
		"  kill_tasks_and_quit_now: %d\n"
		"  sched_kill_tasks:        %d\n"
		"  connected_to_tsource:    %d\n"
		"  ts_done_all_first:       %d\n"
		"  load_permits_starting:   %d\n"
		"  exec_stopped:            %d\n"
		"  last_pend_done_frame_no: %d\n"
		"  tsgod_next_action:       %d\n"
		"  pending_action:          %d   (last frame %d)\n"
		"  nth_call_to_get_task:    %d\n"
		"  resched (tid0):          %s\n"
		"  tid_ts_cwait:            %ld\n"
		"  tid_load_poll:           %ld\n"
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
		kill_tasks_and_quit_now ? 1 : 0,
		sched_kill_tasks,
		connected_to_tsource ? 1 : 0,
		ts_done_all_first ? 1 : 0,
		load_permits_starting ? 1 : 0,
		exec_stopped ? 1 : 0,
		last_pend_done_frame_no,
		tsgod_next_action,
		pending_action,last_pend_done_frame_no,
		nth_call_to_get_task,
		TimerInterval(tid0)<0 ? "no" : "yes",
		TimerInterval(tid_ts_cwait),
		TimerInterval(tid_load_poll));
		
	
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
		char st[3]={'R','F','D'};
		assert(i->state>=0 && i->state<3);
		fprintf(stderr," [%d:%c]",i->frame_no,st[i->state]);
		assert(i->d.any());
	}
	fprintf(stderr,"\n");
	
	fprintf(stderr,"  Tasks done (%d):",tasklist.done_nelem);
	for(CompleteTask *i=tasklist.done.first(); i; i=i->next)
	{
		fprintf(stderr," [%d:%c%c]",i->frame_no,
			_ProcessedTask(i) ? 'p' : '-',
			(i->state==CompleteTask::ToBeRendered ? 'r' : 
			  i->state==CompleteTask::ToBeFiltered ? 'f' : 
			   i->state==CompleteTask::TaskDone ? 'd' : '?'));
		assert(!i->d.any());
	}
	fprintf(stderr,"\n");
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
	exec_timeout_spec(failflag),
	starttime(),
	tasklist(failflag),
	tsource_name(failflag),tdinterface_name(failflag),opmode_name(failflag)
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
	recovering=0;
	sched_kill_tasks=0;
	told_interface_to_quit=0;
	kill_tasks_and_quit_now=0;
	exec_stopped=0;
	
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
	timeout_id=InstallTimeout(HTime(HTime::Invalid));
	cyc_timeout_id=InstallTimeout(HTime(HTime::Invalid));
	if(!timeout_id || !cyc_timeout_id)  ++failed;
	
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
		if(!tasklist.todo.is_empty())
		{  Warning("OOPS: Tasks left in todo queue.\n");  }
		while(!tasklist.todo.is_empty())
		{  delete tasklist.todo.popfirst();  --tasklist.todo_nelem;  }
		if(!tasklist.proc.is_empty())
		{  Warning("OOPS: Tasks left in proc queue.\n");  }
		while(!tasklist.proc.is_empty())
		{  delete tasklist.proc.popfirst();  --tasklist.proc_nelem;  }
		if(!tasklist.done.is_empty())
		{  Warning("OOPS: Tasks left in done queue.\n");  }
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
