/*
 * driver.cpp
 * 
 * Filter driver interface: implementation. 
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
//#include "filter.hpp"

#include <tsource/taskfile.hpp>
#include <tsource/tasksource.hpp>

#include <assert.h>


// This outputs the primary reason message (i.e. the first line of the 
// errors / status messages). See tdriver.cpp for more info. 
// Return value: print_cmd. 
int FilterDriver::ProcessError_PrimaryReasonMessage(
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
void FilterDriver::ProcessError_PrintCommand(int print_cmd,
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	TaskDriver::ProcessError_PrintCommand(print_cmd,prefix,prg_name,pei);
}


// Can be called by ProcessError() function (from lowest level, 
// e.g. generic filter driver) to output standard messages. 
// prefix: put at beginning of line, e.g. "GFD"
// prg_name: name of program to be executed in some fany manner, 
//           e.g. "invert-filter". 
// Both MAY NOT be NULL. 
void FilterDriver::ProcessErrorStdMessage(
	const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	// Print primary reason (first line only): 
	int print_cmd=ProcessError_PrimaryReasonMessage(prefix,prg_name,pei);
	
	const FilterTaskParams *ftp = pei->pinfo ? 
		(const FilterTaskParams*)pei->pinfo->tp : NULL;
	const FilterTask *ft = pei->pinfo ? 
		(const FilterTask *)pei->pinfo->tsb : NULL;
	
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
			if(ft)
			{
				Error("%s:   Binary: %s\n",prefix,ft->fdesc->binpath.str());
				Error("%s:   Working dir: %s\n",prefix,
					ft->wdir.str() ? ft->wdir.str() : "[cwd]");
				Error("%s:   Search path:",prefix);
				for(const RefStrList::Node *i=component_db()->GetBinSearchPath(
					ft->dtype)->first(); i; i=i->next)
				{  Error(" %s",i->str());  }
				Error("\n");
				if(ft->fdesc->binpath[0]=='/')
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


int FilterDriver::Run(
	const TaskStructBase *tsb,
	const TaskParams *tp)
{
	assert(tsb);
	assert(tsb->dtype==DTFilter);
	assert(!tp || tp->dtype==DTFilter);
	
	FilterTask *tsk=(FilterTask*)tsb;
	FilterTaskParams *tskp=(FilterTaskParams*)tp;
	
	return(Execute(tsk,tskp));
}


FilterDriver::FilterDriver(TaskDriverFactory *df,TaskDriverInterface_Local *tdif,
	int *failflag) : 
	TaskDriver(df,tdif,failflag)
{
	/*int failed=0;
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("FilterDriver");  }*/
}


FilterDriver::~FilterDriver()
{
	// ~TaskDriver() unregisteres us. 
}

