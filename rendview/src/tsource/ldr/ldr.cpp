/*
 * ldr.cpp
 * 
 * Task source implementing Local Distributed Rendering. 
 * This file is pretty boring because it's just the TaskSource 
 * interface. See ldrsconn.cpp for more interesting stuff. 
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


int TaskSource_LDR::TellTaskManagerToExecSimpleCCmd(
	ClientControlCommand cccmd)
{
	TSNotifyInfo ni;
	ni.action=AActive;
	ni.activestat=TASSimpleCommand;
	ni.cccmd=cccmd;
	
	return(call_tsnotify(cclient,&ni));
}


int TaskSource_LDR::TellTaskManagerToGiveBackTasks(int may_keep)
{
	TSNotifyInfo ni;
	ni.action=AActive;
	ni.activestat=TASGiveBackTasks;
	ni.int_payload=may_keep;
	
	return(call_tsnotify(cclient,&ni));
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
	assert(quit_reason==0);
	
	if(reason==1)
	{
		const TaskManager_TaskList *lst=
			component_db->taskmanager()->GetTaskList();
		if(lst->todo_nelem || lst->proc_nelem || lst->done_nelem)
		{  Warning("LDR: Server %s quit request although still tasks in queue:\n"
			"     Task queues: todo: %d; proc: %d; done: %d\n",
			sc->addr.GetAddress().str(),
			lst->todo_nelem,lst->proc_nelem,lst->done_nelem);  }
		else
		{  Verbose(TSLLR,"LDR: Disconnect due to quit request from "
			"server %s.\n",sc->addr.GetAddress().str());  }
		quit_reason=2;
	}
	else if(reason==3)
	{
		assert(ts_quit);
		Verbose(TSLLR,"LDR: Now disconnecting from server %s.\n",
			sc->addr.GetAddress().str());
	}
	else
	{
		Error("LDR: Unexpected connection close with auth server %s.\n",
			sc->addr.GetAddress().str());
		quit_reason=1;
	}
	
	// Dump some nice statistics: 
	sc->DumpNetIOStatistics(VERBOSE_TSLLR,&sc->connected_since,
		/*side=*/-1,sc->addr.GetAddress().str());
	
	// Forget about reporting new tasks now. 
	// It is deleted by the server connection. 
	active_taketask=NULL;
	
	if(ts_quit)
	{
		// In this case, we handle that just like a successful quit from 
		// the server. 
		_StartSchedTimer();
		sconn.dequeue(sc)->DeleteMe();
		return;
	}
	
	// Okay, when we are here, what we have to do is basically: 
	// Get into a state similar to when the program was started. 
	// Wait for new connections. 
	// What is being done: 
	//   The server is removed. 
	//   We set the recovering flag. 
	//   We start the "schedule" timer and call tsnotify(TASRecovering*) 
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
	// _ProcessDoneTask() gets called twice for each done task. 
	// The first time, it triggers the action to tell the LDR server that 
	// the task was done, the next time it actually deletes the task. 
	// Note that there are 2 positions for the delete and this if() is 
	// needed for correct operation if we go the TaskReportedDone() way. 
	if(!current_done_task)
	{  return(DTSOkay);  }
	
	TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
	if(srv)
	{
		srv->TellServerDoneTask(current_done_task);
		// This will call TaskReportedDone() when done. 
		done_task_status=2;
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
		current_done_task->Completely_Partly_Not_Processed(&hlvl));
	if(hlvl)
	{  Warning(tmp);  }
	else
	{  Verbose(TSLLR,tmp);  }
	
	// Dump all the information to the user (if requested):
	snprintf(tmp,128,"Task [frame %d] being uselessly destroyed (%s processed):",
		current_done_task->frame_no,
		current_done_task->Completely_Partly_Not_Processed());
	component_db->taskmanager()->DumpTaskInfo(current_done_task,
		tmp,TaskManager::DTSK_ReportBackDone,VERBOSE_TSLLR);
	
	// We must delete the CompleteTask (because that's the duty of 
	// the task source). 
	done_task_status=0;
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
	
	// Dump all the information to the user (if requested):
	component_db->taskmanager()->DumpTaskInfo(current_done_task,
		NULL,TaskManager::DTSK_ReportBackDone,VERBOSE_TSLLR);
	
	// We must delete the CompleteTask (because that's the duty of 
	// the task source). 
	done_task_status=0;
	DELETE(current_done_task);
	
	// NOTE: Caller relies on the fact that we call tsnotify() via 
	//       the schedule timer and NOT directly. 
	_StartSchedTimer();   // -> DTSOkay
}


