/*
 * dif_ldr.cpp
 * 
 * Local distributed rendering (LDR) server task driver interface 
 * for task manager. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "../../taskmanager.hpp"

#include "dif_ldr.hpp"
#include "dif_param.hpp"
#include "ldrclient.hpp"

#include <assert.h>

#include <lib/ldrproto.hpp>

using namespace LDR;

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


// Not inline because virtual: 
int TaskDriverInterface_LDR::Get_njobs()
{
	return(njobs);
}

		
int TaskDriverInterface_LDR::AreThereJobsRunning()
{
	// We return 0 here only if we are not connected to any client 
	// any longer. In this case, the TaskManager may quit. 
	if(nclients || !clientlist.is_empty())
	{  return(1);  }
	return(0);
}


// Called when everything is done to disconnect from the clients. 
// Local interface can handle that quickly. 
void TaskDriverInterface_LDR::PleaseQuit()
{
	if(shall_quit)  return;
	
	// MUST BE SET before the for()-loop: 
	shall_quit=1;
	
	if(clientlist.is_empty())
	{
		// Do it immediately if there are no clients to disconnect. 
		component_db()->taskmanager()->CheckStartNewJobs(/*special=*/-1);
		return;
	}
	
	Verbose(TDR,"Disconnecting from clients (%d usable)...\n",nclients);
	
	// Schedule disconnect from all clients: 
	for(LDRClient *_i=clientlist.first(); _i; )
	{
		LDRClient *i=_i;
		_i=_i->next;
		if(i->Disconnect())
		{
			// Already disconnected or not connected. 
			delete i;  // NO NEED TO DEQUEUE. 
		}
	}
}


void TaskDriverInterface_LDR::StopContTasks(int signo)
{
	assert(0);
	
	// send signal to the tasks: 
	Warning("Sending %s to all clients... ",
		signo==SIGCONT ? "CONT" : "STOP");
	
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		//i->ActuallySendCommand();
	}
	
	// FIXME: missing: success info (no of clients stopped)
}

// Schedule SIGTERM & SIGKILL on all jobs. 
// Returns number of killed jobs. 
int TaskDriverInterface_LDR::TermAllJobs(int reason)
{
	assert(0 && reason);
	
	// Schedule TERM & KILL for all tasks. 
	Warning("Scheduling TERM & KILL on all jobs...\n");
	
	int nkilled=0;
	for(LDRClient *i=clientlist.first(); i; i=clientlist.next(i))
	{
		//if(i->pinfo.pid<0)  continue;
		//int rv=i->KillProcess(reason);
		//if(!rv)
		{  ++nkilled;  }
	}
	
	return(nkilled);
}


// Actually launch a job for a task. No check if we may do that. 
int TaskDriverInterface_LDR::LaunchTask(CompleteTask *ctsk)
{
	// Otherwise GetTaskToStart() has a bug: 
	assert(ctsk && (ctsk->rt || ctsk->ft));   
	
	LDRClient *c=ctsk->d.ldrc;
	assert(c);  // Otherwise bug near/in GetTaskToStart(). 
	
	int rv=c->SendTaskToClient(ctsk);
	if(rv<0)
	{
	}
	
	assert(0 && ctsk);
}


// Called for every new task obtained from TaskSource: 
int TaskDriverInterface_LDR::DealWithNewTask(CompleteTask *ctsk)
{
	// First, set up ctsk->state: 
	TaskDriverInterface::NewTask_SetUpState(ctsk);
	
	TaskDriverInterfaceFactory_LDR::DTPrm *prm=p->prm;
	
	// ctsk->rtp and ctsk->ftp is NULL here. 
	
	// Okay, RenderTask and FilterTask (derived from TaskStructBase) 
	// are the two structures which are passed from the task source to 
	// the task driver (interface); {Render,Filter}TaskParams come from 
	// the TaskDriver(Interface). 
	// So, we cannot pass {Render,Filter}TaskParams to the LDR client 
	// but only the TaskStructBase-derived classes. 
	// Ergo: no need to set up ctsk->rtp, ctsk->ftp. 
	
	// However, we must adjust the timeouts in RenderTask/FilterTask. 
	
	for(int i=0; i<_DTLast; i++)
	{
		TaskStructBase *tsb=NULL;
		switch(i)
		{
			case DTRender:  tsb=ctsk->rt;  break;
			case DTFilter:  tsb=ctsk->ft;  break;
			default:  assert(0);  break;
		}
		
		if(!tsb)  continue;
		
		TaskDriverInterfaceFactory_LDR::DTPrm *p=&prm[i];
		
		if(p->timeout>0 && tsb->timeout>0 && 
		   p->timeout<tsb->timeout )
		{  tsb->timeout=p->timeout;  }
	}
	
	return(0);
}


