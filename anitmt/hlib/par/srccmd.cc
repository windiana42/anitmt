#include "srccmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>


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


// _topsect may NOT be NULL here: 
int ParameterSource_CmdLine::_CheckSpecialOpts(ParamArg *pa,Section *topsect)
{
	// --version: (only accepted in highmost section)
	if(pa->atype==ParamArg::Option && 
	   topsect==manager->TopSection())
	{
		if(pa->namelen==7 && 
		   pa->name[0]=='v' && 
		   !strncmp(pa->name,"version",pa->namelen))
		{
			if(PrintVersion())
			{
				#warning set used var??
				++n_iquery_opts;
				return(1);
			}
		}
	}
	
	// --help: (section help only prints help for the specified section)
	if(pa->atype==ParamArg::Option && pa->namelen>=4)
	{
		// --sect-sect-help
		if(pa->name>pa->arg.str() &&  // this guarantees that pa->name[-1] is valid, so...
		   !strcmp(&pa->name[pa->namelen-4]-1,"-help"))  // ...namelen-5 is save here. 
		{
			// Must find correct subsection and, if found 
			// call PrintHelp(). 
			const char *end=NULL;
			Section *s=manager->_LookupSection(pa->name,topsect,&end);
			if(s && end==pa->name+pa->namelen-4)
			{
				if(PrintHelp(s))
				{
					#warning set used var??
					
					++n_iquery_opts;
					return(1);
				}
			}
		}
	}
	
	return(0);
}

int ParameterSource_CmdLine::Parse(ParamArg *pa,Section *topsect)
{
	if(pa->pdone)
	{  return(1);  }
	
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	// Check for implicit args: 
	if(_CheckSpecialOpts(pa,topsect))
	{  return(2);  }
	
	// Check if this is a "no-xxx" switch: 
	bool is_switch=(pa->namelen>=3 && !strncmp(pa->name,"no-",3));
	
	ParamInfo *pi=manager->FindParam(
		is_switch ? (pa->name+3) : pa->name,
		pa->namelen,topsect);
	if(!pi)
	{  ParameterError(PETUnknown,pa,pi/*=NULL*/,topsect);  return(-1);  }
	if(is_switch && pi->ptype!=PTSwitch)
	{  ParameterError(PETNotASwitch,pa,pi,topsect);  return(-1);  }
	
	// This will copy the param call error handler if necessary 
	// and warn the user if it will be set more than once. 
	ParamCopy *pc=CopyParamCheck(pi,pa);
	if(!pc)
	{  return(-1);  }
	
	// Parse the value: 
	ParParseState pps=pc->info->vhdl->parse(pi,pc->copyval,pa);
	if(pps)  // >0 -> errors <0 -> warnings
	{
		ValueParseError(pps,pa,pc);
		goto reterror;
	}
	
	// Set the origin...
	// (RefString gets referenced here.) 
	pc->porigin=pa->origin;
	
	return(0);
reterror:;
	_RemoveParamCopy(pc);   // dequeue & free 
	return(-1);
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
