/*
 * render.cpp
 * 
 * Implementation of render part of component database 
 * (data node types). 
 * 
 * Copyright (c) 2001--2003 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <lib/mymath.hpp>   /* for NAN, etc. */


RenderDesc::RenderDesc(int *failflag) :
	RF_DescBase(failflag),
	binpath(failflag),
	required_args(failflag),
	include_path(failflag)
{
	dtype=DTRender;
	
	can_resume_render=false;
	can_pass_frame_clock=false;
}

RenderDesc::~RenderDesc()
{
	dtype=DTNone;
}


RenderTaskParams::RenderTaskParams(int *failflag) : 
	TaskParams(failflag)
{
	dtype=DTRender;
	
  	stdout_fd=-1;
	stderr_fd=-1;
	stdin_fd=-1;
}

RenderTaskParams::~RenderTaskParams()
{
	#warning what about std*_fd?
}


RenderTask::RenderTask(int *failflag) : 
	TaskStructBase(failflag)
{
	dtype=DTRender;
	rdesc=NULL;
	width=-1;
	height=-1;
	oformat=NULL;
	//timeout=-1;
	resume=0;
	resume_flag_set=0;
	use_frame_clock=0;
	// FrameClockVal: UNUSED
	//frame_clock=NAN;
}

RenderTask::~RenderTask()
{
	// add_args freed by destructor. 
}