int TaskSource_LDR::fdnotify(FDInfo *fdi)
{
	Verbose(DBG,"--<TSLDR:fdnotify>--<fd=%d, ev=0x%x, rev=0x%x>--\n",
		fdi->fd,int(fdi->events),int(fdi->revents));
	
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
		
		// Check if this address is allowed: 
		TaskSourceFactory_LDR::ServerNet *acc_sn=NULL;
		if(!p->server_net_list.is_empty())
		{
			for(TaskSourceFactory_LDR::ServerNet *sn=p->server_net_list.first(); 
				sn; sn=sn->next)
			{
				// Check server net. 
				if(addr.IsInNet(sn->adr,sn->mask))
				{  acc_sn=sn;  goto adr_okay;  }
			}
			Error("LDR: Rejecting connection from %s (not in server "
				"net list)\n",addr.GetAddress().str());
			shutdown(as,2);  close(as);
			return(0);
			adr_okay:;
		}
		
		// We will NOT generate new TaskSource_LDR_ServerConn's here in 
		// case we're recovering. 
		if(recovering || ts_quit)
		{
			Warning("LDR: Closig connection from %s: %s\n",
				addr.GetAddress().str(),
				ts_quit ? "quitting" : "currently recovering");
			shutdown(as,2);  close(as);
			return(0);
		}
		
		// Okay, we may receive a connection. 
		TaskSource_LDR_ServerConn *sc=NEW1<TaskSource_LDR_ServerConn>(this);
		if(sc && sc->Setup(as,&addr))
		{  sc->DeleteMe();  sc=NULL;  }
		if(!sc)
		{
			Error("LDR: Accept failed (%s).\n",cstrings.allocfail);
			close(as);
			return(0);
		}
		
		// Okay, accepted a connection: 
		Verbose(TSLLR,"LDR: Accepted connection from %s.\n",
			sc->addr.GetAddress().str());
		if(acc_sn)
		{  Verbose(TSLLR,"LDR:   Address matched server net: %s\n",
			p->ServerNetString(acc_sn).str());  }
		sconn.append(sc);
		return(0);
	}
	if(fdi->revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		Error("LDR: Unknown revents for listening socket %d: 0x%x. Aborting.\n",
			fdi->fd,int(fdi->revents));
		abort();  return(0);
	}
	
	return(0);
}


