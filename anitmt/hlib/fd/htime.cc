/*
 * htime.cc
 * 
 * Implementation of methods supporting class HTime. 
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <hlib/htime.h>


const long long HTime::conv_fact[HTime::_tslast]=
{ 1LL, 1000LL, 1000000LL, 60000000LL, 3600000000LL, 86400000000LL };
const double HTime::conv_factD[HTime::_tslast]=
{ 1.0, 1000.0, 1000000.0, 60000000.0, 3600000000.0, 86400000000.0 };


void HTime::_SetVal(long val,TimeSpec sp,timeval *tv)
{
	tv->tv_usec=0L;
	switch(sp)
	{
		case usec:
			tv->tv_sec=val/1000000L;
			tv->tv_usec=val%1000000L;
			break;
		case msec:
			tv->tv_sec=val/1000L;
			tv->tv_usec=1000L*(val%1000L);
			break;
		case seconds:  tv->tv_sec=val;         break;
		case minutes:  tv->tv_sec=val*60L;     break;
		case hours:    tv->tv_sec=val*3600L;   break;
		case days:     tv->tv_sec=val*86400L;  break;
		default:  tv->tv_sec=0;  break;
	}
}


HTime &HTime::Add(long val,TimeSpec sp)
{
	if(val<0)
	{  return(Sub(-val,sp));  }
	timeval tmp;
	_SetVal(val,sp,&tmp);
	tv.tv_usec+=tmp.tv_usec;
	tv.tv_sec+=tv.tv_usec/1000000L;
	tv.tv_usec%=1000000L;
	tv.tv_sec+=tmp.tv_sec;
	return(*this);
}


HTime &HTime::Sub(long val,TimeSpec sp)
{
	if(val<0)
	{  return(Add(-val,sp));  }
	timeval tmp;
	_SetVal(val,sp,&tmp);
	tv.tv_usec-=tmp.tv_usec;
	tv.tv_sec-=tmp.tv_sec;
	while(tv.tv_usec<0)
	{  tv.tv_usec+=1000000L;  --tv.tv_sec;  }
	return(*this);
}


void HTime::_Delta(const HTime *endtime,long long *delta) const
{
	timeval tmp,*end;
	if(endtime)
	{  end=(timeval*)(&endtime->tv);  }
	else
	{  end=&tmp;  gettimeofday(end,NULL);  }
	*delta = 
		(((long long)end->tv_sec)*1000000LL+((long long)end->tv_usec)) - 
		(((long long)tv.tv_sec)*1000000LL+((long long)tv.tv_usec));
}

