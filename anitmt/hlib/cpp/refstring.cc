/*
 * refstring.cc 
 * 
 * Simple implementation of a allocate once & reference - string. 
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

#include "refstring.h"
#include <string.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on
#include <assert.h>
#else
#define assert(x)
#endif


RefString::RefString(const char *str_to_copy,int *failflag=NULL)
{
	ref=NULL;
	int failed=_copy(str_to_copy);  // returns 0 or -1. 
	
	if(failflag)
	{  *failflag+=failed;  }   // failed<0 -> failflag decreased. 
	else if(failed)
	{  ConstructorFailedExit();  }
}


int RefString::_copy(const char *str_to_copy)
{
	assert(!ref);
	
	if(str_to_copy)
	{
		ref=(int*)LMalloc(strlen(str_to_copy)+1+sizeof(*ref));
		if(ref)
		{
			*ref=1;  // correct
			strcpy((char*)str(),str_to_copy);
		}
		else
		{  return(-1);  }
	}
	return(0);
}


void RefString::_destroy()
{
	//if(ref)  // <- checked by caller
	{
		assert(*ref==0);
		LFree(ref);   // REALLY!! str=ref+sizeof(*ref);
		//ref=NULL;  // <- done by caller. 
	}
}
