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

// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// Pass -1 if re-rendering message shall not be written on error. 
static int _FirstIsNewer(RefString *a,RefString *b,int frame_no)
{
	struct stat a_st,b_st;
	const char *sfile=a->str();
	if(stat(sfile,&a_st))  goto error;
	sfile=b->str();
	if(stat(sfile,&b_st))  goto error;
	return((a_st.st_mtime>b_st.st_mtime) ? 1 : 0);
error:;
	// ...or should this be a warning?
	Error("Local: stat() failed on \"%s\": %s\n",
		sfile,strerror(errno));
	if(frame_no>=0)
	{  Warning("Local:   (rendering frame %d to be sure but expect trouble)\n",
		frame_no);  }
	return(-1);
}


static void _DeleteFile(RefString path,int may_not_exist,
	const char *desc)
{
	if(!path.str())  return;
	if(!unlink(path.str()))
	{
		Verbose("Local: Deleted %s: %s\n",desc,path.str());
		return;
	}
	if(errno==ENOENT && may_not_exist)  return;
	Warning("Failed to unlink \"%s\": %s\n",path.str(),strerror(errno));
}

// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
static int _RenameFile(RefString old_name,RefString new_name,int may_not_exist)
{
	if(!old_name.str() || !new_name.str())  return(-1);
	if(!rename(old_name.str(),new_name.str()))
	{
		Verbose("Local: Renamed unfinished frame: %s -> %s\n",
			old_name.str(),new_name.str());
		return(0);
	}
	if(errno==ENOENT && may_not_exist)  return(1);
	Warning("Local: Failed to rename \"%s\" -> \"%s\": %s\n",
		old_name.str(),new_name.str(),strerror(errno));
	return(-1);
}

static int _UnfinishedName(RefString *f)
{
	return(f->append("-unfinished"));
}


// Return value: 
//  0 -> okay, render this frame. 
//  1 -> no more tasks
// -1 -> alloc failure
int TaskSource_Local::_GetNextFiles(RefString *inf,RefString *outf,
	int *resume_flag)
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
		
		// Check if unfinished frame exists (always do that): 
		RefString unf_tmp;
		unf_tmp=*outf;
		if(_UnfinishedName(&unf_tmp))  return(-1);
		int o_unf_exists=_CheckExist(&unf_tmp,1);
		
		// Okay, now some logic: 
		char unf_action='\0';  // rename, delete, <nothing>
		char frame_action='\0'; // skip, render, continue
		if(p->cont_flag)
		{
			// Check if finished frame exists: 
			int o_exists=_CheckExist(outf,1);
			
			// Okay, render resume switched on. If the file to resume 
			// exists, rename it back to the original file name: 
			if(p->render_resume_flag)
			{
				if(o_unf_exists && o_exists)
				{
					// Check which one is the newer one: 
					int rv=_FirstIsNewer(&unf_tmp,outf,next_frame_no);
					if(rv==1)   // unf_tmp is newer. Take it. 
					{  unf_action='r';  frame_action='c';  }  // rename
					else if(rv==0)  // *outf is newer; skip frame, delete unfinished: 
					{  unf_action='d';  frame_action='s';  }
					else  // Error. Delete unfinished, (re)render 
					{  unf_action='d';  frame_action='r';  }
				}
				else if(o_unf_exists)
				{  unf_action='r';  frame_action='c';  }
			}
			else
			{
				// If render_resume_flag (rcont) is switched off, we delete the 
				// unfinished file: 
				if(o_unf_exists)
				{  unf_action='d';  }
			}
			// This is the same for cont with and without render resume: 
			if(frame_action=='\0')
			{
				if(o_exists)
				{
					// See if we re-render it: 
					if(_FirstIsNewer(inf,outf,next_frame_no)!=0)  // inf newer or error
					{  frame_action='r';  }
					else
					{  frame_action='s';  }
				}
				else
				{  frame_action='r';  }
			}
		}
		else
		{  frame_action='r';  }
		
		// Okay, do it: 
		if(unf_action=='r')  // rename
		{
			unf_action='\0';
			int rv=_RenameFile(unf_tmp,*outf,0);
			if(rv)  // failed? - then (try to) delete it: 
			{  unf_action='d';  }
		}
		if(unf_action=='d')
		{
			_DeleteFile(unf_tmp,0,"unfinished frame file");
			unf_action='\0';
		}
		assert(unf_action=='\0');
		
		// Check what to do with the frame: 
		if(frame_action=='r' || frame_action=='c')
		{
			Verbose("Local: %s frame %d (job %s -> %s).\n",
				(frame_action=='r') ? 
					"Completely rendering" : "Continuing to render",
				next_frame_no,inf->str(),outf->str());
			if(frame_action=='c')
			{  *resume_flag=1;  }
			break;  // render frame
		}
		if(frame_action=='s')
		{
			Verbose("Local: Not re-rendering frame %d (job %s -> %s).\n",
				next_frame_no,inf->str(),outf->str());
		}
		else assert(0);
		
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
	
	int resume=0;
	int rv=_GetNextFiles(&intmp,&outtmp,&resume);
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
		
		rt->resume=resume;
		
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
	
	// See if successful: 
	if(done_task->rt && done_task->rtes.status!=TTR_Unset && 
	   done_task->rtes.status!=TTR_Success )
	{
		// Render task was not successful. 
		// File must get special treatment: 
		// Delete or rename it if dest file is there: 
		if(done_task->rt->outfile)
		{
			int remove=1;  // YES!
			if(p->render_resume_flag)
			{
				int fail=0;
				RefString tmp(&fail);
				if(!fail)  tmp=done_task->rt->outfile->HDPath();
				if(!fail)  fail=_UnfinishedName(&tmp);
				if(!fail)  fail=(_RenameFile(done_task->rt->outfile->HDPath(),tmp,1)<0);
				if(!fail)  remove=0;
			}
			if(remove)
			{  _DeleteFile(done_task->rt->outfile->HDPath(),1,
				"output file of failed task");  }
		}
	}
	
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
