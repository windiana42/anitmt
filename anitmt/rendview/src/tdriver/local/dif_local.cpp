/*
 * dif_local.cpp
 * 
 * Local task driver interface for task manager. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "../../taskmanager.hpp"

#include "dif_local.hpp"
#include "dif_param.hpp"

#include <assert.h>


// Not inline because virtual: 
int TaskDriverInterface_Local::Get_njobs()
{
	return(p->njobs);
}


// Called when everything is done to disconnect from the clients. 
// Local interface can handle that quickly. 
void TaskDriverInterface_Local::PleaseQuit()
{
	// Can quit at any time. 
	component_db()->taskmanager()->CheckStartNewJobs(/*special=*/-1);
}


void TaskDriverInterface_Local::StopContTasks(int signo)
{
	// send signal to the tasks: 
	Warning("Sending SIG%s to all jobs... ",
		signo==SIGCONT ? "CONT" : "STOP");
	int nkilled=0;
	int nfailed=0;
	for(TaskDriver *i=joblist.first(); i; i=joblist.next(i))
	{
		if(i->pinfo.pid<0)  continue;
		if(::kill(i->pinfo.pid,signo))
		{
			// We simply ignore these errors because the task may 
			// have just exited. 
			if(!nfailed)  Warning("\n");
			if(i->pinfo.ctsk)
			{  Warning("Failed for job %s (pid %ld) [frame %d]: "
				"%s (ignored)\n",i->pinfo.args.first()->str(),
				long(i->pinfo.pid),i->pinfo.ctsk->frame_no,
				strerror(errno));  }
			else
			{  Warning("Failed for pid %ld: %s (ignored)\n",
				i->pinfo.pid,strerror(errno));  }
			++nfailed;
		}
		else ++nkilled;
	}
	if(nfailed)
	{  Warning("Sending signals: success: %d jobs; failed: %d jobs\n",
		nkilled,nfailed);  }
	else
	{  Warning("okay, %d jobs%s\n",nkilled,
		(signo==SIGCONT) ? "" : " [STOPPING]");  }
}


// Schedule SIGTERM & SIGKILL on all jobs. 
// Returns number of killed jobs. 
int TaskDriverInterface_Local::TermAllJobs(int reason)
{
	// Schedule TERM & KILL for all tasks. 
	Warning("Scheduling TERM & KILL on all jobs...\n");
	
	int nkilled=0;
	for(TaskDriver *i=joblist.first(); i; i=joblist.next(i))
	{
		if(i->pinfo.pid<0)  continue;
		int rv=i->KillProcess(reason);
		if(!rv)
		{  ++nkilled;  }
	}
	
	return(nkilled);
}


// Actually launch a job for a task. No check if we may do that. 
int TaskDriverInterface_Local::LaunchTask(CompleteTask *ctsk)
{
	TaskDriver *tmp_td=NULL;
	
	int rv=_DoLaunchTask(ctsk,&tmp_td);
	if(rv)
	{
		// Launching the task failed. 
		
		// So, the task must be queued in the done list with proper 
		// error info set: 
		_HandleFailedJob(ctsk,tmp_td);
	}
	else
	{
		// Successful start. 
		TaskDriverType dtype=tmp_td->GetFactory()->DType();
		assert(dtype!=DTNone);  // otherwise internal error
		
		++running_jobs[dtype];
	}
	
	return(rv);
}


