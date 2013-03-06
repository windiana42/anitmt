/*
 * povray.cpp
 * 
 * Implementation of POVRay renderer driver. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "povray.hpp"

#include <tsource/taskfile.hpp>
#include <tsource/tasksource.hpp>

#include <assert.h>

#include <lib/mymath.hpp>     /* for NAN, etc */


// Overriding virtual; gets called by TaskDriver on an error 
// or to notify driver of things (for verbose messages): 
// Return value: currently ignored; use 0. 
// SEE ALSO: Comments to Execute(). 
// Note: Useful functions: 
//    RenderDriver::ProcessErrorStdMessage() standard handler which 
//       probably works for most messages. 
//    ProcessError_PrintCommand() can be used to print the command to 
//       be executed. 
//    ProcessError_PrimaryReasonMessage() prints the primary reason 
//       (should always be used; is also used by :ProcessErrorStdMessage()). 
// Return value: 
//   0    -> OK
//   else -> something failed (cleaning up the hook (i.e. output FD, 
//           temp file)
int POVRayDriver::ProcessError(ProcessErrorInfo *pei)
{
	static const char *defname="POVRay";
	const char *prg_name=defname;
	if(pei->pinfo && pei->pinfo->tsb)
	{
		const RenderTask *rt = (const RenderTask *)pei->pinfo->tsb;
		if(rt->rdesc->name.str())
		{  prg_name=rt->rdesc->name.str();  }
		else if(rt->rdesc->binpath.str())
		{
			prg_name=rt->rdesc->binpath.str();
			const char *tmp=strrchr(prg_name,'/');
			if(tmp)  prg_name=tmp+1;
		}
	}
	
	if(pei->reason==PEI_StartFailed && 
	   pei->pinfo->pid==SPSi_NotSupported)
	{
		ProcessError_PrimaryReasonMessage("POV",prg_name,pei);
		
		const RenderTask *rt = pei->pinfo ? 
			(const RenderTask *)pei->pinfo->tsb : NULL;
		assert(rt);   // Yes, this MUST be !=NULL here. 
		
		Error("POV:   Error: requested image format %s/%dbpp not supported\n",
			rt->oformat->name,rt->oformat->bits_p_rgb); 
		
		return(0);
	}
	
	// The rest is all done by the standard function in RenderDriver. 
	RenderDriver::ProcessErrorStdMessage("POV",prg_name,pei);
	
	return(0);
}


// Length of decimal string representation of x. 
// 0..9 -> 1, 10..99 -> 2, etc. 
static inline int _DecimalLen(int x)
{
	int l=1;
	if(x<0)
	{  x=-x;  ++l;  }
	for(; x>9; x/=10,l++);
	return(l);
}


