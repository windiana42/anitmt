/*
 * gdriver.cpp
 * 
 * Generic (render/filter) driver stuff: method implementation. 
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


#include "taskmanager.hpp"
#include "gdriver.hpp"
#include <assert.h>


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


// Inform mamager about state change: 
inline void TaskDriver::_StateChanged()
{
	component_db()->taskmanager()->StateChanged(this);
	
	if(estat==ESDone)  // We want to get deleted: 
	{  component_db()->taskmanager()->IAmDone(this);  }
}


int TaskDriver::timernotify(TimerInfo *ti)
{
	// FOR HLIB: long timeralarm return value. 
	// 0,>0 -> new time     oops: what about alignment?
	// -1 -> disable        TAStop  (can also be used at InstallTimer)
	// -2 -> kill           TAKill
	// -3 -> simply reset   TARst  (normal action)
	
	if(ti->tid==tid_timeout)
	{
		if(estat!=ESRunning)
		{
			// ignore it: 
			UpdateTimer(ti->tid,-1,/*align=*/0);
			return(0);
		}
		
		// Execution duration limit exceeded. 
		_SendProcessError(PEI_Timeout);
		
		// Kill process: 
		KillProcess(JK_Timeout);
	}
	else
	{  assert(0);  }
	
	return(0);
}


void TaskDriver::procnotify(const ProcStatus *ps)
{
	if(ps->action==PSExecuted || 
	   ps->action==PSFailed )
	{
		if(esdetail!=EDLaunched)
		{
			Error("Strange: estat,esdetail=%d,%d and ps action,detail=%d,%d\n",
				estat,esdetail,ps->action,ps->detail);
			abort();
		}
		
		// esdetail = EDLaunched here. 
		assert(estat==ESRunning);
		
		if(ps->action==PSExecuted)
		{
			assert(ps->detail==PSSuccess);
			// Okay, successful execution; task is running. 
			esdetail=EDRunning;
			
			// Driver may also know that: 
			_SendProcessError(PEI_ExecSuccess,ps);
			
			// Tell manager: 
			_StateChanged();
		}
		else  // PSFailed 
		{
			estat=ESDone;
			esdetail=EDFailed;
			
			// Kill timeout timer. 
			UpdateTimer(tid_timeout,-1,/*align=*/0);
			
			// Fill in info: 
			pinfo.tes.status=TTR_ExecFailed;
			pinfo.tes.signal=0;  // to be sure 
			_FillInStatistics(ps);
			
			// Tell the driver which writes out the error: 
			_SendProcessError(PEI_ExecFailed,ps);
			
			// Tell manager: 
			_StateChanged();
		}
	}
	else if(ps->action==PSDead)
	{
		assert(estat==ESRunning);
		
		// Switch estat and fill in info...
		
		estat=ESDone;
		TaskTerminationReason tmp_ttr=TTR_Unset;
		int tmp_signal=0;
		switch(int(ps->detail))
		{
			case PSExited:
				if(ps->estatus)  // Non-zero exit status. 
				{
					esdetail=EDTskErr;
					tmp_ttr=TTR_RunFail;
					tmp_signal=ps->estatus;
				} else { // Zero exit status. 
					esdetail=EDSuccess;
					tmp_ttr=TTR_Success;
				}  break;
			case PSKilled:  // fall through
			case PSDumped:
				esdetail=EDATerm;
				tmp_ttr=TTR_ATerm;
				tmp_signal=ps->estatus;
				break;
			//default:  assert(0);  break;  // will be caught by assert(tmp_ttr!=TTR_Unset); below
		}
		assert(tmp_ttr!=TTR_Unset);
		
		// Fill in info: 
		if(pinfo.tes.status==TTR_Unset)  // may be TTR_JobTerm / JK_Timeout
		{
			pinfo.tes.status=tmp_ttr;
			pinfo.tes.signal=tmp_signal;
		}
		_FillInStatistics(ps);
		
		_SendProcessError(
			(esdetail==EDSuccess) ? PEI_RunSuccess : PEI_RunFailed,ps);
		
		// Tell manager: 
		_StateChanged();
	}
}

