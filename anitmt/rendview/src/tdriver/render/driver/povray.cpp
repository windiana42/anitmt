/*
 * povray.cpp
 * 
 * Implementation of POVRay renderer driver. 
 * 
 * Copyright (c) 2001 -- 2003 by Wolfgang Wieser (wwieser@gmx.de) 
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
			char *tmp=strrchr(prg_name,'/');
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
	   rt->width<1 || rt->height<1 )
	{  return(SPSi_IllegalParams);  }
	
	RefString infile=rt->infile.HDPath();
	RefString outfile=rt->outfile.HDPath();
	
	if(!infile.str() || !outfile.str() || 
	   rt->infile.GetIOType()!=TaskFile::IOTRenderInput || 
	   rt->outfile.GetIOType()!=TaskFile::IOTRenderOutput )
	{  return(SPSi_IllegalParams);  }
	
	if(rt->rdesc->dfactory!=f)   // inapropriate driver (factory)
	{  return(SPSi_IllegalParams);  }
	
	// What gets called: 
	// povray +Wwith +Hheight +-/-C +Iinput +Ooutput 
	//        +F? +Lpath +Lpath <required_args> <add_args>
	
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
	
	// Width & height args: 
	char Wtmp[24],Htmp[24];
	snprintf(Wtmp,24,"+W%d",rt->width);    if(args.append(Wtmp))  ++failed;
	snprintf(Htmp,24,"+H%d",rt->height);   if(args.append(Htmp))  ++failed;
	
	// Resume? 
	if(args.append(rt->resume ? "+C" : "-C"))  ++failed;
	
	// Input & output: 
	if(Ifile.sprintf(0,"+I%s",infile.str()))  ++failed;
	if(Ofile.sprintf(0,"+O%s",outfile.str()))  ++failed;
	if(failed)  return(SPS_LMallocFailed);
	if(args.append(Ifile))  ++failed;
	if(args.append(Ofile))  ++failed;
	
	// Format arg: 
	if(args.append(Ftmp))  ++failed;
	
	// Okay, now all the other args: 
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
	if(args.append(&rt->rdesc->required_args))  ++failed;
	if(args.append(&rt->add_args))  ++failed;
	
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
