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

#include <assert.h>


int GenericFilterDriver::ProcessError(ProcessErrorInfo*)
{
	Error("NOT IMPLEMENTED");
	return(0);
}


int GenericFilterDriver::Execute(const FilterTask *ft,const FilterTaskParams *ftp)
{
	#warning "Generic filter driver not implemented."
	Error("NOT IMPLEMENTED");
	assert(0 && ft && ft && ftp);
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
		Error("Failed to initialize generic filter driver\n");
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
	verbose=0;
}


GenericFilterDriverFactory::~GenericFilterDriverFactory()
{
	
}
