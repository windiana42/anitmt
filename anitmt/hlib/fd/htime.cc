/*
 * htime.cc
 * 
 * Implementation of methods supporting class HTime. 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/htime.h>


// Used constants: 
const int64_t HTime::conv_fact[HTime::_tslast]=
#if __GNUC__ < 3
{ 1LL, 1000LL, 1000000LL, 60000000LL, 3600000000LL, 86400000000LL };
#else
{ 1, 1000, 1000000, 60000000, 3600000000, 86400000000 };
#endif

const int64_t HTime::round_delta[HTime::_tslast]=
#if __GNUC__ < 3
{ 0LL,  500LL,  500000LL, 30000000LL, 1800000000LL, 43200000000LL };
#else
{ 0,  500,  500000, 30000000, 1800000000, 43200000000 };
#endif

const double HTime::conv_factD[HTime::_tslast]=
{ 1.0, 1000.0, 1000000.0, 60000000.0, 3600000000.0, 86400000000.0 };


void HTime::_SetVal(long val,TimeSpec sp,timeval *tv)
{
	tv->tv_usec=0L;
	switch(sp)
	{
		case usec:
		{
			register long m=val/1000000L;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=val-m*1000000;
		}  break;
		case msec:
		{
			register long m=val/1000L;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=1000*(val-m*1000);
		}  break;
		case seconds:  tv->tv_sec=val;         break;
		case minutes:  tv->tv_sec=val*60L;     break;
		case hours:    tv->tv_sec=val*3600L;   break;
		case days:     tv->tv_sec=val*86400L;  break;
		default:  tv->tv_sec=0;  break;
	}
}


HTime &HTime::Add(long val,TimeSpec sp)
{
	timeval tmp;
	_SetVal(val,sp,&tmp);
	tv.tv_sec+=tmp.tv_sec;
	tv.tv_usec+=tmp.tv_usec;
	_Normalize(&tv);
	return(*this);
}


HTime &HTime::Sub(long val,TimeSpec sp)
{
	timeval tmp;
	_SetVal(val,sp,&tmp);
	tv.tv_sec-=tmp.tv_sec;
	tv.tv_usec-=tmp.tv_usec;
	_Normalize(&tv);
	return(*this);
}


int64_t HTime::_Delta(const HTime *endtime) const
{
	timeval tmp;
	const timeval *end;
	if(endtime)
	{  end=&endtime->tv;  }
	else
	{  gettimeofday(&tmp,NULL);  end=&tmp;  }
	return(
		(int64_t(end->tv_sec)*int64_t(1000000) + int64_t(end->tv_usec)) - 
		(int64_t(  tv.tv_sec)*int64_t(1000000) + int64_t(  tv.tv_usec)) );
}