int TaskDriverInterface_Local::_DoLaunchTask(CompleteTask *ctsk,TaskDriver **td)
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
	*td=d->dfactory->Create(this);
	
	if(!*td)
	{
		Error("Failed to set up (%s) task driver %s for %s\n",
			DTypeString(dtype),(*td)->f->DriverName(),d->name.str());
		return(1);
	}
	
	// Already set ctsk; task driver removes it again if an error occurs: 
	(*td)->pinfo.ctsk=ctsk;
	int rv=(*td)->Run(tsb,tp);
	if(rv)  // Error happened. 
	{
		// Okay, let's see: 
		// If (*td)->pinfo.ctsk is NULL, then TaskDriver::StartProcess() 
		//   was called and that called ProcessBase::StartProcess() which 
		//   failed. The error is already reported to the user and 
		//   pinfo.tes.status is set to indicate the error. 
		// If (*td)->pinfo.ctsk is NOT NULL, then TaskDriver::StartProcess() 
		//   not called and Run() (or the XYZDriver::Execute()) function 
		//   had an error before attempting to fork. In this case, we still 
		//   have to deal with the error: 
		if((*td)->pinfo.ctsk)
		{
			// Valid return codes in this case: 
			// SPSi_Open{In,Out}Failed, SPSi_IllegalParams, SPS_LMallocFailed 
			(*td)->pinfo.pid=rv;
			(*td)->_StartProcess_ErrorPart(errno);  // sets pinfo.ctsk=NULL
		}
		return(2);  // IAmDone() was called...
	}
	
	// This is that we know which TaskDriver does which CompleteTask: 
	// NOTE that this is set AFTER Run() returns success. 
	// Because now, the task counts as `running' (as we've performed 
	// the fork()). 
	(*td)->pinfo.ctsk=ctsk;
	ctsk->d.td=*td;
	
	return(0);
}


// Called by TaskDriver when ever estat/esdetail changed. 
void TaskDriverInterface_Local::StateChanged(TaskDriver *)
{
	// This is currently unused. IAmDone() gets called if the state 
	// changed to ESDone. 
}

// Called by TaskDriver when he is done and wants to get deleted: 
void TaskDriverInterface_Local::IAmDone(TaskDriver *td)
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
		td->DeleteMe();  // will unregister
		return;
	}
	
	// See if data is consistent: 
	assert(ctsk->d.td==td);
	
	// Okay, the job is done, so the CompleteTask is no longer processed: 
	ctsk->d.td=NULL;
	// Dealt with it: 
	td->pinfo.ctsk=NULL;
	td->DeleteMe();  // will unregister
	
	// Also, fewer processes are running:
	--running_jobs[td->f->DType()];
	
	// See what happened to the task...
	if(td->esdetail==TaskDriver::EDSuccess)
	{
		// Copy status info & statistics: 
		// Also, decide what to do next: 
		switch(ctsk->state)
		{
			case CompleteTask::ToBeRendered:
				ctsk->rtes=td->pinfo.tes;
				// Either waiting to be filtered or done. 
				if(ctsk->ft)  ctsk->state=CompleteTask::ToBeFiltered;
				else  ctsk->state=CompleteTask::TaskDone;
				break;
			case CompleteTask::ToBeFiltered:
				ctsk->ftes=td->pinfo.tes;
				// Task now done. 
				ctsk->state=CompleteTask::TaskDone;
				break;
			default:  assert(0);  break;
		}
		
		component_db()->taskmanager()->HandleSuccessfulJob(ctsk);
	}
	else  // !TaskDriver::EDSuccess
	{
		// Um, yeah. -- Okay, let's deal with failed job. 
		// Must save failure code in CompleteTask, mark it as done 
		// and feed it into the TaskSource next time. 
		// Should stop if more than failed_thresh tasks failed. 
		_HandleFailedJob(ctsk,td);
		// Check if we have to connect to task source done by 
		// TasmManager::HandleFailedTask() which was just called by 
		// _HandleFailedJob()
	}
}


// Called for every new task obtained from TaskSource: 
int TaskDriverInterface_Local::DealWithNewTask(CompleteTask *ctsk)
{
	int rv=_DoDealWithNewTask(ctsk);
	if(rv)
	{
		// Grmbl...
		// Something is wrong with the task or allocation 
		// failure. 
		// (Note that task is already queued at the manager.)

		// This will set internal error and then tell the 
		// Manager (calling HandleFailedTask()) to dequeue it 
		// and increase start failed counter. 
		// (It will even set ts_done_all_first if connected.) 
		_HandleFailedJob(ctsk,NULL);
	}
	return(rv);
}

