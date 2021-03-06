/*
 * refstringp.cc 
 * 
 * Implementation of allocate once & reference string class RefString. 
 * This file contains the string printf routine offered by RefString. 
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
#if HAVE_STDARG_H
#  include <stdarg.h>
#endif

#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on
#include <assert.h>
#else
#define assert(x) do{}while(0)
#endif


int RefString::vsprintf(size_t maxlen,const char *fmt,va_list ap)
{
	// Get rid of old stuff: 
	_deref();
	
	// First, we must get the length of the string in fmt,...
	const int prealloc=64;
	char buf[prealloc];
	int rv=vsnprintf(buf,prealloc,fmt,ap);
	#if TESTING
	if(rv<0)
	{  fprintf(stderr,"refstringw.cc:%d: vsnprintf() not C99 conform."
		" Please upgrade your libc.\n",__LINE__);  abort();  }
	#endif
	
	size_t dlen=rv;  // (without trailing '\0')
	// The length of the string in fmt,... is dlen. 
	// First, allocate the needed bytes: 
	int trunc=0;
	if(maxlen && dlen>=maxlen)  // (dlen without NUL, maxlen with NUL char)
	{  dlen=maxlen-1;  ++trunc;  }
	ref=_alloc(dlen+1,0);   // +1 for terminating '\0'
	if(!ref)
	{  return(-1);  }
	if(rv<prealloc)  // okay.
	{
		// Whole string fits into prealloc buf. Simply copy. 
		memcpy(_str(),buf,dlen);  // okay. 
	}
	else
	{
		// Must re-format the string. 
		int rv2=vsnprintf(_str(),dlen+1,fmt,ap);
		assert(rv==rv2);
	}
	*(_str()+dlen)='\0';  // to be sure...
	return(trunc);
}


int RefString::sprintf(size_t maxlen,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=vsprintf(maxlen,fmt,ap);
	va_end(ap);
	return(rv);
}
