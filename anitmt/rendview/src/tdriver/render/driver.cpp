/*
 * driver.cpp
 * 
 * Render driver interface: implementation. 
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

#include <assert.h>


int RenderDriver::Run(
	const TaskStructBase *tsb,
	const TaskParams *tp)
{
	assert(tsb);
	assert(tsb->dtype==DTRender);
	assert(!tp || tp->dtype==DTRender);
	
	RenderTask *tsk=(RenderTask*)tsb;
	RenderTaskParams *tskp=(RenderTaskParams*)tp;
	
	return(Execute(tsk,tskp));
}


RenderDriver::RenderDriver(TaskDriverFactory *df,TaskDriverInterface_Local *tdif,
	int *failflag) : 
	TaskDriver(df,tdif,failflag)
{
	/*int failed=0;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("RenderDriver");  }*/
}


RenderDriver::~RenderDriver()
{
	// ~TaskDriver() unregisteres us. 
}

