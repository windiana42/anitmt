/*
 * srcenv.cc
 * 
 * Implementation of parameter source capable of reading in the 
 * parameters/arguments passed using the environment. 
 * 
 * Copyright (c) 2003 --2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include "srcenv.h"

#include <string.h>
#include <ctype.h>


#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on (using assert())
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


namespace par
{

int ParameterSource_Environ::ReadEnviron(char **envp,Section *topsect)
{
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	if(!envp)
	{  return(0);  }
	
	int errors=0;
	for(int i=0; envp[i]; i++)
	{
		int fflag=0;
		ParamArg pa(envp[i],&fflag);
		if(fflag)  // alloc failure
		{  errors=-1;  break;  }
		
		int rv=Parse(&pa,topsect);
		if(rv<0)
		{  ++errors;  }
	}
	
	return(errors);
}


int ParameterSource_Environ::Parse(ParamArg *pa,Section *topsect)
{
	if(pa->pdone)
	{  return(1);  }
	
	if(!topsect)
	{  topsect=manager->TopSection();  }
	
	ParamInfo *pi=manager->FindParamForEnviron(pa->name,pa->namelen,topsect);
	//fprintf(stderr,"<%.*s> -> %p\n",int(pa->namelen),pa->name,pi);
	if(!pi)  return(10);
	
	// This will copy the param call error handler if necessary 
	// and warn the user if it will be set more than once. 
	// Then, parse the value, set the origin... and return !=NULL 
	// if all that went okay. 
	ParamCopy *pc=CopyParseParam(pi,pa);
	if(!pc)  return(-4);
	
	return(2);

}


ParameterSource_Environ::ParameterSource_Environ(
	ParameterManager *_manager,int *failflag) : 
	ParameterSource(_manager,failflag)
{
	// empty
}

ParameterSource_Environ::~ParameterSource_Environ()
{
	// empty
}

}  // namespace end 
