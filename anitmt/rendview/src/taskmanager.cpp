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


#define UnsetNegMagic  (-29649)

// NOTE: 
// TaskManager contains a highly non-trivial state machine. 
// I expect that there are some bugs in it. 
// Lots of circumstances should be checked: (TODO, FIXME)
//   (e.g. what if schedule_quit and tid_ts_cwait is active, etc.)

/******************************************************************************/
// NOTE...
// dont_start_more_tasks -> no more tasks will be started. 
// schedule_quit -> may start more tasks but do not start NEW tasks. 
//     set to 2 if exit code shall be non-zero (i.e. 1)
// sched_kill_tasks -> send TERM and schedule KILL on all tasks in _schedule()
//     Value 1/2 for user interrupt / server error
// kill_tasks_and_quit_now -> necessary if connection to task source failed; 
//     always also set sched_kill_tasks. 
// schedule_quit_after: lile schedule_quit but allows to process all 
//     tasks in the queue before quitting. 
/******************************************************************************/


// Returns 1 if the passed task is being processed or was processed 
// by us: 
inline bool TaskManager::_ProcessedTask(const CompleteTask *ctsk)
{
	return(ctsk->td || 
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
	if(ctsk->state==CompleteTask::ToBeRendered && ctsk->td)
	{  if(ctsk->td->GetFactory()->DType()==DTRender)  return(true);  }
	return(false);
}

bool TaskManager::IsAFilteredTask(const CompleteTask *ctsk)
{
	if(!ctsk->ft)  return(false);
	if(ctsk->state==CompleteTask::TaskDone)
	{  return(true);  }
	if(ctsk->state==CompleteTask::ToBeFiltered && ctsk->td)
	{  if(ctsk->td->GetFactory()->DType()==DTFilter)  return(true);  }
	return(false);
}

inline int cmpMAX(int a,int b)
{  return((a>b) ? a : b);  }


void TaskManager::HandleSuccessfulJob(CompleteTask *ctsk)
{
	// Great, everything went fine. 
	// Reset this counter: 
	jobs_failed_in_sequence=0;
	
	if(ctsk->state==CompleteTask::TaskDone)
	{
		// Good, task is done completely. 
		tasklist_todo.dequeue(ctsk);
		tasklist_done.append(ctsk);
		// See if we have to connect to the task source and schedule 
		// that if needed: 
		_CheckStartExchange();
	}
}


// running_jobs: number of still running jobs; ONLY FOR emergency error handling. 
void TaskManager::HandleFailedTask(CompleteTask *ctsk,int running_jobs)
{
	// Error info already stored. 
	
	tasklist_todo.dequeue(ctsk);
	tasklist_done.append(ctsk);
	
	if(kill_tasks_and_quit_now)
	{
		// No, we only kill 'em and quit. Nothing else. 
		if(!running_jobs)
		{
			Warning("No more running jobs, QUITTING NOW.\n");
			fdmanager()->Quit(2);
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
		dont_start_more_tasks=1;
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


int TaskManager::tsnotify(TSNotifyInfo *ni)
{
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
					dont_start_more_tasks=1;
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
				
				assert(ni->ctsk);   // otherwise internal error
				
				// Put task into queue (at the END of the queue): 
				tasklist_todo.append(ni->ctsk);
				
				// NOTE: ni->ctsk->state = TaskDone here. Must set up 
				//       proper value now and set up TaskParams: 
				interface->DealWithNewTask(ni->ctsk);
				// This called HandleFailedTask() on error. 
				
				// Great. Task queued; now see if we want to start a job 
				// and go on talking to task source: 
				_CheckStartTasks();
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
					// Okay, first we report `done´ for all done tasks. 
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
		default:  assert(0);  break;
	}
	
	return(0);
}


int TaskManager::signotify(const SigInfo *si)
{
	int do_sched_quit=0;
	int do_kill_tasks=0;
	
	if(si->info.si_signo==SIGINT || 
	   si->info.si_signo==SIGTERM)
	{
		if(abort_on_signal)
		{
			fprintf(stderr,"*** Caught fatal SIGINT. Aborting. ***\n");
			abort();
		}
		
		if(si->info.si_signo==SIGINT)
		{
			if(!caught_sigint)
			{
				// This is the first SIGINT. 
				++caught_sigint;
				
				Warning("Caught SIGINT. No more tasks will be started. "
					"(Next ^C is more brutal.)\n");
				
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
				
				Warning("Caught second SIGINT. Killing tasks. "
					"(Next ^C will abort %s.)\n",prg_name);
				
				do_kill_tasks=1;
				
				abort_on_signal=1;
			}
		}
		else if(si->info.si_signo==SIGTERM)
		{
			Warning("Caught SIGTERM. Killing tasks. "
				"(Next signal will abort %s.)\n",prg_name);
			
			do_sched_quit=1;
			do_kill_tasks=1;
			
			abort_on_signal=1;
		}
	}
	else if(si->info.si_signo==SIGTSTP || 
	        si->info.si_signo==SIGCONT )
	{
		// Terminal stop and cont. Hmm... check state. 
		if( ( exec_stopped && si->info.si_signo==SIGCONT) || 
		    (!exec_stopped && si->info.si_signo==SIGTSTP) )
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
			si->info.si_signo==SIGCONT ? "CONT" : "TSTP",
			exec_stopped ? "stopped" : "running");  }
	}
	
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
		dont_start_more_tasks=1;
		_KillScheduledForStart();   // No, we won't start new processes now. 
		
		schedule_quit=2;  // 2 -> exit(1);
		sched_kill_tasks=1;   // user interrupt
	}
	if(do_sched_quit || do_kill_tasks)
	{  ReSched();  }  // -> quit now if nothing to do 
	
	return(0);
}


