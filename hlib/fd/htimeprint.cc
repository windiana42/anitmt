/*
 * htimeprint.cc
 * 
 * Implementation of methods supporting string output for class HTime. 
 * 
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/htime.h>

static const char *_is_invalid_str="[invalid time]";

const char *HTime::PrintTime(int local,int with_msec) const
{
	// Negative times do not make sense here. 
	// (Or should that be before 1970?!)
	if(tv.tv_sec<0)  // This check is correct and sufficient. (usec NEVER negative)
	{  return("[negative time]");  }
	if(IsInvalid())
	{  return(_is_invalid_str);  }
	
	time_t timep=tv.tv_sec;
	int msec=(tv.tv_usec+500)/1000;
	if(msec>=1000)
	{  ++timep;  msec-=1000;  }
	if(msec>=500 && !with_msec)  ++timep;  
	
	struct tm *stm;
	if(local)  stm=localtime(&timep);
	else       stm=gmtime(&timep);
	
	static char tmp[48];
	char *ptr=tmp;
	char *end=tmp+48;
	ptr+=strftime(ptr,end-tmp,"%a %b %d %Y, %H:%M:%S",stm);
	if(with_msec && end>tmp)
	{  ptr+=snprintf(ptr,end-ptr,".%03d",msec);  }
	if(end>tmp)
	{  strftime(ptr,end-tmp," %Z",stm);  }
	
	return(tmp);
}


const char *HTime::PrintElapsed(int with_msec) const
{
	if(IsInvalid())
	{  return(_is_invalid_str);  }
	
	static char tmp[32];
	char *ptr=tmp;
	char *end=tmp+32;
	
	long sec,msec;
	if(tv.tv_sec>=0)  // This check is correct and sufficient. (usec NEVER negative)
	{
		sec=tv.tv_sec;
		msec=(tv.tv_usec+500)/1000;
	}
	else
	{
		sec=-tv.tv_sec-1;
		msec=((1000000-tv.tv_usec)+500)/1000;
		*(ptr++)='-';
	}
	if(msec>=1000)
	{  --sec;  msec-=1000;  }
	// Correct rounding. (sec>=0 here because negative case handeled above). 
	if(!with_msec && msec>=500)
	{  ++sec;  }
	
	long hours=(sec/3600);
	if(hours>48)
	{
		ptr+=snprintf(ptr,end-ptr,"%ldd ",hours/24);
		hours%=24;
	}
	snprintf(ptr,end-ptr,with_msec ? "%02ld:%02ld:%02ld.%03ld" : "%02ld:%02ld:%02ld",
		hours,
		(sec/60)%60,
		sec%60,
		msec);
	return(tmp);
}
