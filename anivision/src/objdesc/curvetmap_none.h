/*
 * objdesc/curvetmap_none.h
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

#ifndef _ANIVISION_OBJDESC_CURVETMAP_NONE_H_
#define _ANIVISION_OBJDESC_CURVETMAP_NONE_H_

#include <objdesc/curve.h>


namespace NUM
{

// No-op tmap function: does nothing (just linear time map). 
class CurveTMapFunction_None : public CurveTMapFunction
{
	private:
		// This is the internal time range of the curve: 
		double curve_it0,curve_it1;
		// This is the "external" time range for the curve, 
		// i.e. the OCurve time. 
		double curve_t0,curve_t1;
		
	protected:
		// [overrding virtuals:]
		double function(double x);
		double diff(double x);
		double ddiff(double x);
		
	public:
		CurveTMapFunction_None();
		~CurveTMapFunction_None() {}
		
		// [overriding virtual:]
		int Create(CurveFunctionBase *already_created_curve_to_map,
			double curve_t0,double curve_t1,EvalContext *);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVETMAP_NONE_H_  */
