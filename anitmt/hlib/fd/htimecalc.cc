/*
 * htimecalc.cc
 * 
 * Implementation of more calculation methods for class HTime. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


static inline void _normalize(struct timeval *tv)
{
	// If sec and usec have different signs, correct that: 
	if(tv->tv_usec<0 && tv->tv_sec>0)
	{
		do 
		{  --tv->tv_sec;  tv->tv_usec+=1000000;  }
		while(tv->tv_usec<0 && tv->tv_sec>0);
	}
	else if(tv->tv_usec>0 && tv->tv_sec<0)
	{
		do
		{  ++tv->tv_sec;  tv->tv_usec-=1000000;  }
		while(tv->tv_usec<0 && tv->tv_sec>0);
	}
	
	// Now, correct too large usec values: 
	#warning potential problem with modulo when <0. 
	if(tv->tv_usec>=1000000 || tv->tv_usec<=-1000000)
	{  tv->tv_sec+=tv->tv_usec/1000000;  tv->tv_usec%=1000000;  }
}


HTime HTime::operator-(const HTime &start)
{
	HTime r;
	
	r.tv.tv_sec= tv.tv_sec- start.tv.tv_sec;
	r.tv.tv_usec=tv.tv_usec-start.tv.tv_usec;
	_normalize(&r.tv);
	
	return(r);
}

HTime &HTime::operator-=(const HTime &start)
{
	tv.tv_sec-= start.tv.tv_sec;
	tv.tv_usec-=start.tv.tv_usec;
	_normalize(&tv);
	return(*this);
}


HTime HTime::operator+(const HTime &start)
{
	HTime r;
	
	r.tv.tv_sec= tv.tv_sec+ start.tv.tv_sec;
	r.tv.tv_usec=tv.tv_usec+start.tv.tv_usec;
	_normalize(&r.tv);
	
	return(r);
}

HTime &HTime::operator+=(const HTime &start)
{
	tv.tv_sec+= start.tv.tv_sec;
	tv.tv_usec+=start.tv.tv_usec;
	_normalize(&tv);
	return(*this);
}
