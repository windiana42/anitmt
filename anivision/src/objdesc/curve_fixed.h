/*
 * objdesc/curve_fixed.h
 * 
 * Fixed "curve" class. 
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

#ifndef _ANIVISION_OBJDESC_CURVE_FIXED_H_
#define _ANIVISION_OBJDESC_CURVE_FIXED_H_

#include <objdesc/curve.h>
#include <numerics/spline/cspline.h>


namespace NUM
{

class CurveFunction_Fixed : public CurveFunctionBase
{
	private:
		// The fixed position, set up by Create() 
		// (well, actually earlier and Create() is a no-op). 
		double *pos;
		
	public:
		CurveFunction_Fixed();
		~CurveFunction_Fixed();
		
		//**** API for user parameters: ****
		
		// Set position. 
		// Return value: 
		//   0 -> OK
		//.  1 -> nothing done; pos already set
		int SetPos(double *pos,int dim);
		
		// [overriding virtuals:]
		int Create(EvalContext *);
		int Eval(double t,double *result,EvalContext *) const;
		int EvalD(double t,double *result,EvalContext *) const;
		int EvalDD(double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVE_FIXED_H_ */
