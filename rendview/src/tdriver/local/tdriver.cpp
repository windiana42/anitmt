/*
 * tdriver.cpp
 * 
 * Generic task driver stuff: method implementation. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "tdriver.hpp"
#include "dif_local.hpp"

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
	tdif->StateChanged(this);
	
	if(estat==ESDone)  // We want to get deleted: 
	{  tdif->IAmDone(this);  }
}


int TaskDriver::timernotify(TimerInfo *ti)
{
	Verbose(DBG,"--<TDrv::timernotify>--\n");
	
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
		KillProcess(TTR_JK_Timeout);
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
			pinfo.tes.rflags=(TaskTerminationReason)
				(TTR_ExecFail | (pinfo.tes.rflags & TTR_JK_Mask));
			// Set "signal code": 
			switch(ps->detail)
			{
				case PSExecFailed:
					pinfo.tes.signal=EF_JK_FailedToExec;
					break;
				case PSDupFailed:
				case PSChrootFailed:
				case PSChdirFailed:
				case PSNiceFailed:
				case PSSetSidFailed:
				//case PSSetGidFailed:
				//case PSSetUidFailed:
					pinfo.tes.signal=EF_JK_FailedExecPrep;
					break;
				default:
					pinfo.tes.signal=EF_JK_InternalError;
					break;
			}
			_FillInStatistics(ps);
			
			// Tell the driver which writes out the error: 
			_SendProcessError(PEI_ExecFailed,ps,ps->estatus);
			
			// Tell manager: 
			_StateChanged();
		}
	}
	else if(ps->action==PSDead)
	{
		assert(estat==ESRunning);
		
		// Let's see why we are here and fill in the info: 
		// This holds something != TTR_JK_NotKilled if rendview 
		// killed the job: 
		TaskTerminationReason we_did_it=(TaskTerminationReason)
			(pinfo.tes.rflags & TTR_JK_Mask);
		
		// First, we're done now, in any case (even if killed): 
		estat=ESDone;
		// NOTE: esdetail gets set somewhere further down. 
		
		switch(int(ps->detail))
		{
			case PSExited:
				pinfo.tes.rflags = (TaskTerminationReason)
					(we_did_it | TTR_Exit);
				pinfo.tes.signal=ps->estatus;
				break;
			case PSKilled:  // fall through
			case PSDumped:
				pinfo.tes.rflags = (TaskTerminationReason)( we_did_it | 
					(ps->detail==PSKilled ? TTR_Killed : TTR_Dumped) );
				pinfo.tes.signal=ps->estatus;
				break;
			default:  assert(0);  break;
		}
		
		#if TESTING
		{ int ef=(pinfo.tes.rflags & TTR_TermMask); assert(ef!=TTR_Unset); }
		#endif
		
		// NOTE: esdetail not set set. 
		
		_FillInStatistics(ps);
		
		// Okay, now check if we set the info correctly: Ask the driver 
		// about specials and let it decide if the output file is an 
		// unfinished frame suitable for resuming: 
		int rrv=InspectTaskDoneCode(&pinfo);
		assert(rrv==0);  // Because currently unused, must be 0. 
		
		// InspectTaskDoneCode() must set the outfile status: 
		assert(pinfo.tes.outfile_status!=OFS_Unset);
		
		// Finally, set esdetail to reflect complete success or failure: 
		esdetail=(pinfo.tes.IsCompleteSuccess() ? EDSuccess : EDFailed);
		
		// Note: In case we have complete success, the outfile_status 
		//       MUST be OFS_Complete. Else this is a BUG. 
		if(pinfo.tes.IsCompleteSuccess())
		{  assert(pinfo.tes.outfile_status==OFS_Complete);  }
		
		// NOTE: We do not pass *ps here because all relevant info is in 
		// pinfo.tes and the info in ps may even be wrong (e.g. if 
		// InspectTaskDoneCode() decides that a task with successful 
		// exit code actually failed. 
		_SendProcessError(
			(esdetail==EDSuccess) ? PEI_RunSuccess : PEI_RunFailed /*, ps, 0*/);
		
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
	
	// This info is already set by TaskDriverInterface::LaunchTask() 
	// (as it is also needed for error reporting): 
	assert(pinfo.tsb==tsb);
	assert(pinfo.tp==tp);
	
	// Okay, tell the driver that we're starting: 
	_SendProcessError(PEI_Starting);
	
	ProcessBase::ProcArgs _sp_a(sp_a);
	pinfo.pid=ProcessBase::StartProcess(sp_p,&_sp_a,sp_m,sp_f,sp_e);
	if(pinfo.pid<0)
	{
		_StartProcess_ErrorPart(errno);
		return(int(pinfo.pid));
	}
	
	// Well, at least it seems to work...
	estat=ESRunning;
	esdetail=EDLaunched;
	
	// Start timeout timer: 
	long timeout=-1;
	if(tp && tp->timeout>0)
	{  timeout=tp->timeout;  }
	if(tsb->timeout>0 && (timeout>tsb->timeout || timeout<0))
	{  timeout=tsb->timeout;  }  // Another timeout. Take shorter one. 
	if(timeout>0)
	{  UpdateTimer(tid_timeout,timeout,/*align=*/0);  }
	
	// Okay, tell the driver that it seems to work...
	_SendProcessError(PEI_StartSuccess);
	
	// Tell manager: 
	_StateChanged();
	
	return(0);
}


