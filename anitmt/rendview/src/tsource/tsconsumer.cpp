/*
 * tsconsumer.cpp
 * 
 * Task source consumer. 
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

#include "tasksource.hpp"

#include <assert.h>

static const char *tsr="Task source reports";
static const char *wfts="Waiting for task source";


void TaskSourceConsumer::_WriteErrCmd(const TSNotifyInfo *ni)
{
	static const char *fc="  Failed command:";
	
	switch(ni->cmd)
	{
		case EC_none:  assert(0);  break;
		case EC_connect:
			Error("%s connect(MISSING)\n",fc);
			break;
		case EC_read:
			Error("%s read(MISSING)\n",fc);
			break;
		case EC_write:
			Error("%s write(MISSING)\n",fc);
			break;
		default:  assert(0);  break;
	}
	
	Error("  Error code %d: %s\n",ni->cmd_errno,strerror(ni->cmd_errno));
}


void TaskSourceConsumer::_TSWriteError_Connect(const TSNotifyInfo *ni)
{
	switch(ni->connstat)
	{
		case CSNone:  assert(0);  break;   // ...not talking about connect...
		case CSWorking:
			Verbose(TSR0,"TS: %s to connect.\n",wfts);
			break;
		case CSConnected:
			Verbose(TSR0,"TS: Task source connected successfully.\n");
			break;
		case CSTimeout:
			Error("%s connect timeout.\n",tsr);
			break;
		case CSFailed:
			Error("%s failure to connect.\n",tsr);
			_WriteErrCmd(ni);
			break;
		default:  assert(0);  break;
	}
}


// Returns static data: 
char *TaskSourceConsumer::_TSTaskQueueStatStr(int special)
{
	TSDebugInfo info;
	int rv=tsGetDebugInfo(&info);
	// If this assert fails, you forgot to override virtual tsGetDebugInfo(). 
	assert(!rv);
	
	static char tmp[48];
	snprintf(tmp,48,"(task queue: todo: %d%s, done: %d)",
		info.todo_queue,special==1 ? "+1" : "",
		info.done_queue);
	
	return(tmp);
}


void TaskSourceConsumer::_TSWriteError_GetTask(const TSNotifyInfo *ni)
{
	switch(ni->getstat)
	{
		case GTSNone:  assert(0);  break;
		case GTSWorking:
			Verbose(TSR0,"TS: %s to retrieve task.\n",wfts);
			break;
		case GTSGotTask:
			Verbose(TSR1,"TS: Okay, got task [frame %d] from task source %s.\n",
				ni->ctsk->frame_no,_TSTaskQueueStatStr(/*special=*/1));
			break;
		case GTSAllocFailed:
			Error("%s allocation failure.\n",tsr);
			break;
		case GTSNoMoreTasks:
			Verbose(TSR1,"TS: %s no more available tasks %s.\n",
				tsr,_TSTaskQueueStatStr());
			break;
		case GTSEnoughTasks:
			Verbose(TSR1,"TS: thinks we already have enough tasks.\n");
			break;
		default:  assert(0);  break;
	}
}


void TaskSourceConsumer::_TSWriteError_DoneTask(const TSNotifyInfo *ni)
{
	switch(ni->donestat)
	{
		case DTSNone:  assert(0);  break;
		case DTSWorking:
			Verbose(TSR0,"TS: %s working on done task.\n",wfts);
			break;
		case DTSOkay:
			Verbose(TSR1,"TS: Okay, task source accepted done task %s.\n",
				_TSTaskQueueStatStr());
			break;
		default:  assert(0);  break;
	}
}


void TaskSourceConsumer::_TSWriteError_Disconnect(const TSNotifyInfo *ni)
{
	switch(ni->disconnstat)
	{
		case DSNone:  assert(0);  break;
		case DSWorking:
			Verbose(TSR0,"TS: %s to disconnect\n",wfts);
			break;
		case DSOkay:
			Verbose(TSR0,"TS: Okay, task source disconnected successfully.\n");
			break;
		default:  assert(0);  break;
	}
}


void TaskSourceConsumer::_TSWriteError_Active(const TSNotifyInfo *ni)
{
	switch(ni->activestat)
	{
		case TASNone:  assert(0);  break;
		case TASTakeTask:
			Verbose(TSR1,"TS: Okay, task source reports task [frame %d] %s.\n",
				ni->ctsk->frame_no,_TSTaskQueueStatStr(/*special=*/1));
			break;
		case TASRecoveringBad:
			Verbose(TSR1,"TS: Bad; lost connection to server; recovering %s.\n",
				_TSTaskQueueStatStr());
			break;
		case TASRecoveringQuit:
			Verbose(TSR1,"TS: Connection to server closed down; recovering %s.\n",
				_TSTaskQueueStatStr());
			break;
		default:  assert(0);  break;
	}
}


void TaskSourceConsumer::TSWriteError(const TSNotifyInfo *ni)
{
	switch(ni->action)
	{
		case ANone:  break;
		case AConnect:     _TSWriteError_Connect(ni);     break;
		case AGetTask:     _TSWriteError_GetTask(ni);     break;
		case ADoneTask:    _TSWriteError_DoneTask(ni);    break;
		case ADisconnect:  _TSWriteError_Disconnect(ni);  break;
		case AActive:      _TSWriteError_Active(ni);      break;
		default:  assert(0);  break;
	}
}


void TaskSourceConsumer::UseTaskSource(TaskSource *ts)
{
	// The SetPersistentConsumer() function has to be called for active 
	// task sources. Passive ones simply ignore it. 
	if(!ts && tsource)
	{  tsource->SetPersistentConsumer(NULL);  }
	tsource=ts;
	if(tsource)
	{  tsource->SetPersistentConsumer(this);  }
}
