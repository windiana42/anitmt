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
	assert(GetAuthenticatedServer()==sc);
	
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
	// What is being done: 
	//   The server is removed. 
	//   We set the recovering flag. 
	//   We start the "schedule" timer and call tsnotify(TASRecovering) 
	//     (with ctsk=NULL) to tell the TaskManager. 
	//   Task manager's action:
	//     Report all tasks in todo and done queue as done to us 
	//       (the task source). 
	//     Kill all running tasks (make sure they are dead). 
	//     Go back to "waiting for work" state. 
	//     Call GetTask (which is normally not allowed for LDR task 
	//       source) as a special sign that recovery is done. 
	// We may not accept a server or talk to a server until the 
	// recovery is done. 
	
	// Set recovery flag: 
	// It is now 2 and gets 1 when we told the TaskManager. 
	recovering=2;
	_StartSchedTimer();
	// This also will report tsnotify(DTSOkay) to the current done task 
	// (in case a srcDoneTask() is pending). 
	
	sconn.dequeue(sc)->DeleteMe();
}


TaskSource_NAMESPACE::DoneTaskStat TaskSource_LDR::_ProcessDoneTask()
{
	if(!current_done_task)
	{  return(DTSOkay);  }
	
	TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
	if(srv)
	{
		srv->TellServerDoneTask(current_done_task);
		// This will call TaskReportedDone() when done. 
		return(DTSWorking);
	}
	
	// NOTE: If we have no server and the task manager reports a task as done 
	// then this may only happen during recovering. If the task manager 
	// reports the task as done without a server and not during recovering 
	// than that is a BUG. 
	assert(recovering);
	
	int hlvl;
	char tmp[128];
	snprintf(tmp,128,"LDR: Cannot report frame %d as %s processed "
		"(lost connection to server).\n",
		current_done_task->frame_no,
		TaskManager::Completely_Partly_Not_Processed(current_done_task,&hlvl));
	if(hlvl)
	{  Warning(tmp);  }
	else
	{  Verbose(TSLLR,tmp);  }
	
	// We must delete the CompleteTask (because that's the duty of 
	// the task source). 
	DELETE(current_done_task);
	
	// We say "okay" here because there is no point in 
	// returning any error during recovery. 
	return(DTSOkay);
}

void TaskSource_LDR::TaskReportedDone(CompleteTask *ctsk)
{
	// This is called as a response to TaskSource_LDR_ServerConn::TellServerDoneTask(). 
	// It means that the server got the information about the done task 
	// and we may now remove it. 
	assert(ctsk==current_done_task);
	assert(pending==ADoneTask);
	
	// We must delete the CompleteTask (because that's the duty of 
	// the task source). 
	DELETE(current_done_task);
	
	// NOTE: Caller relies on the fact that we call tsnotify() via 
	//       the schedule timer and NOT directly. 
	_StartSchedTimer();   // -> DTSOkay
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
		TSAction new_pending=ANone;
		
		switch(pending)
		{
			case AConnect:
				connected=1;
				// Okay, connected, right?
				ni.connstat=CSConnected;
				break;
			case ADoneTask:
				ni.donestat=_ProcessDoneTask();
				// If we return DTSOkay, current_done_task must have been deleted. 
				assert(ni.donestat!=DTSOkay || !current_done_task);
				if(ni.donestat==DTSWorking)
				{  new_pending=ADoneTask;  }
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
		
		// Reset action before calling tsnotify() so that 
		// this function can call GetTask / DoneTask. 
		pending=new_pending;
		call_tsnotify(cclient,&ni);
	}
	else assert(active_taketask || recovering);
	
	// Okay, see if there is a special active thingy to do: 
	// YES, we must do that even if we're recovering==2 becuase otherwise 
	//      the task would not be given back correctly...
	if(active_taketask)
	{
		TSNotifyInfo ni;
		ni.action=AActive;
		ni.activestat=TASTakeTask;
		ni.ctsk=active_taketask;
		
		active_taketask=NULL;
		
		call_tsnotify(cclient,&ni);
		
		// Must tell the server connection that the task was accepted. 
		TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
		if(srv)
		{  srv->TaskManagerGotTask();  }
		else assert(recovering==2);
		// recovering must be 2 (i.e. recovering and the task manager is 
		// not yet informed) here, otherwise this is a bug and we 
		// started the schedule timer in order to pass a new task 
		// although we're already recovering. 
	}
	
	if(recovering==2)
	{
		recovering=1;
		
		// Must tell server that we lost connection. 
		TSNotifyInfo ni;
		ni.action=AActive;
		ni.activestat=TASRecovering;
		
		call_tsnotify(cclient,&ni);
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
	if(recovering==1)
	{
		// This is special. It tells us that the TaskManager has 
		// finished recovery and is ready for work again. 
		
		Verbose(TSLLR,"LDR: Recovery finished. Ready again.\n");
		recovering=0;
		return(0);
	}
	
	if(!connected)  return(2);
	
	// LDR task source uses tsnotify(TASTakeTask) instead of srcGetTask(). 
	fprintf(stderr,"OOPS: GetTask called for LDR task source.\n");
	assert(0);
	return(-1);  // <-- (invalid code)
}


int TaskSource_LDR::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	assert(cclient==cons);
	if(!connected)  return(2);
	
	assert(!current_done_task);
	current_done_task=ct;
	
	pending=ADoneTask;
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


void TaskSource_LDR::ServerHasNowAuthenticated(TaskSource_LDR_ServerConn *sc)
{
	// In case this assert fails, there is an internal error somewhere else. 
	// Because we may not accept an auth if there is already another server 
	// or we are recovering. 
	TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
	assert((!srv || srv==sc) && !recovering);
	
	// Put the authenticated server at the beginning of the list: 
	sconn.dequeue(sc);
	sconn.insert(sc);
}


long TaskSource_LDR::ConnectRetryMakesSense()
{
	// No, it makes absolutely no sense to re-try to connect to 
	// this task source. If it failed, it failed definitely. 
	return(0);
}


void TaskSource_LDR::SetPersistentConsumer(TaskSourceConsumer *persistent)
{
	// Note: This gets called with persistent=NULL before quit. 
	assert(!cclient || !persistent || cclient==persistent);
	cclient=persistent;
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
	current_done_task=NULL;
	connected=0;
	recovering=0;
	
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
	assert(!current_done_task);  // If this fails, simply DELETE() instead. 
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