int TaskSource_LDR::timernotify(TimerInfo *ti)
{
	Verbose(DBGV,"--<TSLDR::timernotify>--\n");
	
	assert(ti->tid==rtid);
	
	_StopSchedTimer();
	
	// Do not enter for ADoneTask when done_task_status==2. 
	if(pending!=ANone && 
	   (pending!=ADoneTask || done_task_status!=2) )
	{
		// We may not call tsnotify() in case ts_quit is set (i.e. 
		// Disconnect(ts_quit) was called. This should not happen, 
		// though, because we reject commands once ts_quit is set. 
		assert(pending==ADisconnect || !ts_quit);   // Otherwise: BUG!
		
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
				ni.disconnstat=(ts_quit ? DSQuitting : DSOkay);
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
	else assert(active_taketask || recovering || ts_quit);
	
	// Okay, see if there is a special active thingy to do: 
	// YES, we must do that even if we're recovering==2 becuase otherwise 
	//      the task would not be given back correctly...
	if(active_taketask)
	{
		CompleteTask *ctask=active_taketask;
		active_taketask=NULL;
		
		// If the auth server no longer exists, then IT deleted the task 
		// and we may no longer use it. 
		TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
		if(srv)
		{
			assert(srv->_GetCurrTask()==ctask);
			
			if(ts_quit)
			{
				// Ugly...
				Warning("LDR: Got task [frame %d] while quitting. Bad...\n",
					ctask->frame_no);
				// So, this is a bit hacky: Tell the server that the manager 
				// has the task... and then siletly delete it... (below)
			}
			else
			{
				TSNotifyInfo ni;
				ni.action=AActive;
				ni.activestat=TASTakeTask;
				ni.ctsk=ctask;
				
				call_tsnotify(cclient,&ni);
			}
			
			// Must tell the server connection that the task was accepted. 
			srv->TaskManagerGotTask();
			
			if(ts_quit)
			{  delete ctask;  }
		}
		else assert(recovering==2 || ts_quit);
		// recovering must be 2 (i.e. recovering and the task manager is 
		// not yet informed) here, otherwise this is a bug and we 
		// started the schedule timer in order to pass a new task 
		// although we're already recovering. 
	}
	
	if(recovering==2)
	{
		assert(quit_reason);
		
		recovering=1;
		
		// Must tell server that we lost connection. Unless we shall quit. 
		if(!ts_quit)
		{
			TSNotifyInfo ni;
			ni.action=AActive;
			ni.activestat=(quit_reason==2) ? TASRecoveringQuit : TASRecoveringBad;
			
			call_tsnotify(cclient,&ni);
		}
	}
	
	if(ts_quit)
	{
		TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
		if(srv)
		{
			// Make sure we quit then...
			int rv=srv->QuitFromServerNow();
			// This will call ConnClose(reason=3) when done. 
			if(rv==1)  // already done
			{  srv=NULL;  }
			else
			{  Verbose(TSLLR,"LDR: Waiting for disconnect from server %s...\n",
				srv->addr.GetAddress().str());  }
		} // <-- NO else
		if(!srv)
		{
			TSNotifyInfo ni;
			ni.action=ADisconnect;
			ni.disconnstat=DSQuitOkay;
			
			call_tsnotify(cclient,&ni);
		}
	}
	
	return(0);
}


// overriding virtuals from TaskSource: 
int TaskSource_LDR::srcConnect(TaskSourceConsumer *cons)
{
	assert(cclient==cons);
	if(pending)  return(1);
	if(connected)  return(2);
	if(ts_quit)  return(3);
	
	// Okay, then let's connect...
	pending=AConnect;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcGetTask(TaskSourceConsumer * /*cons*/)
{
	if(recovering==1)
	{
		// This is special. It tells us that the TaskManager has 
		// finished recovery and is ready for work again. 
		
		LMallocUsage lmu;
		LMallocGetUsage(&lmu);
		
		Verbose(TSLLR,"LDR: Recovery finished. Ready again. "
			"(Alloc: %u bytes in %d chunks)\n",
			lmu.curr_used,lmu.used_chunks);
		recovering=0;
		quit_reason=0;
		ts_quit=0;
		return(0);
	}
	
	if(pending)  return(1);
	if(!connected)  return(2);
	if(ts_quit)  return(3);
	
	// LDR task source uses tsnotify(TASTakeTask) instead of srcGetTask(). 
	fprintf(stderr,"OOPS: GetTask called for LDR task source.\n");
	assert(0);
	return(-1);  // <-- (invalid code)
}


int TaskSource_LDR::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	assert(cclient==cons);
	if(pending)  return(1);
	if(!connected)  return(2);
	if(ts_quit)  return(3);
	
	assert(!current_done_task);
	assert(done_task_status==0);
	
	current_done_task=ct;
	done_task_status=1;
	
	pending=ADoneTask;
	_StartSchedTimer();
	return(0);
}


int TaskSource_LDR::srcDisconnect(TaskSourceConsumer *cons,int special)
{
	assert(cclient==cons);
	
	if(special==2)
	{
		// This means: do not report more tasks 
		// We just tell the server and hope that it does not 
		// send more tasks. If it does... oh well, that won't 
		// happen... After all, we give them right back again 
		// in this case. 
		// THERE IS NO tsnotify() BACKCALL. 
		if(ts_quit)
		{  return(11);  }
		// In case we're recovering, there is no need to call 
		// this because we already lost connection to the server. 
		if(recovering)
		{  return(0);  }
	
		TaskSource_LDR_ServerConn *srv=GetAuthenticatedServer();
		if(srv)
		{  srv->DontWantMoreTasks();  }
		
		return(0);
	}
	
	if(pending)  return(1);
	if(!connected && !special)  return(2);
	
	if(special==1)  // quit
	{
		ts_quit=1;
		recovering=1;
	}
	else if(special!=0)
	{  return(10);  }
	
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
	assert((!srv || srv==sc) && !recovering && !ts_quit);
	
	// Put the authenticated server at the beginning of the list: 
	sconn.dequeue(sc);
	sconn.insert(sc);
	
	// Tell TaskManager for some fancy output: 
	component_db->taskmanager()->PrintWorkCycleStart();
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
	done_task_status=0;
	connected=0;
	recovering=0;
	quit_reason=0;
	ts_quit=0;
	
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
	
	p->DeleteWorkingDir();
	
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
