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


int GenericFilterDriver::ProcessError(ProcessErrorInfo *pei)
{
	Error("NOT IMPLEMENTED");
	return(0);
}


int GenericFilterDriver::Execute(const FilterTask *ft,const FilterTaskParams *ftp)
{
	#warning "Generic filter driver not implemented."
	Error("NOT IMPLEMENTED");
	assert(0 && ft && ft && ftp);
	
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
	int infd=-1,outfd=-1;
#warning check errors!!!!!!!!!!!!
assert(0);
	infd=OpenIOFile(&infile,-1);
	if(infd<0)
	{
		return(SPSi_OpenInFailed);
	}
	outfd=OpenIOFile(&outfile,+1);
	if(outfd<0)
	{
		
		close(infd);  infd=-1;
		return(SPSi_OpenOutFailed);
	}
	
	ProcessBase::ProcFDs sp_f;
	{
		int fail=0;
		fail|=sp_f.Add(infd, 0);
		fail|=sp_f.Add(outfd,1);
		if(fail)
		{
			close(infd);  infd=-1;
			close(outfd);  outfd=-1;
			return(SPS_LMallocFailed);
		}
	}
	
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