// This is part of StartProcess() dealing with error at 
// ProcessBase::StartProcess() (called from TaskDriver::StartProcess()) 
// and errors in XYZDriver::Execute() (called from 
// TaskDriverInterface::LaunchTask()) 
void TaskDriver::_StartProcess_ErrorPart(int save_errno)
{
	estat=ESDone;
	esdetail=EDFailed;
	
	// Fill in that info: 
	assert((pinfo.tes.rflags & TTR_TermMask)==TTR_Unset);
	TaskTerminationReason jkflags=(TaskTerminationReason)
		(pinfo.tes.rflags & TTR_JK_Mask);
	pinfo.tes.rflags=(TaskTerminationReason)(jkflags | TTR_ExecFail);
	
	switch(pinfo.pid)
	{
		case SPS_AccessFailed:   pinfo.tes.signal=EF_JK_FailedToExec;    break;
		case SPSi_OpenInFailed:  pinfo.tes.signal=EF_JK_FailedToOpenIn;  break;
		case SPSi_OpenOutFailed: pinfo.tes.signal=EF_JK_FailedToOpenOut; break;
		case SPSi_NotSupported:  pinfo.tes.signal=EF_JK_NotSupported;    break;
		default:  /* ... rest is internal error */
			pinfo.tes.signal=EF_JK_InternalError;  break;
	}
	
	// Error reported on driver layer (lowest layer): 
	_SendProcessError(PEI_StartFailed,NULL,save_errno);
	
	// This is important for TaskDriverInterface_Local::IAmDone: 
	assert(pinfo.ctsk->d.td==NULL);  // If this fails, assign NULL to it here (?) 
	pinfo.ctsk=NULL;
	
	// Tell manager: 
	_StateChanged();
}


int TaskDriver::_SendProcessError(ProcessErrorType pet,
	const ProcStatus *ps,int errno_val)
{
	ProcessErrorInfo pei;
	pei.reason=pet;
	pei.pinfo=&pinfo;
	pei.ps=ps;
	pei.errno_val=errno_val;
	pei.is_last_call=(estat==ESDone);
	
	// Call the virtual function: 
	return(ProcessError(&pei));
}