// Called by the driver (instead of directly calling 
// ProcessBase::StartProcess()). 
int TaskDriver::StartProcess(
			const TaskStructBase *tsb,
			const TaskParams *tp,
			ProcessBase::ProcPath *sp_p,
			RefStrList            *sp_a,   // proc args
			ProcessBase::ProcMisc *sp_m,
			ProcessBase::ProcFDs  *sp_f,
			ProcessBase::ProcEnv  *sp_e)
{
	assert(estat==ESNone);
	
	// Okay, make sure these things are set correctly: 
	if(tp && tp->niceval!=TaskParams::NoNice)
	{  sp_m->niceval(tp->niceval);  }
	if(tp && tp->call_setsid)
	{  sp_m->setsid(tp->call_setsid);  }
	//sp_m->uid();
	//sp_m->gid();
	
	// Copy command line (args): 
	pinfo.args.clear();    // be sure...
	if(pinfo.args.append(sp_a))
	{  return(-1);  }
	
	// Copy other info: 
	pinfo.tsb=tsb;
	pinfo.tp=tp;
	
	// Okay, tell the driver that we're starting: 
	_SendProcessError(PEI_Starting);
	
	ProcessBase::ProcArgs _sp_a(sp_a);
	pinfo.pid=ProcessBase::StartProcess(*sp_p,_sp_a,*sp_m,*sp_f,*sp_e);
	if(pinfo.pid<0)
	{
		int save_errno=errno;
		
		estat=ESDone;
		esdetail=EDFailed;
		
		// Fill in that info: 
		pinfo.tes.status=TTR_ExecFailed;
		if(pinfo.pid==SPS_AccessFailed /* ... rest is internal error */)
		{  pinfo.tes.signal=JK_FailedToExec;  }
		else
		{  pinfo.tes.signal=JK_InternalError;  }
		
		// Error reported on driver layer (lowest layer): 
		_SendProcessError(PEI_StartFailed,NULL,save_errno);
		
		// This is important for TaskManager::IAmDone: 
		pinfo.ctsk=NULL;
		
		// Tell manager: 
		_StateChanged();
		
		return(int(pinfo.pid));
	}
	
	// Well, at least it seems to work...
	estat=ESRunning;
	esdetail=EDLaunched;
	// Start timeout timer: 
	if(tp && tp->timeout>=0)
	{  UpdateTimer(tid_timeout,tp->timeout,/*align=*/0);  }
	
	// Okay, tell the driver that it seems to work...
	_SendProcessError(PEI_StartSuccess);
	
	// Tell manager: 
	_StateChanged();
	
	return(0);
}


int TaskDriver::_SendProcessError(ProcessErrorType pet,
	const ProcStatus *ps,int errno_val)
{
	ProcessErrorInfo pei;
	pei.reason=pet;
	pei.pinfo=&pinfo;
	pei.ps=ps;
	pei.errno_val=errno_val;
	// Call the virtual function: 
	return(ProcessError(&pei));
}


// reason_detail: one of the JK_* - values
int TaskDriver::KillProcess(int reason_detail)
{
	Warning("Killing (SIGTERM) %s (pid %ld) [frame %d] (%s)\n",
		pinfo.args.first()->str(),long(pinfo.pid),pinfo.ctsk->frame_no,
		TaskExecutionStatus::JK_String(reason_detail));
	
	errno=0;
	int rv=TermProcess(pinfo.pid);
	if(rv)
	{
		assert(rv==-2 || rv==-3);
		
		Error("While sending SIGTERM to %s (pid %ld): %s\n",
			pinfo.args.first()->str(),long(pinfo.pid),
			(rv==-3) ? "process not known to process manager" : strerror(errno));
		// Damn. What shall we do. 
		// If the process is not known, we assume it is already dead 
		// but we did not get notified. This is an internal error.  
		if(rv==-3)  abort();
		
		// Now, if we failed to send the signal, that is bad, but 
		// we go on. 
		#warning FIXME!!! Must change that; it is a race: process may just \
			have terminated between timeout and here. 
		abort();
	}
	
	// Must fill in kill reason into pinfo.tes. 
	if(pinfo.tes.status!=TTR_Unset)
	{  fprintf(stderr,"OOPS: during kill: reason %d set in pinfo.tes.status.\n",
		pinfo.tes.status);  }
	pinfo.tes.status=TTR_JobTerm;
	pinfo.tes.signal=reason_detail;
	
	return(rv);
}


