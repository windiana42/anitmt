/*
 * init.cpp
 * 
 * Task source factory initialisation. 
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


#include "tsfactory.hpp"

#include "local/param.hpp"


// Called on program start to set up the TaskSourceFactory classes, 
// so that they register at the ComponentDataBase. 
// Return value: 0 -> OK; >0 -> failed. 
int TaskSourceFactory::init_factories(ComponentDataBase *cdb)
{
	// Add an entry for each TaskSourceFactory here: 
	int failed=0;
	
	Verbose("Initializing task sources: ");
	
	// List the init function of all drivers here. 
	// Currently the POVRay driver is feeling lonely...
	failed+=TaskSourceFactory_Local::init(cdb);
	
	Verbose(failed ? "FAILED\n" : "OK\n");
	
	return(failed);
}

