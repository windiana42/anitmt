/*
 * generic.cpp
 * 
 * Implementation of generic filter driver. 
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

#include "generic.hpp"

#include <tsource/taskfile.hpp>
#include <tsource/tasksource.hpp>

#include <assert.h>


// Prefix GFD: generic filter driver 

static const char *_qmfilter="[?filter?]";


int GenericFilterDriver::ProcessError(ProcessErrorInfo *pei)
{
	const char *prg_name=_qmfilter;
	if(pei->pinfo && pei->pinfo->tsb)
	{
		const FilterTask *ft = (const FilterTask *)pei->pinfo->tsb;
		if(ft->fdesc->name.str())
		{  prg_name=ft->fdesc->name.str();  }
		else if(ft->fdesc->binpath.str())
		{
			prg_name=ft->fdesc->binpath.str();
			char *tmp=strrchr(prg_name,'/');
			if(tmp)  prg_name=tmp+1;
		}
	}
	
	if(pei->reason==PEI_StartFailed && 
		(pei->pinfo->pid==SPSi_OpenInFailed || 
		 pei->pinfo->pid==SPSi_OpenOutFailed ) )
	{
		assert(pei->pinfo);
		
		ProcessError_PrimaryReasonMessage("GFD",prg_name,pei);
		
		Error("GFD:   Error: Failed to open %s file \"%s\"\n",
			pei->pinfo->pid==SPSi_OpenInFailed ? 
				"input" : "output",
			pei->pinfo->pid==SPSi_OpenInFailed ? 
				pei->pinfo->tsb->infile->HDPath().str() : 
				pei->pinfo->tsb->outfile->HDPath().str() );
		Error("GFD:   Reason: %s\n",strerror(pei->errno_val));
		
		return(0);
	}
	
	FilterDriver::ProcessErrorStdMessage("GFD",prg_name,pei);
	
	// Check if we have to close I/O FDs: 
	// (We can close the FDs just after we have forked.)
	if(pei->pinfo->tp->hook && 
	   (pei->is_last_call || pei->reason==PEI_StartSuccess) )
	{
		const TaskParams *tp=pei->pinfo->tp;
		DataHook *dh=(DataHook*)(tp->hook);
		
		int fail=0;
		
		// Closing the input FD may not make trouble. 
		if(dh->infd>=0)
		{  close(dh->infd);  dh->infd=-1;  }
		// Closing the output FD may make trouble e.g. on NFS. 
		if(dh->outfd>=0)
		{
			fail=(close(dh->outfd)<0);
			dh->outfd=-1;
		}
		
		delete dh;
		((TaskParams*)tp)->hook=NULL;
		
		if(fail)
		{
			// ...and here the trouble begins. Closing the output failed, 
			// there may be no space left on the device or things like that. 
#warning IMPLEMENT CLEANUP FAIL HANDLING.
			Error("Not implemented.\n");
			assert(0);
			
			return(1);
		}
	}
	
	return(0);
}


int GenericFilterDriver::Execute(const FilterTask *ft,const FilterTaskParams *ftp)
{
// Failure simulation code ;-)
//static int _frames=0;
//if(++_frames==4)  return(SPSi_IllegalParams);
	
	// Check for illegal *fp: 
	if(!ft->fdesc || 
	   !ft->infile || !ft->outfile )
	{  return(SPSi_IllegalParams);  }
	
	RefString infile=ft->infile->HDPath();
	RefString outfile=ft->outfile->HDPath();
	
	if(!infile.str() || !outfile.str() || 
	   ft->infile->GetIOType()!=TaskFile::IOTFilterInput || 
	   ft->outfile->GetIOType()!=TaskFile::IOTFilterOutput )
	{  return(SPSi_IllegalParams);  }
	
	if(ft->fdesc->dfactory!=f)   // inapropriate driver (factory)
	{  return(SPSi_IllegalParams);  }
	
	// What gets called: 
	// <binary> <required_args> <add_args>
	
	int failed=0;
	RefStrList args(&failed);
	if(failed)  return(SPS_LMallocFailed);
	
	// Binary path (arg[0]): 
	if(args.append(ft->fdesc->binpath))  ++failed;
	
	// Input & output: 
	//if(Ifile.sprintf(0,"+I%s",infile.str()))  ++failed;
	//if(Ofile.sprintf(0,"+O%s",outfile.str()))  ++failed;
	//if(failed)  return(SPS_LMallocFailed);
	//if(args.append(Ifile))  ++failed;
	//if(args.append(Ofile))  ++failed;
	
	if(args.append(&ft->fdesc->required_args))  ++failed;
	if(args.append(&ft->add_args))  ++failed;
	
	// Okay, that should be all args now.
	if(failed)  return(SPS_LMallocFailed);
	
	// Set up varous other stuff for program launch: 
	ProcessBase::ProcMisc pmisc;
	if(ft->wdir.str())   pmisc.wdir(ft->wdir);
	// niceval set by TaskDriver; timeout dealt with at TaskDriver. 
	
	// Tie input file to stdin and output file to stdout: 
	// Errors reported by ProcessError(). 
	int infd=-1,outfd=-1;
	infd=OpenIOFile(&infile,-1);
	if(infd<0)
	{  return(SPSi_OpenInFailed);  }
	outfd=OpenIOFile(&outfile,+1);
	if(outfd<0)
	{
		close(infd);  infd=-1;
		return(SPSi_OpenOutFailed);
	}
	
	// IMPORTANT: Data hook must be free'd by *US* unless we 
	// actually call FilterDriver::StartProcess(). 
	// Only after a call to FilterDriver::StartProcess(), the 
	// cleanup function is called (in this case, hook MUST be set). 
	
	ProcessBase::ProcFDs sp_f;
	DataHook *dhook=NEW<DataHook>();  // failure checked below. 
	{
		int fail=0;
		fail|=sp_f.Add(infd, 0);
		fail|=sp_f.Add(outfd,1);
		if(ftp && ftp->stderr_fd>=0)  fail|=sp_f.Add(ftp->stderr_fd,2);
		if(fail || !dhook)
		{
			close(infd);  infd=-1;
			close(outfd);  outfd=-1;
			if(dhook)  delete dhook;  dhook=NULL;
			return(SPS_LMallocFailed);
		}
	}
	dhook->infd=infd;
	dhook->outfd=outfd;
	
	// Assign hook. This is a bit ugly: 
	((FilterTaskParams*)ftp)->hook=dhook;
	
	// Okay, construct the command line: 
	ProcessBase::ProcPath sp_p(
		ft->fdesc->binpath,
		component_db()->GetBinSearchPath(ft->dtype));
	
	// Yeah... and pass standard environment: 
	ProcessBase::ProcEnv sp_e;
	int rv=FilterDriver::StartProcess(ft,ftp,&sp_p,&args,&pmisc,&sp_f,&sp_e);
	
	// All the error reporting done in ProcessError(). 
	
	return(rv);
}


GenericFilterDriver::GenericFilterDriver(GenericFilterDriverFactory *gf,
	TaskDriverInterface_Local *tdif,int *failflag) : 
	FilterDriver(gf,tdif,failflag)
{
	
}


GenericFilterDriver::~GenericFilterDriver()
{
	
}


GenericFilterDriver::DataHook::DataHook(int *failflag) : 
	TaskParams::DriverHook(failflag)
{
	infd=-1;
	outfd=-1;
}

GenericFilterDriver::DataHook::~DataHook()
{
	// Bug in rendview if this assertion fails becuase then 
	// some function cleaning up the fds was not called. 
	assert(infd<0 && outfd<0);
}


/********************************** FACTORY **********************************/


