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
	if(cclient && tstype!=TST_Active)  return(1);
	
	return(srcConnect(tsc));
}

int TaskSource::GetTask(TaskSourceConsumer *tsc)
{
	if(!tsc)  return(0);
	if(cclient && tstype!=TST_Active)  return(1);
	
	return(srcGetTask(tsc));
}

int TaskSource::DoneTask(TaskSourceConsumer *tsc,CompleteTask *ct)
{
	if(!tsc)  return(0);
	if(cclient && tstype!=TST_Active)  return(1);
	if(!ct)  return(10);
	
	return(srcDoneTask(tsc,ct));
}

int TaskSource::Disconnect(TaskSourceConsumer *tsc)
{
	if(!tsc)  return(0);
	if(cclient && tstype!=TST_Active)  return(1);
	
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
	tstype=TST_Passive;  // pre-set default
	component_db=cdb;
	cclient=NULL;
}

TaskSource::~TaskSource()
{
	assert(!cclient);
}


/******************************************************************************/

const char *CompleteTask::StateString(State s)
{
	switch(s)
	{
		case TaskDone:  return("done");
		case ToBeRendered:  return("to be rendered");
		case ToBeFiltered:  return("to be filtered");
		// default: see below
	}
	return("???");
}

CompleteTask::CompleteTask(int * failflag) : 
	LinkedListBase<CompleteTask>(),
	rtes(failflag),
	ftes(failflag)
{
	state=TaskDone;  // okay. 
	frame_no=-1;
	task_id=0;
	d.td=NULL;
	d.ldrc=NULL;
	assert(!d.any());
	rt=NULL;   ft=NULL;
	rtp=NULL;  ftp=NULL;
	radd.nfiles=0;   fadd.nfiles=0;
	radd.file=NULL;  fadd.file=NULL;
}

CompleteTask::~CompleteTask()
{
	assert(!d.any());
	DELETE(rt);
	DELETE(ft);
	DELETE(rtp);
	DELETE(ftp);
	assert(rt==NULL);  // can be removed 
	LFree(radd.file);  radd.file=NULL;
	LFree(fadd.file);  fadd.file=NULL;
}


CompleteTask::TES::TES(int *failflag) : 
	tes(failflag),
	processed_by(failflag)
{
	
}

/******************************************************************************/

TaskSource::TSNotifyInfo::TSNotifyInfo(/*int *failflag*/)
{
	action=ANone;
	
	connstat=CSNone;
	getstat=GTSNone;
	donestat=DTSNone;
	disconnstat=DSNone;
	activestat=TASNone;
	
	ctsk=NULL;
	
	cmd=EC_none;
	cmd_errno=0;
}
