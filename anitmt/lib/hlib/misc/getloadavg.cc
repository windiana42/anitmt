/*
 * getloadavg.c 
 * Get load average. 
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include "prototypes.h"

#include <fcntl.h>


/* This function returns the 1-minute load average multiplied with 100. 
 * Negative return values indicate error. 
 * If it failed once, failure is memorized and all subsequent calls will 
 * return failure. 
 */
int GetLoadAverage()
{
	static int fail=0;
	if(fail)  return(fail);
	
	#if HAVE_GETLOADAVG
	{
		double lv;
		int rv=getloadavg(&lv,1);
		if(rv<0)
		{  fail=-1;  return(fail);  }
		return((int)(100.0*lv+0.5));
	}
	#else   /* no HAVE_GETLOADAVG */
	{
		/* See if we can get load from /proc/loadavg: */
		int fd;
		int lvi=-1;
		char buf[64],*end;
		ssize_t rd;
		do {
			/* Open load source: */
			fd=open("/proc/loadavg",O_RDONLY);
			if(fd<0)  break;
			
			/* Read string: */
			rd=read(fd,buf,64-1);
			if(rd<0)  break;
			
			/* Parse load val. */
			/* We need the first value as it is the 1min load avg. */
			buf[rd]='\0';
			lvi=(int)(strtod(buf,&end)*100+0.5);
			if(end<=buf || lvi<0)  lvi=-1;
		}  while(0);
		
		if(fd>=0)  close(fd);
		if(lvi<0)
		{  fail=-1;  return(fail);  }
		return(lvi);
	}
	#endif
}
