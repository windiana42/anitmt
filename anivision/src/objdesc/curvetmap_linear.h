/*
 * objdesc/curvetmap_linear.h
 * 
 * Curve time mapping function for linar (=constant speed) movement. 
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

#ifndef _ANIVISION_OBJDESC_CURVETMAP_LINEAR_H_
#define _ANIVISION_OBJDESC_CURVETMAP_LINEAR_H_

#include <objdesc/curve.h>


namespace NUM
{

class SplineBase;

// Function for constant speed: 
class CurveTMapFunction_Linear : public CurveTMapFunction
{
	private:
		// This is the spline used as interpolation function: 
		SplineBase *ialf;
		
		// This is the total length of the curve: 
		double totlen;
		// This is the "external" time range for the curve, 
		// i.e. the OCurve time. 
		double curve_t0,curve_t1;
		
	protected:
		// [overrding a virtual:]
		double function(double x);
		double diff(double x);
		double ddiff(double x);
		
	public:
		CurveTMapFunction_Linear();
		~CurveTMapFunction_Linear();
		
		// [overriding virtual:]
		int Create(CurveFunctionBase *already_created_curve_to_map,
			double curve_t0,double curve_t1,EvalContext *);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVETMAP_LINEAR_H_  */
