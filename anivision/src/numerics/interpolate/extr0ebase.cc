/*
 * numerics/interpolate/extr0ebase.cc
 * 
 * Base class for vector extrapolators to 0 with error. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "extr0ebase.h"


namespace NUM
{

void Extrapolator_0E_Base::FirstPoint(double /*_t*/,const double * /*_p*/,
	double * /*px*/,double * /*dp*/)
{
	// Must be overridden if used. 
	assert(0);
}

void Extrapolator_0E_Base::AddPoint(double /*_t*/,const double * /*_p*/,
	double * /*px*/,double * /*dp*/)
{
	// Must be overridden if used. 
	assert(0);
}

}  // end of namespace NUM
