/*
 * objdesc/curvetmap_none.cc
 * 
 * No-op curve time mapping function (only linar time range scaling). 
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

#include "curvetmap_none.h"


namespace NUM
{

double CurveTMapFunction_None::function(double x)
{
	// Okay, so we're seeing values curve_it0..curve_it1 for x here 
	// Perform linear mapping: 
	double res = (x-curve_t0) * (curve_it1-curve_it0) / 
		(curve_t1-curve_t0) + curve_it0;
	//fprintf(stderr,"tmap_none: %g -> %g\n",x,res);
	return(res);
}

double CurveTMapFunction_None::diff(double /*x*/)
{
	return( (curve_it1-curve_it0) / (curve_t1-curve_t0) );
}

double CurveTMapFunction_None::ddiff(double /*x*/)
{
	return(0.0);
}


int CurveTMapFunction_None::Create(CurveFunctionBase *curve,
	double _curve_t0,double _curve_t1,EvalContext *)
{
	curve_it0=curve->iT0();
	curve_it1=curve->iT1();
	curve_t0=_curve_t0;
	curve_t1=_curve_t1;
	
	fprintf(stderr,"\"None\" curve tmap: %g..%g -> %g..%g\n",
		curve_t0,curve_t1,
		curve_it0,curve_it1);
	
	return(0);
}


CurveTMapFunction_None::CurveTMapFunction_None() : 
	CurveTMapFunction()
{
	curve_it0=NAN;
	curve_it1=NAN;
	curve_t0=NAN;
	curve_t1=NAN;
}

}  // end of namespace NUM
