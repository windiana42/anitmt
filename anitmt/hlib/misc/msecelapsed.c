/*
 * msecelapsed.c 
 * 
 * Copyright (c) 2000 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <unistd.h>
#include <sys/time.h>

#include <hlib/prototypes.h>

/* calc the time between *old and *current in msec. 
 * current may be NULL (then it is queried from the system). 
 */
long msec_elapsed(const struct timeval *old,const struct timeval *current)
{
	struct timeval _curr;
	long du,ds;
	if(!current)
	{
		gettimeofday(&_curr,NULL);
		current=&_curr;
	}
	du = current->tv_usec - old->tv_usec;
	ds = current->tv_sec  - old->tv_sec;
	if(du<0L && ds>=0L)  {  --ds;  du+=1000000L;  }
	if(du>=0L && ds<0L)  {  ++ds;  du-=1000000L;  }
	return(ds*1000L + du/1000L);
}
