/*
 * refstring.cc 
 * 
 * Implementation of allocate once & reference string class RefString. 
 * This file contains only the RefString core implementation, other 
 * features are in other files. 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "refstring.h"
#include <string.h>

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on
#include <assert.h>
#else
#define assert(x) do{}while(0)
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


int RefString::set0(const char *str_to_copy,size_t len)
{
	_deref();
	ref=_alloc(len+1,0);
	if(!ref)  return(-1);
	// Copy string...
	char *dest=(char*)(ref+1);
	memcpy(dest,str_to_copy,len);
	dest[len]='\0';   // ...and terminate
	return(0);
}


int RefString::zero()
{
	if(!ref)  return(1);
	if((*ref)>3)
	{  _deref();  return(1);  }
	
	// we're the only ref. 
	if(_sflag())
	{
		memset(_str(),0,_rlength());
		*(((size_t*)ref)-1)=0;  // zero size, too
	}
	else
	{
		for(char *c=_str(); *c; )
		{  *(c++)='\0';  }
	}
	
	_deref();
	return(0);
}


bool RefString::operator==(const RefString &b) const
{
	// First, the fast path: if it is the same ref, then also 
	// the same string: 
	if(b.ref==ref)  return(true);
	// The case that both refs were NULL was dealed with above. 
	// See if they are of the same type: 
	int ot=stype();
	if(ot!=b.stype())  return(false);
	// Okay, neither ref is NULL here (same type!). 
	if(ot)   // data string (with size flag)
	{
		// Okay, first check size: 
		size_t ol=_rlength();
		if(ol!=b._rlength())  return(false);
		return(!memcmp(_str(),b._str(),ol));
	}
	// '\0'-terminated string; no size flag. 
	return(!strcmp(_str(),b._str()));
}


bool RefString::operator==(const char *b) const
{
	// If not '\0'-terminated, return 0: 
	if(stype()==1)  return(false);
	// First, the fast path: if it is the same pointer, 
	// then also the same string: 
	const char *a=str();
	if(a==b)  return(true);
	// The case that both were NULL was dealed with above. 
	if(!a || !b)  return(false);
	return(!strcmp(a,b));
}


void RefString::_destroy()
{
	//if(ref)  // <- checked by caller
	{
		assert(ref && (*ref==0 || *ref==1));
		// Now, free the string...
		// If the size flag is set, we must go back sizeof(size_t) 
		// bytes, else *ref is the right pointer. 
		// NOTE: MAY NOT look at size because it may have been zeroed. 
		LFree(_sflag() ? (int*)(((size_t*)ref)-1) : ref);
		//ref=NULL;  // <- done by caller. 
	}
}
