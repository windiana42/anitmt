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

#include "../taskfile.hpp"
#include "../../taskmanager.hpp"

#include <assert.h>


inline void TaskSource_Local::_StartRTimer()
{
	// Well, actually, these timers need not be 0 msec. 
	// It's an easy simultation of task sources which tend to not return 
	// immedialtely to use a little delay: 
	UpdateTimer(rtid,p->response_delay,0);
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
	int rv=FirstFileIsNewer(a,b,"Local: ");
	if(rv<0 && frame_no>=0)
	{  Warning("Local:   (rendering frame %d to be sure but expect trouble)\n",
		frame_no);  }
	return(rv);
}


static void _DeleteFile(RefString path,int may_not_exist,const char *desc)
{
	int rv=DeleteFile(&path,may_not_exist,"Local: ");
	if(!rv)
	{  Verbose(TSLR,"Local: Deleted %s: %s\n",desc,path.str());  }
}

// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
static int _RenameFile(RefString old_name,RefString new_name,int may_not_exist)
{
	int rv=RenameFile(&old_name,&new_name,may_not_exist,"Local: ");
	if(!rv)
	{  Verbose(TSLR,"Local: Renamed unfinished frame: %s -> %s\n",
			old_name.str(),new_name.str());  }
	return(rv);
}


static int _UnfinishedName(RefString *f)
{
	return(f->append("-unfinished"));
}


