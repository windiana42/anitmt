/*
 * init.cpp
 * 
 * Task driver interface factory initialisation. 
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


#include <lib/prototypes.hpp>

#include "tdfactory.hpp"

#include "local/dif_param.hpp"
#include "ldr/dif_param.hpp"


// Called on program start to set up the TaskDriverInterfaceFactory 
// classes, so that they register at the ComponentDataBase. 
// Return value: 0 -> OK; >0 -> failed. 
int TaskDriverInterfaceFactory::init_factories(ComponentDataBase *cdb)
{
	// Add an entry for each TaskDriverInterfaceFactory here: 
	int failed=0;
	
	Verbose(BasicInit,"Initializing task driver interfaces: ");
	
	// List the init function of all task sources here. 
	failed+=TaskDriverInterfaceFactory_Local::init(cdb);
	failed+=TaskDriverInterfaceFactory_LDR::init(cdb);
	
	Verbose(BasicInit,failed ? "FAILED\n" : "OK\n");
	
	return(failed);
}

