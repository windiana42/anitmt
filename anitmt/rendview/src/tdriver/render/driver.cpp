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

//#include "../../database.hpp"
//#include "render.hpp"

#include <tsource/taskfile.hpp>
#include <tsource/tasksource.hpp>

#include <assert.h>


// This outputs the primary reason message (i.e. the first line of the 
// errors / status messages). See tdriver.cpp for more info. 
// Return value: print_cmd. 
int RenderDriver::ProcessError_PrimaryReasonMessage(
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	return(TaskDriver::ProcessError_PrimaryReasonMessage(prefix,prg_name,pei));
}


// Helper function: print command to be executed.
// print_cmd: 
//    0 -> do nothing 
//    1 -> print using Error() 
//    2 -> print using Verbose() 
void RenderDriver::ProcessError_PrintCommand(int print_cmd,
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	TaskDriver::ProcessError_PrintCommand(print_cmd,prefix,prg_name,pei);
}


// Can be called by ProcessError() function (from lowest level, 
// e.g. POVRayDriver) to output standard messages. 
// prefix: put at beginning of line, e.g. "POV"
// prg_name: name of program to be executed in some fany manner, 
//           e.g. "POVRay". 
// Both MAY NOT be NULL. 
void RenderDriver::ProcessErrorStdMessage(
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	// Print primary reason (first line only): 
	int print_cmd=ProcessError_PrimaryReasonMessage(prefix,prg_name,pei);
	
	const RenderTaskParams *rtp = pei->pinfo ? 
		(const RenderTaskParams*)pei->pinfo->tp : NULL;
	const RenderTask *rt = pei->pinfo ? 
		(const RenderTask *)pei->pinfo->tsb : NULL;
	
	switch(pei->reason)
	{
		// *** verbose messages: ***
		case PEI_Starting:      // This is...
		case PEI_StartSuccess:  // ...all handeled...
		case PEI_ExecSuccess:   // ...by ProcessError_PrimaryReasonMessage(). 
		case PEI_RunSuccess:   break;
		
		// *** warning/error messages (as you like to define it): ***
		case PEI_Timeout:
			print_cmd=TaskDriver::ProcessErrorStdMessage_Timeout(
				prefix,prg_name,pei);
			break;
		
		// *** error messages: ***
		case PEI_StartFailed:
			Error("%s:   Error: %s\n",prefix,StartProcessErrorString(pei));
			if(rt)
			{
				Error("%s:   Binary: %s\n",prefix,rt->rdesc->binpath.str());
				Error("%s:   Working dir: %s\n",prefix,
					rt->wdir.str() ? rt->wdir.str() : "[cwd]");
				Error("%s:   Search path:",prefix);
				for(const RefStrList::Node *i=component_db()->GetBinSearchPath(
					rt->dtype)->first(); i; i=i->next)
				{  Error(" %s",i->str());  }
				Error("\n");
				if(rt->rdesc->binpath[0]=='/')
				{  Error("%s:   Note: search path not used as binary "
					"contains absolute path\n",prefix);  }
			}
			break;
		case PEI_ExecFailed:
			print_cmd=TaskDriver::ProcessErrorStdMessage_ExecFailed(
				prefix,prg_name,pei);
			break;
		case PEI_RunFailed:
			print_cmd=TaskDriver::ProcessErrorStdMessage_RunFailed(
				prefix,prg_name,pei);
			break;
	}
	
	ProcessError_PrintCommand(print_cmd,prefix,prg_name,pei);
}


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

