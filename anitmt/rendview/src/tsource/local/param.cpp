/*
 * param.cpp
 * 
 * Implementation of parameter & factory class of local task source. 
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


#include "local.hpp"
#include "param.hpp"

#include <assert.h>


		
// Create a local TaskSource (TaskSource_Local): 
TaskSource *TaskSourceFactory_Local::Create()
{
	return(NEW1<TaskSource_Local>(this));
}


int TaskSourceFactory_Local::_RegisterParams()
{
	if(SetSection("l","loacal task source"))
	{  return(-1);  }
	
	AddParam("pattern","frame pattern (e.g. frame_dir/f%07d.pov)",&frame_pattern);
	AddParam("nframes|n","number of frames to process",&nframes);
	
	return(add_failed ? (-1) : 0);
}


// Called on program start to set up the TaskSourceFactory_Local. 
// TaskSourceFactory_Local registers at ComponentDataBase. 
// Return value: 0 -> OK; >0 -> error. 
int TaskSourceFactory_Local::init(ComponentDataBase *cdb)
{
	TaskSourceFactory_Local *s=NEW1<TaskSourceFactory_Local>(cdb);
	if(!s)
	{
		Error("Failed to initialize local task source.\n");
		return(1);
	}
	Verbose("[local] ");
	return(0);
}


TaskSourceFactory_Local::TaskSourceFactory_Local(
	ComponentDataBase *cdb,int *failflag) : 
	TaskSourceFactory("local",cdb,failflag),
	frame_pattern(failflag)
{
	nframes=1;
	
	int failed=0;
	
	if(frame_pattern.set("f%07d.pov"))
	{  ++failed;  }
	if(_RegisterParams())
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("TaskSourceFactory_Local");  }
}

TaskSourceFactory_Local::~TaskSourceFactory_Local()
{

}
