/*
 * ldr.cpp
 * 
 * Task source implementing Local Distributed Rendering. 
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

#include "ldr.hpp"
#include "param.hpp"

#include "../taskfile.hpp"

#include <assert.h>

#include <netinet/in.h>

#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif


int TaskSource_LDR::fdnotify(FDInfo *fdi)
{
	if(fdi->fd==p->listen_fd)
	{
		_ListenFdNotify(fdi);
		return(0);
	}
}


void TaskSource_LDR::_ListenFdNotify(FDInfo *fdi)
{
	if(fdi->revents & (POLLERR | POLLHUP))
	{
		Error("Unknown revents for listening socket %d: %d. Aborting.\n",
			fdi->fd,fdi->revents);
		abort();  return;
	}
	if(fdi->revents & POLLIN)
	{
		// Okay, we may receive a connection. 
		sockaddr_in sin;
		socklen_t slen=sizeof(sin);
		memset(&sin,0,sizeof(sin));
		int as=::accept(p->listen_fd,(sockaddr*)&sin,&slen);
		if(as<0)
		{
			// Oh dear, something failed. 
			Warning("LDR: Failed to accept connection: %s\n",
				strerror(errno));
			return;
		}
		
		// Okay, accepted a connection: 
		Verbose("Accepted connection from %s:%u\n",
			inet_ntoa(sin.sin_addr),ntohs(sin.sin_port));
		
			Error("GO ON!!! (aborting)\n");
			abort();
		#if 0
		if(server.connected)
		{
			// Well, we're already connected to a server. 
		}
		#endif
	}
}


int TaskSource_LDR::timernotify(TimerInfo *ti)
{
	assert(ti->tid==rtid);
	
	_StopSchedTimer();
	
	TSNotifyInfo ni;
	ni.action=pending;
	
	switch(pending)
	{
		case ANone:
			// should never happen
			fprintf(stderr,"OOPS: ldr:%d:pending=ANone\n",__LINE__);
			abort();
			break;
		case AConnect:
			connected=1;
			// Okay, connected, right?
			ni.connstat=CSConnected;
			break;
		case AGetTask:
			break;
		case ADoneTask:
			break;
		case ADisconnect:
			connected=0;
			// Of course, we disconnected...
			ni.disconnstat=DSOkay;
			break;
	}
	
	TaskSourceConsumer *client=cclient;
	// Reset cclient and action before calling the virtual 
	// function so that this function can call 
	// GetTask / DoneTask. 
	cclient=NULL;
	pending=ANone;
	
	call_tsnotify(client,&ni);
	
	return(0);
}


// overriding virtuals from TaskSource: 
int TaskSource_LDR::srcConnect(TaskSourceConsumer *cons)
{
	if(connected)  return(2);
	
	// Okay, then let's connect...
	pending=AConnect;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcGetTask(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	pending=AGetTask;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	if(!connected)  return(2);
	
	pending=ADoneTask;
	cclient=cons;
#warning done task...
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDisconnect(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	cclient=cons;
	_StartSchedTimer();
	return(0);
}


long TaskSource_LDR::ConnectRetryMakesSense()
{
	// No, it makes absolutely no sense to re-try to connect to 
	// this task source. If it failed, it failed definitely. 
	return(0);
}


TaskSource_LDR::TaskSource_LDR(TaskSourceFactory_LDR *tsf,int *failflag) : 
	TaskSource(tsf->component_db(),failflag),
	FDBase(failflag)
{
	// Important: set TaskSourceType: 
	tstype=TST_Active;
	
	p=tsf;
	pending=ANone;
	connected=0;
	
	int failed=0;
	
	rtid=InstallTimer(-1,0);
	if(!rtid)
	{  ++failed;  }
	
	// Create a poll node for the listening fd and poll for input: 
	assert(p->listen_fd>=0);  // otherwise we may not be here. 
	l_pid=NULL;
	if(PollFD(p->listen_fd,POLLIN,NULL,&l_pid))
	{  ++failed;  }
	else assert(l_pid);  // l_pid may only be/stay NULL if PollFD returns failure
	
	a_fd=-1;
	a_pid=NULL;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_LDR");  }
	
	// Make sure the LDR header has correct length. 
	// Otherwise the struct LDRHeader should probably have been packed: 
	Error("Iclude assert in next line!!\n"); assert(0);
	//assert(sizeof(LDR::LDRHeader)==6);  <---- UNCOMMENT!!
}

TaskSource_LDR::~TaskSource_LDR()
{
	assert(pending==ANone);
	assert(!connected);
	
	// Must have disconnected...
	assert(a_fd<0);
	
	// Unpoll accept PollID: 
	UnpollFD(a_pid);  a_pid=NULL;
	
	// The listen FD is closed & shut down by the factory. 
	// We just have to explicitly unpoll it: 
	UnpollFD(l_pid);  l_pid=NULL;
}
