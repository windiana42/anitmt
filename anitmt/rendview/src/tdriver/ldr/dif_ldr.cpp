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

#include "taskmanager.hpp"

#include "dif_ldr.hpp"
#include "dif_param.hpp"

#include <assert.h>


int TaskDriverInterface_LDR::AreThereJobsRunning()
{
	assert(0);
	return(0);
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
	//SendTaskToClient();
	assert(0 && ctsk);
}


// Called for every new task obtained from TaskSource: 
int TaskDriverInterface_LDR::DealWithNewTask(CompleteTask *ctsk)
{
	assert(0 && ctsk);
}


// Decide on task to start and return it. 
// Return NULL if there is no task to start. 
CompleteTask *TaskDriverInterface_LDR::GetTaskToStart(
	LinkedList<CompleteTask> *tasklist_todo,int schedule_quit)
{
	assert(0 && tasklist_todo && schedule_quit);
}


void TaskDriverInterface_LDR::_WriteStartProcInfo(const char *msg)
{
	// Write out useful verbose information: 
	VerboseSpecial("Okay, %s work: %d parallel tasks on %d clients",
		msg,njobs,nclients);
	
	Verbose("  task-thresh: low=%d, high=%d\n",
		todo_thresh_low,todo_thresh_high);
	
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
	if(!clientlist.prev(client) && client!=clientlist.first())  return;
	
	// Dequeue client: 
	clientlist.dequeue(client);
	
	component_db()->taskmanager()->CheckStartNewJobs();
}


TaskDriverInterface_LDR::TaskDriverInterface_LDR(
	TaskDriverInterfaceFactory_LDR *f,int *failflag) : 
	TaskDriverInterface(f->component_db(),failflag),
	FDBase(failflag),
	FDCopyBase(failflag),
	clientlist(failflag)
{
	p=f;
	
	// Initial vals: 
	nclients=0;
	njobs=0;
	
	todo_thresh_low=p->thresh_param_low;
	todo_thresh_high=p->thresh_param_high;
	
	
}

TaskDriverInterface_LDR::~TaskDriverInterface_LDR()
{
	assert(clientlist.is_empty());
}
