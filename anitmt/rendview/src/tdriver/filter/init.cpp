/*
 * init.cpp
 * 
 * Driver initialisation routine. 
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

#include "../../database.hpp"
#include "filter.hpp"

#include "driver/generic.hpp"


// Called at startup to initialize all the filter driver factories
// Return: 0 -> OK; >0 -> error
int FilterDriver::init_factories(ComponentDataBase *cdb)
{
	int failed=0;
	
	Verbose(BasicInit,"Initializing filter drivers: ");
	
	// List the init function of all drivers here. 
	// Currently the generic driver is feeling lonely...
	failed+=GenericFilterDriverFactory::init(cdb);
	
	Verbose(BasicInit,failed ? "FAILED\n" : "OK\n");
	
	return(failed);
}

