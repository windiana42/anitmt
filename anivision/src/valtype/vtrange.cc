/*
 * valtype/vtrange.cc
 * 
 * Range value (double..double) implementation. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "valtypes.h"


Range Range::operator-() const
{
	switch(valid)
	{
		case RInvalid:  return(*this);
		case RValidA:
		{
			Range r;
			r.b=-a;
			r.valid=RValidB;
			return(r);
		}
		case RValidB:
		{
			Range r;
			r.a=-b;
			r.valid=RValidA;
			return(r);
		}
		case RValidAB:
		{
			Range r;
			r.a=-b;
			r.b=-a;
			r.valid=RValidAB;
			return(r);
		}
		default: assert(0);
	}
}


void Range::_op_minus()
{
	switch(valid)
	{
		case RInvalid: break;
		case RValidA:   b=-a;  valid=RValidB;  break;
		case RValidB:   a=-b;  valid=RValidA;  break;
		case RValidAB:
		{
			double tmp=a;
			a=-b;
			b=-tmp;
			break;
		}
		default: assert(0);
	}
}