// reason_detail: one of the TTR_JK_* - values
int TaskDriver::KillProcess(TaskTerminationReason reason_detail)
{
	Warning("Killing (SIGTERM) %s (pid %ld) [frame %d] (%s)\n",
		pinfo.args.first()->str(),long(pinfo.pid),pinfo.ctsk->frame_no,
		TaskExecutionStatus::TTR_JK_String(reason_detail));
	
	errno=0;
	int rv=TermProcess(pinfo.pid);
	if(rv)
	{
		assert(rv==-2 || rv==-3);
		
		Error("While sending SIGTERM to %s (pid %ld): %s\n",
			pinfo.args.first()->str(),long(pinfo.pid),
			(rv==-3) ? "process not known to process manager" : strerror(errno));
		// Damn. 
		// If the process is not known, we assume it is already dead 
		// but we did not get notified. This is an internal error, because 
		// ProcessManager may not delete the task (and thus no longer know 
		// it) before we get notified that it no longer exists. 
		if(rv==-3)  abort();
		
		// Now, if we failed to send the signal, that is bad, but 
		// we go on. If the user starts unkillable processes (SUID or 
		// things) he hopefully knows what he is doing. 
	}
	
	// Must fill in kill reason into pinfo.tes. 
	if(pinfo.tes.rflags!=TTR_Unset)
	{  fprintf(stderr,"OOPS: during kill: reason %d set in pinfo.tes.rflags.\n",
		pinfo.tes.rflags);  }
	pinfo.tes.rflags=(TaskTerminationReason)reason_detail;  // NO |= so that we have at least a sane state
	
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


// Print error when ProcessBase::PSFailed occurs. 
const char *TaskDriver::PSFailedErrorString(ProcessErrorInfo *pei)
{
	const ProcStatus *ps=pei->ps;
	assert(ps->action==PSFailed);  // Otherwise this function is inapropriate. 
	assert(ps->estatus==pei->errno_val);  // <- should be set using _SendProcessError()
	
	static char tmp[128];
	snprintf(tmp,128,"while calling %s: %s (code %d)",
		ProcessBase::PSDetail_SyscallString(ps->detail),
		strerror(ps->estatus),ps->estatus);
	return(tmp);
}


// Can be used to get error code string returned by StartProcess. 
// Returns error string (static data). 
const char *TaskDriver::StartProcessErrorString(ProcessErrorInfo *pei)
{
	static char tmp[80];
	switch(pei->pinfo->pid)
	{
		case SPSi_IllegalParams:
			return("internal error");
		case SPSi_OpenInFailed:
		// The next two are supposed to be dealt with in a 
		// custom function (XYZDriver::ProcessError()) anyway so 
		// it is okay if they don't give too much info. 
		case SPSi_OpenOutFailed:
			snprintf(tmp,80,"failed to open %s file: %s",
				(pei->pinfo->pid==SPSi_OpenInFailed) ? "input" : "output",
				strerror(pei->errno_val));
			break;
		case SPSi_NotSupported:
			return("requested action not supported");
		default:  // including SPSi_Success: 
		{
			int etmp=errno;
			errno=pei->errno_val;
			ProcessBase::StartProcessErrorString(pei->pinfo->pid,tmp,80);
			errno=etmp;
		}  break;
	}
	return(tmp);
}


// Helper function: print command to be executed.
// print_cmd: 
//    0 -> do nothing 
//    1 -> print using Error() 
//    2 -> print using Verbose() 
void TaskDriver::ProcessError_PrintCommand(int print_cmd,
	const char *prefix,const char * /*prg_name*/,
	ProcessErrorInfo *pei)
{
		// Uh, yes this is an ugly code duplication: 
	if(!pei->pinfo)
	{  print_cmd=0;  }
	if(print_cmd==1)
	{
		Error("%s:   Command:",prefix);
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Error(" %s",i->str());  }
		Error("\n");
		
		if(pei->pinfo->tsb)
		{  const char *tmp=pei->pinfo->tsb->wdir.str();
			Error("%s:   Working dir: %s\n",prefix,tmp ? tmp : "[cwd]");  }
	}
	else if(print_cmd==2)
	{
		Verbose(TDR,"%s:   Command:",prefix);
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Verbose(TDR," %s",i->str());  }
		Verbose(TDR,"\n");
		
		if(pei->pinfo->tsb)
		{  const char *tmp=pei->pinfo->tsb->wdir.str();
			Verbose(TDR,"%s:   Working dir: %s\n",prefix,tmp ? tmp : "[cwd]");  }
	}
}


