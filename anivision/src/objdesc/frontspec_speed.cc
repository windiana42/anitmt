/*
 * objdesc/frontspec_speed.cc
 * 
 * Class for the front position using speed vector. 
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

#include "frontspec_speed.h"


namespace NUM
{

int OFrontSpec_Speed::Create(OCurve * /*ocurve*/,EvalContext *)
{
	// There is nothing to do here. 
	return(0);
}


int OFrontSpec_Speed::Eval(OCurve *ocurve,const double * /*pos_already_known*/,
	double t,double *result,EvalContext *ectx) const
{
	int rv=ocurve->EvalD(t,result,ectx);
	return(rv);
}


OFrontSpec_Speed::OFrontSpec_Speed() : 
	OFrontSpec()
{
}

OFrontSpec_Speed::~OFrontSpec_Speed()
{
}

}  // end of namespace NUM