TaskDriver *GenericFilterDriverFactory::Create(TaskDriverInterface_Local *tdif)
{
	return(NEW2<GenericFilterDriver>(this,tdif));
}


int GenericFilterDriverFactory::CheckDesc(RF_DescBase *d)
{
	assert(d->dtype==DTFilter);
	FilterDesc *fd=(FilterDesc*)d;
	//fd->blah=blah2;
	return(0 && fd);
}


// Called on program start to set up the GenericFilterDriverFactory; 
// GenericFilterDriverFactory registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int GenericFilterDriverFactory::init(ComponentDataBase *cdb)
{
	GenericFilterDriverFactory *f=NEW1<GenericFilterDriverFactory>(cdb);
	if(!f)
	{
		Error("Failed to initialize generic filter driver.\n");
		return(1);
	}
	Verbose(BasicInit,"[generic] ");
	return(0);
}


const char *GenericFilterDriverFactory::DriverDesc() const
{
	return("Generic filter driver.");
}


GenericFilterDriverFactory::GenericFilterDriverFactory(
	ComponentDataBase *_cdb,int *failflag) : 
	TaskDriverFactory(_cdb,"generic",DTFilter,failflag)
{
	
}


GenericFilterDriverFactory::~GenericFilterDriverFactory()
{
	
}