int TaskDriverInterface_Local::_DoDealWithNewTask(CompleteTask *ctsk)
{
	// First, set up state (is in TaskDone here as gotten from source): 
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
	
	TaskDriverInterfaceFactory_Local::DTPrm *prm=p->prm;
	
	int dev_null_fd=component_db()->taskmanager()->DevNullFD();
	
	// Then, create TaskParams: 
	if(ctsk->rt)
	{
		ctsk->rtp=NEW<RenderTaskParams>();
		if(!ctsk->rtp)  return(1);
		
		// Setup special fields for renderer: 
		if(prm[DTRender].quiet || prm[DTRender].mute)
		{  ctsk->rtp->stdout_fd=dev_null_fd;  }
		if(prm[DTRender].quiet)
		{  ctsk->rtp->stderr_fd=dev_null_fd;  }
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
		TaskStructBase *tsb=NULL;
		switch(i)
		{
			case DTRender:  tp=ctsk->rtp;  tsb=ctsk->rt;  break;
			case DTFilter:  tp=ctsk->ftp;  tsb=ctsk->ft;  break;
			default:  assert(0);  break;
		}
		// tp=NULL -> not a `i´ (DTRender/DTFilter) - task. 
		if(!tp)  continue;
		
		TaskDriverInterfaceFactory_Local::DTPrm *p=&prm[i];
		if(p->niceval!=TaskParams::NoNice)
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
		tp->timeout=(p->timeout<=0) ? (-1) : p->timeout;
		if(tsb->timeout>0 && tp->timeout>tsb->timeout)
		{  tp->timeout=tsb->timeout;  }  // Another timeout. Take shorter one. 
		tp->call_setsid=p->call_setsid ? 1 : 0;
	}
	
	return(0);
}


void TaskDriverInterface_Local::_HandleFailedJob(CompleteTask *ctsk,TaskDriver *td)
{
	// Store error info: 
	TaskExecutionStatus *uset=NULL;
	switch(ctsk->state)
	{
		case CompleteTask::ToBeRendered:  uset=&ctsk->rtes;  break;
		case CompleteTask::ToBeFiltered:  uset=&ctsk->ftes;  break;
		default:  assert(0);  break;
	}
	if(td)
	{  *uset=td->pinfo.tes;  }
	else
	{
		// We have td=NULL. This can happen if we fail at launch time. 
		uset->status=TTR_ExecFailed;
		uset->signal=JK_InternalError;
	}
	
	component_db()->taskmanager()->HandleFailedTask(ctsk,RunningJobs());
}


// Decide on task to start and return it. 
// Return NULL if there is no task to start. 
CompleteTask *TaskDriverInterface_Local::GetTaskToStart(
	LinkedList<CompleteTask> *tasklist_todo,int schedule_quit)
{
	if(RunningJobs()>=p->njobs)  return(NULL);
	
	TaskDriverInterfaceFactory_Local::DTPrm *prm=p->prm;
	
	bool may_start[_DTLast];
	int may_start_anything=0;
	for(int i=0; i<_DTLast; i++)
	{
		bool tmp=(running_jobs[i]<prm[i].maxjobs);
		may_start[i]=tmp;
		if(tmp)  ++may_start_anything;
	}
	if(!may_start_anything)  return(NULL);
	
	// Okay, less jobs are running than possible. 
	// We should start one if there is one. 
	CompleteTask *startme=NULL;
	for(CompleteTask *i=tasklist_todo->first(); i; i=i->next)
	{
		assert(i->state!=CompleteTask::TaskDone);
		if(i->d.td)  continue;  // task is currently processed 
		// See if we can start a job for task *i: 
		if( (i->state==CompleteTask::ToBeRendered && may_start[DTRender]) ||
			(i->state==CompleteTask::ToBeFiltered && may_start[DTFilter]) )
		{
			startme=i;
			// Okay, if we have quit scheduled, we do not start NEW tasks. 
			if(schedule_quit && 
			    ( i->state==CompleteTask::ToBeRendered || 
			     (i->state==CompleteTask::ToBeFiltered && 
			      !TaskManager::IsARenderedTask(i))) )
			{  startme=NULL;  }
			if(startme)  break;
		}
	}
	
	return(startme);
}


