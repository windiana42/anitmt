/*
 * param.cpp
 * 
 * Implementation of parameter & factory class of LDR task source. 
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

#include <lib/myaddrinfo.hpp>

#include "ldr.hpp"
#include "param.hpp"

#include <assert.h>
#include <ctype.h>

#include <lib/ldrproto.hpp>


using namespace LDR;

#define UnsetNegMagic  (-29659)

		
const char *TaskSourceFactory_LDR::TaskSourceDesc() const
{
	return("Use the LDR task source to let rendview act as LDR client "
		"getting frames to process from an LDR server.");
}


/*static void _CheckAlloc(int x)
{
	if(x)
	{  Error("Allocation failure.\n");  abort();  }
}*/


// Create a LDR TaskSource (TaskSource_LDR): 
TaskSource *TaskSourceFactory_LDR::Create()
{
	return(NEW1<TaskSource_LDR>(this));
}


int TaskSourceFactory_LDR::FinalInit()
{
	int failed=0;
	
	LDRGetPassIfNeeded(&password,"Enter client password: ",NULL);
	if(!password.str())
	{  Warning("LDR client: no server password specified "
		"(authentification disabled).\n");  }
	
	// Okay, LDR task source listenes to port listen_port: 
	// Actually bind to that port: 
	do {
		if(failed)  break;
		failed=1;
		
		MyAddrInfo addr;
		addr.SetPassiveAny(listen_port);
		
		// Open socket: 
		listen_fd=addr.socket();
		if(listen_fd<0)
		{  Error("Failed to open inet socket: %s\n",
			strerror(errno));  break;  }
		
		// Bind socket to port: 
		if(addr.bind(listen_fd))
		{  Error("Failed to bind to port %d: %s\n",addr.GetPort(),
			strerror(errno));  break;  }
		
		// Say we want connections to that port: 
		if(MyAddrInfo::listen(listen_fd,/*backlog=*/2))
		{  Error("Failed to listen to port %d: %s\n",addr.GetPort(),
			strerror(errno));  break;  }
		
		// We want non-blocking IO, right?
		if(SetNonblocking(listen_fd))
		{  Error("Failed to set nonblocking IO for listen socket: %s",
			strerror(errno));  break;  }
		
		// Successfully done. 
		failed=0;
	} while(0);
	if(failed && listen_fd>0)
	{  MyAddrInfo::close(listen_fd);  listen_fd=-1;  }
	
	if(!failed)
	{
		Verbose(TSP,"LDR task source: Listening on port %d (procotol version %d)\n",
			listen_port,LDRProtocolVersion);
		Verbose(TSP,"  Server authentification (password): %s\n",
			password.str() ? "enabled" : "disabled");
		Verbose(TSP,"  Transferring files:\n"
			"    Render source: %s     Render dest: %s\n"
			"    Filter dest:   %s     Additional:  %s\n",
			transfer.render_src  ? "yes" : "no ",
			transfer.render_dest ? "yes" : "no",
			transfer.filter_dest ? "yes" : "no ",
			transfer.additional  ? "yes" : "no");
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::CheckParams()
{
	int failed=0;
	
	if(listen_port==UnsetNegMagic)
	{  listen_port=DefaultLDRPort;  }
	
	if(listen_port<=0 || listen_port>65535)
	{
		Error("LDR: Illegal port number %d\n",listen_port);
		++failed;
	}
	
	if(transfer_spec_str.str())
	{
		const char *s=transfer_spec_str.str();
		int f_error=0;
		while(*s)
		{
			const char *e=s;
			int mode=0;
			while(isalpha(*e))  ++e;
			
			if(*e=='+')  mode=1;
			else if(*e=='-')  /*mode=0*/;
			else
			{  ++f_error;  break;  }
			
			do {
				if(e-s==1)
				{
				 	if(*s=='a')  {  transfer.additional=mode;  break;  }
				}
				else if(e-s==2)
				{
					if(*s=='r' && s[1]=='s')
					{  transfer.render_src=mode;  break;  }
					else if(*s=='r' && s[1]=='d')
					{  transfer.render_dest=mode;  break;  }
					else if(*s=='f' && s[1]=='d')
					{  transfer.filter_dest=mode;  break;  }
				}
				else if(e-s==3)
				{
					if(*s=='a' && s[1]=='l' && s[2]=='l')
					{
						transfer.additional=mode;
						transfer.render_src=mode;
						transfer.render_dest=mode;
						transfer.filter_dest=mode;
						break;
					}
				}
				++f_error;  goto breakboth;
			} while(0);
			
			s=e+1;
		}
		breakboth:;
		if(f_error)
		{
			Error("Invalid transfer spec \"%s\".\n",transfer_spec_str.str());
			++failed;
		}
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::_RegisterParams()
{
	if(SetSection("L","LDR (Local Distributed Rendering) task source: "
		"\"LDR client\" settings:"))
	{  return(-1);  }
	
	AddParam("port","inet port to listen for connections from LDR server",
		&listen_port);
	
	AddParam("transfer","specify which files shall/shall not be tranferred "
		"from the server to THIS client (files not being transferred are "
		"assumed to be found on the hd becuase e.g. it is an NFS volume); "
		"possible files:\n"
		"  \"a\"   -> additional files   \"rd\" -> render dest\n"
		"  \"rs\"  -> render source      \"fd\" -> filter dest\n"
		"  \"all\" -> all files\n"
		"concatenation using postfix `+' (transfer) and `-' (do not transfer); "
		"e.g. \"a+rs-rd-fd-\"; (default: \"all+\")",&transfer_spec_str);
	
	AddParam("password","password required by the server to be allowed "
		"to connect to this client; leave away or \"none\" to disable; "
		"\"prompt\" to prompt for one ",&password);
	
	return(add_failed ? (-1) : 0);
}


// Called on program start to set up the TaskSourceFactory_LDR. 
// TaskSourceFactory_LDR registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int TaskSourceFactory_LDR::init(ComponentDataBase *cdb)
{
	TaskSourceFactory_LDR *s=NEW1<TaskSourceFactory_LDR>(cdb);
	if(!s)
	{
		Error("Failed to initialize LDR task source.\n");
		return(1);
	}
	Verbose(BasicInit,"[LDR] ");
	return(0);
}


TaskSourceFactory_LDR::TaskSourceFactory_LDR(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("LDR",cdb,failflag),
	password(failflag),
	transfer_spec_str(failflag)
{
	listen_fd=-1;
	
	listen_port=UnsetNegMagic;
	
	transfer.render_src=1;
	transfer.render_dest=1;
	transfer.filter_dest=1;
	transfer.additional=1;
	
	int failed=0;
	
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory_LDR");  }
}

TaskSourceFactory_LDR::~TaskSourceFactory_LDR()
{
	// Make sure the listening FD is shut down: 
	if(listen_fd>=0)
	{
		::shutdown(listen_fd,2);
		::close(listen_fd);
		listen_fd=-1;
	}
	
	password.zero();
}
