/*
 * dif_local.cpp
 * 
 * Local task driver interface for task manager (virtualisation). 
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

#include "../taskmanager.hpp"

#include <assert.h>


void TaskDriverInterface::NewTask_SetUpState(CompleteTask *ctsk)
{
	// First, set up state (is in TaskDone here as gotten from source): 
	if(ctsk->rt)
	{  ctsk->state=CompleteTask::ToBeRendered;  }
	else if(ctsk->ft)
	{  ctsk->state=CompleteTask::ToBeFiltered;  }
	else 
	{
		// This is an internal error. Task source may not 
		// return a task without anything to do (i.e. with 
		// rt and ft set to NULL). 
		assert(ctsk->rt || ctsk->ft);
	}
}


TaskDriverInterface::TaskDriverInterface(ComponentDataBase *cdb,int *failflag)
{
	_comp_db=cdb;
	
	todo_thresh_low=-1;  // initial value
	todo_thresh_high=-1;
	done_thresh_high=-1;
}

TaskDriverInterface::~TaskDriverInterface()
{
}
