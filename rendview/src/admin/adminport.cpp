/*
 * adminport.cpp
 * 
 * Main implementation of admin port class. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "adminport.hpp"
#include <lib/ldrproto.hpp>

#include <ctype.h>

#include <assert.h>


void RendViewAdminPort::ExecuteCommand(const RefString &cmd,
	RVAPGrowBuffer *dest)
{
	// Copy it: 
	char *cmd_copy=cmd.str() ? LStrDup(cmd.str()) : NULL;
	
	RendViewAdmin_CommandExecuter::ADMCmd acmd;
	acmd.arg=NULL;
	
	bool allocfail=1;
	do {
		if(!cmd_copy)  break;
		
		// First, split it into arguments: 
		bool in_space=1;
		int argc=0;
		for(char *c=cmd_copy; *c; c++)
		{
			if(isspace(*c))
			{  in_space=1;  }
			else if(in_space)
			{  in_space=0;  ++argc;  }
		}
		
		if(argc==0)
		{
			// Nothing to do. 
			allocfail=0;
			break;
		}
		
		acmd.arg=(const char**)LMalloc(argc*sizeof(char*));
		if(!acmd.arg)  break;
		acmd.argc=argc;
		argc=0;
		
		in_space=1;
		for(char *c=cmd_copy; *c; c++)
		{
			if(isspace(*c))
			{
				if(!in_space)
				{  *c='\0';  }
				in_space=1;
			}
			else if(in_space)
			{
				acmd.arg[argc]=c;
				++argc;
				in_space=0;
			}
		}
		
		#if 0
		fprintf(stderr,"CMD(");
		for(int i=0; i<acmd.argc; i++)
		{  fprintf(stderr,"\"%s\" ",acmd.arg[i]);  }
		fprintf(stderr,")\n");
		#endif
		
		// Now, actually execute it: 
		acmd.command=&cmd;
		int n_okay=0;
		for(RendViewAdmin_CommandExecuter *x=exec_list.first(); x; x=x->next)
		{
			int rv=x->admin_command(&acmd,dest);
			if(rv==0)  ++n_okay;
		}
		if(!n_okay)
		{
			if(strcmp(acmd.arg[0],"?"))  // If NOT match. 
			{  dest->printf("Unknown command \"%s\".\n",acmd.arg[0]);  }
		}
		
		allocfail=0;
	} while(0);
	
	if(allocfail)
	{  dest->append0("[alloc failure]\n");  }
	
	LFree(acmd.arg);
	LFree(cmd_copy);
}


int RendViewAdminPort::fdnotify(FDInfo *fdi)
{
	Verbose(DBG,"--<Admin:fdnotify>--<fd=%d, ev=0x%x, rev=0x%x>--\n",
		fdi->fd,int(fdi->events),int(fdi->revents));
	
	assert(fdi->dptr==NULL);  // Can be removed. 
	
	if(fdi->revents & POLLIN)
	{
		MyAddrInfo addr;
		int as=addr.accept(listen_fd);
		if(as<0)
		{
			// Oh dear, something failed. 
			Warning("Failed to accept connection on admin port: %s\n",
				strerror(errno));
			return(0);
		}
		
		// Create a new connection: 
		RendViewAdminConnection *ac=NEW1<RendViewAdminConnection>(cdb);
		if(ac && ac->Setup(as,&addr))
		{  ac->DeleteMe();  ac=NULL;  }
		if(!ac)
		{
			Error("Accepting admin connection failed (%s).\n",
				cstrings.allocfail);
			close(as);
			return(0);
		}
		else
		{  conn_list.append(ac);  }
		
		return(0);
	}
	if(fdi->revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		Error("Unknown revents for admin port (listening) socket "
			"%d: 0x%x. Aborting.\n",
			fdi->fd,int(fdi->revents));
		abort();  return(0);
	}
	
	return(0);
}


void RendViewAdminPort::ConnClose(RendViewAdminConnection *conn,int reason)
{
	char *rstr=NULL;
	switch(reason)
	{
		case 0:  rstr="error";         break;
		case 1:  rstr="hangup";        break;
		case 2:  rstr="auth failure";  break;
		case 3:  rstr="timeout";       break;
		default: rstr="unknown reason";  break;  // <-- Should not happen. 
	}
	
	Verbose(MiscInfo,"Closed connection with admin shell %s (%s).\n",
		conn->addr.GetAddress().str(),rstr);
	Verbose(MiscInfo,"  Network traffic: in: %lld bytes, out %lld bytes\n",
		conn->in.tot_transferred,conn->out.tot_transferred);
	
	conn_list.dequeue(conn)->DeleteMe();	
}


void RendViewAdminPort::Register(RendViewAdmin_CommandExecuter *exc)
{
	assert(this);
	
	if(exc)
	{  exec_list.append(exc);  }
}

void RendViewAdminPort::Unregister(RendViewAdmin_CommandExecuter *exc)
{
	if(exc)
	{  exec_list.dequeue(exc);  }
}


int RendViewAdminPort::FinalInit()
{
	int failed=0;
	
	if(enable_admin_port && !failed)
	{
		LDR::LDRGetPassIfNeeded(&password,"Enter admin port password: ",NULL);
		if(!password.str())
		{  Error("Empty password for admin password specified. "
			"Disabling admin port.\n");  enable_admin_port=0;  }
	}
	
	if(enable_admin_port)  // Cannot be joined with above "if". 
	{
		// Set up listening port: 
		do {
			if(failed)  break;
			failed=1;
			
			MyAddrInfo addr;
			addr.SetPassiveAny(listen_port);
			
			// Open socket: 
			listen_fd=addr.socket();
			if(listen_fd<0)
			{  Error("Failed to create inet socket: %s\n",
				strerror(errno));  break;  }
			
			// Bind socket to port: 
			if(addr.bind(listen_fd))
			{  Error("Failed to bind to port %d (for admin): %s\n",
				addr.GetPort(),strerror(errno));  break;  }
			
			// Say we want connections to that port: 
			if(MyAddrInfo::listen(listen_fd,/*backlog=*/2))
			{  Error("Failed to listen to port %d (for admin): %s\n",
				addr.GetPort(),	strerror(errno));  break;  }
			
			// We want non-blocking IO, right?
			if(SetNonblocking(listen_fd))
			{  Error("Failed to set nonblocking IO for listen socket: %s",
				strerror(errno));  break;  }
			
			// Successfully done. 
			failed=0;
		} while(0);
	}
	
	if(failed && listen_fd>0)
	{  MyAddrInfo::close(listen_fd);  listen_fd=-1;  }
	
	if(!failed && enable_admin_port)
	{
		if(PollFD(listen_fd,POLLIN,NULL,&l_pid))
		{  ++failed;  }
		else assert(l_pid);
	}
	
	if(!failed)
	{
		if(!enable_admin_port)
		{  Verbose(MiscInfo,"Admin port: [disabled]\n");  }
		else
		{
			char attmp[32],ittmp[32];
			const char *disabled="[disabled]";
			if(auth_timeout>=0)
			{  snprintf(attmp,32,"%ld msec",auth_timeout);  }
			else  strcpy(attmp,disabled);
			if(idle_timeout>=0)
			{  snprintf(ittmp,32,"%ld sec",idle_timeout/1000);  }
			else  strcpy(ittmp,disabled);
			
			Verbose(MiscInfo,"Admin port: port %d; "
				"timeouts: auth: %s, idle: %s\n",
				listen_port,attmp,ittmp);
		}
	}
	
	return(failed ? 1 : 0);
}


