/*
 * local.cpp
 * 
 * Implementation of local task source. 
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

#include "local.hpp"

#include "../taskfile.hpp"
#include "../../taskmanager.hpp"

#include <assert.h>

#include <lib/mymath.hpp>    /* for NAN, etc */


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


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
// save_mtime_a, if non-NULL stores a's mtime (or invalid if !existing). 
static int _FirstIsNewer(RefString *a,RefString *b,int frame_no,
	TaskDriverType dtype,HTime *save_mtime_a=NULL)
{
	int rv=FirstFileIsNewer(a,b,"Local: ",save_mtime_a);
	if(rv<0)
	{  Warning("Local:   (%sing frame %d to be sure but expect trouble)\n",
		DTypeString(dtype),frame_no);  }
	return(rv);
}


static void _DeleteFile(RefString path,int may_not_exist,const char *desc)
{
	int rv=DeleteFile(&path,may_not_exist,"Local: ");
	if(!rv)
	{  Verbose(TSLR,"Local: Deleted %s: %s\n",desc,path.str());  }
}


static int _UnfinishedName(RefString *f)
{
	return(f->append("-unfinished"));
}


// Check additional files and see if the frame has to be re-processed. 
// Return value: 
//   1 -> must re-render
//   0 -> no need to re-rennder because of additional files
int TaskSource_Local::_CheckAddFilesMTime(HTime *output_mtime,
	TaskDriverType dtype,const PerFrameTaskInfo *fi,int frame_no)
{
	assert(!output_mtime->IsInvalid());
	
	const RefStrList *flist=NULL;
	switch(dtype)
	{
		case DTRender:  flist=&fi->radd_files;  break;
		case DTFilter:  flist=&fi->fadd_files;  break;
	}
	assert(flist);
	
	// I use a stat cache for this so that I do not have to call 
	// stat() all the time. 
	// NOTE: The cache gets invalidated upon task source disconnect. 
	// Thus, we cache additional file stat(2) lookup as long as we're 
	// connected to task manager (most notably when skipping several 
	// frames in sequence and also when simply reporting several 
	// frames to the TaskManager while being connected). 
	for(const RefStrList::Node *n=flist->first(); n; n=n->next)
	{
		HTime mtime;
		int64_t size;
		int rv=statcache_add.GetCache(n,&mtime,&size);
		// If not found in cache or invalidated in cache: 
		// size=-1 means "stat error" which is also cached. 
		// size=-2 means "invalid(dated) entry"
		if(rv || size==-2)
		{
			size=GetFileLength(n->str(),&mtime);
			if(size<0)
			{
				Warning("Local: [frame %d] failed to stat add. %s file \"%s\": %s\n",
					frame_no,DTypeString(dtype),n->str(),strerror(errno));
				assert(mtime.IsInvalid());
				size=-1;  // -2 -> "invalid" 
			}
			// Add it to cache: 
			statcache_add.UpdateCache(n,&mtime,size);
		}
		
		if(size<0)  continue;  // stat(2) error
		
		// We WILL re-render in case the input and output files have 
		// equal time stamp. 
		if(mtime>=(*output_mtime))
		{  return(1);  }
	}
	
	return(0);
}


