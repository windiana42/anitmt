/*
 * filter.cpp
 * 
 * Implementation of filter part of component database 
 * (data node types). 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
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


FilterDesc::FilterDesc(int *failflag) :
	RF_DescBase(failflag),
	binpath(failflag),
	required_args(failflag)
{
	dtype=DTFilter;
}

FilterDesc::~FilterDesc()
{
	dtype=DTNone;
}


FilterTaskParams::FilterTaskParams(int *failflag) : 
	TaskParams(failflag)
{
	dtype=DTFilter;
}

FilterTaskParams::~FilterTaskParams()
{
}


FilterTask::FilterTask(int *failflag) : 
	TaskStructBase(failflag)
{
	dtype=DTFilter;
	fdesc=NULL;
	ofotmat=NULL;
	//timeout=-1;
}

FilterTask::~FilterTask()
{
	// add_args freed by destructor. 
}

