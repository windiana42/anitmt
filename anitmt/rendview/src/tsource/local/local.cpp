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
#include <unistd.h>
#include <sys/stat.h>


inline void TaskSource_Local::_Start0msecTimer()
{
	// Well, actually, these timers need not be 0msec. 
	// It's an easy simultation of task sources which tend to not return 
	// immedialtely to use a little delay: 
	UpdateTimer(rtid,p->response_delay,0);
}

inline void TaskSource_Local::_Stop0msecTimer()
{
	UpdateTimer(rtid,-1,0);
}


// Check if a file exists and we have the passed permissions: 
static int _CheckExist(RefString *f,int want_read,int want_write=0)
{
	if(!f->str())
	{  return(0);  }
	int af=0;
	if(want_read)  af|=R_OK;
	if(want_write)  af|=W_OK;
	if(!access(f->str(),af))
	{  return(1);  }
	return(0);
}

// Check if in is newer than out or out does not exist. 
// Return value: 
//  1 -> redo
//  0 -> dont redo
// -1 -> stat failed
static int _CheckIfRedo(RefString *in,RefString *out)
{
	// Check if output exists: 
	if(!_CheckExist(out,0))
	{  return(1);  }
	
	// Input and output exist. Check time stamps: 
	struct stat in_st,out_st;
	if(stat(in->str(),&in_st))  return(-1);
	if(stat(out->str(),&out_st))  return(-1);
	
	return((in_st.st_mtime>out_st.st_mtime) ? 1 : 0);
}


// Return value: 
//  0 -> okay, render this frame. 
//  1 -> no more tasks
// -1 -> alloc failure
int TaskSource_Local::_GetNextFiles(RefString *inf,RefString *outf)
{
	int nonexist_in_seq=0;
	
	for(;;next_frame_no+=p->fjump)
	{
		if(p->nframes>=0)  // <0 -> unlimited
		{
			if( (p->fjump>0 && next_frame_no>=p->nframes+p->startframe) || 
			    (p->fjump<0 && next_frame_no<p->startframe ) )
			{  return(1);  }
		}
		
		if(inf->sprintf(0,p->inp_frame_pattern,next_frame_no))  return(-1);
		
		if(outf->sprintf(0,p->outp_frame_pattern,next_frame_no))  return(-1);
		
		// Check if *inf exists: 
		if(!_CheckExist(inf,1))
		{
			Error("Local: Access check failed on input file \"%s\": %s\n",
				inf->str(),strerror(errno));
			++nonexist_in_seq;
			if(nonexist_in_seq>=3)
			{
				Error("Local: Access check failed for last %d files. "
					"Assuming no more files (done).\n",nonexist_in_seq);
				return(1);
			}
			continue;
		}
		
		if(p->cont_flag)
		{
			// Okay, check if we must re-render the frame: 
			int rv=_CheckIfRedo(inf,outf);
			if(rv==1)  break;  // render frame
			if(rv==-1)
			{
				Error("Local: stat() failed on \"%s\" -> \"%s\": %s\n",
					inf->str(),outf->str(),strerror(errno));
				Error("Local:   (rendering frame to be sure but expect trouble)\n");
				break;
			}
			Verbose("Local: Not re-rendering job %s -> %s\n",
				inf->str(),outf->str());
		}
		else  break;  // render frame
		
		// Switch on one frame: done in for() statement. 
	}
	return(0);
}


void TaskSource_Local::_ProcessGetTask(TSNotifyInfo *ni)
{
	int fflag=0;
	RefString intmp(&fflag),outtmp(&fflag);
	if(fflag)
	{  ni->getstat=GTSAllocFailed;  return;  }
	
	int rv=_GetNextFiles(&intmp,&outtmp);
	switch(rv)
	{
		case -1:  ni->getstat=GTSAllocFailed;  return;
		case  0:  break;
		case  1:  ni->getstat=GTSNoMoreTasks;  return;
	}
	
	CompleteTask *ctsk=NULL;
	do {
		ctsk=NEW<CompleteTask>();
		if(!ctsk)  break;
		
		ctsk->frame_no=next_frame_no;
		
		RenderTask *rt=NEW<RenderTask>();
		if(!rt)  break;
		ctsk->rt=rt;
		
		rt->rdesc=p->rdesc;
		assert(p->rdesc);   // Otherwise FinalInit() should have failed. 
		rt->width=p->width;
		rt->height=p->height;
		rt->oformat=p->oformat;
		
		rt->infile=NEW2<TaskFile>(TaskFile::FTFrame,TaskFile::IOTRenderInput);
		if(!rt->infile)  break;
		rt->infile->SetHDPath(intmp);
		
		rt->outfile=NEW2<TaskFile>(TaskFile::FTImage,TaskFile::IOTRenderOutput);
		if(!rt->outfile)  break;
		rt->outfile->SetHDPath(outtmp);
		
		if(rt->add_args.append(&p->radd_args))  break;
		
		ni->ctsk=ctsk;
		ni->getstat=GTSGotTask;
		
		// Okay, nothing failed till now. 
		// Switch on one frame: 
		next_frame_no+=p->fjump;
		return;
	} while(0);
	
	// If we reach here, allocation failed. 
	ni->getstat=GTSAllocFailed;
	
	// Free stuff again: 
	delete ctsk;
	// This is actually enough. All the other data was on the stack 
	// of is deleted by the destructor of CompleteTask (or subsequent call 
	// to destructor of RenderTask or FilterTask). 
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
	
	if(p->fjump>0)
	{  next_frame_no=p->startframe;  }
	else
	{
		assert(p->nframes>=0);  // checked by TaskSourceFactory_Local::CheckParams()
		// Make sure we stop at start frame: 
		int jv=-p->fjump;
		int njumps=(p->nframes+jv-1)/jv - 1;
		next_frame_no = p->startframe + njumps*jv;
	}
	
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
