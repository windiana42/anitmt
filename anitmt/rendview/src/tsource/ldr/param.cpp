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



#define UnsetNegMagic  (-29659)

static const int DefaultListenPort=3104;

		
// Create a LDR TaskSource (TaskSource_LDR): 
TaskSource *TaskSourceFactory_LDR::Create()
{
	return(NEW1<TaskSource_LDR>(this));
}


int TaskSourceFactory_LDR::FinalInit()
{
	int failed=0;
	
	// Okay, LDR task source listenes to port listen_port: 
	// Actually bind to that port: 
	do {
		if(failed)  break;
		failed=1;
		
		// Open socket: 
		listen_fd=MyAddrInfo::TCPSocket();
		if(listen_fd<0)
		{  Error("Failed to open inet socket: %s\n",
			strerror(errno));  break;  }
		
		// Bind socket to port: 
		MyAddrInfo addr;
		addr.SetPassiveAny(listen_port);
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
		Verbose("LDR task source: Listening on port %d (procotol version %d)\n",
			listen_port,LDRProtocolVersion);
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::CheckParams()
{
	int failed=0;
	
	if(listen_port==UnsetNegMagic)
	{  listen_port=DefaultListenPort;  }
	
	if(listen_port<=0 || listen_port>65535)
	{
		Error("LDR: Illegal port number %d\n",listen_port);
		++failed;
	}
	
	return(failed ? 1 : 0);
}


int TaskSourceFactory_LDR::_RegisterParams()
{
	if(SetSection("L","LDR (Local Distributed Rendering) task source"))
	{  return(-1);  }
	
	AddParam("port","inet port to listen for connections from LDR server",
		&listen_port);
	
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
	Verbose("[LDR] ");
	return(0);
}


TaskSourceFactory_LDR::TaskSourceFactory_LDR(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("LDR",cdb,failflag)
{
	listen_fd=-1;
	
	listen_port=UnsetNegMagic;
	
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
}
