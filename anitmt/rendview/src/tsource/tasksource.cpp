/*
 * tasksource.cpp
 * 
 * Task source virtualisation. 
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


int TaskSource::Connect(TaskSourceConsumer *tsc)
{
	if(!tsc)  return(0);
	if(cclient)  return(1);
	
	return(srcConnect(tsc));
}

int TaskSource::GetTask(TaskSourceConsumer *tsc)
{
	if(!tsc)  return(0);
	if(cclient)  return(1);
	
	return(srcGetTask(tsc));
}

int TaskSource::DoneTask(TaskSourceConsumer *tsc,CompleteTask *ct)
{
	if(!tsc)  return(0);
	if(cclient)  return(1);
	if(!ct)  return(10);
	
	return(srcDoneTask(tsc,ct));
}

int TaskSource::Disconnect(TaskSourceConsumer *tsc)
{
	if(!tsc)  return(0);
	if(cclient)  return(1);
	
	return(srcDisconnect(tsc));
}


int TaskSource::call_tsnotify(TaskSourceConsumer *cons,TSNotifyInfo *ni)
{
	assert(cons);
	assert(ni);
	assert(ni->action!=ANone);
	return(cons->tsnotify(ni));
}


TaskSource::TaskSource(ComponentDataBase *cdb,int * /*failflag*/)
{
	component_db=cdb;
	cclient=NULL;
}

TaskSource::~TaskSource()
{
	assert(!cclient);
}


/******************************************************************************/

CompleteTask::CompleteTask(int * /*failflag*/) : 
	LinkedListBase<CompleteTask>()
{
	state=TaskDone;  // okay. 
	td=NULL;
	rt=NULL;   ft=NULL;
	ftp=NULL;  ftp=NULL;
}

CompleteTask::~CompleteTask()
{
	assert(!td);
	if(rt)   {  delete rt;   rt=NULL;   }
	if(ft)   {  delete ft;   ft=NULL;   }
	if(rtp)  {  delete rtp;  rtp=NULL;  }
	if(ftp)  {  delete ftp;  ftp=NULL;  }
}


/******************************************************************************/

TaskSource::TSNotifyInfo::TSNotifyInfo(/*int *failflag*/)
{
	action=ANone;
	
	connstat=CSNone;
	getstat=GTSNone;
	donestat=DTSNone;
	disconnstat=DSNone;
	
	ctsk=NULL;
	
	cmd=EC_none;
	cmd_errno=0;
}
