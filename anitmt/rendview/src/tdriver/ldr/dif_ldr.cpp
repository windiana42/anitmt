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

#include <assert.h>


void TaskDriverInterface::StopContTasks(int signo)
{
	// send signal to the tasks: 
	Warning("Sending %s to all clients... ",
		signo==SIGCONT ? "CONT" : "STOP");
	
	for(TaskDriver *i=clientlist.first(); i; i=clientlist.next(i))
	{
		i->ActuallySendCommand();
	}
	
	#error missing: success info (no of clients)
}

// Schedule SIGTERM & SIGKILL on all jobs. 
// Returns number of killed jobs. 
int TaskDriverInterface::TermAllJobs(int reason)
{
	// Schedule TERM & KILL for all tasks. 
	Warning("Scheduling TERM & KILL on all jobs...\n");
	
	int nkilled=0;
	for(TaskDriver *i=joblist.first(); i; i=joblist.next(i))
	{
		if(i->pinfo.pid<0)  continue;
		int rv=i->KillProcess(reason);
		if(!rv)
		{  ++nkilled;  }
	}
	
	return(nkilled);
}


// Actually launch a job for a task. No check if we may do that. 
int TaskDriverInterface::LaunchTask(CompleteTask *ctsk)
{
	SendTaskToClient();
}


// Called for every new task obtained from TaskSource: 
int TaskDriverInterface::DealWithNewTask(CompleteTask *ctsk)
{
	DoIt();
}


// Decide on task to start and return it. 
// Return NULL if there is no task to start. 
CompleteTask *TaskDriverInterface::GetTaskToStart(
	LinkedList<CompleteTask> *tasklist_todo,int schedule_quit)
{
	Decide();
}


// These are called by the constructor/destructor of TaskDriver: 
int TaskDriverInterface::RegisterTaskDriver(TaskDriver *td)
{
	if(!td)  return(0);
	
	// Check if queued: 
	if(joblist.prev(td) || td==joblist.first())  return(1);
	
	joblist.append(td);
	
	
	return(0);
}


void TaskDriverInterface::UnregisterLDRClient(TaskDriver *client)
{
	if(!client)  return;
	// Check if queued: 
	if(!clientlist.prev(td) && client!=clientlist.first())  return;
	
	// Dequeue client: 
	clientlist.dequeue(td);
	
	component_db()->taskmanager()->CheckStartNewJobs();
}


TaskDriverInterface::TaskDriverInterface(ComponentDataBase *cdb,int *failflag) : 
	clientlist(failflag)
{
	_comp_db=cdb;
}

TaskDriverInterface::~TaskDriverInterface()
{
	assert(clientlist.is_empty());
}
