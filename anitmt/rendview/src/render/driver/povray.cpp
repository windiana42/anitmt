/*
 * povray.cpp
 * 
 * Implementation of POVRay renderer driver. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
int POVRayDriver::ProcessError(ProcessErrorInfo *pei)
{
	int print_cmd=0;
	
	#warning FIXME: this function should probably be moved up; \
		i.e. POVRayDriver::ProcessError() should call a function on higher level \
		unless the error is really POVRay-specific. 
	
	char _frame_no_str[24];
	if(pei->pinfo && pei->pinfo->ctsk && pei->pinfo->ctsk->frame_no>=0)
	{  snprintf(_frame_no_str,24,"%d",pei->pinfo->ctsk->frame_no);  }
	else
	{  strcpy(_frame_no_str,"???");  }
	
	//const RenderTaskParams *rtp=(const RenderTaskParams*)pei->pinfo->tp;
	const RenderTask *rt=(const RenderTask *)pei->pinfo->tsb;
	
	const char *tmp=NULL;
	const char *errno_str=NULL;
	switch(pei->reason)
	{
		// *** verbose messages: ***
		case PEI_Starting:
			if(!settings()->verbose)  break;
			Verbose("POV: Starting POVRay [frame %s].\n",_frame_no_str);
			print_cmd=2;
			break;
		case PEI_StartSuccess:
			if(!settings()->verbose)  break;
			Verbose("POV: Forked to launch POVRay [frame %s]...\n",_frame_no_str);
			break;
		case PEI_ExecSuccess:
			if(!settings()->verbose)  break;
			Verbose("POV: POVRay started successfully [frame %s].\n",
				_frame_no_str);
			break;
		case PEI_RunSuccess:
			if(!settings()->verbose)  break;
			Verbose("POV: POVRay terminated successfully [frame %s].\n",
				_frame_no_str);
			break;
		
		// *** warning/error messages (as you like to define it): ***
		case PEI_Timeout:
			Error("POV: POVRay [frame %s] exceeded time limit (%ld s).\n",
				_frame_no_str,(pei->pinfo->tp->timeout+500)/1000);
			print_cmd=1;
			break;
		
		// *** error messages: ***
		case PEI_StartFailed:
			Error("POV: Failed to start POVRay [frame %s].\n",_frame_no_str);
			tmp=StartProcessErrorString(pei,&errno_str);
			Error("POV:   Error: %s%s\n",tmp,errno_str ? errno_str : "");
			Error("POV:   Binary: %s\n",rt->rdesc->binpath.str());
			Error("POV:   Working dir: %s\n",
				pei->pinfo->tsb->wdir.str() ? pei->pinfo->tsb->wdir.str() : "[cwd]");
			Error("POV:   Search path:");
			for(const RefStrList::Node *i=component_db()->GetBinSearchPath(
				rt->dtype)->first(); i; i=i->next)
			{  Error(" %s",i->str());  }
			Error("\n");
			if(rt->rdesc->binpath[0]=='/')
			{  Error("POV:   Note: search path not used as binary "
				"contains absolute path\n");  }
			break;
		case PEI_ExecFailed:
			Error("POV: Failed to execute POVRay [frame %s].\n",_frame_no_str);
			Error("POV:   Failure: %s\n",PSFailedErrorString(pei));
			break;
		case PEI_RunFailed:
			Error("POV: POVRay [frame %s] execution failed.\n",_frame_no_str);
			Error("POV:   Failure: ");
			switch(pei->ps->detail)
			{
				case PSExited:
					Error("Exited with non-zero status %d.\n",
						pei->ps->estatus);
					print_cmd=1;
					break;
				case PSKilled:
					Error("Killed by signal %d (pid %ld)\n",
						pei->ps->estatus,long(pei->pinfo->pid));
					break;
				case PSDumped:
					Error("Dumped (pid %ld, signal %d)\n",
						long(pei->pinfo->pid),pei->ps->estatus);
					print_cmd=1;
					break;
				default:
					Error("???\n");
					abort();
					break;
			}
			break;
	}
	
	// Uh, yes this is an ugly code duplication: 
	if(print_cmd==1)
	{
		Error("POV:   Command:");
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Error(" %s",i->str());  }
		Error("\n");
		const char *tmp=pei->pinfo->tsb->wdir.str();
		Error("POV:   Working dir: %s\n",tmp ? tmp : "[cwd]");
	}
	else if(print_cmd==2)
	{
		Verbose("POV:   Command:");
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Verbose(" %s",i->str());  }
		const char *tmp=pei->pinfo->tsb->wdir.str();
		Verbose("POV:   Working dir: %s\n",tmp ? tmp : "[cwd]");
	}
	
	return(0);
}


int POVRayDriver::Execute(
	const RenderTask *rt,
	const RenderTaskParams *rtp)
{
	// Check for illegal *rp: 
	if(!rt->rdesc || !rt->oformat || 
	   !rt->infile || !rt->outfile || 
	   rt->width<1 || rt->height<1)
	{  return(1);  }
	
	RefString infile=rt->infile->HDPath();
	RefString outfile=rt->outfile->HDPath();
	
	if(!infile.str() || !outfile.str() || 
	   rt->infile->GetIOType()!=TaskFile::IOTRenderInput || 
	   rt->outfile->GetIOType()!=TaskFile::IOTRenderOutput )
	{  return(1);  }
	
	if(rt->rdesc->dfactory!=f)   // inapropriate driver (factory)
	{  return(1);  }
	
	// What gets called: 
	// povray +Wwith +Hheight +-/-C +Iinput +Ooutput 
	//        +F? +Lpath +Lpath <required_args> <add_args>
	
	// Output format:
	char Ftmp[8];
	int failed=_FillOutputFormat(Ftmp,8,rt->oformat) ? 1 : 0;
	if(failed)
	{
		if(failed>0)
		{  Error("POV: Requested image format %s/%dbpp not supported.\n",
			rt->oformat->name,rt->oformat->bitspp);  }
		return(-1);
	}
	
	RefStrList args(&failed);
	RefStrList lib_args(&failed);
	RefString Ifile(&failed),Ofile(&failed);
	if(failed)  return(-1);
	
	// Binary parh (arg[0]): 
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
	if(failed)  return(-1);
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
	if(failed)  return(-1);
	
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
		if(fail)  return(-1);
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
			snprintf(dest,len,"+FN%d",fmt->bitspp/3);
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


POVRayDriver::POVRayDriver(POVRayDriverFactory *pf,int *failflag) : 
	RenderDriver(pf,failflag)
{
	
}


POVRayDriver::~POVRayDriver()
{
	
}


/********************************** FACTORY **********************************/


TaskDriver *POVRayDriverFactory::Create()
{
	return(NEW1<POVRayDriver>(this));
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
		Error("Failed to initialize POVRay driver\n");
		return(1);
	}
	Verbose("[POVRay] ");
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
	verbose=2;
}


POVRayDriverFactory::~POVRayDriverFactory()
{
	
}
