/*
 * gdriverfact.cpp
 * 
 * Generic (render/filter) driver factory stuff and some constructors 
 * and destructors of smaller classes. 
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


#include "taskmanager.hpp"
#include "gdriver.hpp"

#include "tsource/taskfile.hpp"


// Returns string representation of TaskDriverType: 
char *DTypeString(TaskDriverType dt)
{
	switch(dt)
	{
		case DTNone:    return("[none]");
		case DTRender:  return("render");
		case DTFilter:  return("filter");
	}
	return("???");
}


// Called on statup to initialize the (render and filter) 
// driver factories; Return value: 0 -> OK; >0 -> error. 
int TaskDriverFactory::init(ComponentDataBase *cdb)
{
	int failed=0;
	
	failed+=RenderDriver::init_factories(cdb);
	failed+=FilterDriver::init_factories(cdb);
	
	return(failed);
}


TaskDriverFactory::TaskDriverFactory(
	ComponentDataBase *_cdb,
	const char *_driver_name,
	TaskDriverType tdt,
	int *failflag=NULL) : 
	LinkedListBase<TaskDriverFactory>(),
	par::ParameterConsumer_Overloaded(_cdb->parmanager(),failflag),
	dtype(tdt),
	driver_name(_driver_name,failflag)
{
	int failed=0;
	
	component_db=_cdb;
	
	if(!driver_name.str())
	{  ++failed;  }
	if(component_db->RegisterDriverFactory(this,dtype))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskDriverFactory");  }
}

TaskDriverFactory::~TaskDriverFactory()
{
	// ...even if we're not registered. 
	component_db->UnregisterDriverFactory(this,dtype);
}


/******************************************************************************/

TaskParams::TaskParams(int *failflag=NULL) : 
	add_args(failflag),
	crdir(failflag),
	wdir(failflag)
{
	dtype=DTNone;
	
	niceval=NoNice;
	timeout=-1;
};

TaskParams::~TaskParams()
{

}


/******************************************************************************/

TaskStructBase::TaskStructBase(int *failflag) : 
	add_args(failflag)
{
	infile=NULL;
	outfile=NULL;
	
	dtype=DTNone;
}

TaskStructBase::~TaskStructBase()
{
	if(infile)
	{  delete infile;  infile=NULL;  }
	if(outfile)
	{  delete outfile;  outfile=NULL;  }
	
	dtype=DTNone;
}


/******************************************************************************/

TaskDriver::PInfo::PInfo(int *failflag) : 
	args(failflag)
{
	pid=-1;
	tsb=NULL;
	tp=NULL;
	
	ct=NULL;
}

TaskDriver::PInfo::~PInfo()
{
	// be sure...
	tsb=NULL;
	tp=NULL;
	ct=NULL;
}
