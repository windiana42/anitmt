/*
 * numerics/spline/cspline.h
 * 
 * Numerics library cubic poly spline. 
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

#ifndef _WW_NUMERICS_SPLINE_CubicPolySpline_H_
#define _WW_NUMERICS_SPLINE_CubicPolySpline_H_ 1

#include "sl_base.h"


namespace NUM
{

class CubicPolySpline : public SplineBase
{
	public:
		//static const double tval_eps=1.0e-7;
	private:
		// Commented member vars are in SplineBase. 
		//int n;       // number of polynomials = npoints-1
		//int dim;     // vector size / dimension
		//double *x;   // control point vectors: [dim*(n+1)]
		double *p;   // momenta vectors [dim*(n+1)]
		//double *tv;  // t values for the control/momenta vectors [n+1]
		             // or NULL in case of fixed distance = 1
		
		// Returns momentum "vector" of point with index idx=0..n: 
		inline double *pv(int idx)  {  return(p+idx*dim);  }
		inline const double *pv(int idx) const  {  return(p+idx*dim);  }
		
	public:
		CubicPolySpline();
		~CubicPolySpline();  // [overridden virtual]
		
		// First, the easy part: 
		const Properties *SplineProperties() const;  // [overridden virtual]
		
		// Create a spline. 
		//   If tvals=NULL, use fixed t-distance = 1 between points. 
		//   If tvals=SplineBase::TVALS_DIST, accumulate euclidic 
		//     point distances and use them as t-values. 
		//     Read note in sl_base.h. 
		//   If tvals=SplineBase::TVALS_CPTS, use the last component 
		//     of the control point vectors as t-values. 
		//   Otherwise, tvals[] is an array of size [n+1] specifying 
		//     the t values at the control points. The t values must 
		//     increase steadily. 
		// Control points are specified in a vector array. 
		// Additionally two more conditions must be supplied; these 
		// two additional args are vectors of same size as those 
		// in the VectorArray and their meaning depends on the 
		// SplineBoundaryCondition: 
		// ('A' stands for 'beginning', 'B' for 'end') 
		// The boundary condition is a bitwise OR of exactly. two SBC_* 
		// values as defined above in the enum: Supported combinations 
		// are: 
		//  -------------sbc---------------paramA------paramB----
		//   SBC_MomentumA|SBC_MomentumB   momentumA   momentumB
		//   SBC_SlopeA   |SBC_SlopeB      slopeA      slopeB
		//   SBC_SlopeA   |SBC_MomentumA   slopeA      momentumA  (*)
		//   SBC_SlopeB   |SBC_MomentumB   slopeB      momentumB  (*)
		//   SBC_SlopeA   |SBC_MomentumB   slopeA      momentumB
		//   SBC_MomentumA|SBC_SlopeB      momentumA   slopeB
		//   SBC_Periodic                  NULL        NULL
		// In case you specify MomentumA,B you may set paramA,B to 
		// NULL for "natural" boundary conditions, i.e. value 0. 
		// Safety NOTE: (*) are a very bad idea. They're easy to 
		//              compute but will MOST CERTAINLY NOT behave 
		//              as you expect it. Use other splines which 
		//              behave nicely if you need that sort of 
		//              boundary condition. 
		// NOTE: For SBC_Periodic, i.e. periodic boundary conditions, 
		//       you MUST make sure that the first and the last point 
		//       are equal. The period length is then tval[n]-tval[0] 
		//       (or n for tvals=NULL). 
		// Return value: 
		//   0 -> OK
		//  -2 -> too few control points (need at least 2)
		//        or vectors with dimension <1 (<2 for TVALS_CPTS)
		//  -3 -> illegal sbc spec or missing value for paramA/B
		//  -4 -> t values do not increase steadily (i.e. 
		//        t[i+1]-t[i]<=tval_eps). 
		int Create(
			const VectorArray<double> &cpoints,const double *tvals,
			SplineBoundaryCondition sbc,
			const double *paramA,const double *paramB);
		
		// Just remove all contents and free all memory, i.e. 
		// destroy the spline. 
		void Clear();  // [overridden virtual]
		
		// Description: See SplineBase. 
		// [Overridden virtuals]
		int Eval(double t,double *result) const;
		int EvalD(double t,double *result) const;
		int EvalDD(double t,double *result) const;
		
		// Description: See SplineBase. 
		// [Using default implementation:]
		// double CalcLength(NUM::Integrator_R_R *integrator,int int_by_piece);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_SPLINE_CubicPolySpline_H_ */
