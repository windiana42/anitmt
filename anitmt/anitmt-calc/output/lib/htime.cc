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

#include "htime.h"

#if CALC_ELAPSED_TIME

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

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


#include <stdio.h>
#include <math.h>

std::string Throughput_Str(size_t nbytes,double elapsed)
{
	char tmp[64];
	if(elapsed<=0.0000001)
	{  strcpy(tmp,"lots of ");  }
	else
	{
		double tr=(nbytes/elapsed);
		if(tr<10240.0)
		{  snprintf(tmp,64,"%.0f ",tr);  }
		else if(tr<10240000.0)
		{  snprintf(tmp,64,"%.0f k",(tr/1024.0));  }
		else
		{  snprintf(tmp,64,"%.1f M",(tr/(1024.0*1024.0)));  }
	}
	char tim[64];
	if(elapsed<1.0)
	{
		int nd=3+int(-log10(elapsed*1000));
		snprintf(tim,64,"%.*f m",nd,elapsed*1000.0);
	}
	else if(elapsed<60.0)
	{  snprintf(tim,64,"%.1f ",elapsed);  }
	else 
	{  snprintf(tim,64,"%.0f ",elapsed);  }
	
	char kbv[32];
	if(nbytes<10240)
	{  snprintf(kbv,32,"%u ",nbytes);  }
	else
	{  snprintf(kbv,32,"%u k",((nbytes+512)/1024));  }
	
	std::string rv;
	rv=std::string(kbv)+"b in "+tim+"sec ("+tmp+"b/sec)";
	return(rv);
}

#endif  /* CALC_ELAPSED_TIME */
