/*
 * refstring.cc 
 * 
 * Implementation of allocate once & reference string class RefString. 
 * This file contains only the RefString core implementation, other 
 * features are in other files. (See COPYING.LGPL for details.)
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
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

// Internal layout: 
//                +---ref-pointer---+
// [bytes -4...-1][bytes 0...3|bit00][bytes 4...n]
// [string length][ref counter|sflag][string data]
// sflag: size flag: 1 -> string has a string length field of type 
//                        size_t before *ref. 
//                   0 -> no string length field (0-terminated)
// ref counter: NOTE: the counter always is decreased/increased 
//                    by 2 NOT by 1 (as the first bit is the size 
//                    flag). Using 2 inc/decrement is easier than 
//                    messing around with bitmasks. 

void RefString::_setup_constr(int *failflag,const char *str_to_copy,
	const size_t *lenptr)
{
	ref=NULL;
	int failed=_copy(str_to_copy,lenptr);  // returns 0 or -1. 
	
	if(failflag)
	{  *failflag+=failed;  }   // failed<0 -> failflag decreased. 
	else if(failed)
	{  ConstructorFailedExit();  }
}


// Allocate a reference. A size flag is allocated if sflag!=0. 
// Return value: pointer to the reference counter or NULL. 
// The ref counter is set up to ONE (i.e. 2 or 3 depending on 
// sflag), the size flag (if present) to datalen. 
int *RefString::_alloc(size_t datalen,int sflag)
{
	size_t alloclen=datalen+sizeof(*ref);
	if(sflag)
	{  alloclen+=sizeof(size_t);  }
	char *nref=(char*)LMalloc(alloclen);
	if(nref)
	{
		if(sflag)
		{
			*((size_t*)nref)=datalen;
			nref+=sizeof(size_t);
			*((int*)nref)=3;  // sflag is set
		}
		else
		{  *((int*)nref)=2;  }  // sflag not set
	}
	return((int*)nref);
}


// lenptr: NULL -> create string without length info 
//       !=NULL -> read *lenptr for string length and create string 
//                 with length info entry (set size flag). 
int RefString::_copy(const char *str_to_copy,const size_t *lenptr)
{
	assert(!ref);
	
	if(str_to_copy)
	{
		size_t str_len = lenptr ? (*lenptr) : (strlen(str_to_copy)+1);
		// Allocate data and set up reference counter and size field: 
		ref=_alloc(str_len,lenptr ? 1 : 0);
		if(!ref)
		{  return(-1);  }
		
		// Copy string: 
		memcpy((char*)(ref+1),str_to_copy,str_len);
	}
	return(0);
}


void RefString::_destroy()
{
	//if(ref)  // <- checked by caller
	{
		assert(ref && (*ref==0 || *ref==1));
		// Now, free the string...
		// If the size flag is set, we must go back sizeof(size_t) 
		// bytes, else *ref is the right pointer. 
		LFree(_sflag() ? (int*)(((size_t*)ref)-1) : ref);
		//ref=NULL;  // <- done by caller. 
	}
}
