/*
 * ldr.cpp
 * 
 * Task source implementing Local Distributed Rendering. 
 * This file is pretty boring because it's just the TaskSource 
 * interface. See ldrsconn.cpp for more interesting stuff. 
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
#include "../../taskmanager.hpp"

#include <string.h>
#include <assert.h>

using namespace LDR;

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


void TaskSource_LDR::TellTaskManagerToGetTask(CompleteTask *ctsk)
{
	// Actually, this does not have to be non-NULL but we are using only 
	// one TaskManager which is the cclient all the time, so this assert 
	// will not fail. [Maybe at late shutdown when DeletePending()?]
	assert(cclient);
	
	// Okay, we have to tell the TaskManager to get this task. 
	// We will always do that in the next step. 
	assert(active_taketask==NULL);
	active_taketask=ctsk;
	
	// NOTE that pending may have ANY value. We may not change that. 
	_StartSchedTimer();
}


void TaskSource_LDR::ConnClose(TaskSource_LDR_ServerConn *sc,int reason)
{
	// Okay, if that was not our server, then everything is okay: 
	if(!sc->Authenticated())
	{
		Verbose(TSLLR,"LDR: Closed connection to %s.\n",
			sc->addr.GetAddress().str());
		// Hehe... simply delete it: 
		sconn.dequeue(sc)->DeleteMe();
		return;
	}
	assert(reason!=2);  // Auth failure may only happen if !sc->authenticated. 
	
	if(reason==1)
	{
		Verbose(TSLLR,"LDR: Got quit request from server %s.\n",
			sc->addr.GetAddress().str());
		#warning should warn instead of verbose here if there are still tasks
	}
	else
	{
		Error("LDR: Unexpected connection close with auth server %s.\n",
			sc->addr.GetAddress().str());
	}
	
	// Okay, when we are here, what we have to do is basically: 
	// Get into a state similar to when the program was started. 
	// Wait for new connections. 
	// Task manager's action:
	//    Delete all tasks in todo and done queue. 
	//    Kill all running tasks (make sure they are deas). 
	//    Go back to "waiting for work" state. 
	fprintf(stderr,"IMPORTANT!!! HACK ME (re-start...) ldr.cpp:%d\n",__LINE__);
	//--> tell task manager. (missing)
	
	sconn.dequeue(sc)->DeleteMe();
}


int TaskSource_LDR::fdnotify(FDInfo *fdi)
{
	assert(fdi->dptr==NULL);  // Can be removed. 
	
	if(fdi->revents & POLLIN)
	{
		MyAddrInfo addr;
		int as=addr.accept(p->listen_fd);
		if(as<0)
		{
			// Oh dear, something failed. 
			Warning("LDR: Failed to accept connection: %s\n",
				strerror(errno));
			return(0);
		}
		
		// Okay, we may receive a connection. 
		TaskSource_LDR_ServerConn *sc=NEW1<TaskSource_LDR_ServerConn>(this);
		if(sc && sc->Setup(as,&addr))
		{  sc->DeleteMe();  sc=NULL;  }
		if(!sc)
		{
			Error("LDR: Accept failed (alloc failure).\n");
			close(as);
			return(0);
		}
		
		// Okay, accepted a connection: 
		Verbose(TSLLR,"LDR: Accepted connection from %s.\n",sc->addr.GetAddress().str());
		sconn.append(sc);
		return(0);
	}
	if(fdi->revents & (POLLERR | POLLHUP))
	{
		Error("LDR: Unknown revents for listening socket %d: %d. Aborting.\n",
			fdi->fd,fdi->revents);
		abort();  return(0);
	}
	
	return(0);
}


int TaskSource_LDR::timernotify(TimerInfo *ti)
{
	assert(ti->tid==rtid);
	
	_StopSchedTimer();
	
	if(pending!=ANone)
	{
		TSNotifyInfo ni;
		ni.action=pending;
		
		switch(pending)
		{
			case AConnect:
				connected=1;
				// Okay, connected, right?
				ni.connstat=CSConnected;
				break;
			case ADoneTask:
				break;
			case ADisconnect:
				connected=0;
				// Of course, we disconnected...
				ni.disconnstat=DSOkay;
				break;
			case AGetTask:   // <-- May not be called for active task source. 
			//case ANone:   // Excluded by if() above. 
			default:  assert(0);  break;
		}
		
		// Reset action before calling the virtual function so that 
		// this function can call GetTask / DoneTask. 
		pending=ANone;
		call_tsnotify(cclient,&ni);
	}
	else assert(active_taketask);
	
	// Okay, see if there is a special active thingy to do: 
	if(active_taketask)
	{
		TSNotifyInfo ni;
		ni.action=AActive;
		ni.activestat=TASTakeTask;
		ni.ctsk=active_taketask;
		
		active_taketask=NULL;
		
		call_tsnotify(cclient,&ni);
		
		// Must tell the server connection that the task was accepted. 
		assert(sconn.first() && sconn.first()->Authenticated());
		sconn.first()->TaskManagerGotTask();
	}
	
	return(0);
}


// overriding virtuals from TaskSource: 
int TaskSource_LDR::srcConnect(TaskSourceConsumer *cons)
{
	assert(cclient==cons);
	if(connected)  return(2);
	
	// Okay, then let's connect...
	pending=AConnect;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcGetTask(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	fprintf(stderr,"OOPS: GetTask called for LDR task source.\n");
	assert(0);
	return(-1);  // <-- (invalid code)
}


int TaskSource_LDR::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	assert(cclient==cons);
	if(!connected)  return(2);
	
	pending=ADoneTask;
#warning done task...
	fprintf(stderr,"TODO: IMPLEMENT DoneTask for LDR!\n");
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDisconnect(TaskSourceConsumer *cons)
{
	assert(cclient==cons);
	if(!connected)  return(2);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	_StartSchedTimer();
	return(0);
}


long TaskSource_LDR::ConnectRetryMakesSense()
{
	// No, it makes absolutely no sense to re-try to connect to 
	// this task source. If it failed, it failed definitely. 
	return(0);
}


TaskSource_NAMESPACE::TaskSourceType TaskSource_LDR::GetTaskSourceType(
	TaskSourceConsumer *persistent)
{
	assert(!cclient || cclient==persistent);
	cclient=persistent;
	return(tstype);
}


TaskSource_LDR::TaskSource_LDR(TaskSourceFactory_LDR *tsf,int *failflag) : 
	TaskSource(tsf->component_db(),failflag),
	FDBase(failflag),
	sconn(failflag)
{
	// Important: set TaskSourceType: 
	tstype=TST_Active;
	
	p=tsf;
	pending=ANone;
	active_taketask=NULL;
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
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_LDR");  }
	
	// Make sure the LDR header has correct length. 
	// Otherwise the struct LDRHeader should probably have been packed: 
	assert(sizeof(LDR::LDRHeader)==6);
}


TaskSource_LDR::~TaskSource_LDR()
{
	assert(pending==ANone);
	assert(!active_taketask);
	assert(!connected);
	
	// Make sure we disconnect from all servers...
	while(!sconn.is_empty())
	{
		TaskSource_LDR_ServerConn *sc=sconn.popfirst();
		//delete sc;   // We can use DeleteMe() here IMO. TaskManager guarantees 
		             // that there is a HLIB cycle to really delete them. 
		sc->DeleteMe();
	}
	
	// The listen FD is closed & shut down by the factory. 
	// We just have to explicitly unpoll it: 
	UnpollFD(l_pid);
}
