/*
 * rvapgrowbuffer.cpp
 * 
 * Implementation of growing buffer which can store data written using 
 * standard RVAPGrowBuffer::printf() calls. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "rvapgrowbuffer.hpp"

#include <string.h>
#if HAVE_STDARG_H
#  include <stdarg.h>
#endif

#include <assert.h>



int RVAPGrowBuffer::vprintf(const char *fmt,va_list ap)
{
	// See if we can stuff it into the current buffer size. 
	size_t avail=_size-_len;
	char *dest=_str+_len;
	int rv=vsnprintf(dest,avail,fmt,ap);
	// Otherwise: Need to update libc. 
	assert(rv>=0);
	
	size_t want=rv;  // length of string without trailing '\0'
	if(want<=avail)
	{
		// Okay, the complete string fit into the buffer. 
		_len+=want;
		return(0);
	}
	
	// Need to enlarge buffer: 
	size_t delta=want-avail;
	if(delta<1024)  delta=1024;
	if(_realloc(_size+delta))
	{
		// Alloc failure. 
		const char *afstr="\n[alloc failure]\n";
		const size_t aflen=17;
		if(_size>=aflen)
		{  memcpy(_str+_size-aflen,afstr,aflen);  }
		return(-1);
	}
	
	// Finally, print it: 
	avail=_size-_len;
	rv=vsnprintf(dest,avail,fmt,ap);
	assert(rv==want);
	
	_len+=want;
	return(0);
}


int RVAPGrowBuffer::printf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=vprintf(fmt,ap);
	va_end(ap);
	return(rv);
}
