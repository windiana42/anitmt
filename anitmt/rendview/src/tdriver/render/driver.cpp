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


// Can be called by ProcessError() function (from lowest level, 
// e.g. POVRayDriver) to output standard messages. 
// prefix: put at beginning of line, e.g. "POV"
// prg_name: name of program to be executed in some fany manner, 
//           e.g. "POVRay". 
// Both MAY NOT be NULL. 
// Return value: 0. 
int RenderDriver::ProcessErrorStdMessage(const char *prefix,const char *prg_name,
	ProcessErrorInfo *pei)
{
	int print_cmd=0;
	
	char _frame_no_str[24];
	if(pei->pinfo && pei->pinfo->ctsk && pei->pinfo->ctsk->frame_no>=0)
	{  snprintf(_frame_no_str,24,"%d",pei->pinfo->ctsk->frame_no);  }
	else
	{  strcpy(_frame_no_str,"???");  }
	
	//const RenderTaskParams *rtp=(const RenderTaskParams*)pei->pinfo->tp;
	const RenderTask *rt = pei->pinfo ? 
		(const RenderTask *)pei->pinfo->tsb : NULL;
	
	switch(pei->reason)
	{
		// *** verbose messages: ***
		case PEI_Starting:
			Verbose(0,"%s: Starting %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			print_cmd=2;
			break;
		case PEI_StartSuccess:
			Verbose(0,"%s: Forked to launch %s [frame %s]...\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_ExecSuccess:
			Verbose(0,"%s: %s started successfully [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		case PEI_RunSuccess:
			Verbose(0,"%s: %s terminated successfully [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			break;
		
		// *** warning/error messages (as you like to define it): ***
		case PEI_Timeout:
			Error("%s: %s [frame %s] exceeded time limit (%ld s).\n",
				prefix,prg_name,
				_frame_no_str,(pei->pinfo->tp->timeout+500)/1000);
			#warning THERE ARE 2 TIMEOUTS!!!! pinfo->tp MAY BE NULL???
			print_cmd=1;
			break;
		
		// *** error messages: ***
		case PEI_StartFailed:
			Error("%s: Failed to start %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			Error("%s:   Error: %s\n",prefix,StartProcessErrorString(pei));
			if(rt)
			{
				Error("%s:   Binary: %s\n",prefix,rt->rdesc->binpath.str());
				Error("%s:   Working dir: %s\n",prefix,
					pei->pinfo->tsb->wdir.str() ? pei->pinfo->tsb->wdir.str() : "[cwd]");
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
			Error("%s: Failed to execute %s [frame %s].\n",
				prefix,prg_name,_frame_no_str);
			Error("%s:   Failure: %s\n",
				prefix,PSFailedErrorString(pei));
			break;
		case PEI_RunFailed:
			Error("%s: %s [frame %s] execution failed.\n",
				prefix,prg_name,_frame_no_str);
			Error("%s:   Failure: ",prefix);
			switch(pei->ps->detail)
			{
				case PSExited:
					Error("Exited with non-zero status %d.\n",
						pei->ps->estatus);
					print_cmd=1;
					break;
				case PSKilled:
					Error("Killed by signal %d (pid %ld)\n",
						pei->ps->estatus,long(pei->pinfo->pid));
					break;
				case PSDumped:
					Error("Dumped (pid %ld, signal %d)\n",
						long(pei->pinfo->pid),pei->ps->estatus);
					print_cmd=1;
					break;
				default:
					Error("???\n");
					abort();
					break;
			}
			break;
	}
	
	// Uh, yes this is an ugly code duplication: 
	if(!pei->pinfo)
	{  print_cmd=0;  }
	if(print_cmd==1)
	{
		Error("%s:   Command:",prefix);
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Error(" %s",i->str());  }
		Error("\n");
		
		if(pei->pinfo->tsb)
		{  const char *tmp=pei->pinfo->tsb->wdir.str();
			Error("%s:   Working dir: %s\n",prefix,tmp ? tmp : "[cwd]");  }
	}
	else if(print_cmd==2)
	{
		Verbose(0,"%s:   Command:",prefix);
		for(const RefStrList::Node *i=pei->pinfo->args.first(); i; i=i->next)
		{  Verbose(0," %s",i->str());  }
		Verbose(0,"\n");
		
		if(pei->pinfo->tsb)
		{  const char *tmp=pei->pinfo->tsb->wdir.str();
			Verbose(0,"%s:   Working dir: %s\n",prefix,tmp ? tmp : "[cwd]");  }
	}
	
	return(0);
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

