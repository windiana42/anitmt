/*
 * srccmd.cc
 * 
 * Implementation of parameter source capable of reading in the 
 * parameters/arguments passed on the command line. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "srccmd.h"

#include <string.h>


#ifndef TESTING
#define TESTING 1
#endif

#warning NEED QUIT-AFTER-FLAG (for --help, --version)

#if TESTING
#warning TESTING switched on 
#endif

namespace par
{

int ParameterSource_CmdLine::ReadCmdLine(CmdLineArgs *cmd,
	Section *_topsect)
{
	int errors=0;
	for(int i=1; i<cmd->argc; i++)
	{
		ParamArg *pa=&cmd->args[i];
		#warning introduce ParamArg::ignore; must call ParamSource::approve() \
			[delete global handler]. 
		#warning should introduce possibility for custom virtual parser in SectionParameterHandler 
		
		int rv=Parse(pa,_topsect);
		if(rv<0)
		{  ++errors;  }
	}
	
	return(errors);
}


int ParameterSource_CmdLine::Parse(ParamArg *pa,Section *topsect)
{
	if(pa->pdone)
	{  return(0);  }
	
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	// Check for implicit args: 
	if(parmanager()->CheckHandleHelpOpts(pa,topsect))
	{
		++n_iquery_opts;
		return(3);
	}
	
	return(FindCopyParseParam(pa,NULL,NULL,topsect));
}


ParameterSource_CmdLine::ParameterSource_CmdLine(
	ParameterManager *_manager,int *failflag) : 
	ParameterSource(_manager,failflag)
{
	n_iquery_opts=0;
}

ParameterSource_CmdLine::~ParameterSource_CmdLine()
{
	// empty
}

}  // namespace end 