int POVRayDriver::InspectTaskDoneCode(PInfo *pinfo)
{
	assert(pinfo->tsb && pinfo->tsb->dtype==DTRender);
	RenderTask *rt=(RenderTask*)pinfo->tsb;
	//RenderTaskParams *rtp=(RenderTaskParams*)pinfo->tp;
	// NOTE: rtp MAY BE NULL. 
	
	// Well, this routine was introduced for POVRay. Because POVRay happily 
	// reports zero exit status when exiting due to a parse error or 
	// even when killed by SIGSEGV. Oh dear. 
	// So, the easiest thing is to check if the output file exists. 
	// However, this is not enough: 
	//  - In case we were resuming, the output file always exists. 
	//  - When killed by TERM/SEGV/whatever, POVRay leaves an unfinished 
	//    output file which has to be treated as such. 
	// So, I actually see if POVRay touched the file by checking the 
	// time stamp. 
	
	// Get path to inspect the file (relative to RendView's CWD). 
	const char *outfile=rt->outfile.HDPathRV().str();
	
	// First: If we are using the frame clock, POVRay changed the 
	//        output file name. Rename it back. 
	if(rt->use_frame_clock)
	{
		// Try to find the file name: 
		// Simply rename the first guesses until one call succeeds. 
		// If none succeeds, we run into an error below. 
		const int i_end=11;
		char *old_name=NULL;
		size_t old_name_len=0;
		size_t base_len=0;
		const char *lastdot=strrchr(outfile,'.');
		if(!lastdot)  goto rename_okay;
		
		old_name_len=strlen(outfile)+i_end+2;
		old_name=(char*)LMalloc(old_name_len);
		if(!old_name)
		{
			Error("OOPS: %s during success detection\n",
				cstrings.allocfail);
			goto rename_okay;
		}
		base_len=lastdot-outfile;
		strncpy(old_name,outfile,base_len);
		for(int i=_DecimalLen(rt->frame_no); i<i_end; i++)
		{
			snprintf(old_name+base_len,old_name_len-base_len,
				"%0*d%s",i,rt->frame_no,lastdot);
			
			if(!_RenameFile(old_name,outfile,
				/*may_not_exist=*/1,/*error_prefix=*/NULL/*no errors*/))
			{
				Verbose(TDR,"POV: Renamed: \"%s\" -> \"%s\" (frame clock).\n",
					old_name,outfile);
				goto rename_okay;
			}
		}
		
		snprintf(old_name+base_len,old_name_len-base_len,"*%s",lastdot);
		Verbose(TDR,"POV: Strange: unable to rename \"%s\" -> \"%s\".\n",
			old_name,outfile);
		
		rename_okay:;
		old_name=(char*)LFree(old_name);
	}
	
	// Sigh... POVRay changed again. If killed it exits with code 2 
	// instead of 0. 
	bool pov_kill_ok=
		(pinfo->tes.GetTermReason()==TTR_Exit && 
		 pinfo->tes.WasKilledByUs() && 
		 (pinfo->tes.signal==0 || pinfo->tes.signal==2) );
	
	// For this, only normal program termination 
	// is interesting (No killed/dumped/execve() failed): 
	if(pinfo->tes.GetTermReason()!=TTR_Exit)
	{
		// Well, we MUST set this: 
		pinfo->tes.outfile_status=OFS_Bad;
		goto doreturn;
	}
	
	// If POV failed, then we assume real failure except for the 
	// interruption thingy...
	if(pinfo->tes.signal!=0 && !pov_kill_ok)
	{
		// Well, we MUST set this: 
		pinfo->tes.outfile_status=OFS_Bad;
		goto doreturn;
	}
	
	// Okay, see if that was a real success...
	// NOTE: POVRay returns success in case we kill it and it finishes 
	//       up, so this is the right place to check if we can resume: 
	// NOTE: For POVRay-3.50c this is no longer true. It exits with 
	//       code 2 when being killed. 
	
	{  /* begin block */
	
	// This is the TTR_JK_* part of TTR: 
	TaskTerminationReason ttr_jk_val=(TaskTerminationReason)
		(pinfo->tes.rflags & TTR_JK_Mask);
	
	// First, see if the output file exists: 
	HTime starttime(HTime::Invalid);
	if(!pinfo->tes.starttime.IsInvalid())
	{
		// Note: We have to cut off the sub-second part because 
		//       the filesystem time only stores complete seconds. 
		starttime=pinfo->tes.starttime;
		starttime.PruneUsec();
	}
	HTime mtime(HTime::Invalid);
	int64_t oflen=GetFileLength(outfile,&mtime);
	int errno0=errno;
	// file_touched: 0 -> no; 1 -> yes; -1 -> don't know
	int file_touched = (starttime.IsInvalid() || mtime.IsInvalid()) ? (-1) : 
			(mtime>=starttime ? 1 : 0);
	if(oflen<0)
	{
		// stat() failed for output file. 
		if(rt->resume)
		{  Verbose(TDR,"POV: Strange: resuming [frame %d] but stat'ing output "
			"\"%s\" reports: %s\n",pinfo->ctsk->frame_no,
			outfile,strerror(errno0));  }
		
		if(ttr_jk_val==TTR_JK_NotKilled)
		{
			Verbose(TDR,"POV: Spurious success [frame %d]: "
				"stat'ing output \"%s\": %s\n",
				pinfo->ctsk->frame_no,outfile,strerror(errno0));
			
			// So, this is NOT success and we cannot resume. 
			//pinfo->tes.rflags has already TTR_Exit set, just "update" 
			// the exit status: 
			pinfo->tes.signal=-1;  // "exit code"!=0
			pinfo->tes.outfile_status=OFS_Bad;
		}
		else  // rendview killed it. 
		{
			Verbose(TDR,"POV: [frame %d] Hmm... "
				"POVRay did not leave an unfinished output file.\n",
				pinfo->ctsk->frame_no);
			
			pinfo->tes.outfile_status=OFS_Bad;
		}
	}
	else
	{
		// Okay, output file exists. 
		if(file_touched==0)
		{
			if(ttr_jk_val==TTR_JK_NotKilled)
			{
				Verbose(TDR,"POV: Spurious success [frame %d]: "
					"Output file \"%s\" was not even touched.\n",
					pinfo->ctsk->frame_no,outfile);
			
				// And yes, this is NOT success. We catch that in case there 
				// is a parse error. 
				// I hope this does not get caught in the seldom case that we 
				// want to continue an already complete frame and call POVRay 
				// on that. 
				//pinfo->tes.rflags has already TTR_Exit set, just "update" 
				// the exit status: 
				pinfo->tes.signal=-1;  // "exit code"!=0
			}
			
			// Always set resume status: 
			pinfo->tes.outfile_status=OFS_Resume;
		}
		else if(file_touched==1 || !rt->resume)
		{
			// In case the file was touched or we were not resuming: 
			// If we killed POVRay, assume unfinished, else assume finished. 
			do_the_same:;
			if(ttr_jk_val==TTR_JK_NotKilled)
			{  pinfo->tes.outfile_status=OFS_Complete;  }
			else
			{  pinfo->tes.outfile_status=OFS_Resume;  }
		}
		else
		{
			// What shall we do...?
			// Assume sane things: 
			goto do_the_same;
		}
	}
	
	
	}  /* end block */
	
doreturn:;
	// We must set the outfile status here: 
	assert(pinfo->tes.outfile_status!=OFS_Unset);
	
	//if(pinfo->tes.outfile_status!=OFS_Bad)
	{  Verbose(TDR,"POV: [frame %d] Output file \"%s\" status: %s\n",
		pinfo->ctsk->frame_no,outfile,
		TaskExecutionStatus::OFS_String(pinfo->tes.outfile_status));  }
	
	return(0);
}


