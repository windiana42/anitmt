/*
 * tsfactory.cpp
 * 
 * Task source factory class. 
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
#include "../database.hpp"


TaskSourceFactory::TaskSourceFactory(const char *_ts_name,
	ComponentDataBase *cdb,int *failflag) : 
	ParameterConsumer_Overloaded(cdb->parmanager(),failflag)
{
	_component_db=cdb;
	ts_name=_ts_name;
	int failed=0;
	
	if(_component_db->RegisterSourceFactory(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory");  }
}

TaskSourceFactory::~TaskSourceFactory()
{
	// ...even if we're not registered. 
	_component_db->UnregisterSourceFactory(this);
}
