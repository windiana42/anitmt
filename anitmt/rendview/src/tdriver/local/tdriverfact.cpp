/*
 * tdriverfact.cpp
 * 
 * Generic task driver factory stuff and some constructors 
 * and destructors of smaller classes. 
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
#include "tdriver.hpp"

#include <tsource/taskfile.hpp>


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

TaskParams::TaskParams(int *failflag) : 
	add_args(failflag)
{
	dtype=DTNone;
	
	niceval=NoNice;
	call_setsid=false;
	timeout=-1;
};

TaskParams::~TaskParams()
{

}


/******************************************************************************/

TaskStructBase::TaskStructBase(int *failflag) : 
	add_args(failflag),
	wdir(failflag)
{
	dtype=DTNone;
	
	infile=NULL;
	outfile=NULL;
	
	timeout=-1;
}

TaskStructBase::~TaskStructBase()
{
	if(infile)
	{  delete infile;  infile=NULL;  }
	if(outfile)
	{  delete outfile;  outfile=NULL;  }
	
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

const char *TaskExecutionStatus::JK_String(int jk_value)
{
	switch(jk_value)
	{
		case JK_UserInterrupt:   return("user interrupt");
		case JK_Timeout:         return("timeout");
		// JK_ServerError: rendview does not want to go on for what reason ever
		case JK_ServerError:     return("rendview error");
		
		case JK_FailedToOpenIn:  return("failed to open input file");
		case JK_FailedToOpenOut: return("failed to open output file");
		case JK_FailedToExec:    return("failed to start job");
		case JK_InternalError:   return("internal error");
	}
	return("???");
}

const char *TaskExecutionStatus::StatusString()
{
	static char tmp[64];
	switch(status)
	{
		case TTR_Unset:  return("unspecified");
		case TTR_Success:  return("success");
		case TTR_ExecFailed:
			snprintf(tmp,64,"execution failed: %s",JK_String(signal));
			return(tmp);
		case TTR_RunFail:
			snprintf(tmp,64,"failed: job exited with non-zero code %d",signal);
			return(tmp);
		case TTR_ATerm:
			snprintf(tmp,64,"job (abnormally) killed by signal %d",signal);
			return(tmp);
		case TTR_JobTerm:
			snprintf(tmp,64,"job killed by rendview: %s",JK_String(signal));
			return(tmp);
		// default: see below
	}
	return("???");
}

TaskExecutionStatus::TaskExecutionStatus(int * /*failflag*/) : 
	starttime(HTime::Null),
	endtime(HTime::Null),
	utime(HTime::Null),
	stime(HTime::Null)
{
	status=TTR_Unset;
	signal=0;
}