// dtype: DTRender -> render job; DTFilter -> filter job. 
// Return value: 
//   0 -> OK (render/filter it) [or: check tobe_{rendered,filtered}!]
//   1 -> error; do not [render and also do not] filter it [if that is planned]. 
//   2 -> no more tasks (only DTRender) 
//  -1 -> allocation failure
int TaskSource_Local::_FillInJobFiles(TaskDriverType dtype,FrameToProcessInfo *ftpi)
{
	assert(dtype==DTRender || dtype==DTFilter);
	if(dtype==DTRender)
	{	assert(ftpi->fi && ftpi->fi->rdesc);  // else: we may not be here
		assert(ftpi->tobe_rendered);
	} else {
		assert(ftpi->fi && ftpi->fi->fdesc);  // else: we may not be here
		assert(ftpi->tobe_filtered);
	}
	
	const PerFrameTaskInfo *fi=ftpi->fi;
	
	// inf, outf: paths for RendView (rdir (to be) prepended)
	RefString inf,outf;
	if(dtype==DTRender)
	{
		if(ftpi->r_infile.sprintf(0,fi->rinfpattern,ftpi->frame_no))  return(-1);
		if(ftpi->r_outfile.sprintf(0,fi->routfpattern,ftpi->frame_no))  return(-1);
		inf=ftpi->r_infile;
		outf=ftpi->r_outfile;
	} else {
		if(ftpi->f_infile.sprintf(0,fi->routfpattern,ftpi->frame_no))  return(-1);
		if(ftpi->f_outfile.sprintf(0,fi->foutfpattern,ftpi->frame_no))  return(-1);
		inf=ftpi->f_infile;
		outf=ftpi->f_outfile;
	}
	assert(inf.str() && outf.str());  // otherwise we should have returned some lines above
	
	// Prepend rdir (DTRender) or rdir/fdir (DTFilter): 
	// NOTE!!! SIMILAR CODE IN param.cpp (if(nframes<0 && fjump<0))
	if(dtype==DTFilter)
	{
		if(fi->rdir.str())   // YES!!! THIS IS rdir NOT fdir!!!
		{
			assert(fi->rdir.str()[fi->rdir.len()-1]=='/');  // FinalInit() should append '/'
			// Prepend rdir if not absolute path: 
			if(inf.str()[0]!='/')
			{  if(inf.prepend(fi->rdir))  return(-1);  }
		}
		if(fi->fdir.str())
		{
			assert(fi->fdir.str()[fi->fdir.len()-1]=='/');  // FinalInit() should append '/'
			// Prepend fdir if not absolute path: 
			if(outf.str()[0]!='/')
			{  if(outf.prepend(fi->fdir))  return(-1);  }
		}
	}
	else if(/*dtype==DTRender && */ fi->rdir.str())
	{
		assert(fi->rdir.str()[fi->rdir.len()-1]=='/');  // FinalInit() should append '/'
		// Prepend rdir if not absolute path: 
		if(inf.str()[0]!='/')
		{  if(inf.prepend(fi->rdir))  return(-1);  }
		if(outf.str()[0]!='/')
		{  if(outf.prepend(fi->rdir))  return(-1);  }
	}
	
	// Check if inf exists: (Filter input file will most likely not 
	// yet exist.) 
	if(dtype==DTRender && !CheckExistFile(&inf,1))
	{
		Error("Local: Access check failed on %s input file \"%s\": %s\n",
			DTypeString(dtype),inf.str(),strerror(errno));
		++nonexist_in_seq;
		if(nonexist_in_seq>=3)
		{
			Error("Local: Access check failed for last %d files. "
				"Assuming no more files (done).\n",nonexist_in_seq);
			return(2);  // no more tasks
		}
		return(1);   // error
	}
	
	// Check if unfinished frame exists (always do that): 
	RefString unf_tmp;
	unf_tmp=outf;
	if(_UnfinishedName(&unf_tmp))  return(-1);
	int o_unf_exists=CheckExistFile(&unf_tmp,1);
	
	// Okay, now some logic: 
	char unf_action='\0';  // rename, delete, <nothing>
	char frame_action='\0'; // skip, render/filter, continue
	if(p->cont_flag)
	{
		// Check if finished frame exists: 
		int o_exists=CheckExistFile(&outf,1);
		
		if(dtype==DTRender)
		{
			if(fi->render_resume_flag)
			{
				// Okay, render resume switched on. If the file to resume 
				// exists, rename it back to the original file name: 
				if(o_unf_exists && o_exists)
				{
					// Check which one is the newer one: 
					int rv=_FirstIsNewer(&unf_tmp,&outf,ftpi->frame_no);
					if(rv==1)   // unf_tmp is newer. Take it. 
					{  unf_action='r';  frame_action='c';  }  // rename
					else if(rv==0)  // outf is newer; skip frame, delete unfinished: 
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
		}
		else
		{  fprintf(stderr,"*** hack unfinished file support for filter ***\n");  }
		
		// This is the same for cont with and without render resume: 
		if(frame_action=='\0')
		{
			if(!o_exists)
			{  frame_action='r';  }
			// See if we re-render it: 
			else if(_FirstIsNewer(&inf,&outf,ftpi->frame_no)!=0)  // inf newer or error
			{  frame_action='r';  }
			else
			{  frame_action='s';  }
		}
	}
	else
	{  frame_action='r';  }
	
	// Now, if we re-render a frame then we also have to re-filter 
	// it in any case: 
	if(dtype==DTFilter && ftpi->tobe_rendered)
	{  frame_action='r';  }
	
// Resume not implemented for filter. 
assert(dtype!=DTFilter || (frame_action!='c' && unf_action=='\0'));
	
	// Okay, do it: 
	if(unf_action=='r')  // rename
	{
		unf_action='\0';
		int rv=_RenameFile(unf_tmp,outf,0);
		if(rv)  // failed? - then (try to) delete it: 
		{  unf_action='d';  }
	}
	if(unf_action=='d')
	{
		_DeleteFile(unf_tmp,0,"unfinished frame file");
		unf_action='\0';
	}
	assert(unf_action=='\0');
	
	int retval=1;   // Will never be returned (due to assert(0) in else branch). 
	
	// Check what to do with the frame: 
	if(frame_action=='r' || frame_action=='c')
	{
		Verbose(TSLR,"Local: %s %s%s frame %d (job %s -> %s).\n",
			(frame_action=='r') ? "Completely" : "Continuing to",
			DTypeString(dtype),(frame_action=='r') ? "ing" : "",
			ftpi->frame_no,inf.str(),outf.str());
		if(frame_action=='c')
		{  ftpi->r_resume_flag=1;  }
		retval=0;  // render/filter frame
	}
	else if(frame_action=='s')
	{
		Verbose(TSLR,"Local: Not re-%sing frame %d (job %s -> %s).\n",
			DTypeString(dtype),ftpi->frame_no,inf.str(),outf.str());
		if(dtype==DTRender)  ftpi->tobe_rendered=0;
		else                 ftpi->tobe_filtered=0;
		retval=0;  // render/filter frame; nothing will be done as 
	}	           // tobe_rendered/filtered=0. 
	else assert(0);
	
	// Reset nonexistant file counter: 
	if(retval!=1 && dtype==DTRender)
	{  nonexist_in_seq=0;  }
	return(retval);
}


// Just fills in the correct frame number and tobe_rendered/filtered 
// flags of the next frame which either should be rendered or filtered. 
// No check if we actually have to do that (because of -cont or whatever) 
// and the paths are NOT filled in. 
// Return value: 
//  0 -> okay, this frame has to be processed. 
//  1 -> no more frames to process
int TaskSource_Local::_GetNextFrameToProcess(FrameToProcessInfo *ftpi)
{
	const PerFrameTaskInfo *fi;
	for(;;next_frame_no+=p->fjump)
	{
		fi=p->GetPerFrameTaskInfo(next_frame_no);
		if(!fi)  break;  // out of range; must stop: no more frames to process
		
		// See what has to be done: 
		ftpi->tobe_rendered=fi->rdesc ? 1 : 0;
		ftpi->tobe_filtered=fi->fdesc ? 1 : 0;
		if(!ftpi->tobe_rendered && !ftpi->tobe_filtered)
		{
			Warning("Local: Frame %d shall neither be rendered nor filtered.\n",
				next_frame_no);
			continue;
		}
		ftpi->fi=fi;
		ftpi->frame_no=next_frame_no;
		
		// Make sure no paths are left: 
		ftpi->r_infile.deref();
		ftpi->r_outfile.deref();
		ftpi->f_infile.deref();
		ftpi->f_outfile.deref();
		
		// Finally, switch on one frame: 
		next_frame_no+=p->fjump;
		return(0);
	}
	return(1);
}


// Return value: 
//  0 -> okay, render this frame. 
//  1 -> no more tasks
// -1 -> alloc failure
int TaskSource_Local::_GetNextFTPI_FillInFiles(FrameToProcessInfo *ftpi)
{
	for(;;)
	{
		int rv=_GetNextFrameToProcess(ftpi);
		if(rv==1)  // no more tasks
		{  return(1);  }
		
		if(ftpi->tobe_rendered)
		{
			rv=_FillInJobFiles(DTRender,ftpi);
			if(rv==2)  return(1);  // no more tasks
			if(rv==1)  continue;   // file not found error
			if(rv<0)  return(-1);  // alloc failure
		}
		
		if(ftpi->tobe_filtered)
		{
			rv=_FillInJobFiles(DTFilter,ftpi);
			assert(rv!=2);   // may never happen
			assert(rv!=1);   // file not found has no sense here and may not be returned. 
			if(rv<0)  return(-1);  // alloc failure
		}
		
		// Okay, see if we really have to do something: 
		if(!ftpi->tobe_rendered && !ftpi->tobe_filtered)
		{
			// No, okay, let's take next frame...
			continue;
		}
		
		// Well, we've found a frame to process. 
		break;
	}
	return(0);
}


void TaskSource_Local::_ProcessGetTask(TSNotifyInfo *ni)
{
	int fflag=0;
	FrameToProcessInfo ftpi(&fflag);
	if(fflag)
	{  ni->getstat=GTSAllocFailed;  return;  }
	
	switch(_GetNextFTPI_FillInFiles(&ftpi))
	{
		case -1:  ni->getstat=GTSAllocFailed;  return;
		case  0:  break;
		case  1:  ni->getstat=GTSNoMoreTasks;  return;
	}
	
	const PerFrameTaskInfo *fi=ftpi.fi;
	assert(fi);
	
	// Okay, set up a CompleteTask structure: 
	CompleteTask *ctsk=NULL;
	do {
		ctsk=NEW<CompleteTask>();
		if(!ctsk)  break;
		
		ctsk->frame_no=ftpi.frame_no;
		
		if(ftpi.tobe_rendered)
		{
			RenderTask *rt=NEW<RenderTask>();
			if(!rt)  break;
			ctsk->rt=rt;
			
			rt->rdesc=fi->rdesc;
			assert(fi->rdesc);   // Otherwise tobe_rendered may not be set
			rt->width=fi->width;
			rt->height=fi->height;
			rt->oformat=fi->oformat;
			rt->timeout=fi->rtimeout;
			
			rt->infile=NEW2<TaskFile>(TaskFile::FTFrame,TaskFile::IOTRenderInput);
			if(!rt->infile)  break;
			rt->infile->SetHDPath(ftpi.r_infile);
			
			rt->outfile=NEW2<TaskFile>(TaskFile::FTImage,TaskFile::IOTRenderOutput);
			if(!rt->outfile)  break;
			rt->outfile->SetHDPath(ftpi.r_outfile);
			
			if(rt->add_args.append(&fi->radd_args))  break;
			
			rt->wdir=fi->rdir;
			
			rt->resume=ftpi.r_resume_flag;
		}
		
		if(ftpi.tobe_filtered)
		{
			FilterTask *ft=NEW<FilterTask>();
			if(!ft)  break;
			ctsk->ft=ft;
			
			ft->fdesc=fi->fdesc;
			assert(fi->fdesc);   // Otherwise tobe_filtered may not be set
			ft->timeout=fi->ftimeout;
			
			ft->infile=NEW2<TaskFile>(TaskFile::FTImage,TaskFile::IOTFilterInput);
			if(!ft->infile)  break;
			ft->infile->SetHDPath(ftpi.f_infile);
			
			ft->outfile=NEW2<TaskFile>(TaskFile::FTImage,TaskFile::IOTFilterOutput);
			if(!ft->outfile)  break;
			ft->outfile->SetHDPath(ftpi.f_outfile);
			
			if(ft->add_args.append(&fi->fadd_args))  break;
			
			ft->wdir=fi->fdir;
		}
		
		ni->ctsk=ctsk;
		ni->getstat=GTSGotTask;
		
		// Okay, nothing failed till now. 
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
	const PerFrameTaskInfo *fi=p->GetPerFrameTaskInfo(done_task->frame_no);
	assert(fi);  // HUGE internal bug if that fails. 
	
	Verbose(TSLR,"  Done task [frame %d] was from per-frame block %s.\n",
		done_task->frame_no,p->_FrameInfoLocationString(fi));
	
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
			// We always delete it unless user interrupt or timeout 
			// was the reason for failure. 
			// And only if render_resume_flag (-rcont) is active. 
			if(fi->render_resume_flag && 
			   TaskManager::IsPartlyRenderedTask(done_task) )
			{
				if(fi->render_resume_flag)
				{
					int fail=0;
					RefString tmp(&fail);
					if(!fail)  tmp=done_task->rt->outfile->HDPath();
					if(!fail)  fail=_UnfinishedName(&tmp);
					if(!fail)  fail=(_RenameFile(done_task->rt->outfile->HDPath(),tmp,1)<0);
					if(!fail)  remove=0;
				}
			}
			if(remove)
			{  _DeleteFile(done_task->rt->outfile->HDPath(),1,
				"output file of failed render task");  }
		}
	}
	if(done_task->ft && done_task->ftes.status!=TTR_Unset && 
	   done_task->ftes.status!=TTR_Success )
	{
		// Filter task was not successful. 
		// Delete it if dest file is there: 
		if(done_task->ft->outfile)
		{
			_DeleteFile(done_task->ft->outfile->HDPath(),1,
				"output file of failed filter task");
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
	
	_StopRTimer();
	
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
	_StartRTimer();
	return(0);
}


int TaskSource_Local::srcGetTask(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	pending=AGetTask;
	cclient=cons;
	_StartRTimer();
	return(0);
}


int TaskSource_Local::srcDoneTask(TaskSourceConsumer *cons,CompleteTask *ct)
{
	if(!connected)  return(2);
	
	pending=ADoneTask;
	cclient=cons;
	done_task=ct;
	_StartRTimer();
	return(0);
}


int TaskSource_Local::srcDisconnect(TaskSourceConsumer *cons)
{
	if(!connected)  return(2);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	cclient=cons;
	_StartRTimer();
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
	// Set TaskSourceType (to be sure): 
	tstype=TST_Passive;
	
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
	
	nonexist_in_seq=0;
	
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


/******************************************************************************/

TaskSource_Local::FrameToProcessInfo::FrameToProcessInfo(int *failflag) : 
	r_infile(failflag),
	r_outfile(failflag),
	f_infile(failflag),
	f_outfile(failflag)
{
	fi=NULL;
	
	frame_no=-1;
	tobe_rendered=0;
	tobe_filtered=0;
	r_resume_flag=0;
}
