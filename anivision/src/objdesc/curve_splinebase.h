/*
 * objdesc/curve_splinebase.h
 * 
 * Base class for spline curve classes. Just for the interface :)
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

#ifndef _ANIVISION_OBJDESC_CURVE_SPLINEBASE_H_
#define _ANIVISION_OBJDESC_CURVE_SPLINEBASE_H_

#include <objdesc/curve.h>
#include <numerics/spline/sl_base.h>


namespace NUM
{

class CurveFunction_Spline_Base : public CurveFunctionBase
{
	public:
		enum AdditionalPointSpec
		{
			APS_Unset=-1,
			APS_None=0,
			APS_Start,
			APS_End,
			APS_Both,
			APS_Periodic  // periodic boundary conditions
		};
		
		enum TValueMode
		{
			TVM_Unset=-1,
			TVM_Inc=0,  // const increment: 0,1,2,3... 
			TVM_Dist,   // Euclidic point distance
			TVM_Pts_T,  // specified with points
		};
		
	protected:
		// This accumulates the user-specified stuff until 
		// Create() gets called: (Is NULL after Create().)
		struct UData_Base
		{
			VectorArray<double> *pts;
			TValueMode tvals_mode;
			AdditionalPointSpec addpts;
			
			_CPP_OPERATORS
			UData_Base();
			~UData_Base();
		};
		
		// The constructor of the derived class is responsible for 
		// (de)allocation of a UData_Base (-derived) class. 
		UData_Base *udata;
		
		// Must be used as derived class. 
		CurveFunction_Spline_Base();
	public:
		virtual ~CurveFunction_Spline_Base();
		
		//**** API for user parameters: ****
		
		// Set control points. Data from cpts will be _transferred_, 
		// i.e. cpts will be empty after the call. 
		// with_tvals: If set, the last component of the cpts vectors 
		//    is considered the t(ime) value of the spline. 
		//    Otherwise the time value is considered to be 0..nvectors-1. 
		// (The ani time is mapped to the curve t value lateron.)
		// Return value: 
		//   0 -> OK
		//.  1 -> nothing done; points already set
		//   2 -> too few points
		//   3 -> cannot use with_tvals together with current tvals_mode.  
		virtual int SetPoints(VectorArray<double> &cpts,bool with_tvals);
		
		// Set additional points property which allows to add 
		// one point at the beginning and/or end to achieve continuacy. 
		// Return value: 
		//   0 -> OK
		//   1 -> nothing done; already set
		//   2 -> illegal value; allowed: "none","start","end","both"
		//        cspline also allows "periodic" 
		virtual int SetAddPoints(const char *val);
		
		// Set t value mode: 
		//  "inc", "dist"
		// Return value: 
		//  0 -> OK
		//  1 -> was already set (nothing changed)
		//  2 -> illegal mode
		//  3 -> cannot set mode (used with_tvals set in SetPoints())
		virtual int SetTValueMode(const char *val);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVE_SPLINEBASE_H_ */
