/*
 * growbuffer.cc 
 * 
 * Implementation of a grwoing buffer. 
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

#define HLIB_IN_HLIB 1
#include "growbuffer.h"

// NOTE: len is not changed by this function 
//       returns -1 on realloc failure 
int GrowBuffer::_realloc(size_t newsize)
{
	char *old=_str;
	_str=(char*)LRealloc(_str,newsize);
	if(!_str && newsize)
	{
		// keep old string
		_str=old;
		return(-1);
	}
	_size=newsize;
	return(0);
}


int GrowBuffer::append(const char *buf,size_t len)
{
	if(!buf || !len)
	{  return(0);  }
	
	// See if it fits into the buffer: 
	size_t need=len+_len;
	if(need>_size)   // must realloc
	{
		if(_realloc(need))
		{  return(-1);  }
	}
	memcpy(_str+_len,buf,len);
	_len+=len;
	return(0);
}


int GrowBuffer::set(const char *buf,size_t len)
{
	// See if it fits into the buffer: 
	if(len>_size)
	{
		if(_realloc(len))
		{  return(-1);  }
	}
	if(len)
	{  memcpy(_str,buf,len);  }
	_len=len;
	return(0);
}


int GrowBuffer::trunc(size_t newsize,int free_rest)
{
	if(newsize<_len)
	{  _len=newsize;  }
	
	if(free_rest && _len>_size)
	{  return(_realloc(_len));  }
	
	return(0);
}

int GrowBuffer::skip(size_t nbytes,int free_rest)
{
	if(nbytes>=_len)
	{  _len=0;  }
	else
	{
		_len-=nbytes;
		memcpy(_str,_str+nbytes,_len);
	}
	
	if(free_rest && _len>_size)
	{  return(_realloc(_len));  }
	
	return(0);
}


GrowBuffer::GrowBuffer(size_t reserv_len,int *failflag)
{
	_size=reserv_len;
	_len=0;
	_str=(char*)(_size ? LMalloc(_size) : NULL);
	if(!_str && _size)
	{
		_size=0;
		if(failflag)  --(*failflag);
		else  ConstructorFailedExit();
	}
}

GrowBuffer::~GrowBuffer()
{
	_str=(char*)LFree(_str);
	_len=0;
	_size=0;
}