// This outputs the primary reason message (i.e. the first line of the 
// errors / status messages). 
// Also used by ProcessErrorStdMessage(). 
// ("Failed to start <prg_name> [frame xyz]", etc.)
// Returns value for print_cmd (0 -> no; 1 -> error; 2 -> verbose). 
//   (NOTE: Return value is 0 for PEI_StartFailed, PEI_ExecFailed and 
//    PEI_RunFailed)
int TaskDriver::ProcessError_PrimaryReasonMessage(
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	int print_cmd=0;
	
	char _frame_no_str[24];
	if(pei->pinfo && pei->pinfo->ctsk && pei->pinfo->ctsk->frame_no>=0)
	{  snprintf(_frame_no_str,24,"%d",pei->pinfo->ctsk->frame_no);  }
	else
	{  strcpy(_frame_no_str,"???");  }
	
	TaskTerminationReason it_was_us=(TaskTerminationReason)
		(pei->pinfo->tes.rflags & TTR_JK_Mask);
	
	switch(pei->reason)
	{
		// *** verbose messages: ***
		case PEI_Starting:
			Verbose(TDR,"%s: Starting %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			print_cmd=2;
			break;
		case PEI_StartSuccess:
			Verbose(TDR,"%s: Forked to launch %s [frame %s]...\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_ExecSuccess:
			Verbose(TDR,"%s: %s started successfully [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_RunSuccess:
			Verbose(TDR,"%s: %s terminated successfully [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		
		// *** warning/error messages (as you like to define it): ***
		case PEI_Timeout:
			Error("%s: %s [frame %s] exceeded time limit.\n",
				prefix,prg_name,_frame_no_str);
			print_cmd=1;
			break;
		
		// *** error messages: ***
		case PEI_StartFailed:
			Error("%s: Failed to start %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_ExecFailed:
			Error("%s: Failed to execute %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_RunFailed:
			(*(it_was_us ? &MessageLogger::_Warning : &MessageLogger::_Error))(
				"%s: %s [frame %s] execution %s%s.\n",
				prefix,prg_name,_frame_no_str,
				it_was_us ? "terminated by " : "failed",
				it_was_us ? ::prg_name : "");
			break;
	}
	
	return(print_cmd);
}


// Special functions printing the standard messages for Timeout, etc.
// Used by implementations overriding ProcessError() virtual. 
// Return value: print_cmd. 
int TaskDriver::ProcessErrorStdMessage_Timeout(
	const char *prefix,const char *  /*prg_name*/,
	ProcessErrorInfo *pei)
{
	const TaskStructBase *tsb = pei->pinfo ? pei->pinfo->tsb : NULL;
	const TaskParams *tp = pei->pinfo ? pei->pinfo->tp : NULL;
	
	int written=0;
	if(tsb && tsb->timeout>=0)
	{
		Error("%s:   Timeout: %ld seconds  (timeout from task source)\n",
			prefix,(tsb->timeout+500)/1000);
		++written;
	}
	if(tp && tp->timeout>=0)
	{
		Error("%s:   Timeout: %ld seconds  (local timeout)\n",
			prefix,(tp->timeout+500)/1000);
		++written;
	}
	assert(written);
	
	return(1);
}

int TaskDriver::ProcessErrorStdMessage_ExecFailed(
	const char *prefix,const char * /*prg_name*/,
	ProcessErrorInfo *pei)
{
	Error("%s:   Failure: %s\n",
		prefix,PSFailedErrorString(pei));
	return(0);
}

int TaskDriver::ProcessErrorStdMessage_RunFailed(
	const char *prefix,const char * /*prg_name*/,
	ProcessErrorInfo *pei)
{
	// See if we actually killed it: 
	bool it_was_us=(pei->pinfo->tes.WasKilledByUs());
	int (*efunc)(const char*,...) = it_was_us ? 
		&MessageLogger::_Warning : &MessageLogger::_Error;
	
	// Set this to 1 and the executed command will be printed: 
	int print_cmd=0;
	
	// NOTE: pei->ps is NULL. 
	TaskTerminationReason term_ttr=(TaskTerminationReason)
		(pei->pinfo->tes.rflags & TTR_TermMask);
	switch(term_ttr)
	{
		// BE CAREFUL! FALL-THROUGH LOGIC. 
		case TTR_Unset:
			assert(0);   // May not happen here.
		case TTR_Exit:
		case TTR_ExecFail:  // fall through
		case TTR_Killed:  // fall through
		case TTR_Dumped:
			(*efunc)("%s:   Exit status: %s\n",prefix,
				pei->pinfo->tes.TermStatusString());
			break;
		default:  assert(0);  // Illegal code
	}
	bool kill_reason_dumped=(term_ttr==TTR_Killed || term_ttr==TTR_Dumped);
	if(pei->pinfo->tes.WasKilledByUs() && !kill_reason_dumped)
	{
		(*efunc)("%s:   Killed by %s: %s\n",
			prefix,::prg_name,
			TaskExecutionStatus::TTR_JK_String(pei->pinfo->tes.rflags));
	}
	
	// Well... always print the command for now unless killed by us. 
	// May change that if it gets annoying. 
	print_cmd=!it_was_us;
	
	return(print_cmd);
}


TaskDriver::TaskDriver(TaskDriverFactory *_f,TaskDriverInterface_Local *_tdif,
	int *failflag) : 
	LinkedListBase<TaskDriver>(),
	FDBase(failflag),
	ProcessBase(failflag),
	pinfo(failflag)
{
	int failed=0;
	
	f=_f;
	tdif=_tdif;
	estat=ESNone;
	esdetail=EDNone;
	
	tid_timeout=InstallTimer(-1,/*align=*/0);
	if(!tid_timeout)
	{  ++failed;  }
	
	// Register at TaskDriverInterface_Local (-> task manager): 
	assert(component_db()->taskmanager());
	if(tdif->RegisterTaskDriver(this))
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
	assert(estat!=ESDone || pinfo.tes.rflags!=TTR_Unset);
	
	// Unrergister at TaskDriverInterface_Local (-> task manager): 
	tdif->UnregisterTaskDriver(this);
}