// Check if passed file name is an ini file. 
static inline int _IsPOVINIFile(const RefString *path)
{
	// The decision making is kept simple here. 
	// If it has a .ini extension, it is an ini file, 
	// otherwise it is a pov file. 
	const char *lastdot=strrchr(path->str(),'.');
	if(!lastdot)  return(0);
	if(!strcasecmp(lastdot,".ini"))  return(1);
	return(0);
}


// Okay, execute must actually do the lowest level job and put together the 
// command line etc. 
// All that info is then passed up to RenderDriver::StartProcess(). 
// This function returns 0 on success, !=0 on error. 
// Return valze of Execute(): 
//   That of RenderDriver::StartProcess() or in case something fails: 
//     SPSi_IllegalParams -> internal error with params
//     SPSi_NotSupported -> error with params: not supported (e.g. wrong 
//             image format) 
//     SPS_LMallocFailed -> self-explaining
//     SPS_SPSi_Open{In,Out}Failed -> failed to open required I/O file 
//             (Primarily useful for filters or in case the driver reads 
//              in an additional config file.)
// POVRayDriver::Execute() Need not write any errors. For error handling, 
//   ProcessError() is called. You can use RenderDriver::ProcessErrorStdMessage() 
//   for most of the messages but if there are special messages (especially 
//   for SPSi_NotSupported, SPS_SPSi_Open{In,Out}Failed) then these should be 
//   handeled in ProcessError(). 
int POVRayDriver::Execute(
	const RenderTask *rt,
	const RenderTaskParams *rtp)
{
// Failure simulation code ;-)
//static int _frames=0;
//if(++_frames==4)  return(SPSi_IllegalParams);
	
	// Check for illegal *rp: 
	if(!rt->rdesc || !rt->oformat || 
	   !rt->infile || !rt->outfile || 
	   rt->width<1 || rt->height<1 || rt->frame_no<0 )
	{  return(SPSi_IllegalParams);  }
	
	RefString infile=rt->infile.HDPathJob(DTRender);
	RefString outfile=rt->outfile.HDPathJob(DTRender);
	
	if(!infile.str() || rt->infile.GetIOType()!=TaskFile::IOTRenderInput)
	{  return(SPSi_IllegalParams);  }
	
	if(!outfile.str() || 
	   (rt->outfile.GetIOType()!=TaskFile::IOTRenderOutput && 
	    rt->outfile.GetIOType()!=TaskFile::IOTFilterInput) )
	{  return(SPSi_IllegalParams);  }
	
	if(rt->rdesc->dfactory!=f)   // inapropriate driver (factory)
	{  return(SPSi_IllegalParams);  }
	
	// What gets called: 
	// povray [+KFI0] +Iinput <required_args> <add_args> 
	//        +Ooutput +Wwith +Hheight +/-C [+SFxxx +EFxxx|+K...]
	//        +F? +Lpath +Lpath 
	// Now support for .ini "input" files; this is the reason why 
	// the input file is first on command line (overriding). 
	
	// Output format:
	char Ftmp[8];
	int failed=_FillOutputFormat(Ftmp,8,rt->oformat) ? 1 : 0;
	if(failed)
	{  return(SPSi_NotSupported);  }
	
	RefStrList args(&failed);
	RefStrList lib_args(&failed);
	RefString Ifile(&failed),Ofile(&failed);
	if(failed)  return(SPS_LMallocFailed);
	
	// Binary path (arg[0]): 
	if(args.append(rt->rdesc->binpath))  ++failed;
	
	// The early K-options, if needed: 
	if(rt->use_frame_clock)
	{
		if(args.append("+KFI0"))  ++failed;
		//if(args.append("+KC"))  ++failed;
	}
	
	// Input: 
	if(_IsPOVINIFile(&infile))  Ifile=infile;
	else if(Ifile.sprintf(0,"+I%s",infile.str()))  ++failed;
	if(args.append(Ifile))  ++failed;
	if(failed)  return(SPS_LMallocFailed);
	
	// Required args: 
	if(args.append(&rt->rdesc->required_args))  ++failed;
	if(failed)  return(SPS_LMallocFailed);
	
	// Additional args: 
	if(args.append(&rt->add_args))  ++failed;
	if(failed)  return(SPS_LMallocFailed);
	
	// Output: 
	if(Ofile.sprintf(0,"+O%s",outfile.str()))  ++failed;
	if(args.append(Ofile))  ++failed;
	
	// Width & height args: 
	{
		char WHtmp[24];
		snprintf(WHtmp,24,"+W%d",rt->width);    if(args.append(WHtmp))  ++failed;
		snprintf(WHtmp,24,"+H%d",rt->height);   if(args.append(WHtmp))  ++failed;
	}
	
	// Resume? 
	if(args.append(rt->resume ? "+C" : "-C"))  ++failed;
	
	// Pass frame clock value?
	if(rt->use_frame_clock)
	{
		char tmp[32];
		// FrameClockVal: UNUSED
		//snprintf(tmp,32,"+K%g",rt->frame_clock);
		snprintf(tmp,32,"+SF%d",rt->frame_no);
		if(args.append(tmp))  ++failed;
		// +SFxxx -> +EFxxx
		tmp[1]='E';
		if(args.append(tmp))  ++failed;
	}
	else
	{
		// Make sure that we do NOT use the frame clock: 
		if(args.append("-KFI0"))  ++failed;
		if(args.append("-KFF0"))  ++failed;
	}
	
	// Format arg: 
	if(args.append(Ftmp))  ++failed;
	
	// Okay, now all the library args: 
	int _fail=0;
	for(const RefStrList::Node *i=rt->rdesc->include_path.first(); i; i=i->next)
	{
		// Must copy the include path because the `+L' must be prepended. 
		RefString tmp(&_fail);
		tmp.sprintf(0,"+L%s",i->str());
		if(args.append(tmp))  --_fail;
	}
	if(_fail)  ++failed;
	if(args.append(&lib_args))  ++failed;
	
	// Okay, that should be all args now.
	if(failed)  return(SPS_LMallocFailed);
	
	// Set up varous other stuff for program launch: 
	ProcessBase::ProcMisc pmisc;
	if(rt->wdir.str())   pmisc.wdir(rt->wdir);
	// niceval set by TaskDriver; timeout dealt with at TaskDriver. 
	
	// Fiddle around with the FDs if needed: 
	ProcessBase::ProcFDs sp_f;
	if(rtp)
	{
		int fail=0;
		if(rtp->stdin_fd>=0)   fail|=sp_f.Add(rtp->stdin_fd, 0);
		if(rtp->stdout_fd>=0)  fail|=sp_f.Add(rtp->stdout_fd,1);
		if(rtp->stderr_fd>=0)  fail|=sp_f.Add(rtp->stderr_fd,2);
		if(fail)  return(SPS_LMallocFailed);
	}
	
	// Okay, construct the command line: 
	ProcessBase::ProcPath sp_p(
		rt->rdesc->binpath,
		component_db()->GetBinSearchPath(rt->dtype));
	
	// Yeah... and pass standard environment: 
	ProcessBase::ProcEnv sp_e;
	int rv=RenderDriver::StartProcess(rt,rtp,&sp_p,&args,&pmisc,&sp_f,&sp_e);
	
	// All the error reporting done in ProcessError(). 
	
	return(rv);
}


