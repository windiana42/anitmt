/*
 * numerics/interpolate/extrr0poly.cc
 * 
 * Polynomial extrapolation to 0 for single values and without 
 * error using the Neville algorithm. 
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

#include "extrr0poly.h"


namespace NUM
{

void Extrapolator_R_0_Poly::_assertfail()
{
	assert(!"n<max_n");
}

}  // end of namespace NUM
