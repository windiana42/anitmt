/*
 * dif_param.cpp
 * 
 * Implementation of parameter & factory class of LDR task driver interface. 
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

#include "dif_ldr.hpp"
#include "dif_param.hpp"

#include <assert.h>
#include <ctype.h>

#define UnsetNegMagic  (-28649)


// Create a LDR TaskDriverInterface (TaskDriverInterface_LDR): 
TaskDriverInterface *TaskDriverInterfaceFactory_LDR::Create()
{
	return(NEW1<TaskDriverInterface_LDR>(this));
}


int TaskDriverInterfaceFactory_LDR::FinalInit()
{
	int failed=0;
	
	
	return(failed ? 1 : 0);
}


int TaskDriverInterfaceFactory_LDR::CheckParams()
{
	int failed=0;
	
	return(failed ? 1 : 0);
}


static struct _ParamDescStuff
{
	const char *timeout_str;
} _pds[_DTLast]=
{
  { // _DTRender: 
	"render job time limit (seconds; -1 for none)",
  },
  { // _DTFilter;
	"filter job nice value",
  }
};


int TaskDriverInterfaceFactory_LDR::_SetUpParams(TaskDriverType dtype,
	Section *top)
{
	switch(dtype)
	{
		case DTRender:
			if(SetSection("r",
				"LDR task driver (interface): render parameters",top))
			{  return(1);  }
			break;
		case DTFilter:
			if(SetSection("f",
				"LDR task driver (interface): filter parameters",top))
			{  return(1);  }
			break;
		default:  assert(0);  break;
	}
	
	DTPrm *p=&prm[dtype];
	const _ParamDescStuff *s=&_pds[dtype];
	AddParam("timeout",s->timeout_str,&p->timeout);
	
	return(0);  // add_failed checked somewhere else
}


int TaskDriverInterfaceFactory_LDR::_RegisterParams()
{
	if(SetSection("Ld","LDR task driver (interface)"))
	{  return(1);  }
	Section *ld_sect=CurrentSection();
	
	add_failed=0;
	
	AddParam("task-thresh-low",
		"start getting new tasks from the task source if there are less "
		"than this number of non-completely processed tasks in the task "
		"queue",&thresh_param_low);
	AddParam("task-thresh-high",
		"never store more than this number of tasks in the local task queue",
		&thresh_param_high);
	
	if(add_failed)
	{  return(1);  }
	
	if(_SetUpParams(DTRender,ld_sect) ||
	   _SetUpParams(DTFilter,ld_sect) )
	{  return(1);  }
	
	return(add_failed ? (-1) : 0);
}


// Called on program start to set up the TaskDriverInterfaceFactory_LDR 
// (registration at ComponentDataBase). 
// Return value: 0 -> OK; >0 -> error. 
int TaskDriverInterfaceFactory_LDR::init(ComponentDataBase *cdb)
{
	TaskDriverInterfaceFactory_LDR *s=
		NEW1<TaskDriverInterfaceFactory_LDR>(cdb);
	if(!s)
	{
		Error("Failed to initialize LDR task driver interface.\n");
		return(1);
	}
	Verbose("[LDR] ");
	return(0);
}


TaskDriverInterfaceFactory_LDR::TaskDriverInterfaceFactory_LDR(
	ComponentDataBase *cdb,int *failflag) : 
	TaskDriverInterfaceFactory("LDR",cdb,failflag)
{
	int failed=0;
	
	// Set up defaults/initial values: 
	thresh_param_low=thresh_param_high=-1;
	
	for(int i=0; i<_DTLast; i++)
	{
		DTPrm *p=&prm[i];
		p->timeout=-1;
	}
	
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverInterfaceFactory_LDR");  }
}

TaskDriverInterfaceFactory_LDR::~TaskDriverInterfaceFactory_LDR()
{
}