// Returns -1 on failure; 0 on success; +1 if not supported
int POVRayDriver::_FillOutputFormat(char *dest,int len,const ImageFormat *fmt)
{
	switch(fmt->fmtid)
	{
		case IF_PNG:
			snprintf(dest,len,"+FN%d",fmt->bits_p_rgb);
			break;
		case IF_PPM:
			snprintf(dest,len,"+FP");
			break;
		case IF_TGA:
			snprintf(dest,len,"+FT");
			break;
		case IF_None: assert(0);  break;
		default:  return(+1);
	}
	return((int(strlen(dest)+1)>=len) ? -1 : 0);
}


POVRayDriver::POVRayDriver(POVRayDriverFactory *pf,TaskDriverInterface_Local *tdif,
	int *failflag) : 
	RenderDriver(pf,tdif,failflag)
{
	
}


POVRayDriver::~POVRayDriver()
{
	
}


/********************************** FACTORY **********************************/


TaskDriver *POVRayDriverFactory::Create(TaskDriverInterface_Local *tdif)
{
	return(NEW2<POVRayDriver>(this,tdif));
}


int POVRayDriverFactory::CheckDesc(RF_DescBase *d)
{
	assert(d->dtype==DTRender);
	RenderDesc *rd=(RenderDesc*)d;
	rd->can_resume_render=true;
	rd->can_pass_frame_clock=true;
	return(0);
}


// Called on program start to set up the POVRayDriverFactory; 
// POVRayDriverFactory registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int POVRayDriverFactory::init(ComponentDataBase *cdb)
{
	POVRayDriverFactory *f=NEW1<POVRayDriverFactory>(cdb);
	if(!f)
	{
		Error("Failed to initialize POVRay driver.\n");
		return(1);
	}
	Verbose(BasicInit,"[POVRay] ");
	return(0);
}


const char *POVRayDriverFactory::DriverDesc() const
{
	return(
		"POVRay driver capable of driving several versions of the "
		"Persistence of Vision Raytracer.");
}


POVRayDriverFactory::POVRayDriverFactory(ComponentDataBase *_cdb,int *failflag) : 
	TaskDriverFactory(_cdb,"povray",DTRender,failflag)
{
	
}


POVRayDriverFactory::~POVRayDriverFactory()
{
	
}
