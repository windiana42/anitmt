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

#define HLIB_IN_HLIB 1
#include <hlib/htime.h>

// NOTE on sec and usec values: 
// True value is always sec+usec/1000000. 
// However, usec is ALWAYS in range 0...999999; NEVER negative. 


HTime HTime::operator-(const HTime &start) const
{
	HTime r(Invalid);
	
	if(!IsInvalid() && !start.IsInvalid())
	{
		r.tv.tv_sec= tv.tv_sec- start.tv.tv_sec;
		r.tv.tv_usec=tv.tv_usec-start.tv.tv_usec;
		_Normalize(&r.tv);
	}
	
	return(r);
}

HTime &HTime::operator-=(const HTime &start)
{
	if(!IsInvalid())
	{
		if(start.IsInvalid())
		{  SetInvalid();  }
		else
		{
			tv.tv_sec-= start.tv.tv_sec;
			tv.tv_usec-=start.tv.tv_usec;
			_Normalize(&tv);
		}
	}
	return(*this);
}


HTime HTime::operator+(const HTime &start) const
{
	HTime r(Invalid);
	
	if(!IsInvalid() && !start.IsInvalid())
	{
		r.tv.tv_sec= tv.tv_sec+ start.tv.tv_sec;
		r.tv.tv_usec=tv.tv_usec+start.tv.tv_usec;
		_Normalize(&r.tv);
	}
	
	return(r);
}

HTime &HTime::operator+=(const HTime &start)
{
	if(!IsInvalid())
	{
		if(start.IsInvalid())
		{  SetInvalid();  }
		else
		{
			tv.tv_sec+= start.tv.tv_sec;
			tv.tv_usec+=start.tv.tv_usec;
			_Normalize(&tv);
		}
	}
	return(*this);
}


HTime &HTime::operator-()
{
	if(!IsInvalid())
	{
		tv.tv_sec=-tv.tv_sec;
		tv.tv_usec=-tv.tv_usec;
		_Normalize(&tv);
	}
	return(*this);
}


HTime &HTime::Div(int fact)
{
	if(!IsInvalid())
	{
		if(!fact)
		{  SetInvalid();  }
		else
		{
			if(fact<0)
			{
				tv.tv_sec=-tv.tv_sec;
				tv.tv_usec=-tv.tv_usec;
				_Normalize(&tv);
				fact=-fact;
			}
			
			// This may overflow if fact is large 
			// (around 2000 on 32bit systems). 
			tv.tv_usec=((tv.tv_sec%fact)*1000000+tv.tv_usec)/fact;
			tv.tv_sec/=fact;
			_Normalize(&tv);
		}
	}
	return(*this);
}
