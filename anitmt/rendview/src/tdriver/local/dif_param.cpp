/*
 * dif_param.cpp
 * 
 * Implementation of parameter & factory class of local task driver interface. 
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


#include <lib/taskmanagement.h>
#include "../../database.hpp"

#include "dif_local.hpp"
#include "dif_param.hpp"

#include <assert.h>
#include <ctype.h>

#define UnsetNegMagic  (-28659)


const char *TaskDriverInterfaceFactory_Local::DriverInterfaceDesc() const
{
	return("The local task driver executes (render and filter) processes "
		"on your local computer.");
}


// Create a local TaskDriverInterface (TaskDriverInterface_Local): 
TaskDriverInterface *TaskDriverInterfaceFactory_Local::Create()
{
	return(NEW1<TaskDriverInterface_Local>(this));
}


int TaskDriverInterfaceFactory_Local::FinalInit()
{
	int failed=0;
	
	// Adjust task limits (if not told otherwise by user): 
	if(njobs<0)
	{
		if(njobs==UnsetNegMagic)
		{
			njobs=GetNumberOfCPUs();
			assert(njobs>=1);
		}
		else
		{
			Error("Illegal njobs value %d.\n",njobs);
			return(1);  // return NOW. 
		}
	}
	
	for(int i=0; i<_DTLast; i++)
	{
		DTPrm *p=&prm[i];
		
		if(p->maxjobs<0)
		{  p->maxjobs=njobs;  }
		
		// Convert timeout from seconds to msec: 
		ConvertTimeout2MSec(&p->timeout,"local task driver");
	}
	
	// Task queue thresh values: 
	if(thresh_param_low<1)  // YES: <1, NOT <0
	{  thresh_param_low=1;  }
	
	if(thresh_param_high<=thresh_param_low)
	{
		int oldval=thresh_param_high;
		thresh_param_high=thresh_param_low+5;
		if(oldval>=0)  // oldval=-1 for initial value set by constructor
		{  Warning("Adjusted high task queue threshold to %d (was %d)\n",
			thresh_param_high,oldval);  }
	}
	
	
	
	return(failed ? 1 : 0);
}


int TaskDriverInterfaceFactory_Local::CheckParams()
{
	int failed=0;
	
	return(failed ? 1 : 0);
}


static struct _ParamDescStuff
{
	const char *jobs_max_str;
	const char *nice_str;
	const char *timeout_str;
	const char *nice_jitter_str;
	const char *detach_term_str;
	const char *mute_str;    // or NULL
	const char *quiet_str;   // or NULL
} _pds[_DTLast]=
{
  { // _DTRender: 
	"max number of simultanious render jobs",
	"render job nice value",
	"render job time limit (seconds; -1 for none)",
	"switch on/off render job nice jitter (change nice value +/- 1 to "
		"prevent jobs from running completely simultaniously; no effect "
		"unless -rnice is used)",
	"disable this to allow terminal to control render process "
		"(-no-rdetach-term is NOT recommended)",
	"direct renderer output (stdout) to /dev/null",
	"direct renderer output (stdout & stderr) to /dev/null; implies -rmute"
  },
  { // _DTFilter;
	"max number of simultanious filter jobs",
	"filter job nice value",
	"filter job time limit (seconds; -1 for none)",
	"switch on/off filter job nice jitter (see also -r-nice-jitter)",
	"terminal control over filter process",
	NULL,
	NULL
  }
};


int TaskDriverInterfaceFactory_Local::_SetUpParams(TaskDriverType dtype,
	Section *top)
{
	switch(dtype)
	{
		case DTRender:
			if(SetSection("r",
				"local task driver (interface): render parameters",top))
			{  return(1);  }
			break;
		case DTFilter:
			if(SetSection("f",
				"local task driver (interface): filter parameters",top))
			{  return(1);  }
			break;
		default:  assert(0);  break;
	}
	
	DTPrm *p=&prm[dtype];
	const _ParamDescStuff *s=&_pds[dtype];
	if(s->mute_str)   AddParam("mute",s->mute_str,&p->mute);
	if(s->quiet_str)  AddParam("quiet",s->quiet_str,&p->quiet);
	AddParam("nice",s->nice_str,&p->niceval);
	AddParam("timeout",s->timeout_str,&p->timeout);
	AddParam("jobs-max",s->jobs_max_str,&p->maxjobs);
	AddParam("nice-jitter",s->nice_jitter_str,&p->nice_jitter);
	AddParam("detach-term",s->detach_term_str,&p->call_setsid);
	
	return(0);  // add_failed checked somewhere else
}


int TaskDriverInterfaceFactory_Local::_RegisterParams()
{
	if(SetSection("ld","Local task driver (interface)"))
	{  return(1);  }
	Section *ld_sect=CurrentSection();
	
	add_failed=0;
	AddParam("njobs","number of simultanious jobs",&njobs);
	
	AddParam("todo-thresh-low",
		"start getting new tasks from the task source if there are less "
		"than this number of non-completely processed tasks in the todo "
		"queue",&thresh_param_low);
	AddParam("todo-thresh-high",
		"never store more than this number of tasks in the local todo queue",
		&thresh_param_high);
	
	if(add_failed)
	{  return(1);  }
	
	if(_SetUpParams(DTRender,ld_sect) ||
	   _SetUpParams(DTFilter,ld_sect) )
	{  return(1);  }
	
	return(add_failed ? (-1) : 0);
}


// Called on program start to set up the TaskDriverInterfaceFactory_Local 
// (registration at ComponentDataBase). 
// Return value: 0 -> OK; >0 -> error. 
int TaskDriverInterfaceFactory_Local::init(ComponentDataBase *cdb)
{
	TaskDriverInterfaceFactory_Local *s=
		NEW1<TaskDriverInterfaceFactory_Local>(cdb);
	if(!s)
	{
		Error("Failed to initialize local task driver interface.\n");
		return(1);
	}
	Verbose(BasicInit,"[local] ");
	return(0);
}


TaskDriverInterfaceFactory_Local::TaskDriverInterfaceFactory_Local(
	ComponentDataBase *cdb,int *failflag) : 
	TaskDriverInterfaceFactory("local",cdb,failflag)
{
	int failed=0;
	
	// Set up defaults/initial values: 
	njobs=UnsetNegMagic;
	
	thresh_param_low=thresh_param_high=-1;
	
	for(int i=0; i<_DTLast; i++)
	{
		DTPrm *p=&prm[i];
		p->maxjobs=-1;   // initial value
		p->niceval=TaskParams::NoNice;
		p->call_setsid=true;
		p->nice_jitter=true;
		p->timeout=-1;
		p->mute=false;
		p->quiet=false;
	}
	
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverInterfaceFactory_Local");  }
}

TaskDriverInterfaceFactory_Local::~TaskDriverInterfaceFactory_Local()
{
}
