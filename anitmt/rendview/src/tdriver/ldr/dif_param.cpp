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

#include <lib/ldrproto.hpp>

#include <netdb.h>


using namespace LDR;

#define UnsetNegMagic  (-28649)



static volatile void __CheckAllocFailFailed()
{
	Error("Allocation failure.\n");
	abort();
}

static inline void _CheckAllocFail(int failed)
{
	if(failed)
	{  __CheckAllocFailFailed();  }
}


const char *TaskDriverInterfaceFactory_LDR::DriverInterfaceDesc() const
{
	return("The LDR task driver executes (render and filter) processes "
		"on remote LDR clients and thus makes rendview act as LDR server.");
}


// Create a LDR TaskDriverInterface (TaskDriverInterface_LDR): 
TaskDriverInterface *TaskDriverInterfaceFactory_LDR::Create()
{
	return(NEW1<TaskDriverInterface_LDR>(this));
}


int TaskDriverInterfaceFactory_LDR::FinalInit()
{
	int failed=0;
	
	if(cparam.is_empty() && !failed)
	{  Error("No LDR clients specified. (Cannot work.)\n");  ++failed;  }
	
	return(failed ? 1 : 0);
}


int TaskDriverInterfaceFactory_LDR::CheckParams()
{
	int failed=0;
	
	// Check port: 
	if(default_port<1 || default_port>=65536)
	{
		Error("Illegal port specified: %d\n",default_port);
		default_port=DefaultLDRPort;
		++failed;
	}
	
	// Try to resolve the client names: 
	if(str_clients.first())
	{
		Verbose("Looking up LDR clients...\n");
		for(const RefStrList::Node *n=str_clients.first(); n; n=n->next)
		{
			const char *name=n->str();
			if(!name || !(*name))
			{
				Warning("Skipping LDR client with NULL name.\n");
				continue;
			}
			ClientParam *cp=NEW<ClientParam>();
			assert(cp);  // otherwise alloc failure (may abort in CheckParams())

			// See if a port is specified: 
			// THIS WILL HAVE problems with IPv6...
			char *pstr=strrchr(name,':');
			int port=default_port;
			if(pstr)
			{
				char *end;
				port=(int)strtol(pstr,&end,10);
				if(*end || port<1 || port>=65536)
				{
					Warning("Illegal port spec in \"%s\". Defaulting to %d.\n",
						name,default_port);
					port=default_port;
				}
				// Okay, cut off port: 
				_CheckAllocFail(cp->name.set0(name,pstr-name));
			}
			else
			{  cp->name=*n;  }

			bool queued=0;
			do {
				const char *name=cp->name.str();
				int rv=cp->addr.SetAddress(name,port);
				if(rv==-1)
				{  Error("Failed to resolve host \"%s\": %s\n",
					name,hstrerror(h_errno));  ++failed;  break;  }
				else if(rv==-2)
				{  Error("gethostbyname() did not return AF_INET for \"%s\".\n",
					name);  ++failed;  break;  }
				else assert(!rv);

				// Okay, everything went fine; put client params in queue: 
				cparam.append(cp);
				queued=1;
			} while(0);
			if(!queued)
			{  delete cp;  cp=NULL;  }
		}
	}
	
	// Clear the client list, it's no longer needed: 
	str_clients.clear();
	
	return(failed ? 1 : 0);
}


static struct _ParamDescStuff
{
	const char *timeout_str;
} _pds[_DTLast]=
{
  { // _DTRender: 
	"render job time limit (seconds; -1 for none)"
  },
  { // _DTFilter;
	"filter job time limit (seconds; -1 for none)"
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
	if(SetSection("Ld","LDR task driver (interface): \"LDR server\" settings:"))
	{  return(1);  }
	Section *ld_sect=CurrentSection();
	
	add_failed=0;
	
	AddParam("clients",
		"list of client IP addresses and/or host names, optionally with "
		"port separated via `:� (e.g. 192.168.10.1:3104)",
		&str_clients);
	AddParam("port",
		"default LDR port if not specified after client",
		&default_port);
	
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
	TaskDriverInterfaceFactory("LDR",cdb,failflag),
	str_clients(failflag),
	cparam(failflag)
{
	int failed=0;
	
	// Set up defaults/initial values: 
	thresh_param_low=thresh_param_high=-1;
	
	default_port=DefaultLDRPort;
	
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
	// Clean up client parameter list: 
	while(!cparam.is_empty())
	{  delete cparam.popfirst();  }
}


/******************************************************************************/

TaskDriverInterfaceFactory_LDR::ClientParam::ClientParam(int *failflag=NULL) : 
	name(failflag),
	addr(failflag)
{
	
}