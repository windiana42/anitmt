/*
 * objdesc/curve_cspline.h
 * 
 * Cubic poly spline curve class. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_OBJDESC_CURVE_CSPLINE_H_
#define _ANIVISION_OBJDESC_CURVE_CSPLINE_H_

#include <objdesc/curve_splinebase.h>
#include <numerics/spline/cspline.h>


namespace NUM
{

class CurveFunction_CSpline : public CurveFunction_Spline_Base
{
	private:
		// The actual spline; set up by Create(). 
		CubicPolySpline *cspline;
		
	public:
		CurveFunction_CSpline();
		~CurveFunction_CSpline();
		
		//**** API for user parameters: ****
		// See the base class CurveFunction_Spline_Base for more information. 
		// [overriding virtuals:]
		//[default] int SetPoints(VectorArray<double> &cpts,bool with_tvals);
		int SetAddPoints(const char *val);
		//[default] int SetTValueMode(const char *val);
		
		// [overriding virtuals:]
		int Create(EvalContext *);
		int Eval(double t,double *result,EvalContext *) const;
		int EvalD(double t,double *result,EvalContext *) const;
		int EvalDD(double t,double *result,EvalContext *) const;
		
		// [overriding virtuals:]
		int TMapCreator_GetPreferredInterpolSegCount();
		double TMapCreator_GetInterpolSegITime(int n);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVE_CSPLINE_H_ */
