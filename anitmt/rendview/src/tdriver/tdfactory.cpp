/*
 * tdfactory.cpp
 * 
 * Task driver interface factory class. 
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
#include "../database.hpp"


TaskDriverInterfaceFactory::TaskDriverInterfaceFactory(const char *_tdif_name,
	ComponentDataBase *cdb,int *failflag) : 
	ParameterConsumer_Overloaded(cdb->parmanager(),failflag)
{
	_component_db=cdb;
	tdif_name=_tdif_name;
	int failed=0;
	
	if(_component_db->RegisterDriverInterfaceFactory(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverInterfaceFactory");  }
}

TaskDriverInterfaceFactory::~TaskDriverInterfaceFactory()
{
	// ...even if we're not registered. 
	_component_db->UnregisterDriverInterfaceFactory(this);
}
