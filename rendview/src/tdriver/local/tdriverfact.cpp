/*
 * tdriverfact.cpp
 * 
 * Generic task driver factory stuff and some constructors 
 * and destructors of smaller classes. 
 * 
 * Copyright (c) 2001--2003 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <tsource/taskfile.hpp>

#include <assert.h>



// Returns string representation of TaskDriverType: 
char *DTypeString(TaskDriverType dt)
{
	switch(dt)
	{
		case DTNone:    return("[none]");
		case DTRender:  return("render");
		case DTFilter:  return("filter");
	}
	return("???");
}


// Called on statup to initialize the (render and filter) 
// driver factories; Return value: 0 -> OK; >0 -> error. 
int TaskDriverFactory::init(ComponentDataBase *cdb)
{
	int failed=0;
	
	failed+=RenderDriver::init_factories(cdb);
	failed+=FilterDriver::init_factories(cdb);
	
	return(failed);
}


TaskDriverFactory::TaskDriverFactory(
	ComponentDataBase *_cdb,
	const char *_driver_name,
	TaskDriverType tdt,
	int *failflag) : 
	LinkedListBase<TaskDriverFactory>(),
	par::ParameterConsumer_Overloaded(_cdb->parmanager(),failflag),
	dtype(tdt),
	driver_name(_driver_name,failflag)
{
	int failed=0;
	
	component_db=_cdb;
	
	if(!driver_name.str())
	{  ++failed;  }
	if(component_db->RegisterDriverFactory(this,dtype))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverFactory");  }
}

TaskDriverFactory::~TaskDriverFactory()
{
	// ...even if we're not registered. 
	component_db->UnregisterDriverFactory(this,dtype);
}


/******************************************************************************/

TaskParams::TaskParams(int * /*failflag*/) /*: 
	add_args(failflag)*/
{
	dtype=DTNone;
	
	niceval=NoNice;
	call_setsid=false;
	timeout=-1;
	
	hook=NULL;
};

TaskParams::~TaskParams()
{
	if(hook)
	{
		delete hook;
		hook=NULL;
		// If you get this assert, then there is a BUG!!
		// This is probably caused because the task manager does not call 
		// the apropriate execution-is-done cleanp function to free the 
		// hook (and do other things like deleting temp files, closing FDs, 
		// etc.) The hook MUST be tidied up by the mentioned function. 
		assert(0);
	}
}


/******************************************************************************/

TaskStructBase::TaskStructBase(int *failflag) : 
	infile(failflag),
	outfile(failflag),
	add_args(failflag),
	wdir(failflag)
{
	dtype=DTNone;
	
	frame_no=-1;
	
	timeout=-1;
}

TaskStructBase::~TaskStructBase()
{
	dtype=DTNone;
}


/******************************************************************************/

TaskDriver::PInfo::PInfo(int *failflag) : 
	args(failflag),
	tes(failflag)
{
	pid=-1;
	tsb=NULL;
	tp=NULL;
	
	ctsk=NULL;
}

TaskDriver::PInfo::~PInfo()
{
	// be sure...
	tsb=NULL;
	tp=NULL;
	ctsk=NULL;
}


/******************************************************************************/

const char *TaskExecutionStatus::OFS_String(OutputFileStatus ofs)
{
	switch(ofs)
	{
		case OFS_Unset:     return("[unset]");
		case OFS_Bad:       return("bad");
		case OFS_Resume:    return("can resume");
		case OFS_Complete:  return("complete");
	}
	return("???");
}

const char *TaskExecutionStatus::EF_JK_String(int jk_value)
{
	switch(jk_value)
	{
		case EF_JK_FailedToOpenIn:  return("failed to open input file");
		case EF_JK_FailedToOpenOut: return("failed to open output file");
		case EF_JK_FailedToExec:    return("failed to start job");
		case EF_JK_FailedExecPrep:  return("failed to prepare exec (chdir/nice/...)");
		case EF_JK_NotSupported:    return("action not supported");
		case EF_JK_LDRFail_File:    return("LDR file error");
		case EF_JK_LDRFail:         return("LDR error (generic)");
		case EF_JK_InternalError:   return("internal error");
	}
	return("???");
}

const char *TaskExecutionStatus::TTR_JK_String(TaskTerminationReason rflags)
{
	switch((rflags & TTR_JK_Mask))
	{
		case TTR_JK_NotKilled:  return("[not killed]");
		case TTR_JK_UserIntr:   return("user interrupt");
		case TTR_JK_Timeout:    return("timeout");
		// TTR_JK_ServerErr: rendview does not want to go on for what reason ever
		case TTR_JK_ServerErr:  return("rendview error");
	}
	return("???");
}

const char *TaskExecutionStatus::TermStatusString() const
{
	static char tmp[92];
	switch(rflags & TTR_TermMask)
	{
		case TTR_Unset:
			return("[unspecified]");
		case TTR_ExecFail:
			snprintf(tmp,92,"execution failed: %s",EF_JK_String(signal));
			return(tmp);
		case TTR_Exit:
			if(signal==0)  return("success");
			snprintf(tmp,92,"job exited with non-zero code %d",signal);
			return(tmp);
		case TTR_Killed:  // fall through
		case TTR_Dumped:
			snprintf(tmp,92,"job killed%s by signal %d (%s)",
				(rflags & TTR_TermMask)==TTR_Dumped ? " (dumped)" : "",
				signal,WasKilledByUs() ? TTR_JK_String(rflags) : "abnormally");
			return(tmp);
	}
	return("???");
}


TaskExecutionStatus::TaskExecutionStatus(int * /*failflag*/) : 
	starttime(HTime::Null),
	endtime(HTime::Null),
	utime(HTime::Null),
	stime(HTime::Null)
{
	rflags=TTR_Unset;
	signal=0;
	outfile_status=OFS_Unset;
}
