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
		int rv=TermProcess(pinfo.pid);
		if(rv)
		{
			assert(rv==-2);
			Error("While sending SIGTERM to %s (pid %ld): %s\n",
				pinfo.args.first()->str(),long(pinfo.pid),strerror(errno));
			// What shall we do? Giving up is safest. 
			#warning FIXME!!! Must change that; it is a race: process may just \
				have terminated between timeout and here. 
			abort();
		}
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
			_SendProcessError(PEI_StartSuccess,ps);
			
			// Tell manager: 
			_StateChanged();
		}
		else  // PSFailed 
		{
			estat=ESDone;
			esdetail=EDFailed;
			
			// Kill timeout timer. 
			UpdateTimer(tid_timeout,-1,/*align=*/0);
			
			// Tell the driver which writes out the error: 
			_SendProcessError(PEI_ExecFailed,ps);
			
			// Tell manager: 
			_StateChanged();
		}
	}
	else if(ps->action==PSDead)
	{
		assert(estat==ESRunning);
		
		estat=ESDone;
		switch(int(ps->detail))
		{
			case PSExited:
				esdetail=(ps->estatus ? EDTskErr : EDSuccess);
				break;
			case PSKilled:  // fall through
			case PSDumped:
				esdetail=EDATerm;
				break;
			default:  assert(0);  break;
		}
		
		_SendProcessError(
			(esdetail==EDSuccess) ? PEI_RunSuccess : PEI_RunFailed,ps);
		
		// FIXME: ps contains execution time, etc...; can make statistics. 
		
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
		estat=ESDone;
		esdetail=EDFailed;
		
		// Error reported on driver layer (lowest layer): 
		_SendProcessError(PEI_StartFailed);
		
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


int TaskDriver::_SendProcessError(ProcessErrorType pet,const ProcStatus *ps)
{
	ProcessErrorInfo pei;
	pei.reason=pet;
	pei.pinfo=&pinfo;
	pei.ps=ps;
	// Call the virtual function: 
	return(ProcessError(&pei));
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
	
	// Unrergister at task manager: 
	component_db()->taskmanager()->UnregisterTaskDriver(this);
}

