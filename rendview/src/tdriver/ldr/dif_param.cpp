/*
 * dif_param.cpp
 * 
 * Implementation of parameter & factory class of LDR task driver interface. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
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
	Error("%s.\n",cstrings.allocfail);
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
	
	// We need to do a little test for padding issues...
	LDRCheckCorrectLDRPaketSizes();
	
	// Try to resolve the client names: 
	Verbose(TDR,"Looking up LDR clients...\n");
	for(const RefStrList::Node *n=str_clients.first(); n; n=n->next)
	{
		const char *name=n->str();
		if(!name || !(*name))
		{
			Warning("Skipping LDR client with NULL name.\n");
			continue;
		}
		ClientParam *cp=NEW<ClientParam>();
		_CheckAllocFail(!cp);
		
		char sep='/';
		// HOST
		// HOST/port
		// HOST/port/passwd
		// HOST//passwd
		char *first_sep=strchr(name,sep);
		char *second_sep=first_sep ? strchr(first_sep+1,sep) : NULL;
		// See what is specified...
		char *pass_str = second_sep ? second_sep+1 : NULL;
		char *port_str = first_sep ? first_sep+1 : NULL;
		if(port_str+1==pass_str)  port_str=NULL;
		
		// Read in port: 
		int port=default_port;
		if(port_str)
		{
			char *end;
			port=(int)strtol(port_str,&end,10);
			if((*end && end!=second_sep) || port<1 || port>=65536)
			{
				Warning("Illegal port spec in \"%s\". Defaulting to %d. %s\n",
					name,default_port,port_str);
				port=default_port;
			}
		}
		
		// Get password (read it in further below): 
		_CheckAllocFail(cp->password.set(pass_str));
		
		// At the end: store name: 
		if(first_sep)
		{
			// Okay, cut off the other things (port, passwd)
			_CheckAllocFail(cp->name.set0(name,first_sep-name));
		}
		else
		{  cp->name=*n;  }
		
		bool queued=0;
		do {
			// Resolve the name: 
			const char *name=cp->name.str();
			int rv=cp->addr.SetAddressError(name,port);
			if(rv)  // Error already reported by SetAddressError(). 
			{  ++failed;  break;  }
			
			// Read in password if needed: 
			{
				char prompt[128];
				snprintf(prompt,128,"Password for client %s (%s): ",
					cp->name.str(),cp->addr.GetAddress().str());
				LDRGetPassIfNeeded(&cp->password,prompt,&default_password);
			}
			
			// Okay, everything went fine; put client params in queue: 
			cparam.append(cp);
			queued=1;
		} while(0);
		if(!queued)
		{  delete cp;  cp=NULL;  }
	}

	// Clear the client list, it's no longer needed: 
	str_clients.clear();
	
	// See if we have clients more than once: 
	for(ClientParam *cp=cparam.first(); cp; cp=cp->next)
	{
		int n_same=0;
		for(ClientParam *_i=cp->next; _i; )
		{
			ClientParam *i=_i;
			_i=_i->next;
			
			if(cp->addr==i->addr)
			{
				++n_same;
				delete cparam.dequeue(i);
			}
		}
		if(n_same)
		{  Warning("Client %s specified %d times. Using only once.\n",
			cp->addr.GetAddress().str(),n_same+1);  }
	}
	// Warn if same IP address (but different ports, of course) are used: 
	for(ClientParam *cp=cparam.first(); cp; cp=cp->next)
	{
		int n_same_adr=0;
		for(ClientParam *i=cparam.first(); i!=cp; i=i->next)
		{
			if(cp->addr.same_address(i->addr))
			{  goto already_reported;  }
		}
		for(ClientParam *i=cp->next; i; i=i->next)
		{
			if(!cp->addr.same_address(i->addr))  continue;
			assert(!cp->addr.same_port(i->addr));  // otherwise we should have deleted it some lines above
			++n_same_adr;
		}
		if(n_same_adr)
		{  Warning("Client %s specified %d times with different ports.\n",
			cp->addr.GetAddress(0).str(),n_same_adr+1);  }
		already_reported:;
	}
	
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
	
	// NOTE: We may want to allow reconnect_interval=0 some day in future. 
	if(connect_timeout<=0)  connect_timeout=-1;
	if(reconnect_interval<=0)  reconnect_interval=-1;
	ConvertTimeout2MSec(&reconnect_interval,"reconnect interval timer");
	
	if(cmd_resp_timeout<=0)  cmd_resp_timeout=-1;
	if(keepalive_interval<=0)  keepalive_interval=-1;
	ConvertTimeout2MSec(&keepalive_interval,"keepalive interval timer");
	
	if(reconnect_interval>=0 && connect_timeout<0)
	{  Error("Cannot use reconnection interval (-rcinterval) if connect "
		"timeout (-ctimeout) is switched off.\n");  ++failed;  }
	else if((reconnect_interval/2-1000)<connect_timeout)
	{  Error("Reconnection interval (-rcinterval; seconds) must be "
		"significantly larger than connect timeout (-ctimeout; msec).\n");
		++failed;  }
	
	if(connect_timeout<0)
	{  Warning("Warning: connect timeout disabled.\n");  }
	
	if(cmd_resp_timeout>=0 && keepalive_interval<0)
	{  Warning("Warning: It is pretty useless to enable -rtimeout when "
		"disabling -keepalive.\n");  }
	else if(keepalive_interval<0)
	{  Warning("Warning: Keepalive (-keepalive) disabled.\n");  }
	else if(cmd_resp_timeout<0)
	{  Warning("Warning: Keepalive response timeout (-rtimeout) disabled.\n");  }
	
	if(cmd_resp_timeout>keepalive_interval*2)
	{  Warning("Warning: You really mean to set -rtimeout (%ld) considerably "
		"larger than -keepalive (%ld)?\n",
		cmd_resp_timeout,keepalive_interval);  }
	
	if(todo_thresh_low<1)  // YES: <1, NOT <0
	{
		Warning("Adjusted low todo queue threshold to 1.\n");
		todo_thresh_low=1;
	}
	
	if(todo_thresh_high<=todo_thresh_low)
	{
		int oldval=todo_thresh_high;
		todo_thresh_high=todo_thresh_low+2;  // For LDR, just use +2. 
		if(oldval>=0)  // oldval=-1 for initial value set by constructor
		{  Warning("Adjusted high todo queue threshold to %d (was %d)\n",
			todo_thresh_high,oldval);  }
	}
	
	if(done_thresh_high<1)
	{
		Warning("Adjusted high done queue threshold to 1.\n");
		done_thresh_high=1;
	}
	
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
		"port and/or password separated via `/´ "
		"(host, host/port, host/port/pass, host//pass); password spec: "
		"password string to use OR \"none\" for none, empty/leave away "
		"for default, \"prompt\" to ask, \"file:PATH\" to read it from "
		"PATH",
		&str_clients);
	AddParam("port",
		"default LDR port if not specified after client",
		&default_port);
	AddParam("password",
		"default password if not specified after client: password string, "
		"\"none\", \"prompt\" or \"file:PATH\"",
		&default_password);
	
	AddParam("ctimeout",
		"connection timeout in _msec_ (time until connection & auth "
		"must have succeded)",
		&connect_timeout);
	AddParam("rcinterval",
		"reonnect interval: try to re-connect to lost clients after the "
		"specified time in seconds (note: actual reconnect may not happen "
		"until two times that interval; -1 to disable)",
		&reconnect_interval);
	AddParam("keepalive",
		"keepalive (ping command) interval time in seconds, -1 to disable",
		&keepalive_interval);
	AddParam("rtimeout",
		"keepalive (and control command _only_) response timeout: if no "
		"response is received after rtimeout _msec_, the client is kicked; "
		"use -1 to disable",
		&cmd_resp_timeout);
	
	AddParam("todo-thresh-low",
		"start getting new tasks from the task source if there are less "
		"than this number of tasks in the todo queue",
		&todo_thresh_low);
	AddParam("todo-thresh-high",
		"never store more than this number of tasks in the local todo queue",
		&todo_thresh_high);
	AddParam("done-thresh-high",
		"always report tasks as done when there are this many task in "
		"done queue",
		&done_thresh_high);
	
	AddParam("max-jobs-per-client",
		"max number of client njobs value (if client reports more, "
		"use this val instead)",
		&max_jobs_per_client);
	AddParam("max-client-task-thresh",
		"max task thresh value for client (if it reports more, use this "
		"val instead)",
		&max_high_thresh_of_client);
	
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
	Verbose(BasicInit,"[LDR] ");
	return(0);
}


TaskDriverInterfaceFactory_LDR::TaskDriverInterfaceFactory_LDR(
	ComponentDataBase *cdb,int *failflag) : 
	TaskDriverInterfaceFactory("LDR",cdb,failflag),
	default_password(failflag),
	str_clients(failflag),
	cparam(failflag)
{
	int failed=0;
	
	// Set up defaults/initial values: 
	todo_thresh_low=2;
	todo_thresh_high=4;   // Only 2 safety-tasks for LDR. 
	done_thresh_high=10;   // 1 -> immediately report done task
	
	max_jobs_per_client=24;
	max_high_thresh_of_client=36;
	
	default_port=DefaultLDRPort;
	
	connect_timeout=15000;   // 15 seconds  (MSEC, YES!)
	reconnect_interval=300;  // 5 minutes (SECONDS)
	keepalive_interval=30;   // (SECONDS)
	cmd_resp_timeout=5000;   // (MSEC)
	
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
	
	default_password.zero();
}


/******************************************************************************/

TaskDriverInterfaceFactory_LDR::ClientParam::ClientParam(int *failflag) : 
	name(failflag),
	addr(failflag),
	password(failflag)
{
	client=NULL;
}

TaskDriverInterfaceFactory_LDR::ClientParam::~ClientParam()
{
	password.zero();
	assert(!client);
}
