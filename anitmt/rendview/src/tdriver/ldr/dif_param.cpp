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
			int rv=cp->addr.SetAddress(name,port);
			if(rv==-1)
			{  Error("Failed to resolve host \"%s\": %s\n",
				name,hstrerror(h_errno));  ++failed;  break;  }
			else if(rv==-2)
			{  Error("gethostbyname() did not return AF_INET for \"%s\".\n",
				name);  ++failed;  break;  }
			else assert(!rv);
			
			// Read in password if needed: 
			{
				char tmp[128];
				snprintf(tmp,128,"Password for client %s (%s): ",
					cp->name.str(),cp->addr.GetAddress().str());
				LDRGetPassIfNeeded(&cp->password,tmp,&default_password);
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
	#warning could warn if same ports are used. 
	/*for(ClientParam *cp=cparam.first(); cp; cp=cp->next)
	{
		int n_same_adr=0;
		for(ClientParam *i=cp->next; i; i=i->next)
		{
			if(!cp->addr.same_address(i->addr))  continue;
			assert(!cp->addr.same_port(i->addr));  // otherwise we should have deleted it some lines above
			++n_same_adr;
		}
		// NOTE!!! That is tricky. FIXME: I report a client 17 times 
		//         which was spefified 18 times...
		if(n_same_adr)
		{  Warning("Client %s specified %d times with different ports.\n",
			cp->addr.GetAddress(0).str(),n_same_adr+1);  }
	}*/
	
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
		"port and/or password separated via `/� "
		"(host, host/port, host/port/pass, host//pass); password spec: "
		"password string to use OR \"none\" for none, empty/leave away "
		"for default, \"prompt\" to ask",
		&str_clients);
	AddParam("port",
		"default LDR port if not specified after client",
		&default_port);
	AddParam("password",
		"default password if not specified after client: password string, "
		"\"none\" or \"prompt\"",
		&default_password);
	
	AddParam("ctimeout",
		"connection timeout in _msec_ (time until connection & auth "
		"must have succeded)",
		&connect_timeout);
	
	AddParam("task-res-min",
		"try to always have that many task MORE than currently running",
		&todo_thresh_reserved_min);
	AddParam("task-res-max",
		"stop getting tasks when having that many tasks MORE than currenty "
		"running",
		&todo_thresh_reserved_max);
	
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
	todo_thresh_reserved_min=2;
	todo_thresh_reserved_max=6;
	
	default_port=DefaultLDRPort;
	
	connect_timeout=15000;  // 15 seconds
	
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

TaskDriverInterfaceFactory_LDR::ClientParam::ClientParam(int *failflag) : 
	name(failflag),
	addr(failflag),
	password(failflag)
{
	
}

TaskDriverInterfaceFactory_LDR::ClientParam::~ClientParam()
{
	password.zero();
}
