/*
 * local.cpp
 * 
 * Implementation of local task source. 
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

#include "local.hpp"
#include "param.hpp"

#include "../taskfile.hpp"

#include <assert.h>


inline void TaskSource_Local::_Start0msecTimer()
{
	UpdateTimer(rtid,0,0);
}

inline void TaskSource_Local::_Stop0msecTimer()
{
	UpdateTimer(rtid,-1,0);
}


void TaskSource_Local::_ProcessGetTask(TSNotifyInfo *ni)
{
	if(next_frame_no>=p->nframes)
	{   ni->getstat=GTSNoMoreTasks;  return;  }
	
	CompleteTask *ctsk=NEW<CompleteTask>();
	if(!ctsk)
	{  ni->getstat=GTSAllocFailed;  return;  }
	
	RenderTask *rt=NEW<RenderTask>();
	ctsk->rt=rt;
	
	rt->rdesc=component_db->FindRenderDescByName("povray3.1g");
	rt->width=320;
	rt->height=240;
	#warning FIXME: 
static ImageFormat oformat;
	oformat.fmtid=IF_PNG;  // use IF_PPM here for PPM
	oformat.bitspp=8;
	rt->oformat=&oformat;
	
	// This is ultra-ugly!! (because of the %d in frame_pattern)
	RefString intmp;
	intmp.sprintf(0,p->frame_pattern,next_frame_no);
	rt->infile=NEW2<TaskFile>(TaskFile::FTFrame,TaskFile::IOTRenderInput);
	rt->infile->SetHDPath(intmp);
	
	RefString outtmp;
	#warning need oformat->FilenameSuffix()
	outtmp.sprintf(0,"frame-image-%07d.png",next_frame_no);
	rt->outfile=NEW2<TaskFile>(TaskFile::FTImage,TaskFile::IOTRenderOutput);
	rt->outfile->SetHDPath(outtmp);
	
	ni->ctsk=ctsk;
	ni->getstat=GTSGotTask;
	
	++next_frame_no;
}


void TaskSource_Local::_ProcessDoneTask(TSNotifyInfo *ni)
{
	fprintf(stderr,"***DoneTask(%s)***\n",
		done_task->rt->infile->HDPath().str());
	
	// Deleting done_task is okay for DoneTask(). 
	delete done_task;
	done_task=NULL;
	
	ni->donestat=DTSOkay;
}


int TaskSource_Local::timernotify(TimerInfo *ti)
{
	assert(ti->tid==rtid);
	
	_Stop0msecTimer();
	
	TSNotifyInfo ni;
	ni.action=pending;
	
	switch(pending)
	{
		case ANone:
			// should never happen
			fprintf(stderr,"OOPS: local:%d:pending=ANone\n",__LINE__);
			abort();
			break;
		case AConnect:
			connected=1;
			// Okay, connected, right?
			ni.connstat=CSConnected;
			break;
		case AGetTask:
			_ProcessGetTask(&ni);
			break;
		case ADoneTask:
			_ProcessDoneTask(&ni);
			break;
		case ADisconnect:
			connected=0;
			// Of course, we disconnected...
			ni.disconnstat=DSOkay;
			break;
	}
	
	TaskSourceConsumer *client=cclient;
	// Reset cclient and action before calling the virtual 
	// function so that this function can call 
	// GetTask / DoneTask. 
	cclient=NULL;
	pending=ANone;
	
	call_tsnotify(client,&ni);
	
	return(0);
}


// overriding virtuals from TaskSource: 
int TaskSource_Local::srcConnect(TaskSourceConsumer *cons)
{
	if(connected)  return(2);
	
	// Okay, then let's connect...
	pending=AConnect;
	cclient=cons;
	_Start0msecTimer();
	return(0);
}


int TaskSource_Local::srcGetTask(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	pending=AGetTask;
	cclient=cons;
	_Start0msecTimer();
	return(0);
}


int TaskSource_Local::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	if(!connected)  return(2);
	
	pending=ADoneTask;
	cclient=cons;
	done_task=ct;
	_Start0msecTimer();
	return(0);
}


int TaskSource_Local::srcDisconnect(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	cclient=cons;
	_Start0msecTimer();
	return(0);
}


long TaskSource_Local::ConnectRetryMakesSense()
{
	// No, it makes absolutely no sense to re-try to connect to 
	// this task source. If it failed, it failed definitely. 
	return(0);
}


TaskSource_Local::TaskSource_Local(TaskSourceFactory_Local *tsf,int *failflag) : 
	TaskSource(tsf->component_db(),failflag),
	FDBase(failflag)
{
	p=tsf;
	pending=ANone;
	connected=0;
	done_task=NULL;
	
	next_frame_no=0;
	
	int failed=0;
	
	rtid=InstallTimer(-1,0);
	if(!rtid)
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSource_Local");  }
}

TaskSource_Local::~TaskSource_Local()
{
	assert(pending==ANone);
	assert(!connected);
	assert(!done_task);
}
