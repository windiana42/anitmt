/*
 * objdesc/frontspec.cc
 * 
 * Class for the front position of object. Can also contain a curve. 
 * This is part of the AniVision project. 
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

#include "frontspec.h"


namespace NUM
{

int OFrontSpec::Create(OCurve * /*ocurve*/,EvalContext *)
{
	// Must be overridden.
	assert(0);
	return(1);
}


int OFrontSpec::Eval(OCurve * /*ocurve*/,const double * /*pos_already_known*/,
	double /*t*/,double * /*result*/,EvalContext *) const
{
	// Must be overridden if used.
	assert(0);
	return(-2);
}

}  // end of namespace NUM