// dtype: DTRender -> render job; DTFilter -> filter job. 
// Return value: 
//   0 -> OK (render/filter it) [or: check tobe_{rendered,filtered}!]
//   1 -> error; do not [render and also do not] filter it [if that is planned]. 
//   2 -> no more tasks (only DTRender) 
//  -1 -> allocation failure
int TaskSource_Local::_FillInJobFiles(TaskDriverType dtype,
	FrameToProcessInfo *ftpi)
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
		if(ftpi->r_infile_job.sprintf(0,fi->rinfpattern,ftpi->frame_no) ||
		   ftpi->r_outfile_job.sprintf(0,fi->routfpattern,ftpi->frame_no) )
		{  return(-1);  }
		inf=ftpi->r_infile_job;
		outf=ftpi->r_outfile_job;
	}
	else if(dtype==DTFilter)
	{
		if(ftpi->f_infile_job.sprintf(0,fi->routfpattern,ftpi->frame_no) || 
		   ftpi->f_outfile_job.sprintf(0,fi->foutfpattern,ftpi->frame_no) )
		{  return(-1);  }
		inf=ftpi->f_infile_job;
		outf=ftpi->f_outfile_job;
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
	
	if(dtype==DTRender)
	{
		ftpi->r_infile_rv=inf;
		ftpi->r_outfile_rv=outf;
	}
	else if(dtype==DTFilter)
	{
		ftpi->f_infile_rv=inf;
		ftpi->f_outfile_rv=outf;
	}
	
	// Check if inf exists: (Filter input file will most likely not 
	// yet exist if we shall also render the frame.) 
	if((dtype==DTRender || (dtype==DTFilter && !ftpi->tobe_rendered)) && 
	   !CheckExistFile(&inf,1))
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
	
	RefString unf_tmp;
	unf_tmp=outf;
	if(_UnfinishedName(&unf_tmp))  return(-1);
	
	// Okay, now some logic: 
	char unf_action='\0';  // rename, delete, <nothing>
	char frame_action='\0'; // skip, render/filter, continue
	int outf_exists=-1;  // -1 -> unknown; 0 -> no; 1 -> yes
	if(p->cont_flag)
	{
		// Check if finished frame exists: 
		int o_exists=CheckExistFile(&outf,1);
		outf_exists=(o_exists ? 1 : 0);
		
		if(dtype==DTRender)
		{
			// Check if unfinished frame exists: 
			int o_unf_exists=CheckExistFile(&unf_tmp,1);
			
			if(fi->render_resume_flag)
			{
				// Okay, render resume switched on. If the file to resume 
				// exists, rename it back to the original file name: 
				if(o_unf_exists && o_exists)
				{
					// Check which one is the newer one: 
					int rv=_FirstIsNewer(&unf_tmp,&outf,ftpi->frame_no,dtype);
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
		{
			// For filter, this is easy because there is no resume. 
			// However, we decide on re-filtering based on time stamps 
			// just as we also do when rendering. 
			//frame_action='\0'; <--- stays '\0'...
		}
		
		HTime output_mtime(HTime::Invalid);
		// This is the same for cont with and without render resume: 
		if(frame_action=='\0')
		{
			// NOTE: The dtype==DTFilter &&... is here just against a stupid 
			//       error in _FirstIsNewer() complaining that the input 
			//       file does not exist. This is okay, if we render 
			//       the frame before. 
			if(!o_exists || (dtype==DTFilter && ftpi->tobe_rendered))
			{  frame_action='r';  }
			// See if we re-render/filter it: 
			else
			{
				// We WILL re-render frames if the input 
				// has the same age as the output. 
				int rv=_FirstIsNewer(&outf,&inf,ftpi->frame_no,dtype,
					&output_mtime);
				if(!rv)   // outf older or equally old
				{  frame_action='r';  }   
				else if(rv<0)  // error
				{  frame_action='r';  }
				else  // outf newer
				{  frame_action='s';  }
			}
		}
		
		if(p->check_add_mtime)
		{
			if(frame_action=='s' || frame_action=='c')
			{
				// Skip or continue. 
				// Must see if we really do that based on the additional files. 
				// Therefore, need output file modification time if not set 
				// already. 
				if(output_mtime.IsInvalid())
				{
					const char *outfile_path=outf.str();
					if(unf_action=='r')
					{  outfile_path=unf_tmp.str();  }
					if(GetFileLength(outfile_path,&output_mtime)<0)
					{
						// This should never happen. 
						Error("Local: Failed to stat \"%s\": %s "
							"[frame %d] (expect trouble)\n",
							outfile_path,strerror(errno),ftpi->frame_no);
						frame_action='r';
					}
				}
			}
			// DO NOT MERGE
			if(frame_action=='s' || frame_action=='c')
			{
				// If this assert fails, there is a trivial bug in the logic above. 
				// The previous if() block is just there to ensure that 
				// output_mtime IS valid for frame_actions 's' and 'c'. 
				assert(!output_mtime.IsInvalid());
				
				// See if one of the additional files was modified: 
				int rv=_CheckAddFilesMTime(&output_mtime,dtype,fi,ftpi->frame_no);
				if(rv==1)
				{  frame_action='r';  }
			}
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
		int rv=RenameFile(&unf_tmp,&outf,0,"Local: ");
		if(!rv)
		{
			Verbose(TSLR,"Local: Renamed unfinished frame: %s -> %s\n",
				unf_tmp.str(),outf.str());
			outf_exists=1;
		}
		else  // failed? - then (try to) delete it: 
		{  unf_action='d';  }
	}
	if(unf_action=='d')
	{
		_DeleteFile(unf_tmp,0,"unfinished frame file");
		unf_action='\0';
	}
	assert(unf_action=='\0');
	
	// Okay, if we do not continue or skip, then we have to delete the 
	// destination file first. 
	if(frame_action=='r' && outf_exists!=0)  // not 's' or 'c'
	{  _DeleteFile(outf.str(),(outf_exists==-1),"job output file");  }
	
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
	int retval=1;
	// I do not want to report a "neither rendered nor filtered" 
	// for each frame. Instead I do it for a range start..end: 
	int nrnf_start=-1,nrnf_end=-1;
	for(;;next_frame_no+=p->fjump)
	{
		fi=p->GetPerFrameTaskInfo(next_frame_no);
		if(!fi)  break;  // out of range; must stop: no more frames to process
		
		// See what has to be done: 
		ftpi->tobe_rendered=fi->rdesc ? 1 : 0;
		ftpi->tobe_filtered=fi->fdesc ? 1 : 0;
		if(!ftpi->tobe_rendered && !ftpi->tobe_filtered)
		{
			if(nrnf_start<0)
			{  nrnf_start=nrnf_end=next_frame_no;  }
			else if(nrnf_start>next_frame_no)
			{  nrnf_start=next_frame_no;  }
			else if(nrnf_end<next_frame_no)
			{  nrnf_end=next_frame_no;  }
			else assert(0);  // huh?
			continue;
		}
		ftpi->fi=fi;
		ftpi->frame_no=next_frame_no;
		
		// Make sure no paths are left: 
		ftpi->r_infile_job.deref();
		ftpi->r_outfile_job.deref();
		ftpi->f_infile_job.deref();
		ftpi->f_outfile_job.deref();
		ftpi->r_infile_rv.deref();
		ftpi->r_outfile_rv.deref();
		ftpi->f_infile_rv.deref();
		ftpi->f_outfile_rv.deref();
		
		// Finally, switch on one frame: 
		next_frame_no+=p->fjump;
		retval=0;
		break;
	}
	if(nrnf_start>=0)
	{
		if(nrnf_start==nrnf_end)
		{  Warning("Local: Frame %d shall neither be rendered nor filtered.\n",
			nrnf_start);  }
		else
		{  Warning("Local: Frames %d..%d (jump %d) shall neither be rendered "
			"nor filtered.\n",nrnf_start,nrnf_end,p->fjump);  }
	}
	return(retval);
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
			if(rv==2)   // no more tasks
			{  assert(!ftpi->tobe_rendered);  return(1);  }
			if(rv==1)  // file not found 
			{  assert(!ftpi->tobe_rendered);  continue;  }
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


// Return value: 0 -> OK; -1 -> alloc failure; -2 -> GetTaskFile() failed. 
int TaskSource_Local::_SetUpAddTaskFiles(CompleteTask::AddFiles *af,
	const RefStrList *flist,TaskFile::IOType iotype,int frame_no)
{
	assert(!af->tfile);
	af->nfiles=flist->count();
	if(!af->nfiles)  return(0);
	af->tfile=NEWarray<TaskFile>(af->nfiles);
	if(!af->tfile)  return(-1);
	int i=0;
	for(const RefStrList::Node *n=flist->first(); n; n=n->next,i++)
	{
		int collide_error;  // =0 not necessary. 
		af->tfile[i]=TaskFile::GetTaskFile(n->str(),
			TaskFile::FTAdd,iotype,TaskFile::FCLocal,
			/*hdpath_job=*/NULL/*...because it's an additional file*/,
			&collide_error);
		if(!af->tfile[i])
		{
			af->tfile=DELarray(af->tfile);
			af->nfiles=0;
			return(-2);
		}
		if(collide_error)
		{
			TaskFile *tf=&(af->tfile[i]);
			Error("Local: [frame %d] File \"%s\" (%s/%s) "
				"collides with %s/%s.\n",
				frame_no,tf->HDPathRV().str(),
				TaskFile::FTypeString(tf->GetFType()),
				TaskFile::IOTypeString(tf->GetIOType()),
				TaskFile::FTypeString(TaskFile::FTAdd),
				TaskFile::IOTypeString(iotype));
			// ...or should we simply print a warning, skip the file 
			// and go on?
			return(-2);
		}
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
		#warning SUPPLY proper task_id!
		ctsk->task_id=ftpi.frame_no;
		
		if(ftpi.tobe_rendered)
		{
			RenderTask *rt=NEW<RenderTask>();
			if(!rt)  break;
			ctsk->rt=rt;
			
			rt->frame_no=ftpi.frame_no;
			rt->rdesc=fi->rdesc;
			assert(fi->rdesc);   // Otherwise tobe_rendered may not be set
			rt->width=fi->width;
			rt->height=fi->height;
			rt->oformat=fi->oformat;
			rt->timeout=fi->rtimeout;
			
			rt->infile=TaskFile::GetTaskFile(ftpi.r_infile_rv,
				TaskFile::FTFrame,TaskFile::IOTRenderInput,TaskFile::FCLocal,
				&ftpi.r_infile_job);
			if(!rt->infile)  break;
			
			rt->outfile=TaskFile::GetTaskFile(ftpi.r_outfile_rv,
				TaskFile::FTImage,TaskFile::IOTRenderOutput,TaskFile::FCLocal,
				&ftpi.r_outfile_job);
			if(!rt->outfile)  break;
			// This is correct, because ftpi.r_resume_flag is not set if 
			// the file does not exist. 
			if(ftpi.r_resume_flag)
			{  rt->outfile.SetIncomplete(1);  }
			
			if(rt->add_args.append(&fi->radd_args))  break;
			
			rt->wdir=fi->rdir;
			
			rt->resume=ftpi.r_resume_flag;
			rt->resume_flag_set=
				(fi->render_resume_flag && rt->rdesc->can_resume_render) ? 1 : 0;
			
			if(fi->use_clock && rt->rdesc->can_pass_frame_clock)
			{
				#if 0	/* FrameClockVal: UNUSED */
				// I do not use clock_step for the first frame (thus it may 
				// be NAN for per-frame blocks containing only one frame). 
				assert(finite(fi->clock_start));
				rt->frame_clock=fi->clock_start;
				if(ctsk->frame_no!=fi->first_frame_no)
				{
					assert(finite(fi->clock_step));
					rt->frame_clock+=
						fi->clock_step*(ctsk->frame_no-fi->first_frame_no);
				}
				#else
				rt->use_frame_clock=1;
				#endif
			}
			
			#if TESTING
			if((fi->use_clock && !rt->rdesc->can_pass_frame_clock) || 
			   (fi->render_resume_flag && !rt->rdesc->can_resume_render) )
			{  Error("Local: OOPS: use_clock=%s, support=%s; "
				"rcont=%s, support=%s. BUG!\n",
				fi->use_clock ? "yes" : "no",
				rt->rdesc->can_pass_frame_clock ? "yes" : "no",
				fi->render_resume_flag ? "yes" : "no",
				rt->rdesc->can_resume_render ? "yes" : "no");  }
			#endif
			
			if(_SetUpAddTaskFiles(&ctsk->radd,&fi->radd_files,
				TaskFile::IOTRenderInput,ctsk->frame_no))
			{  ni->getstat=GTSFileCollision;  break;  }
		}
		
		if(ftpi.tobe_filtered)
		{
			FilterTask *ft=NEW<FilterTask>();
			if(!ft)  break;
			ctsk->ft=ft;
			
			ft->frame_no=ftpi.frame_no;
			ft->fdesc=fi->fdesc;
			assert(fi->fdesc);   // Otherwise tobe_filtered may not be set
			ft->timeout=fi->ftimeout;
			
			ft->infile=TaskFile::GetTaskFile(ftpi.f_infile_rv,
				TaskFile::FTImage,TaskFile::IOTFilterInput,TaskFile::FCLocal,
				&ftpi.f_infile_job);
			if(!ft->infile)  break;
			
			ft->outfile=TaskFile::GetTaskFile(ftpi.f_outfile_rv,
				TaskFile::FTImage,TaskFile::IOTFilterOutput,TaskFile::FCLocal,
				&ftpi.f_outfile_job);
			if(!ft->outfile)  break;
			
			if(ft->add_args.append(&fi->fadd_args))  break;
			
			ft->wdir=fi->fdir;
			
			if(_SetUpAddTaskFiles(&ctsk->fadd,&fi->fadd_files,
				TaskFile::IOTFilterInput,ctsk->frame_no))
			{  ni->getstat=GTSFileCollision;  break;  }
		}
		
		ni->ctsk=ctsk;
		ni->getstat=GTSGotTask;
		
		// Okay, nothing failed till now. 
		return;
	} while(0);
	
	// If we reach here and getstat is not yet set, allocation failed. 
	if(ni->getstat==GTSNone)
	{  ni->getstat=GTSAllocFailed;  }
	
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
	
	// Special case: if unfinished frame was not processed, rename it 
	// back as "unfinished". 
	if(done_task->rt && done_task->rtes.tes.rflags==TTR_Unset && 
	   done_task->rt->resume)
	{
		assert(done_task->rtes.tes.outfile_status!=OFS_Complete);
		done_task->rtes.tes.outfile_status = 
			(fi->render_resume_flag ? OFS_Resume : OFS_Bad);
	}
	// See if successful: 
	if(done_task->rt && done_task->rtes.tes.outfile_status!=OFS_Complete && 
	   (done_task->rtes.tes.rflags!=TTR_Unset || done_task->rt->resume) )
	{
		// Render task was not completely successful. 
		// File must get special treatment: 
		// Delete or rename it if dest file is there: 
		if(!!done_task->rt->outfile)
		{
			int remove=1;  // YES!
			// We always delete it unless the file has state 
			// OFS_Resume (OFS_Complete not handeled here) and 
			// render_resume_flag (-rcont) is active. 
			if(fi->render_resume_flag && 
			   done_task->rtes.tes.outfile_status==OFS_Resume)
			{
				RefString tmp(done_task->rt->outfile.HDPathRV());
				int fail=_UnfinishedName(&tmp);
				if(!fail)  fail=(done_task->rt->outfile.SetIncompleteRename(
							&tmp,/*may_not_exist*/1,"Local: ")<0);
				if(!fail)  remove=0;
			}
			if(remove)
			{  _DeleteFile(done_task->rt->outfile.HDPathRV(),1,
				"output file of (failed) render task");  }
		}
	}
	
	if(done_task->ft && done_task->ftes.tes.rflags!=TTR_Unset && 
	   done_task->ftes.tes.outfile_status!=OFS_Complete )
	{
		// Filter task was not successful. 
		// Delete it if dest file is there: 
		if(!!done_task->ft->outfile)
		{
			_DeleteFile(done_task->ft->outfile.HDPathRV(),1,
				"output file of (failed) filter task");
		}
	}
	
	// Dump all the information to the user (if requested):
	component_db->taskmanager()->DumpTaskInfo(done_task,
		NULL,TaskManager::DTSK_ReportBackDone,VERBOSE_TSLR);
	Verbose(TSLR,"  Done task [frame %d] was from per-frame block %s.\n",
		done_task->frame_no,p->_FrameInfoLocationString(fi));
	
	// Deleting done_task is okay for DoneTask(). 
	delete done_task;
	done_task=NULL;
	
	ni->donestat=DTSOkay;
}


int TaskSource_Local::timernotify(TimerInfo *ti)
{
	Verbose(DBGV,"--<TSLocal::timernotify>--<pending=%d>--\n",pending);
	
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
			ni.disconnstat=(we_are_quitting ? DSQuitOkay : DSOkay);
			we_are_quitting=0;
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


int TaskSource_Local::srcDisconnect(TaskSourceConsumer *cons,int special)
{
	if(!connected && !special)  return(2);
	if(special!=0 && special!=1)  return(10);
	
	// Okay, then let's disconnect...
	pending=ADisconnect;
	we_are_quitting=special;
	cclient=cons;
	_StartRTimer();
	
	// Invalidate stat cache on disconnect. 
	// (Will do nothing if the cache was already invalidated.) 
	statcache_add.InvalidateAll();
	
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
	FDBase(failflag),
	statcache_add(failflag)
{
	// Set TaskSourceType (to be sure): 
	tstype=TST_Passive;
	
	p=tsf;
	pending=ANone;
	we_are_quitting=0;
	connected=0;
	done_task=NULL;
	
	if(p->fjump>0)
	{  next_frame_no=p->master_fi.first_frame_no;  }
	else
	{
		assert(p->master_fi.nframes>=0);  // checked by TaskSourceFactory_Local::CheckParams()
		// Make sure we stop at start frame: 
		int jv=-p->fjump;
		int njumps=(p->master_fi.nframes+jv-1)/jv - 1;
		next_frame_no = p->master_fi.first_frame_no + njumps*jv;
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
	assert(!done_task);
	assert(!connected);
}


/******************************************************************************/

TaskSource_Local::FrameToProcessInfo::FrameToProcessInfo(int *failflag) : 
	r_infile_job(failflag),
	r_outfile_job(failflag),
	f_infile_job(failflag),
	f_outfile_job(failflag),
	r_infile_rv(failflag),
	r_outfile_rv(failflag),
	f_infile_rv(failflag),
	f_outfile_rv(failflag)
{
	fi=NULL;
	
	frame_no=-1;
	tobe_rendered=0;
	tobe_filtered=0;
	r_resume_flag=0;
}
