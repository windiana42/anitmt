/*
 * tasksource.cpp
 * 
 * Task source virtualisation. 
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

#include "tasksource.hpp"

#include <assert.h>

const char *TaskSource_NAMESPACE::ClientControlCommandString(
	ClientControlCommand cccmd)
{
	switch(cccmd)
	{
		case CCC_None:                return("[none]");
		case CCC_Kill_UserInterrupt:  return("kill jobs (intr)");
		case CCC_Kill_ServerError:    return("kill jobs (error)");
		case CCC_StopJobs:            return("stop jobs");
		case CCC_ContJobs:            return("cont jobs");
		// default: fall through
	}
	return("???");
}

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

int TaskSource::Disconnect(TaskSourceConsumer *tsc,int special)
{
	if(!tsc)  return(0);
	if(cclient && tstype!=TST_Active)  return(1);
	
	return(srcDisconnect(tsc,special));
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
	
	cccmd=CCC_None;
	
	int_payload=0;
}