void TaskManager::_schedule(TimerInfo *ti)
{
	assert(ti->tid==tid0);
	
	// (This MUST be done here, NOT at the end of the function so 
	// that other functions can re-enable the timer.) 
	UpdateTimer(tid0,-1,0);  // First, stop timer. 
	
	// Okay, let's see what we have to do...
	if(!tasksource())
	{
		// We're in the 0msec timer installed at the constuctor to get 
		// things running: 
		int rv=_StartProcessing();
		if(rv)
		{  fdmanager()->Quit((rv==1) ? 0 : 1);  }
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
		
		Verbose("Sent SIGTERM to %d jobs.%s",nkilled,
			kill_tasks_and_quit_now ? "" : "\n");
		
		// Okay, did that. 
		sched_kill_tasks=0;
		
		if(kill_tasks_and_quit_now)
		{
			if(nkilled==0)
			{
				Verbose(" QUITTING\n");
				fdmanager()->Quit(2);  // QUIT NOW. 
				return;
			}
			else
			{  Verbose(" Waiting for jobs to quit.\n");  }
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
		
		if(!interface->AreThereJobsRunning() && 
		   tasklist_todo.is_empty() && 
		   tasklist_done.is_empty() && 
		   !connected_to_tsource && 
		   pending_action==ANone )
		{
			// Okay, we may actually quit. 
			//fprintf(stderr,"<<%d,%d>>\n",schedule_quit,schedule_quit_after);
			_DoQuit(cmpMAX(schedule_quit,schedule_quit_after)-1);
			return;
		}
		
		// We will start to put back all non-processed and 
		// all non-done tasks to source NOW: (If needed only, of couse.)
		#warning these may cause trouble. check that. 
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


static int _int_cmp(const void *a,const void *b)
{
	return(*(int*)a - *(int*)b);
}

// Simply call fdmanager()->Quit(status) and write 
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
	Verbose("  exiting at (local): %s\n",endtime.PrintTime(1,1));
	Verbose("  elapsed time: %s\n",elapsed.PrintElapsed());
	
	int loadval=_GetLoadValue();
	Verbose("  load control: ");
	if(load_low_thresh<=0)
	{  Verbose(" [disabled]\n");  }
	else
	{
		char max_tmp[32];
		if(max_load_measured<0)  *max_tmp='\0';
		else  snprintf(max_tmp,32,"; max: %.2f",double(max_load_measured)/100.0);
		Verbose("%d times; curr: %.2f%s  (%.2f/%.2f)\n",
			load_control_stop_counter,double(loadval)/100.0,max_tmp,
			double(load_low_thresh)/100.0,double(load_high_thresh)/100.0);
	}
	
	if(lpf_hist_size)
	{
		Verbose("  last successfully done frames:");
		qsort(last_proc_frames,lpf_hist_size,sizeof(int),&_int_cmp);
		int nw=0;
		for(int i=0; i<lpf_hist_size; i++)
		{
			int l=last_proc_frames[i];
			if(l<0)  continue;
			Verbose(" %d",l);  ++nw;
		}
		Verbose("%s\n",nw ? "" : " [none]");
	}
	
	double rv_cpu=100.0*(ptu_self.stime.GetD(HTime::seconds)+
		ptu_self.utime.GetD(HTime::seconds)) / elapsed.GetD(HTime::seconds);
	double ch_cpu=100.0*(ptu_chld.stime.GetD(HTime::seconds)+
		ptu_chld.utime.GetD(HTime::seconds)) / elapsed.GetD(HTime::seconds);
	
	char tmp[48];
	snprintf(tmp,48,"%s",ptu_self.utime.PrintElapsed());
	Verbose("  RendView:  %.2f%% CPU  (user: %s; sys: %s)\n",
		rv_cpu,tmp,ptu_self.stime.PrintElapsed());
	snprintf(tmp,48,"%s",ptu_chld.utime.PrintElapsed());
	Verbose("  Jobs:     %.2f%% CPU  (user: %s; sys: %s)\n",
		ch_cpu,tmp,ptu_chld.stime.PrintElapsed());
	snprintf(tmp,48,"%s",(ptu_chld.utime+ptu_self.utime).PrintElapsed());
	Verbose("  Together: %.2f%% CPU  (user: %s; sys: %s)\n",
		ch_cpu+rv_cpu,tmp,(ptu_chld.stime+ptu_self.stime).PrintElapsed());
	
	#warning FIXME: more info to come
	
	fdmanager()->Quit(status);
}



// Called via timernotify() after everything has been set up and 
// as the first action in the main loop. Used to get the TaskSource 
// and begin actually doing something...
// Return value: 0 -> OK; -1 -> error; 1 -> simply quit
int TaskManager::_StartProcessing()
{
	assert(tasksource()==NULL);  // yes, must be NULL here. 
	assert(!interface);
	
	const char *tdif_name="local";
	#warning fixme: allow for other interfaces: 
	const char *ts_name="local";
	#warning fixme: allow for other task sources
	//ts_name="LDR";
	
	Verbose("Choosing: %s task source; %s task driver (interface)\n",
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
	
	// Set some important vars: 
	connected_to_tsource=0;
	pending_action=ANone;
	UseTaskSource(ts);
	
	// Write out useful verbose information: 
	
	interface->WriteProcessingInfo(0,
		(GetTaskSourceType()==TST_Active) ? "waiting for" : "beginning to");
	
	char tmp[24];
	if(max_failed_in_sequence)  snprintf(tmp,24,"%d",max_failed_in_sequence);
	else  strcpy(tmp,"OFF");
	Verbose("  max-failed-in-seq=%s\n",tmp);
	
	// Get load val for first time; see if it is >0: 
	int loadval=_GetLoadValue();
	
	Verbose("  load control: ");
	if(load_low_thresh<=0)
	{
		Verbose("[disabled]\n");
		assert(load_poll_msec<0);
	}
	else
	{
		Verbose("min: %.2f; max: %.2f; poll: %ld msec; curr: %.2f\n",
			double(load_low_thresh)/100.0,double(load_high_thresh)/100.0,
			load_poll_msec,double(loadval)/100.0);
		assert(load_high_thresh>=load_low_thresh);
		assert(load_poll_msec>0);  // we want it LARGER THAN 0
	}
	
	ProcessManager::ProcTimeUsage ptu;
	ProcessManager::manager->GetTimeUsage(-1,&ptu);
	starttime=ptu.starttime;
	Verbose("  starting at (local): %s\n",starttime.PrintTime(1,1));
	
	if(GetTaskSourceType()==TST_Passive)
	{
		// Do start things up (yes, REALLY now): 
		int rv=_CheckStartExchange();
		if(rv==1)
		{
			Error("Erm... Nothing will be done. I'm bored.\n");
			return(1);
		}
	}
	
	return(0);
}


// Called by TaskDriverInterface becuase a TaskDriver or LDR client 
// unregistered 
// Only there to check if we want to start new tasks. 
void TaskManager::CheckStartNewJobs()
{
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
	// immediately (i.e. on the stack now). 
	
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
			// currently getting processed if(i->td)
		}
		if(n_todo<interface->Get_todo_thresh_low())  // NOT <= 
		{  must_connect=1;  }
	}
	if(must_connect<0)
	{  must_connect=0;  }
	
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
			if( (dont_start_more_tasks && !i->td) || 
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
				assert(!ctsk->td);
				// However, the task could be scheduled for start: 
				if(scheduled_for_start==ctsk)
				{  _KillScheduledForStart();  }
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
	Verbose("      Started: %s\n",tes->starttime.PrintTime(1,1));
	Verbose("      Done:    %s\n",tes->endtime.PrintTime(1,1));
	HTime duration=tes->endtime-tes->starttime;
	Verbose("      Elapsed: %s  (%.2f%% CPU)\n",
		duration.PrintElapsed(),
		100.0*(tes->utime.GetD(HTime::seconds)+tes->stime.GetD(HTime::seconds))/
			duration.GetD(HTime::seconds));
	char tmp[48];
	snprintf(tmp,48,"%s",tes->utime.PrintElapsed());
	Verbose("      Time in mode: user: %s; system: %s\n",
		tmp,tes->stime.PrintElapsed());
}

// Special function used by _PrintDoneInfo(): 
void TaskManager::_DoPrintTaskExecuted(TaskParams *tp,const char *binpath,
	bool not_processed)
{
	if(not_processed)
	{  VerboseSpecial("    Executed: [not processed]");  return;  }
	char tmpA[32];
	char tmpB[32];
	if(tp)
	{
		if(tp->niceval==TaskParams::NoNice)  strcpy(tmpA,"(none)");
		else  snprintf(tmpA,32,"%d",tp->niceval);
		if(tp->timeout<0)  strcpy(tmpB,"(none)");
		else  snprintf(tmpB,32,"%ld sec",(tp->timeout+500)/1000);
	}
	else
	{  strcpy(tmpA,"??");  strcpy(tmpB,"??");  }
	Verbose("    Executed: %s (nice value: %s; timeout: %s; tty: %s)\n",
		binpath,tmpA,tmpB,tp->call_setsid ? "no" : "yes");
}

void TaskManager::_PrintDoneInfo(CompleteTask *ctsk)
{
	VerboseSpecial("Reporting task [frame %d] as done (%s processed).",
		ctsk->frame_no,
		(ctsk->state==CompleteTask::TaskDone) ? "completely" : 
			(_ProcessedTask(ctsk) ? "partly" : "not"));
	Verbose("  Task state: %s\n",CompleteTask::StateString(ctsk->state));
	
	if(ctsk->rt)
	{
		Verbose("  Render task: %s (%s driver)\n",
			ctsk->rt->rdesc->name.str(),ctsk->rt->rdesc->dfactory->DriverName());
		Verbose("    Input file:  hd path: %s\n",
			ctsk->rt->infile ? ctsk->rt->infile->HDPath().str() : NULL);
		Verbose("    Output file: hd path: %s; size %dx%d; format: %s\n",
			ctsk->rt->outfile ? ctsk->rt->outfile->HDPath().str() : NULL,
			ctsk->rt->width,ctsk->rt->height,
			ctsk->rt->oformat ? ctsk->rt->oformat->name : NULL);
		_DoPrintTaskExecuted(ctsk->rtp,ctsk->rt->rdesc->binpath.str(),
			!IsARenderedTask(ctsk));
		_PrintTaskExecStatus(&ctsk->rtes);
	}
	else
	{  Verbose("  Render task: [none]\n");  }
	
	if(ctsk->ft)
	{
		Verbose("  Filter task: %s (%s driver)\n",NULL,NULL);
		Verbose("** Filter task info not yet supported. [FIXME!] **\n");
		
		_DoPrintTaskExecuted(ctsk->ftp,ctsk->ft->fdesc->binpath.str(),
			!IsAFilteredTask(ctsk));
		_PrintTaskExecStatus(&ctsk->ftes);
	}
	else
	{  Verbose("  Filter task: [none]\n");  }
}


int TaskManager::CheckParams()
{
	int failed=0;
	
	if(max_failed_in_sequence<0)
	{  max_failed_in_sequence=0;  }
	if(!max_failed_in_sequence)
	{  Verbose("Note that max-failed-in-seq feature is disabled.\n");  }
	
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
	
	// Always open /dev/null: 
	dev_null_fd=open("/dev/null",O_RDONLY);
	// We must PollFD() the fd so that it gets properly closed on 
	// execution of other processes. 
	if(dev_null_fd<0 || PollFD(dev_null_fd))
	{  Error("Failed to open /dev/null: %s\n",
		(dev_null_fd<0) ? strerror(errno) : "alloc failure");  }
	
	return(failed ? 1 : 0);
}


// Return value: 0 -> OK; 1 -> error
int TaskManager::_SetUpParams()
{
	if(SetSection(NULL))
	{  return(1);  }
	
	AddParam("max-failed-in-seq|mfis",
		"max number of jobs to fail in sequence until giving up "
		"(0 to disable [NOT recommended])",&max_failed_in_sequence);
	
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

TaskManager::TaskManager(ComponentDataBase *cdb,int *failflag) :
	FDBase(failflag),
	TaskSourceConsumer(failflag),
	par::ParameterConsumer_Overloaded(cdb->parmanager(),failflag),
	starttime(),
	tasklist_todo(failflag),tasklist_done(failflag)
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
	sched_kill_tasks=0;
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
	
	if(_SetUpParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskMan");  }
}


TaskManager::~TaskManager()
{
	CloseFD(dev_null_fd);
	
	// Make sure there are no jobs and no tasks left: 
	assert(!scheduled_for_start);
	if(!kill_tasks_and_quit_now)
	{
		assert(tasklist_todo.is_empty());
		assert(tasklist_done.is_empty());
	}
	
	assert(!connected_to_tsource);
	assert(pending_action==ANone);
	
	TaskSource *ts=tasksource();
	UseTaskSource(NULL);
	if(ts)
	{  delete ts;  }
	
	if(interface)
	{  delete interface;  interface=NULL;  }
	
	if(last_proc_frames)  last_proc_frames=(int*)LFree(last_proc_frames);
}