// Decide on task to start and return it. 
// Return NULL if there is no task to start. 
CompleteTask *TaskDriverInterface_LDR::GetTaskToStart(
	LinkedList<CompleteTask> *tasklist_todo,int schedule_quit)
{
	if(tasklist_todo->is_empty())
	{  return(NULL);  }
	
	// Well, that's simple for the LDR task driver: 
	// Just return the next task if there is a free client. 
	
	if(schedule_quit)
	{  return(NULL);  }
	
	// Find free client: 
	// NOTE: We use an LRU-like method here: We always check the client 
	//       list first-to-last if a client can do a task. If this client 
	//       got its task, then this client is re-queued at the end. 
	LDRClient *free_client=NULL;
	for(LDRClient *i=clientlist.first(); i; i=i->next)
	{
		if(i->CanDoTask())
		{  free_client=i;  break;  }
	}
	
	if(!free_client)
	{
fprintf(stderr,"TaskDriverInterface_LDR: ??? No free client!!!\n");
		return(NULL);
	}
	
	// Find task to be done: 
	CompleteTask *startme=NULL;
	for(CompleteTask *i=tasklist_todo->first(); i; i=i->next)
	{
		assert(i->state!=CompleteTask::TaskDone);
		if(i->d.any())  continue;  // task is currently processed 
		
		startme=i;
		startme->d.ldrc=free_client;   // for LaunchTask() etc. 
		break;
	}
	
if(!startme)
{  fprintf(stderr,"TaskDriverInterface_LDR: ??? No task todo!!!\n");  }
	
	return(startme);
}


void TaskDriverInterface_LDR::_WriteStartProcInfo(const char *msg)
{
	// Write out useful verbose information: 
	VerboseSpecial("Okay, %s work: %d parallel tasks on %d clients.",
		msg,njobs,nclients);
	
	Verbose(TDI,"  task-thresh: low=%d, high=%d\n",
		todo_thresh_low,todo_thresh_high);
	
}

void TaskDriverInterface_LDR::_WriteProcInfoUpdate()
{
	if(!shall_quit && already_started_processing)
	{
		VerboseSpecial("Update: %d parallel tasks on %d clients",
			njobs,nclients);
		Verbose(TDI,"  task-thresh: low=%d, high=%d\n",
			todo_thresh_low,todo_thresh_high);
	}
	if(shall_quit && clientlist.is_empty())
	{
		VerboseSpecial("Update: All clients disconnected.");
	}
}

void TaskDriverInterface_LDR::_WriteEndProcInfo()
{
}

void TaskDriverInterface_LDR::WriteProcessingInfo(int when,const char *msg)
{
	if(when==0)
	{  _WriteStartProcInfo(msg);  }
	else if(when==1)
	{  _WriteEndProcInfo();  }
	else assert(0);
}