void TaskDriver::_FillInStatistics(const ProcStatus *ps)
{
	TaskExecutionStatus *tes=&pinfo.tes;
	
	tes->starttime=ps->starttime;
	tes->endtime=ps->endtime;
	tes->utime=ps->utime;
	tes->stime=ps->stime;
}


// Used by PSFailedErrorString(): 
const char *TaskDriver::_ExecFailedError_SyscallName(PSDetail x)
{
	switch(x)
	{
		case PSExecFailed:  return("execve");
		case PSDupFailed:  return("dup2");
		case PSChrootFailed:  return("chroot");
		case PSChdirFailed:  return("chdir");
		case PSNiceFailed:  return("nice");
		case PSSetSidFailed:  return("setsid");
		case PSSetGidFailed:  return("setgid");
		case PSSetUidFailed:  return("setuid");
		case PSFunctionFailed:  assert(0);  break;
		case PSUnknownError:  return("[unknown]");
	}
	return("???");
}

// Print error when ProcessBase::PSFailed occurs. 
const char *TaskDriver::PSFailedErrorString(ProcessErrorInfo *pei)
{
	const ProcStatus *ps=pei->ps;
	assert(ps->action==PSFailed);  // Otherwise this function is inapropriate. 
	
	static char tmp[128];
	snprintf(tmp,128,"while calling %s: %s (code %d)\n",
		_ExecFailedError_SyscallName(ps->detail),
		strerror(ps->estatus),ps->estatus);
	return(tmp);
}


// Can be used to get error code string returned by StartProcess. 
// Returns error string (static data); if errno-string (or special 
// code string) should also be written, the corresponding strerror() 
// value is in error_str (otherwise NULL). 
const char *TaskDriver::StartProcessErrorString(
	ProcessErrorInfo *pei,const char **error_str)
{
	static char numtmp[24];
	switch(pei->pinfo->pid)
	{
		case SPS_LMallocFailed:
		case SPS_PathAllocFailed:
		case SPS_ArgAllocFailed:
		case SPS_EvnAllocFailed:
			snprintf(numtmp,24,"(code %d)",pei->pinfo->pid);
			if(error_str) *error_str=numtmp;
			return("allocation failure");
		case SPS_AccessFailed:
			return("binary to execute not found");
		case SPS_ForkFailed:
			if(error_str) *error_str=strerror(pei->errno_val);
			return("while calling fork: ");
		case SPS_PipeFailed:
		case SPS_FcntlPipeFailed:
			if(error_str) *error_str=strerror(pei->errno_val);
			return("syscall (pipe/fcnt) failed: ");
		case SPS_IllegalFlags:
		case SPS_ProcessLimitExceeded:
		case SPS_SearchPathError:
		case SPS_ArgListError:
			snprintf(numtmp,24,"(code %d)",pei->pinfo->pid);
			if(error_str) *error_str=numtmp;
			return("internal error ");
		default:  break;  // see below. 
	}
	
	snprintf(numtmp,24,"(code %d)",pei->pinfo->pid);
	if(error_str) *error_str=numtmp;
	return("unknown internal error ");
}


TaskDriver::TaskDriver(TaskDriverFactory *_f,int *failflag=NULL) : 
	LinkedListBase<TaskDriver>(),
	FDBase(failflag),
	ProcessBase(failflag),
	pinfo(failflag)
{
	int failed=0;
	
	f=_f;
	estat=ESNone;
	esdetail=EDNone;
	
	tid_timeout=InstallTimer(-1,/*align=*/0);
	if(!tid_timeout)
	{  ++failed;  }
	
	// Register at task manager: 
	assert(component_db()->taskmanager());
	if(component_db()->taskmanager()->RegisterTaskDriver(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriver");  }
}

TaskDriver::~TaskDriver()
{
	assert(estat==ESNone || estat==ESDone);
	// Make sure we have filled in success/error reason: 
	assert(estat!=ESDone || pinfo.tes.status!=TTR_Unset);
	
	// Unrergister at task manager: 
	component_db()->taskmanager()->UnregisterTaskDriver(this);
}