int RendViewAdminPort::CheckParams()
{
	int failed=0;
	
	if(listen_port<=0 || listen_port>=65536)
	{  Error("Illegal admin port number %d for -a-port.\n",listen_port);
		++failed;  }
	
	if(enable_admin_port && !password)
	{  Error("Must specify password when enabling admin port.\n");
		++failed;  }
	
	// Idle timeout is specified in seconds: 
	idle_timeout*=1000;
	if(idle_timeout<0)  idle_timeout=-1;
	
	// Auth timeout is specified in msec. 
	if(auth_timeout<0)  auth_timeout=-1;
	
	return(failed ? 1 : 0);
}


int RendViewAdminPort::_RegisterParams()
{
	if(SetSection("a","Remote admin port"))
	{  return(1);  }
	
	add_failed=0;
	
	AddParam("enable",
		"enable admin port; must be set explicitly to enable",
		&enable_admin_port);
	AddParam("port",
		"TCP admin port number (to listen to)",
		&listen_port);
	AddParam("password",
		"admin port password; must be set if admin port is enabled; usual "
		"values (literal password or \"none\", \"prompt\", \"file:PATH\")",
		&password);
	AddParam("itimeout",
		"idle timeout; disconnect admin shell after being idle for specified "
		"time in seconds; -1 to disable",
		&idle_timeout);
	AddParam("atimeout",
		"auth timeout; max time in _msec_ for complete admin shell "
		"authentication to take place; -1 to disable",
		&auth_timeout);
	
	return(add_failed ? 1 : 0);
}


RendViewAdminPort::RendViewAdminPort(ComponentDataBase *_cdb,int *failflag) : 
	FDBase(failflag),
	//TimeoutBase(failflag),
	par::ParameterConsumer_Overloaded(_cdb->parmanager(),failflag),
	password(failflag),
	conn_list(failflag),
	exec_list(failflag)
{
	int failed=0;
	
	cdb=_cdb;
	cdb->_SetAdminPort(this);
	
	// Set up defaults: 
	enable_admin_port=0;
	listen_port=3105;
	listen_fd=-1;
	l_pid=NULL;
	auth_timeout=5000;   // 5 seconds
	idle_timeout=5*60;  // 5 minutes
	
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("RVAdminPort");  }
}

RendViewAdminPort::~RendViewAdminPort()
{
	// Make sure we kill all connections: 
	while(!conn_list.is_empty())
	{  conn_list.popfirst()->DeleteMe();  }
	
	UnpollFD(l_pid);
	
	// Make sure the listening FD is shut down: 
	if(listen_fd>=0)
	{
		MyAddrInfo::shutdown(listen_fd);
		MyAddrInfo::close(listen_fd);
		listen_fd=-1;
	}
	
	password.zero();
	
	// Must have unregistered. 
	assert(exec_list.is_empty());
}
