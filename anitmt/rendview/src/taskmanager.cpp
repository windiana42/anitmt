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

#include <assert.h>

#define NoNiceValSpec (-32767)


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
// kill_task_and_quit_now -> 
//    1 -> send TERM and schedule KILL on all tasks in _schedule()
//    2 -> did action done by value `1'. 
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


inline int cmpMAX(int a,int b)
{  return((a>b) ? a : b);  }


// Called by TaskDriver when ever estat/esdetail changed. 
void TaskManager::StateChanged(TaskDriver *)
{
	// This is currently unused. IAmDone() gets called if the state 
	// changed to ESDone. 
}

// Called by TaskDriver when he is done and wants to get deleted: 
void TaskManager::IAmDone(TaskDriver *td)
{
	if(!td)  return;
	
	assert(td->estat==TaskDriver::ESDone);
	
	CompleteTask *ctsk=td->pinfo.ctsk;
	if(!ctsk)
	{
		// Now, if pinfo.ctsk is NULL, then we did not yet successfully start 
		// the task driver. 
		// In this case, simply delete it again. 
		// (We may NOT decrease running_jobs as this job was not RUNING.) 
		// (Also, ctsk->td is NULL here.)
		assert(ctsk->td==NULL);  // YES!!
		td->DeleteMe();  // will unregister
		return;
	}
	
	// See if data is consistent: 
	assert(ctsk->td==td);
	
	// Okay, the job is done, so the CompleteTask is no longer processed: 
	ctsk->td=NULL;
	// Dealt with it: 
	td->pinfo.ctsk=NULL;
	td->DeleteMe();  // will unregister
	
	// Also, fewer processes are running:
	--running_jobs[td->f->DType()];
	
	// See what happened to the task...
	if(td->esdetail==TaskDriver::EDSuccess)
	{
		// Great, everything went fine. 
		// Reset this counter: 
		jobs_failed_in_sequence=0;
		
		// Decide what to do next: 
		switch(ctsk->state)
		{
			case CompleteTask::ToBeRendered:
				if(ctsk->ft)  ctsk->state=CompleteTask::ToBeFiltered;
				else  ctsk->state=CompleteTask::TaskDone;
				break;
			case CompleteTask::ToBeFiltered:
				ctsk->state=CompleteTask::TaskDone;
				break;
			default:  assert(0);  break;
		}
		if(ctsk->state==CompleteTask::TaskDone)
		{
			// Good, task is done completely. 
			tasklist_todo.dequeue(ctsk);
			tasklist_done.append(ctsk);
			// See if we have to connect to the task source and schedule 
			// that if needed: 
			_CheckStartExchange();  // no action if kill_task_and_quit_now
		}
	}
	
	if(kill_task_and_quit_now)
	{
		// No, we only kill 'em and quit. Nothing else. 
		int nrunning=0;
		for(int i=0; i<_DTLast; i++)
		{  nrunning+=running_jobs[i];  }
		if(!nrunning)
		{
			Verbose("No more running jobs, QUITTING NOW.\n");
			fdmanager()->Quit(2);
		}
		return;  // necessary! ...
		// ... because it will be normal that the tasks have 
		//     edsetail==TaskDriver::EDATerm (thus not EDSuccess) 
		//     here. And we won't muck with errors now. 
	}
	
	if(td->esdetail!=TaskDriver::EDSuccess)
	{
		// Um, yeah. -- Okay, let's deal with failed job. 
		// Must save failure code in CompleteTask, mark it as done 
		// and feed it into the TaskSource next time. 
		// Should stop if more than failed_thresh tasks failed. 
		_HandleFailedJob(ctsk);
		// See if we have to connect to the task source and schedule 
		// that if needed: 
		_CheckStartExchange();  // no action if kill_task_and_quit_now
	}
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
					kill_task_and_quit_now=1;
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
				
				// NOTE: ni->ctsk->state = TaskDone here. Must set up 
				//       proper value now and set up TaskParams: 
				int rv=_DealWithNewTask(ni->ctsk);
				if(rv)
				{
					// Grmbl...
					// Something is wrong with the task or allocatio 
					// failure. 
					// (Note that task is already queued.)
					#warning DEAL WITH THIS CASE!!!!
					Error("Please fix me.\n");
					assert(0);
				}
				
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
	
	if(do_sched_quit)
	{
		// Make sure we quit then...
		if(!schedule_quit)
		{  schedule_quit=1;  }  // 1 -> exit(0);
	}
	if(do_kill_tasks)
	{
		dont_start_more_tasks=1;
		if(scheduled_for_start)
		{
			// No, we won't start new processes now. 
			scheduled_for_start=NULL;
		}
		
		schedule_quit=2;  // 2 -> exit(1);
		
		#warning MISSING: kill tasks
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
	
	// High priority; if we do that then we do nothing else!
	if(kill_task_and_quit_now)
	{
		if(kill_task_and_quit_now==1)
		{
			// Schedule TERM & KILL for all tasks. 
			Verbose("Scheduling TERM & KILL on all jobs.");
			int nkilled=0;
			for(TaskDriver *i=joblist.first(); i; i=joblist.next(i))
			{
				if(i->pinfo.pid<0)  continue;
				errno=0;
				int rv=ProcessBase::TermProcess(i->pinfo.pid);
				if(rv)
				{
					Warning("Failed to SIGTERM kill %s (PID %d): %s\n",
						i->pinfo.args.first()->str(),i->pinfo.pid,
						(rv==-3) ? "process not known to process manager" : 
							strerror(errno));
				}
				else
				{  ++nkilled;  }
			}
			
			Verbose("Sent SIGTERM to %d jobs.",nkilled);
			if(nkilled==0)
			{
				Verbose(" QUITTING\n");
				fdmanager()->Quit(2);  // QUIT NOW. 
			}
			else
			{  Verbose(" Waiting for jobs to quit.\n");  }
			
			kill_task_and_quit_now=2;
		}
		return;   // ...we do nothing else
	}
	
	if(scheduled_for_start)
	{
		// We have to start a job for a task. Okay, let's do it: 
		int rv=_LaunchJobForTask(scheduled_for_start);
		if(rv)
		{
			// Launching the task failed. 
			
			// FIXME: 
			#warning ERROR CODE...!
			// So, the task must be queued in the done list with proper 
			// error info set: 
			_HandleFailedJob(scheduled_for_start);
		}
		else
		{
			// Successful start. 
			//  running_jobs[dtype] already incremened 
			//  by _LaunchJobForTask(). 
		}
		
		scheduled_for_start=NULL;
		
		// See if we want to start more task(s): 
		#warning do we want to that on error?
		_CheckStartTasks();
	}
	
	if(schedule_quit || schedule_quit_after)
	{
		// Quit is scheduled. So, we quit if everything is cleaned up: 
		
		//fprintf(stderr,"Counts = %d,%d,%d; connected=%d, pending_action=%d\n",
		//	joblist.count(),tasklist_todo.count(),tasklist_done.count(),
		//	connected_to_tsource,pending_action);
		
		if(joblist.is_empty() && 
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
		#warning these may cause trouble. check that. 
		if(connected_to_tsource && pending_action==ANone)
		{  _TS_GetOrDoneTask();  }
		else if(!connected_to_tsource)
		{  _CheckStartExchange();  }
	}
}


int TaskManager::timernotify(TimerInfo *ti)
{
	if(ti->tid==tid0)
	{  _schedule(ti);  }
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


// Simply call fdmanager()->Quit(status) and write 
void TaskManager::_DoQuit(int status)
{
	Verbose("Now exiting with status=%s (%d)\n",
		status ? "failure" : "success",status);
	Verbose("  [FIXME: more info to come]\n");
	HTime endtime(HTime::Curr);
	Verbose("  exiting at (local): %s\n",endtime.PrintTime(1,1));
	HTime elapsed=endtime-starttime;
	Verbose("  elapsed time: %s\n",elapsed.PrintElapsed());
	
	fdmanager()->Quit(status);
}


// Called via timernotify() after everything has been set up and 
// as the first action in the main loop. Used to get the TaskSource 
// and begin actually doing something...
// Return value: 0 -> OK; -1 -> error; 1 -> simply quit
int TaskManager::_StartProcessing()
{
	assert(tasksource()==NULL);  // yes, must be NULL here. 
	
	#warning fixme: allow for other task sources
	const char *ts_name="local";
	TaskSourceFactory *tsf=component_db->FindSourceFactoryByName(ts_name);
	
	// Let TaskSourceFactory do final init: 
	if(tsf->FinalInit())
	{
		// Failed; message(s) written. 
		return(-1);
	}
	
	TaskSource *ts = tsf ? tsf->Create() : NULL;
	if(!ts)
	{
		// `Task source not available´ means that it was not built 
		// into this program. 
		Error("Failed to set up %s task source: %s\n",ts_name,
			tsf ? "initialisation failed" : "task source not available");
		return(-1);
	}
	
	connected_to_tsource=0;
	pending_action=ANone;
	
	UseTaskSource(ts);
	Verbose("Task source (%s) set up successfully.\n",ts_name);
	
	// Adjust task limits (if not told otherwise by user): 
	if(njobs<1)
	{
		njobs=GetNumberOfCPUs();
		assert(njobs>=1);
	}
	for(int i=0; i<_DTLast; i++)
	{
		if(prm[i].maxjobs<0)  prm[i].maxjobs=njobs;
		// Convert timeout from seconds to msec: 
		prm[i].timeout=(prm[i].timeout<0) ? -1 : (prm[i].timeout*1000);
	}
	
	if(todo_thresh_low<1)  // YES: <1, NOT <0
	{  todo_thresh_low=njobs+1;  }
	
	// Adjust task queue thresholds: 
	if(todo_thresh_low<njobs+1)
	{  Warning("It is very unwise to set low task queue threshold (%d) "
		"smaller than njobs+1 (%d)\n",todo_thresh_low,njobs+1);  }
	if(todo_thresh_high+1<todo_thresh_low)
	{
		int oldval=todo_thresh_high;
		todo_thresh_high=todo_thresh_low+5;
		if(oldval>=0)  // oldval=-1 for initial value set by constructor
		{  Warning("Adjusted high task queue threshold to %d (was %d)\n",
			todo_thresh_high,oldval);  }
	}
	
	// Write out useful verbose information: 
	Verbose("Okay, beginning to work: njobs=%d (parallel tasks)\n",njobs);
	Verbose("  jtype    jmax  nice   timeout  \n");
	for(int i=0; i<_DTLast; i++)
	{
		DTPrm *p=&prm[i];
		Verbose("  %s:  %4d  ",DTypeString(TaskDriverType(i)),p->maxjobs);
		if(p->niceval==NoNiceValSpec)
		{  Verbose("  --");  }
		else
		{  Verbose("%s%3d%s",p->nice_jitter ? "" : " ",
			p->niceval,p->nice_jitter ? "±" : "");  }
		if(p->timeout<0)
		{  Verbose("        --");  }
		else
		{  long x=(p->timeout+500)/1000;
			Verbose("  %02d:%02d:%02d",x/3600,(x/60)%60,x%60);  }
		Verbose("\n");
	}
	
	Verbose("  task-thresh: low=%d, high=%d; max-failed-in-seq=",
		todo_thresh_low,todo_thresh_high);
	if(max_failed_in_sequence)  Verbose("%d\n",max_failed_in_sequence);
	else Verbose("OFF\n");
	
	starttime.SetCurr();
	Verbose("  starting at (local): %s\n",starttime.PrintTime(1,1));
	
	// Do start things up (yes, REALLY now): 
	int rv=_CheckStartExchange();
	if(rv==1)
	{
		Error("Erm... Nothing will be done. I'm bored.\n");
		return(1);
	}
	
	return(0);
}


// Actually launch a job for a task. No check if we may do that. 
int TaskManager::_LaunchJobForTask(CompleteTask *ctsk)
{
	// See which task to launch: 
	const RF_DescBase *d=NULL;
	TaskStructBase *tsb=NULL;
	TaskParams *tp=NULL;
	TaskDriverType dtype=DTNone;
	switch(ctsk->state)
	{
		case CompleteTask::ToBeRendered:
			tsb=ctsk->rt;
			tp=ctsk->rtp;
			d=ctsk->rt ? ctsk->rt->rdesc : NULL;
			dtype=DTRender;
			break;
		case CompleteTask::ToBeFiltered:
			tsb=ctsk->ft;
			tp=ctsk->ftp;
			d=ctsk->ft ? ctsk->ft->fdesc : NULL;
			dtype=DTFilter;
			break;
		default:  assert(0);  break;
	}
	
	assert(d);
	assert(d->dfactory);
	TaskDriver *td=d->dfactory->Create();
	
	if(!td)
	{
		Error("Failed to set up (%s) task driver %s for %s\n",
			DTypeString(dtype),td->f->DriverName(),d->name.str());
		return(1);
	}
	
	int rv=td->Run(tsb,tp);
	if(rv)   // Error was already written. 
	{  return(2);  }
	
	// This is that we know which TaskDriver does which CompleteTask: 
	// NOTE that this is set AFTER Run() returns success. 
	// Because now, the task counts as `running' (as we've performed 
	// the fork()). 
	td->pinfo.ctsk=ctsk;
	ctsk->td=td;
	++running_jobs[dtype];
	
	return(0);
}


// Check if we can start tasks and schedule that if we can: 
void TaskManager::_CheckStartTasks()
{
	// See if we're allowed to start more tasks: 
	if(dont_start_more_tasks)  return;
	
	// See if there's already a task scheduled for start: 
	if(scheduled_for_start)  return;
	
	int jobs_running=0;
	for(int i=0; i<_DTLast; i++)
	{  jobs_running+=running_jobs[i];  }
	if(jobs_running>=njobs)  return;
	
	bool may_start[_DTLast];
	int may_start_anything=0;
	for(int i=0; i<_DTLast; i++)
	{
		bool tmp=(running_jobs[i]<prm[i].maxjobs);
		may_start[i]=tmp;
		if(tmp)  ++may_start_anything;
	}
	if(!may_start_anything)  return;
	
	// Okay, less jobs are running than possible. 
	// We should start one if there is one. 
	CompleteTask *startme=NULL;
	for(CompleteTask *i=tasklist_todo.first(); i; i=i->next)
	{
		assert(i->state!=CompleteTask::TaskDone);
		if(i->td)  continue;  // task is currently processed 
		if( (i->state==CompleteTask::ToBeRendered && may_start[DTRender]) ||
			(i->state==CompleteTask::ToBeFiltered && may_start[DTFilter]) )
		{  startme=i;  break;  }
	}
	if(!startme)  return;
	
	// Okay, actually schedule task for starting: 
	scheduled_for_start=startme;
	ReSched();  // start schedule timer
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
	
	if(kill_task_and_quit_now)
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
		if(n_todo<todo_thresh_low)  // NOT <= 
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
	else if(tasklist_todo.count()>=todo_thresh_high)  can_get=false;
	return(can_get);
}


// Tell TaskSource about done task or request a new task. 
// Currently all that is done alternating until a limit is reached. 
void TaskManager::_TS_GetOrDoneTask()
{
	if(kill_task_and_quit_now)
	{
		Error("OOps: kill_task_and_quit_now set in _TS_GetOrDoneTask().\n");
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
				tasklist_todo.dequeue(ctsk);
			}
			assert(ctsk);  // otherwise tsgod_next_action=ADoneTask illegal
			
			// FIXME: this should include failure info: 
			Verbose("Reporting task as done (processed=%s).\n",
				(ctsk->state==CompleteTask::TaskDone) ? "completely" : 
					(_ProcessedTask(ctsk) ? "partly" : "no"));
			
			int rv=TSDoneTask(ctsk);
			assert(rv==0);  // everything else is internal error
			pending_action=ADoneTask;
			break;
		}
		default:  assert(0);  break;
	}
}


void TaskManager::_HandleFailedJob(CompleteTask *ctsk)
{
	#warning MISSING: store error info info ctsk.
	tasklist_todo.dequeue(ctsk);
	tasklist_done.append(ctsk);
	
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
}


int TaskManager::_DealWithNewTask(CompleteTask *ctsk)
{
	// First, set up state: 
	if(ctsk->rt)
	{  ctsk->state=CompleteTask::ToBeRendered;  }
	else if(ctsk->ft)
	{  ctsk->state=CompleteTask::ToBeFiltered;  }
	else 
	{
		// This is an internal error. Task source may not 
		// return a task without anything to do (i.e. with 
		// rt and ft set to NULL). 
		assert(ctsk->rt || ctsk->ft);
	}
	
	// Queue task: 
	tasklist_todo.append(ctsk);
	
	// Then, create TaskParams: 
	if(ctsk->rt)
	{
		ctsk->rtp=NEW<RenderTaskParams>();
		if(!ctsk->rtp)  return(1);
		
	}
	if(ctsk->ft)
	{
		ctsk->ftp=NEW<FilterTaskParams>();
		if(!ctsk->ftp)  return(1);
		
		Error("Cannot yet deal with filter.\n");
		abort();
	}
	
	// Set up common fields in TaskParams:
	for(int i=0; i<_DTLast; i++)
	{
		TaskParams *tp=NULL;
		switch(i)
		{
			case DTRender:  tp=ctsk->rtp;  break;
			case DTFilter:  tp=ctsk->ftp;  break;
			default:  assert(0);  break;
		}
		// tp=NULL -> not a `i´ (DTRender/DTFilter) - task. 
		if(!tp)  continue;
		
		DTPrm *p=&prm[i];
		if(p->niceval!=NoNiceValSpec)
		{
			tp->niceval=p->niceval;
			if(p->nice_jitter)
			{
				tp->niceval+=(random()%3-1);
				// Prevent nice to be <0 if the original value was >=0. 
				if(p->niceval>=0 && tp->niceval<0)
				{  tp->niceval=0;  }
			}
		}
		tp->timeout=(p->timeout<0) ? (-1) : p->timeout;
		// crdir
		// wdir
	}
	
	return(0);
}


// These are called by the constructor/destructor of TaskDriver: 
int TaskManager::RegisterTaskDriver(TaskDriver *td)
{
	if(!td)  return(0);
	
	// Check if queued: 
	if(joblist.prev(td) || td==joblist.first())  return(1);
	
	joblist.append(td);
	
	
	return(0);
}

void TaskManager::UnregisterTaskDriver(TaskDriver *td)
{
	if(!td)  return;
	// Check if queued: 
	if(!joblist.prev(td) && td!=joblist.first())  return;
	
	// Check state...
	// TaskDriver::ESNone: 
	//   Well, seems this one did not do anything. So 
	//   there should not be a problem. 
	// TaskDriver::ESDone:
	//   That was caught in IAmDone(). 
	if(td->estat!=TaskDriver::ESNone && 
	   td->estat!=TaskDriver::ESDone )
	{
		// How can it unregister when not being in state ESNone or ESDone?
		assert(0);
	}
	
	// Dequeue task: 
	joblist.dequeue(td);
	
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


int TaskManager::CheckParams()
{
	if(max_failed_in_sequence<0)
	{  max_failed_in_sequence=0;  }
	if(!max_failed_in_sequence)
	{  Verbose("Note that max-failed-in-seq feature is disabled.\n");  }
	
	return(0);
}


// Return value: 0 -> OK; 1 -> error
int TaskManager::_SetUpParams()
{
	if(SetSection(NULL))
	{  return(1);  }
	
	AddParam("njobs","number of simultanious jobs",&njobs);
	AddParam("rjobs-max","max number of simultanious render jobs",
		&prm[DTRender].maxjobs);
	AddParam("fjobs-max","max number of simultanious filter jobs",
		&prm[DTFilter].maxjobs);
	AddParam("max-failed-in-seq|mfis",
		"max number of jobs to fail in sequence until giving up "
		"(0 to disable [NOT recommended])",&max_failed_in_sequence);
	AddParam("task-thresh-low",
		"start getting new tasks from the task source if there are less "
		"than this number of non-completely processed tasks in the task "
		"queue",&todo_thresh_low);
	AddParam("task-thresh-high",
		"never store more than this number of tasks in the local task queue",
		&todo_thresh_high);
	AddParam("rnice","render job nice value",&prm[DTRender].niceval);
	AddParam("fnice","filter job nice value",&prm[DTFilter].niceval);
	AddParam("rtimeout","render job time limit (seconds; -1 for none)",
		&prm[DTRender].timeout);
	AddParam("ftimeout","filter job time limit (seconds; -1 for none)",
		&prm[DTFilter].timeout);
	AddParam("rnice-jitter","render job nice jitter (change nice value "
		"+/- 1 to prevent jobs from running completely simultaniously; "
		"no effect unless -rnice is used)",
		&prm[DTRender].nice_jitter);
	AddParam("fnice-jitter","filter job nice jitter",
		&prm[DTFilter].nice_jitter);
	
	#warning further params: delay_between_tasks, max_failed_jobs, \
		dont_fail_on_failed_jobs, launch_if_load_smaller_than, \
		brutal_on_first_sigint, ignore_sigint, signal_never_abort
	
	return(add_failed ? 1 : 0);
}

TaskManager::TaskManager(ComponentDataBase *cdb,int *failflag) :
	FDBase(failflag),
	ProcessBase(failflag),
	TaskSourceConsumer(failflag),
	par::ParameterConsumer_Overloaded(cdb->parmanager(),failflag),
	starttime(),
	joblist(failflag),
	tasklist_todo(failflag),tasklist_done(failflag)
{
	component_db=cdb;
	component_db->_SetTaskManager(this);
	
	// Initial values: 
	njobs=-1;
	
	for(int i=0; i<_DTLast; i++)
	{
		running_jobs[i]=0;
		DTPrm *p=&prm[i];
		p->maxjobs=-1;   // initial value
		p->niceval=NoNiceValSpec;
		p->nice_jitter=true;
		p->timeout=-1;
	}
	jobs_failed_in_sequence=0;
	max_failed_in_sequence=3;   // 0 -> switch off `failed in sequence´-feature
	
	scheduled_for_start=NULL;
	dont_start_more_tasks=0;
	caught_sigint=0;
	abort_on_signal=0;
	schedule_quit=0;
	schedule_quit_after=0;
	kill_task_and_quit_now=0;
	
	todo_thresh_low=-1;  // initial value
	todo_thresh_high=-1;
	
	connected_to_tsource=0;
	pending_action=ANone;
	
	tsgod_next_action=ANone;
	ts_done_all_first=0;
	nth_call_to_get_task=0;
	
	int failed=0;
	
	//if(FDBase::SetManager(1))
	//{  ++failed;  }
	
	// Install 0msec timer to start the stuff: 
	tid0=InstallTimer(0,0);
	// Install disabled timer for connect re-try: 
	tid_ts_cwait=InstallTimer(-1,0);
	if(!tid0 || !tid_ts_cwait)
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
	// Make sure there are no jobs and no tasks left: 
	assert(!scheduled_for_start);
	for(int i=0; i<_DTLast; i++)
	{  assert(!running_jobs[i]);  }
	assert(joblist.is_empty());
	if(!kill_task_and_quit_now)
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
}
