/*
 * fcall.cpp
 * 
 * Function caller class for input lib / exparse. 
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

#include "fcall.hpp"

#include <stdio.h>
#include <assert.h>

namespace exparse
{

FunctionState FunctionCaller::AddArg(const Value *arg)
{
	if(!fa.fdesc)  return(FSNoFunction);
	// Check if the function allows that many args. 
	if(( (*fa.fdesc->check_nargs)(fa.nargs+1) ) == 2)  // too many
	{  return(FSTooManyArgs);  }
	// See if we have to re-alloc fdesc: 
	const int malloc_thresh=8;  // alloc ahead threshold 
	if(!(fa.nargs%malloc_thresh))
	{
		#warning should use mcache here: 
		Value **newargs = new Value*[fa.nargs+malloc_thresh];
		if(fa.nargs)
		{  memcpy(newargs,fa.args,fa.nargs*sizeof(Value*));  }
		delete[] fa.args;
		fa.args=(const Value**)newargs;
	}
	fa.args[fa.nargs++]=arg;
	//fa.nargs=nargs;  (fa.nargs incremented above)
	return(FSSuccess);
}


inline void FunctionCaller::_SetFunction(const FunctionDesc *fd)
{
	// We must reset the stuff: 
	fa.result=NULL;
	fa.nargs=0;
	fa.fdesc=fd;  // ...which may be NULL. 
}

int FunctionCaller::SetFunction(const char *name)
{
	const FunctionDesc *fd=LookupFunction(name);
	_SetFunction(fd);  // This also gets called with fd=NULL. 
	return(fd ? 0 : 1);
}


void FunctionCaller::FreeArgs()
{
	for(int i=0; i<fa.nargs; i++)
	{
		if(fa.args[i])
		{  delete fa.args[i];  fa.args[i]=NULL;  }
	}
}


void FunctionCaller::SetFunction(const FunctionDesc *fd)
{
	_SetFunction(fd);
}


FunctionCaller::FunctionCaller()
{
	fa.result=NULL;
	fa.nargs=0;
	fa.args=NULL;
}

FunctionCaller::FunctionCaller(const FunctionDesc *_fdesc,Value *res)
{
	fa.args=NULL;
	_SetFunction(_fdesc);
	fa.result=res;
}

FunctionCaller::~FunctionCaller()
{
	if(fa.args)  delete[] fa.args;  fa.args=NULL;
}

}  // namespace end 