void TaskDriverInterface_LDR::ReallyStartProcessing()
{
	// Okey, let's begin. We call TaskManager::ReallyStartProcessing() 
	// when the first client is connected. 
	
	// Start connect timeout (if needed): 
	if(p->connect_timeout>0)
	{
		HTime curr(HTime::Curr);
		curr.Add(p->connect_timeout,HTime::msec);
		UpdateTimeout(tid_connedt_to,&curr);
	}
	else
	{  Warning("Warning: connect timeout disabled.\n");  }
	
	{
		char tmp[32];
		if(p->connect_timeout>0)
		{  snprintf(tmp,32,"%ld msec",p->connect_timeout);  }
		else
		{  strcpy(tmp,"[disabled]");  }
		Verbose(TDR,"Simultaniously initiating connections to all clients "
			"(timeout: %s):\n",tmp);
	}
	
	int n_connecting=0;
	for(TaskDriverInterfaceFactory_LDR::ClientParam *i=p->cparam.first();
		i; i=i->next)
	{
		LDRClient *c=NEW1<LDRClient>(this);
		if(!c)
		{
			while(!clientlist.is_empty())
			{  delete clientlist.popfirst();  }
			
			Error("Failed to set up LDR client representation.\n");
			already_started_processing=1;
			component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
			return;
		}
		
		int rv=c->ConnectTo(i);
		if(rv<0)
		{  delete c;  c=NULL;  }
		else 
		{  ++n_connecting;  }
	}
	
	if(!n_connecting)
	{
		while(!clientlist.is_empty())
		{  delete clientlist.popfirst();  }
		
		Error("No usable clients left in list. Giving up.\n");
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
		return;
	}
	
	Verbose(TDR,"Waiting for %d LDR connections to establish.\n",n_connecting);
}


int TaskDriverInterface_LDR::fdnotify(FDInfo *fdi)
{
	assert(fdi->dptr);
	LDRClient *client=(LDRClient*)(fdi->dptr);
	assert(client->pollid==fdi->pollid);  // otherwise data corrupt
	
	client->fdnotify(fdi);
	
	return(0);
}


int TaskDriverInterface_LDR::cpnotify(CopyInfo *cpi)
{
	LDRClient *client=(LDRClient*)(cpi->pump->dptr);
	assert(client);
	
	client->cpnotify(cpi);
	
	return(0);
}


int TaskDriverInterface_LDR::timeoutnotify(TimeoutInfo *ti)
{
	int msg_written=0;
	for(LDRClient *_i=clientlist.first(); _i; )
	{
		LDRClient *client=_i;
		_i=_i->next;
		
		if(client->connected_state==0)  continue;  // not connecting & not connected 
		if(client->auth_passed || client->send_quit_cmd)  continue;
		
		if(!msg_written)
		{  Verbose(TDR,"Connection timeout expired:\n");  msg_written=1;  }
		
		// He has do die...
		Warning("  Timed out during %s: %s\n",
			client->connected_state==1 ? "connect" : "auth",
			client->_ClientName().str());
		if(client->Disconnect())
		{
			// Already disconnected: Delete; then destructor will 
			// unregister it: 
			delete client;
		}
	}
	
	return(0);
}


#if 0
#error HACK ME...
// dir: +1 -> buf -> FD; -1 -> FD -> buf
FDCopyBase::CopyID TaskDriverInterface_LDR::DoCopyFdBuf(LDRClient *client,
	int fd,char *buf,size_t len,int dir)
{
	CopyRequest req;
	
	if(dir<0)  // FD -> buf
	{
		req.srcfd=fd;
		req.destbuf=buf;
	}
	else if(dir>0)
	{
		req.destfd=fd;
		req.srcbuf=buf;
	}
	else assert(0);  // illegal direction
	
	req.len=len;
	req.dptr=client;
	
	#warning might tweak with req.bufxyz
	#warning disable NAGLE algorithm on network socket
	
	// Important flag because we may be using the FD, too. 
	// (It's a network socket after all and we want to go full duplex!). 
	req.recv_fdnotify=1;
	
	CopyID cpid=FDCopyBase::CopyFD(&req,/*FDBase=*/NULL);
	if(!cpid)
	{
		if(req.errcode==-1)
		{  return(NULL);  }
		assert(0);  // The rest are errors which may not happen. 
	}
	
	return(cpid);
}
#endif


FDBase::PollID TaskDriverInterface_LDR::PollFD_Init(LDRClient *client,int fd)
{
	PollID pollid;
	if(FDBase::PollFD(fd,POLLIN,client,&pollid)<0)
	{  pollid=NULL;  }
	return(pollid);
}


