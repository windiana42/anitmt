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


int GenericFilterDriver::Execute(const FilterTask *rt,const FilterTaskParams *rtp)
{
	#warning "Generic filter driver not implemented."
	Error("NOT IMPLEMENTED");
	assert(0);
}


GenericFilterDriver::GenericFilterDriver(GenericFilterDriverFactory *gf,
	int *failflag) : 
	FilterDriver(gf,failflag)
{
	
}


GenericFilterDriver::~GenericFilterDriver()
{
	
}


/********************************** FACTORY **********************************/


TaskDriver *GenericFilterDriverFactory::Create()
{
	return(NEW1<GenericFilterDriver>(this));
}


int GenericFilterDriverFactory::CheckDesc(RF_DescBase *d)
{
	assert(d->dtype==DTFilter);
	FilterDesc *fd=(FilterDesc*)d;
	//fd->blah=blah2;
	return(0);
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
	Verbose("[generic] ");
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
