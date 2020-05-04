/*
 * numerics/spline/sl_base.h
 * 
 * Numerics library spline base header. 
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

#ifndef _WW_NUMERICS_SPLINE_SL_BASE_H_
#define _WW_NUMERICS_SPLINE_SL_BASE_H_ 1

#include <numerics/la_basics.h>
#include <numerics/diff_int/integrate.h>


namespace NUM  // numerics
{


enum SplineBoundaryCondition
{
	// No boundary condition: 
	SBC_None=     0x00,
	
	// Periodic boundary conditions: 
	SBC_Periodic=0x100,
	
	// Boundary condition bitvals; must be OR'ed together: 
	SBC_SlopeA=   0x01,
	SBC_SlopeB=   0x10,
	SBC_MomentumA=0x02,
	SBC_MomentumB=0x20,
	
	// Masks, mainly for internal use: 
	SBC_MaskA=    0x0f,
	SBC_MaskB=    0xf0,
	
	// Special ones for convenience: 
	// natural spline: f''(a)=0=f''(b)
	SBC_Natural = SBC_MomentumA|SBC_MomentumB,
	// Hermite condition: f'(a)=m1, f'(b)=m2
	SBC_Hermite = SBC_SlopeA|SBC_SlopeB
};

// For convenience, to OR together boundary conditions. 
inline SplineBoundaryCondition operator|(
	SplineBoundaryCondition a,SplineBoundaryCondition b)
{  return((SplineBoundaryCondition)(int(a)|int(b)));  }


// (Abstract) Base class for splines. 
class SplineBase
{
	public:
		static constexpr double tval_eps=1.0e-7;
		
		// TVALS_DIST to be passed to Create() functions for splines. 
		// NOTE: Using tvals=NULL or tvals=TVALS_DIST will change the 
		//       shape of the spline curve. 
		// NOTE when creating the inverse arc length function for 
		//      constant speed movement: 
		//      - For smooth splines (like cspline), neither of the 
		//        tvals settings seems to be preferrable over the 
		//        other one; they yield about the same results. 
		//      - For linear spline, always use TVALS_DIST if you 
		//        want to create the inverse arc length function; 
		//        using NULL will wreck havoc on the algorithm 
		//        resuling in negative speeds, etc. NOTE also that 
		//        when using TVALS_DIST with linear spline, the 
		//        inverse arc length function is simply linear 
		//        (t=lambda*s), so it makes no sense to use the 
		//        spline interpolation algotithm here. 
		static const double *TVALS_DIST;
		// TVALS_CPTS to be passed to Create() functions for splines. 
		static const double *TVALS_CPTS;
		
		// Each type entry here corresponds to a derived class: 
		enum SplineType
		{
			ST_None=0,
			ST_LSpline,   // linear spline
			ST_CSpline,   // cubic spline
			ST_XSpline,   // X spline (Blanc/Schlick)
			ST_ASpline,   // Akima spline
		};
		
		// As returned by SplineProperties(): 
		struct Properties
		{
			// Spline type: 
			SplineType splinetype;
			// Name of the spline: 
			const char *name;
			// Continuance order, the cont_order-th derivation 
			// of the spline function is the last steady one; 
			// e.g. for cubic splines, this is 2. 
			// "Wie oft mal stetig partiell diff'bar." 
			int cont_order;
		};
		
		// This is a publically accessible spline curve "function" 
		// with NUM interface: 
		struct CurveFunction : Function_R_Rn
		{
			private:
				SplineBase *spline;
			public:
				CurveFunction(SplineBase *_sb) : Function_R_Rn(_sb->Dim())
					{  spline=_sb;  }
				~CurveFunction()  {  spline=NULL;  }
				
				CurveFunction &operator=(SplineBase &_sb)
					{  spline=&_sb;  SetDDim(spline->Dim());  return(*this);  }
				
				// [overriding virtual:]
				void function(double x,double *result);
		};
		
		// Spline length function: 
		// Returns euclidic norm of first derivation of spline. 
		struct DevLenFunction : Function_R_R
		{
			private:
				SplineBase *spline;
			public:
				DevLenFunction(SplineBase *_sb) : Function_R_R()
					{  spline=_sb;  }
				~DevLenFunction()  {  spline=NULL;  }
				
				DevLenFunction &operator=(SplineBase &_sb)
					{  spline=&_sb;  return(*this);  }
				
				// [overriding virtual:]
				double function(double x);
		};
		
	private:
		// Cache for _EvalFindIdx() lookup: 
		int tv_cache_idx;
		
		// Internally used by CalcLength(): 
		double _DoCalcLength(Integrator_R_R *integrator,int int_by_piece);
		
		// NOT C++ safe: 
		SplineBase &operator=(const SplineBase &) { return(*this); }
		SplineBase(const SplineBase &) {}
	protected:
		int n;       // number of polynomials = npoints-1
		int dim;     // vector size / dimension
		double *x;   // control point vectors: [dim*(n+1)]
		double *tv;  // t values for the control/momenta vectors [n+1]
		             // or NULL in case of fixed distance = 1
		
		// Returns control point "vector" of point with index idx=0..n: 
		inline double *xv(int idx)  {  return(x+idx*dim);  }
		inline const double *xv(int idx) const  {  return(x+idx*dim);  }
		
		// Returns i-th t value, no range check. (i in range 0..n)
		inline double get_t(int i) const
			{  return(tv ? tv[i] : double(i));  }
		
		// Used by Create(): 
		// Check if the passed t values are increasing steadily. 
		// Return value: 0 -> OK; 1 -> check failed
		inline int _TestAscentingOrder(const double *tv,int n) const
			{  return(TestOrderAscenting(tv,n,tval_eps));  }
		// Like _TestAscentingOrder for the last vector components of the 
		// passed vector array: 
		int _TestAscentingOrder_LastCompo(const VectorArray<double> &cpoints) const;
		// Copy the control points and t values (if non-null). 
		// Clear()s [virtual] the old values before. 
		// Handles tvals=TVALS_DIST and TVALS_CPTS. 
		void _CopyPointsAndTVals(const VectorArray<double> &cpoints,
			const double *tvals);
		
		// Used by Eval(), EvalD(), EvalDD(); return value: index. 
		// Uses cache by remembering last time for faster access 
		// and binary search for lookup. 
		// Returned index is in range 0..n-1, hence you may savely 
		// access tv[idx] and tv[idx+1]. 
		int _EvalFindIdx(double t,int *retval) const;
	public:  _CPP_OPERATORS
		SplineBase();
		virtual ~SplineBase();
		
		// Get the number of polynomials = NPoints()-1. 
		inline int N() const
			{  return(n);  }
		// Get the number of control points = N()+1. 
		inline int NPoints() const
			{  return(n+1);  }
		// Get the dimension of the used vector space: 
		inline int Dim() const
			{  return(dim);  }
		
		// Return spline properties; see above. 
		// Default implentation returns NULL. 
		virtual const Properties *SplineProperties() const;
		
		// Get the t value associated with the i-th control point: 
		// Returns NaN if i is out of range 0..n. (n = npoints-1). 
		// If no t values were specified, the return value is i. 
		inline double GetT(int i) const
			{  return((i<0 || i>n) ? NAN : get_t(i));  }
		
		// Get t value range length: 
		inline double GetTRangeLength() const
			{  return(tv ? (tv[n]-tv[0]) : double(n));  }
		
		// Just remove all contents and free all memory, i.e. 
		// destroy the spline. 
		// Function in derived class must explicitly call this 
		// function at the end (it is NOT pure virtual). 
		virtual void Clear();
		
		// Virtual interface: 
		// Evaluate the spline at a given t value and store the 
		// result in passed vector which must be of capable of 
		// holding a vector of the used dimension. 
		//  Eval   -> evaluate spline value
		//  EvalD  -> evaluate the first derivation
		//  EvalDD -> evaluate the second derivation
		// t value should be in the following range: 
		//   GetT(0)..GetT(N())
		// that is: (note: n=npoints-1)
		//         0..n        if no t values were specified (-> Create())
		//   tval[0]..tval[n]  if t values tval[] were specified
		// Return value: 
		//   0   -> OK
		// -1,+1 -> t value is below/above the definition interval 
		//          (i.e. distance to interval is more than tval_eps)
		//          stored result is still valid 
		//  -2   -> spline was not set up; stored result is 0.0. 
		// Virtual function for Eval() must be supplied; EvalD() 
		// and EvalDD() have numerical default implementations. 
		virtual int Eval(double t,double *result) const;
		virtual int EvalD(double t,double *result) const;
		virtual int EvalDD(double t,double *result) const;
		
		// Compute the spline length using the passed integrator. 
		// If int_by_piece is 1, integrate each of the n curve 
		// pieces and sum up the result. Otherwise integrate the 
		// complete curve. Which one you choose depends on the 
		// integrator; for an adaptive one you can use 
		// int_by_piece=0, for ones with a fixed number of eval 
		// points one should use int_by_piece=1. 
		// You may pass integrator=NULL so that the default one 
		// gets used. 
		// Virtual function has suitable default implementation; 
		// override it if the length can be calculated directly 
		// such as for linear splines. 
		// NOTE: Returned length is 0 for not-set-up splines. 
		//       int_by_piece ignored if integrator=NULL. 
		virtual double CalcLength(Integrator_R_R *integrator,
			int int_by_piece);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_SPLINE_SL_BASE_H_ */
