/*
 * memfind.c
 *
 * Copyright (c) 2000--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>

#include <string.h>


char *memfind(const char *buf,const char *tofind,size_t len)
{
	size_t findlen=strlen(tofind);
	if(findlen>len)
	{  return((char*)NULL);  }
	if(len<1)
	{  return((char*)buf);  }
	
	{
		const char *bufend=&buf[len-findlen+1];
		const char *c=buf;
		for(; c<bufend; c++)
		{
			if(*c==*tofind)  // first letter matches
			{
				if(!memcmp(c+1,tofind+1,findlen-1))  // found
				{  return((char*)c);  }
			}
		}
	}
	
	return((char*)NULL);
}