void TaskDriverInterface_Local::_WriteStartProcInfo(const char *msg)
{
	// Write out useful verbose information: 
	VerboseSpecial("Okay, %s work: njobs=%d (parallel tasks)",msg,p->njobs);
	
	Verbose(TDI,"  jtype    jmax  nice   timeout  tty\n");
	for(int i=0; i<_DTLast; i++)
	{
		TaskDriverInterfaceFactory_Local::DTPrm *pr=&p->prm[i];
		Verbose(TDI,"  %s:  %4d  ",DTypeString(TaskDriverType(i)),pr->maxjobs);
		if(pr->niceval==TaskParams::NoNice)
		{  Verbose(TDI,"  --");  }
		else
		{  Verbose(TDI,"%s%3d%s",pr->nice_jitter ? "" : " ",
			pr->niceval,pr->nice_jitter ? "±" : "");  }
		if(pr->timeout<0)
		{  Verbose(TDI,"        --");  }
		else
		{  long x=(pr->timeout+500)/1000;
			Verbose(TDI,"  %02d:%02d:%02d",x/3600,(x/60)%60,x%60);  }
		Verbose(TDI,"  %s",pr->call_setsid ? " no" : "yes");
		if(pr->mute || pr->quiet)
		{  Verbose(TDI,pr->quiet ? "  (quiet)" : "  (mute)");  }
		Verbose(TDI,"\n");
	}
	
	Verbose(TDI,"  task-thresh: low=%d, high=%d\n",
		todo_thresh_low,todo_thresh_high);
	
}

void TaskDriverInterface_Local::_WriteEndProcInfo()
{
	
}

void TaskDriverInterface_Local::WriteProcessingInfo(int when,const char *msg)
{
	if(when==0)
	{  _WriteStartProcInfo(msg);  }
	else if(when==1)
	{  _WriteEndProcInfo();  }
	else assert(0);
}


void TaskDriverInterface_Local::ReallyStartProcessing()
{
	// Really easy. Not inline because virtual: 
	component_db()->taskmanager()->ReallyStartProcessing();
}


// These are called by the constructor/destructor of TaskDriver: 
int TaskDriverInterface_Local::RegisterTaskDriver(TaskDriver *td)
{
	if(!td)  return(0);
	
	// Check if queued: 
	if(joblist.prev(td) || td==joblist.first())  return(1);
	
	joblist.append(td);
	
	
	return(0);
}


void TaskDriverInterface_Local::UnregisterTaskDriver(TaskDriver *td)
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
	
	component_db()->taskmanager()->CheckStartNewJobs(/*njobs_changed=*/0);
}


TaskDriverInterface_Local::TaskDriverInterface_Local(
	TaskDriverInterfaceFactory_Local *f,int *failflag) : 
	TaskDriverInterface(f->component_db(),failflag),
	ProcessBase(failflag),
	joblist(failflag)
{
	p=f;
	
	todo_thresh_low=p->thresh_param_low;
	todo_thresh_high=p->thresh_param_high;
	
	for(int i=0; i<_DTLast; i++)
	{
		running_jobs[i]=0;
	}
}

TaskDriverInterface_Local::~TaskDriverInterface_Local()
{
	for(int i=0; i<_DTLast; i++)
	{  assert(!running_jobs[i]);  }
	assert(joblist.is_empty());
}
