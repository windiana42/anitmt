/*
 * numerics/spline/xspline.h
 * 
 * Numerics library X spline (Blanc/Schlick). 
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

#ifndef _WW_NUMERICS_SPLINE_X_Spline_H_
#define _WW_NUMERICS_SPLINE_X_Spline_H_ 1

#include "sl_base.h"


namespace NUM
{

class X_Spline : public SplineBase
{
	public:
		//static const double tval_eps=1.0e-7;
	private:
		// Commented member vars are in SplineBase. 
		//int n;       // number of polynomials = npoints-1
		//int dim;     // vector size / dimension
		//double *x;   // control point vectors: [dim*(n+1)]
		//double *tv;  // t values for the control/momenta vectors [n+1]
		             // or NULL in case of fixed distance = 1
		double *sv;  // s values ("point weights") [n+1]
		
		// Internally used by the eval function: 
		void _DoEval(double t,double h,double *result,int idx,
			const double *p0,const double *p1,const double *p2,
			const double *p3) const;
		void _DoEvalD(double t,double h,double *result,int idx,
			const double *p0,const double *p1,const double *p2,
			const double *p3) const;
	public:
		X_Spline();
		~X_Spline();  // [overridden virtual]
		
		// First, the easy part: 
		const Properties *SplineProperties() const;  // [overridden virtual]
		
		// Create a spline. 
		//   If tvals=NULL, use fixed t-distance = 1 between points. 
		//   If tvals=SplineBase::TVALS_DIST, accumulate euclidic 
		//   point distances and use them as t-values. 
		//   Read note in sl_base.h. 
		//   Otherwise, tvals[] is an array of size [n+1] specifying 
		//   the t values at the control points. The t values must 
		//   increase steadily. 
		// Control points are specified in a vector array. 
		// weights may be specified for this spline -- one double value 
		// for each point. The weight value is (normally) in range -1..1. 
		// If weights is NULL, it defaults to -1 which is an interpolated 
		// spline. 
		// Return value: 
		//   0 -> OK
		//  -2 -> too few control points (need at least 2)
		//        or vectors with dimension <1 
		//  -4 -> t values do not increase steadily (i.e. 
		//        t[i+1]-t[i]<=tval_eps). 
		int Create(
			const VectorArray<double> &cpoints,const double *tvals,
			const double *weights);
		
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

#endif  /* _WW_NUMERICS_SPLINE_X_Spline_H_ */