// Do njobs calculation because there are now more/less clients. 
// mode: +1 -> add; -1 -> subtract
void TaskDriverInterface_LDR::_JobsAddClient(LDRClient *client,int mode)
{
	int oldval=njobs;
	if(client->_counted_as_client && mode<0)
	{
		--nclients;
		client->_counted_as_client=0;
		njobs-=client->c_jobs;
	}
	if(!client->_counted_as_client && mode>0)
	{
		++nclients;
		client->_counted_as_client=1;
		njobs+=client->c_jobs;
	}
	
	todo_thresh_low= njobs+p->todo_thresh_reserved_min;
	todo_thresh_high=njobs+p->todo_thresh_reserved_max;
	
	#if TESTING
	int cnt=0;
	for(LDRClient *i=clientlist.first(); i; i=i->next)
	{  if(i->auth_passed)  ++cnt;  }
	assert(cnt==nclients);
	#endif
	
	// May be called if shall_quit=1. 
	if(already_started_processing)
	{  component_db()->taskmanager()->CheckStartNewJobs(
		(oldval!=njobs) ? 1 : 0);  }
}


void TaskDriverInterface_LDR::SuccessfullyConnected(LDRClient *client)
{
	assert(client->auth_passed);
	assert(!client->_counted_as_client);
	
	// First, adjust njobs and task queue threshs: 
	_JobsAddClient(client,+1);
	_WriteProcInfoUpdate();
	
	if(!already_started_processing)
	{
		// Great. We can start working. 
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/0);
	}
}


void TaskDriverInterface_LDR::FailedToConnect(LDRClient *client)
{
	delete client;
}


void TaskDriverInterface_LDR::ClientDisconnected(LDRClient *client)
{
	// Oh yes. That is simple. 
	delete client;
	#warning ...what about giving back tasks? (or am I missing sth?)
}


// These are called by the constructor/destructor of LDRClient: 
int TaskDriverInterface_LDR::RegisterLDRClient(LDRClient *client)
{
	if(!client)  return(0);
	
	// Check if queued: 
	if(clientlist.prev(client) || client==clientlist.first())  return(1);
	
	clientlist.append(client);
	
	
	return(0);
}


void TaskDriverInterface_LDR::UnregisterLDRClient(LDRClient *client)
{
	if(!client)  return;
	
	// Check if queued: 
	if(clientlist.prev(client) || client==clientlist.first())
	{
		// Dequeue client: 
		// MUST BE DONE before _JobsAddClient(). 
		clientlist.dequeue(client);
	}
	
	_JobsAddClient(client,-1);
	
	// Un-Set client's ClientParam: 
	client->cp=NULL;
	
	_WriteProcInfoUpdate();
	
	if(already_started_processing)
	{
		if(shall_quit && !nclients)
		{  component_db()->taskmanager()->CheckStartNewJobs(/*special=*/-1);  }
	}
	else if(clientlist.is_empty())
	{
		Error("No usable clients left in list. Giving up.\n");
		already_started_processing=1;
		component_db()->taskmanager()->ReallyStartProcessing(/*error=*/1);
	}
}


TaskDriverInterface_LDR::TaskDriverInterface_LDR(
	TaskDriverInterfaceFactory_LDR *f,int *failflag) : 
	TaskDriverInterface(f->component_db(),failflag),
	FDCopyBase(failflag),
	TimeoutBase(failflag),
	clientlist(failflag)
{
	p=f;
	
	// Initial vals: 
	nclients=0;
	njobs=0;  // NOT -1
	
	todo_thresh_low=p->todo_thresh_reserved_min;
	todo_thresh_high=p->todo_thresh_reserved_max;
	
	already_started_processing=0;
	shall_quit=0;
	
	int failed=0;
	
	tid_connedt_to=InstallTimeout(HTime(HTime::Invalid));
	if(!tid_connedt_to)  ++failed;
	
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverInterface_LDR");  }
}

TaskDriverInterface_LDR::~TaskDriverInterface_LDR()
{
	assert(clientlist.is_empty());
}
