/*
 * htime.cc
 * 
 * Implementation of methods supporting class HTime. 
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
#include <hlib/htime.h>


// Used constants: 
const int64_t HTime::conv_fact[HTime::_tslast]=
{ 1LL, 1000LL, 1000000LL, 60000000LL, 3600000000LL, 86400000000LL };

const int64_t HTime::round_delta[HTime::_tslast]=
{ 0LL,  500LL,  500000LL, 30000000LL, 1800000000LL, 43200000000LL };

const double HTime::conv_factD[HTime::_tslast]=
{ 1.0, 1000.0, 1000000.0, 60000000.0, 3600000000.0, 86400000000.0 };

// Static data: 
HTime HTime::most_current(HTime::Curr);


void HTime::_SetVal(long val,TimeSpec sp,timeval *tv)
{
	tv->tv_usec=0L;
	switch(sp)
	{
		case usec:
		{
			register long m=val/1000000;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=val-m*1000000;
		}  break;
		case msec:
		{
			register long m=val/1000;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=1000*(val-m*1000);
		}  break;
		case seconds:  tv->tv_sec=val;        break;
		case minutes:  tv->tv_sec=val*60;     break;
		case hours:    tv->tv_sec=val*3600;   break;
		case days:     tv->tv_sec=val*86400;  break;
		default:  tv->tv_sec=0;  break;
	}
}

void HTime::_SetValL(int64_t val,TimeSpec sp,timeval *tv)
{
	switch(sp)
	{
		case usec:
		{
			register int64_t m=val/1000000;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=val-m*1000000;
		}  break;
		case msec:
		{
			register int64_t m=val/1000;
			if(val<0)  --m;
			tv->tv_sec=m;
			tv->tv_usec=1000*(val-m*1000);
		}  break;
		case seconds:
			tv->tv_sec=val;
			tv->tv_usec=0L;
			break;
		default:  _SetVal(val,sp,tv);
	}
}


HTime &HTime::Add(long val,TimeSpec sp)
{
	if(!IsInvalid())
	{
		timeval tmp;
		_SetVal(val,sp,&tmp);
		tv.tv_sec+=tmp.tv_sec;
		tv.tv_usec+=tmp.tv_usec;
		_Normalize(&tv);
	}
	return(*this);
}


HTime &HTime::Sub(long val,TimeSpec sp)
{
	if(!IsInvalid())
	{
		timeval tmp;
		_SetVal(val,sp,&tmp);
		tv.tv_sec-=tmp.tv_sec;
		tv.tv_usec-=tmp.tv_usec;
		_Normalize(&tv);
	}
	return(*this);
}


int64_t HTime::_Delta(const HTime *endtime) const
{
	// May not be called if time is invalid. 
	timeval tmp;
	const timeval *end;
	if(endtime)
	{  end=&endtime->tv;  }
	else
	{  do_gettimeofday(&tmp);  end=&tmp;  }
	return(
		(int64_t(end->tv_sec)*int64_t(1000000) + int64_t(end->tv_usec)) - 
		(int64_t(  tv.tv_sec)*int64_t(1000000) + int64_t(  tv.tv_usec)) );
}


// This is msec_elapsed from misc/msecelapsed.c honoring do_gettimeofday(). 
long HTime::my_msec_elapsed(const struct timeval *old)
{
	struct timeval current;
	do_gettimeofday(&current);
	return(
		(current.tv_sec  - old->tv_sec )*1000L + 
		(current.tv_usec - old->tv_usec)/1000L );
}

// This is msec_elapsed_r from misc/msecelapsedr.c honoring do_gettimeofday(). 
long HTime::my_msec_elapsed_r(const struct timeval *old)
{
	struct timeval current;
	long du;
	do_gettimeofday(&current);
	du = (current.tv_usec - old->tv_usec);
	return(
		(current.tv_sec  - old->tv_sec)*1000L + 
		((du<0L) ? (du-500) : (du+500) )/1000L );
}
