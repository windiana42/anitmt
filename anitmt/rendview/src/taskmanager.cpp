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

/* This is for testing: before dequeuing a task from the todo list, check if 
 * is actually on that list: */
#define TEST_TASK_LIST_MEMBERSHIP   1

#define UnsetNegMagic  (-29649)

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
	if(ctsk->rtes.status==TTR_JobTerm && 
	   (ctsk->rtes.signal==JK_UserInterrupt || ctsk->rtes.signal==JK_Timeout) )
	{  return(true);  }
	return(false);
}
bool TaskManager::IsPartlyFilteredTask(const CompleteTask *ctsk)
{
	//if(ctsk->td && ctsk->td->GetFactory()->DType()==DTFilter)  return(true);
	assert(!ctsk->d.any());
	if(ctsk->ftes.status==TTR_JobTerm && 
	   (ctsk->ftes.signal==JK_UserInterrupt || ctsk->ftes.signal==JK_Timeout) )
	{  return(true);  }
	return(false);
}

inline int cmpMAX(int a,int b)
{  return((a>b) ? a : b);  }


void TaskManager::DontStartMoreTasks()
{
	// Note: We may enter this function also in case 
	//       dont_start_more_tasks is already set. In this case 
	//       we MUST NOT return immediately but go through tasklist_todo 
	//       again and re-queue tasks from todo to done queue if needed. 
	
	dont_start_more_tasks=1;
	
	_KillScheduledForStart();    // ALWAYS!
	
	for(CompleteTask *_i=tasklist_todo.first(); _i; )
	{
		CompleteTask *i=_i;  _i=_i->next;
		
		if(!_ProcessedTask(i) || i->d.any())  continue;
		// Here: *i was processed and is currently not processed but 
		// still in the todo list (i.e. *i rendered but not yet 
		// filtered). 
		assert(i->state==CompleteTask::ToBeFiltered);
		
		// In this case: Move task to done queue. 
		tasklist_todo.dequeue(i);
		tasklist_done.append(i);
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
	
	switch(ctsk->state)
	{
		case CompleteTask::TaskDone:
			// Good, task is done completely. 
			
			#if TEST_TASK_LIST_MEMBERSHIP
			// Test: see if task is actually in tasklist_todo: 
			assert(tasklist_todo.find(ctsk));
			#endif
			
			tasklist_todo.dequeue(ctsk);
			tasklist_done.append(ctsk);
			
			// See if we have to connect to the task source and schedule 
			// that if needed: 
			_CheckStartExchange();
			
			break;
		case CompleteTask::ToBeFiltered:
			// Okay, task was rendered and has to be filtered now. 
			
			// In case dont_start_more_tasks is set we must make sure that we 
			// do not bring the state machine into a state where that was 
			// required: If we do not start more tasks, then a successful 
			// task which is now to be filtered cannot be filtered any more 
			// and so has to be put into the lasklist_done queue which is 
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
	// Read comment near _ActuallyQuit() for info. 
	if(_actually_quit_step)
	{  return;  }
	
	// Error info already stored. 
	
	#if TEST_TASK_LIST_MEMBERSHIP
	// Test: see if task is actually in tasklist_todo: 
	assert(tasklist_todo.find(ctsk));
	#endif
	
	tasklist_todo.dequeue(ctsk);
	tasklist_done.append(ctsk);
	
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
		Error("%d jobs failed in sequence. Giving up.\n",
			jobs_failed_in_sequence);
		
		DontStartMoreTasks();
		schedule_quit=2;
		ReSched();
	}
	
	_CheckStartExchange();
	
	// Furthermore, we should probably send back the 
	// done (and failed) tasks first before getting 
	// new ones: 
	if(connected_to_tsource)
	{  ts_done_all_first=true;  }
}


// Called for a task which was just obtained from the task source. 
void TaskManager::_TakeFreshTask(CompleteTask *ctsk)
{
	assert(ctsk);   // otherwise internal error
	
	// Put task into queue (at the END of the queue): 
	tasklist_todo.append(ctsk);
	
	// NOTE: ni->ctsk->state = TaskDone here. Must set up 
	//       proper value now and set up TaskParams: 
	interface->DealWithNewTask(ctsk);
	// This called HandleFailedTask() on error. 
	
	// Great. Task queued; now see if we want to start a job. 
	_CheckStartTasks();
}


void TaskManager::PutBackTask(CompleteTask *ctsk)
{
	assert(ctsk);   // otherwise internal error
	// NOTE: ctsk is queued in tasklist_todo (YES!). 
	
	// If someone puts back a task, then it may not have a 
	// task driver or LDR client attached. 
	assert(!ctsk->d.any());
	
	// NOTE: We MAY dequeue or re-queue the passed ctsk here 
	//       (the caller knows that and has removal-safe loop). 
	
fprintf(stderr,"TaskManager::PutBackTask(): ANYTHING TO DO HERE?\n");
}


int TaskManager::tsnotify(TSNotifyInfo *ni)
{
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
					ReSched();
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
				
				_TakeFreshTask(ni->ctsk);
				
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
				ReSched();
			}
			else if(ni->getstat==GTSEnoughTasks)
			{
				pending_action=ANone;
				
				// So the task source thinks we have enough tasks. 
				// Let's see: 
				if(!tasklist_done.is_empty())
				{
					// Okay, first we report `done� for all done tasks. 
					ts_done_all_first=1;
					_TS_GetOrDoneTask();
				}
				else if(tasklist_todo.is_empty())
				{
					// No, the task source is wrong. 
					Error("Task source thinks we have enough tasks but that's "
						"not true.\n");
					schedule_quit=2;
					ReSched();
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
					ReSched();
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
				
				#if 0  // DO THIS BEFORE CALLING GetTask(): 
				// When recovery is reported to be done, then there may not be any 
				// tasks left. In case this fails, there is a bug in TaskManager's 
				// state machinery. 
				assert(p->component_db()->taskmanager()->GetTaskListTodo()->is_empty());
				assert(number_of_running_tasks==0);
				#endif
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
				if(schedule_quit || schedule_quit_after)
				{  ReSched();  }
			}
		}  break;
		case AActive:
		{
			// Active task source only. 
			// We must neither be connected to the task source to receive 
			// this nor must we have send a query. 
			if(ni->activestat==TASTakeTask)
			{
				_TakeFreshTask(ni->ctsk);
			}
			else if(ni->activestat==TASRecovering)
			{
				// Bad. We lost connection to the server. 
				// This means, we must now start recovery and call 
				// GetTask() when done [which is normally invalid for 
				// active task sources]. 
				// Okay, then let's recover: 
				
				Error("hack recovery.\n");
				assert(0);
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
	{  ReSched();  }  // -> quit now if nothing to do 
}


void TaskManager::_schedule(TimerInfo *ti)
{
	assert(ti->tid==tid0);
	
	// Read instructions near _ActuallyQuit() to understand what is 
	// going on. 
	if(_actually_quit_step)   // Initially 1. 
	{
		fprintf(stderr,"[%d]",_actually_quit_step);
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
		interface->LaunchTask(scheduled_for_start);
		
		_KillScheduledForStart();
		
		// See if we want to start more task(s): 
		_CheckStartTasks();
	}
	
	if(schedule_quit || schedule_quit_after)
	{
		// Quit is scheduled. So, we quit if everything is cleaned up: 
		
		// The interface (in case of an LDR server) must disconnect 
		// from the clients if there are no more jobs to do. 
		// Let him do that. 
		if(tasklist_todo.is_empty() && 
		   !told_interface_to_quit )
		{
			interface->PleaseQuit();
			told_interface_to_quit=1;
			// The interface will call CheckStartNewJobs(-1) when done. 
		}
		
		if(!interface->AreThereJobsRunning() && 
		   tasklist_todo.is_empty() && 
		   tasklist_done.is_empty() && 
		   !connected_to_tsource && 
		   pending_action==ANone )
		{
			// Okay, we may actually quit. 
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
	if(scheduled_for_start)
	{
		scheduled_for_start=NULL;
		if(!load_permits_starting)
		{  UpdateTimer(tid_load_poll,-1,0);  }
	}
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
				ReSched();
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
	
	assert(ti->tid==timeout_id);
	Warning("Execution timeout.\n");
	Warning("  Timeout time: %s\n",ti->timeout->PrintTime(1));
	Warning("  Current time: %s\n",ti->current->PrintTime(1));
	
	#warning allow different actions. (also SIGTERM and abort)
	_ActOnSignal(SIGINT,/*real_signal=*/0);
	
	return(0);
}


static int _int_cmp(const void *a,const void *b)
{
	return(*(int*)a - *(int*)b);
}

// Simply call fdmanager()->Quit(status) and write info. 
// (Actually, _ActuallyQuit() is called...)
void TaskManager::_DoQuit(int status)
{
	VerboseSpecial("Now exiting with status=%s (%d)",
		status ? "failure" : "success",status);
	
	if(interface)
	{  interface->WriteProcessingInfo(1,NULL);  }
	
	ProcessManager::ProcTimeUsage ptu_self,ptu_chld;
	ProcessManager::manager->GetTimeUsage(0,&ptu_self);
	ProcessManager::manager->GetTimeUsage(1,&ptu_chld);
	
	HTime elapsed=ptu_self.uptime;
	HTime endtime=starttime+elapsed;
	Verbose(TDI,"  exiting at (local): %s\n",endtime.PrintTime(1,1));
	Verbose(TDI,"  elapsed time: %s\n",elapsed.PrintElapsed());
	
	int loadval=_GetLoadValue();
	Verbose(TDI,"  load control: ");
	if(load_low_thresh<=0)
	{  Verbose(TDI,"[disabled]\n");  }
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
		Verbose(TDI,"  last successfully done frames:");
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
	
	if(elapsed.GetD(HTime::seconds)>0.2)
	{
		double rv_cpu=100.0*(ptu_self.stime.GetD(HTime::seconds)+
			ptu_self.utime.GetD(HTime::seconds)) / elapsed.GetD(HTime::seconds);
		double ch_cpu=100.0*(ptu_chld.stime.GetD(HTime::seconds)+
			ptu_chld.utime.GetD(HTime::seconds)) / elapsed.GetD(HTime::seconds);
		
		char tmp[48];
		snprintf(tmp,48,"%s",ptu_self.utime.PrintElapsed());
		Verbose(TDI,"  RendView:  %.2f%% CPU  (user: %s; sys: %s)\n",
			rv_cpu,tmp,ptu_self.stime.PrintElapsed());
		snprintf(tmp,48,"%s",ptu_chld.utime.PrintElapsed());
		Verbose(TDI,"  Jobs:     %.2f%% CPU  (user: %s; sys: %s)\n",
			ch_cpu,tmp,ptu_chld.stime.PrintElapsed());
		snprintf(tmp,48,"%s",(ptu_chld.utime+ptu_self.utime).PrintElapsed());
		Verbose(TDI,"  Together: %.2f%% CPU  (user: %s; sys: %s)\n",
			ch_cpu+rv_cpu,tmp,(ptu_chld.stime+ptu_self.stime).PrintElapsed());
	}
	
	#warning FIXME: more info to come
	
	_ActuallyQuit(status);
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
	ReSched();
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
			Error("He... you will give me at least 10 seconds, will you?!\n");
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
		Verbose(TDI,"[disabled]\n");
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
	
	// Active tas source (TST_Active) will call tsnotify(AActive). 
	if(GetTaskSourceType()==TST_Passive)
	{
		// Do start exchange with task source: 
		int rv=_CheckStartExchange();
		if(rv==1)
		{
			Warning("Erm... Nothing will be done. I'm bored.\n");
			_ActuallyQuit(0);
			return;
		}
	}
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
		assert((schedule_quit || schedule_quit_after) && 
			   tasklist_todo.is_empty());
		
		ReSched();
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
			bool just_quitting = tasklist_todo.is_empty() && 
				(schedule_quit || schedule_quit_after);
			if(!just_quitting)
			{  schedule_quit=2;  }
			ReSched();
			return;
		}
	}
	
	// See if we want to start more jobs: 
	_CheckStartTasks();
	
	if(!scheduled_for_start)  // _CheckStartTasks() did not schedule a start 
	{
		// Okay, quit handling: 
		if(schedule_quit_after && tasklist_todo.is_empty())
		{
			if(schedule_quit<schedule_quit_after)
			{  schedule_quit=schedule_quit_after;  }
			ReSched();
		}
	}
}


// Check if we can start tasks and schedule that if we can: 
void TaskManager::_CheckStartTasks()
{
	// See if we're allowed to start more tasks: 
	if(dont_start_more_tasks)  return;
	
	// See if there's already a task scheduled for start: 
	if(scheduled_for_start)  return;
	
	CompleteTask *startme=interface->GetTaskToStart(&tasklist_todo,schedule_quit);
	
	if(!startme)  return;
	
	// Okay, actually schedule task for starting: 
	_DoScheduleForStart(startme);
}

inline void TaskManager::_DoScheduleForStart(CompleteTask *startme)
{
	assert(!scheduled_for_start);
	scheduled_for_start=startme;
	_DoCheckLoad();
	if(load_permits_starting)
	{  ReSched();  }  // start schedule timer
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
	
	int must_connect=-1;
	if(dont_start_more_tasks || schedule_quit)
	{
		// We will immediately contact the task source and put back 
		// the tasks that need to be put back...
		must_connect=_TS_CanDo_DoneTask() ? 1 : 0;
	}
	if(must_connect<0 && !schedule_quit_after)
	{
		int n_todo=0;
		for(CompleteTask *i=tasklist_todo.first(); i; i=i->next)
		{
			assert(i->state!=CompleteTask::TaskDone);
			++n_todo;
			// currently getting processed if(i->d.any())
		}
		if(n_todo<interface->Get_todo_thresh_low())  // NOT <= 
		{  must_connect=1;  }
	}
	if(must_connect<0)
	{  must_connect=0;  }
	
	if(!must_connect && schedule_quit_after && 
	   tasklist_todo.is_empty() && !tasklist_done.is_empty())
	{  must_connect=1;  }
	
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
	     if(!tasklist_done.is_empty())  can_done=true;
	else if(dont_start_more_tasks || schedule_quit)
	{
		for(CompleteTask *i=tasklist_todo.first(); i; i=i->next)
		{
			assert(i->state!=CompleteTask::TaskDone);
			if( (dont_start_more_tasks && !i->d.any()) || 
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
	     if(dont_start_more_tasks)  can_get=false;
	else if(schedule_quit || schedule_quit_after)  can_get=false;
	else if(ts_done_all_first && can_done)  can_get=false;
	else if(tasklist_todo.count()>=interface->Get_todo_thresh_high())
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
			CompleteTask *ctsk=tasklist_done.popfirst();
			if(!ctsk)
			{
				ctsk=special_done;
				// We may not dequeue and call TaskDone() if the task 
				// is currently processed. This should not happen, though. 
				assert(!ctsk->d.any());
				// However, the task could be scheduled for start: 
				if(scheduled_for_start==ctsk)
				{  _KillScheduledForStart();  }
				
				#if TEST_TASK_LIST_MEMBERSHIP
				assert(tasklist_todo.find(ctsk));
				#endif
				
				tasklist_todo.dequeue(ctsk);
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
	{  ReSched();  }
}


void TaskManager::_PrintTaskExecStatus(TaskExecutionStatus *tes)
{
	VerboseSpecial("      Status: %s",tes->StatusString());
	if(tes->status==TTR_Unset)  return;
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
		_PrintTaskExecStatus(&ctsk->rtes);
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
		_PrintTaskExecStatus(&ctsk->ftes);
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
		"or relative time (\"now + {DD | [[HH:]MM]:SS\") or \"none\" to "
		"switch off",&exec_timeout_spec);
	
	AddParam("load-max","do not start jobs if the system load multiplied "
		"with 100 is >= this value; instead wait unitl it is lower than "
		"load-min",&load_high_thresh);
	AddParam("load-min","resume to start jobs if the system load (multiplied "
		"with 100) is < than this value; 0 turns off load check feature",
		&load_low_thresh);
	AddParam("load-poll-msec","load value poll delay",&load_poll_msec);
	
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
		"  dont_start_more_tasks:   %d\n"
		"  caught_sigint:           %d\n"
		"  abort_on_signal:         %d\n"
		"  schedule_quit:           %d\n"
		"  schedule_quit_after:     %d\n"
		"  told_interface_to_quit:  %d\n"
		"  kill_tasks_and_quit_now: %d\n"
		"  sched_kill_tasks:        %d\n"
		"  connected_to_tsource:    %d\n"
		"  ts_done_all_first:       %d\n"
		"  load_permits_starting:   %d\n"
		"  exec_stopped:            %d\n"
		"  last_pend_done_frame_no: %d\n"
		"  tsgod_next_action:       %d\n"
		"  nth_call_to_get_task:    %d\n"
		"  resched (tid0):          %s\n"
		"  tid_ts_cwait:            %ld\n"
		"  tid_load_poll:           %ld\n"
		,scheduled_for_start ? 
		  (scheduled_for_start->state==CompleteTask::ToBeRendered ? 
		    "render" : "filter") : "none",
		jobs_failed_in_sequence,
		dont_start_more_tasks ? 1 : 0,
		caught_sigint,
		abort_on_signal ? 1 : 0,
		schedule_quit,
		schedule_quit_after,
		told_interface_to_quit ? 1 : 0,
		kill_tasks_and_quit_now ? 1 : 0,
		sched_kill_tasks,
		connected_to_tsource ? 1 : 0,
		ts_done_all_first ? 1 : 0,
		load_permits_starting ? 1 : 0,
		exec_stopped ? 1 : 0,
		last_pend_done_frame_no,
		tsgod_next_action,
		nth_call_to_get_task,
		TimerInterval(tid0)<0 ? "no" : "yes",
		TimerInterval(tid_ts_cwait),
		TimerInterval(tid_load_poll));
		
	
	fprintf(stderr,"  Tasks todo:");
	for(CompleteTask *i=tasklist_todo.first(); i; i=i->next)
	{
		char st[3]={'-','-','\0'};
		if(i->rt)
		{  st[0] = (i->state==CompleteTask::ToBeRendered ? 'r' : 'R');  }
		if(i->ft)
		{  st[1] = (i->state==CompleteTask::ToBeFiltered ? 'f' : 'F');  }
		fprintf(stderr," [%d:%s]",i->frame_no,st);
	}
	fprintf(stderr,"\n");
	
	fprintf(stderr,"  Tasks done:");
	for(CompleteTask *i=tasklist_done.first(); i; i=i->next)
	{
		fprintf(stderr," [%d:%c%c]",i->frame_no,
			_ProcessedTask(i) ? 'p' : '-',
			(i->state==CompleteTask::ToBeRendered ? 'r' : 
			  i->state==CompleteTask::ToBeFiltered ? 'f' : 
			   i->state==CompleteTask::TaskDone ? 'd' : '?'));
	}
	fprintf(stderr,"\n");
}


TaskManager::TaskManager(ComponentDataBase *cdb,int *failflag) :
	FDBase(failflag),
	TimeoutBase(failflag),
	TaskSourceConsumer(failflag),
	par::ParameterConsumer_Overloaded(cdb->parmanager(),failflag),
	exec_timeout_spec(failflag),
	starttime(),
	tasklist_todo(failflag),tasklist_done(failflag),
	tsource_name(failflag),tdinterface_name(failflag),opmode_name(failflag)
{
	component_db=cdb;
	component_db->_SetTaskManager(this);
	
	interface=NULL;
	
	// Initial values: 
	jobs_failed_in_sequence=0;
	max_failed_in_sequence=3;   // 0 -> switch off `failed in sequence�-feature
	
	dev_null_fd=-1;
	
	scheduled_for_start=NULL;
	dont_start_more_tasks=0;
	caught_sigint=0;
	abort_on_signal=0;
	schedule_quit=0;
	schedule_quit_after=0;
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
	if(!timeout_id)  ++failed;
	
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
	
	//if(kill_tasks_and_quit_now)
	{
		if(!tasklist_todo.is_empty())
		{  Warning("OOPS: Tasks left in todo queue.\n");  }
		while(!tasklist_todo.is_empty())
		{  delete tasklist_todo.popfirst();  }
		if(!tasklist_done.is_empty())
		{  Warning("OOPS: Tasks left in done queue.\n");  }
		while(!tasklist_done.is_empty())
		{  delete tasklist_done.popfirst();  }
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
	
	assert(tasklist_todo.is_empty());
	assert(tasklist_done.is_empty());
	
	assert(!connected_to_tsource);
	assert(pending_action==ANone);
	
	assert(!tasksource());
	assert(!interface);
	
	if(last_proc_frames)  last_proc_frames=(int*)LFree(last_proc_frames);
}
