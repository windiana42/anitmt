/*
 * objdesc/curve.h
 * 
 * Curve class to move object position around in space. 
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

#ifndef _ANIVISION_OBJDESC_CURVE_H_
#define _ANIVISION_OBJDESC_CURVE_H_

#include <numerics/num_math.h>
#include <numerics/function.h>

#include <valtype/valtypes.h>


namespace NUM
{

// This is merely a name for a pointer which is passed to the eval 
// functions and to be used by evaulation functions provided by 
// a higher level. 
struct EvalContext
{
	_CPP_OPERATORS
	EvalContext(){}
	~EvalContext(){}
};


// Base class for curve functions. 
// Spline curve functions are derived from this but also the 
// full-featured OCurve. 
// All the CurveFunctionBase-derived classes have the following in common: 
//   - They are set up after creation by storing all the user-supplied data; 
//     at this time, they are not yet functional curves. 
//   - They are converted into functional curves using Create() which then 
//     creates internal data (e.g. momenta for the cspline). 
class CurveFunctionBase
{
	public:
		// Returns euclidic norm of first derivation of curve. 
		struct DevLenFunction_ctx : Function_R_R
		{
			private:
				CurveFunctionBase *curve;
			public:
				EvalContext *ectx;  // <-- Do not forget to set this one!
			public:
				DevLenFunction_ctx(CurveFunctionBase *_cb) : NUM::Function_R_R()
					{  curve=_cb;  ectx=NULL;  }
				~DevLenFunction_ctx()  {  curve=NULL;  ectx=NULL;  }
				
				DevLenFunction_ctx &operator=(CurveFunctionBase &_cb)
					{  curve=&_cb;  ectx=NULL;  return(*this);  }
				
				// [overriding virtual:]
				double function(double x);
		};
		
	protected:
		// Dimenstion of target vector space. (Initially -1)
		int dim;  
		
		// Curve time range for internal time value (i.e. the 
		// value range for the Eval*() funcions). 
		// Initially NaN. 
		double it0,it1;
		
	public:  _CPP_OPERATORS
		CurveFunctionBase()
			{  dim=-1;  it0=NAN;  it1=NAN;  }
		virtual ~CurveFunctionBase()
			{  }
		
		// Get the dimension of the used vector space (or -1): 
		inline int Dim() const
			{  return(dim);  }
		
		// See it0,it1 and Eval*(). 
		inline double iT0() const  {  return(it0);  }
		inline double iT1() const  {  return(it1);  }
		
		// Create curve (i.e. intenal data required) now. 
		// NOTE: Create() must set these members: dim, it0, it1. 
		// Return value: 
		//   0 -> OK
		//   1 -> error (reported); internal curve (spline/...) not created 
		virtual int Create(EvalContext *);
		
		// Evaluate function, first and second derivation, respectively. 
		// Make sure, t is in range iT0()..iT1(). 
		// Return value: 
		//   0 -> OK
		// -1,+1 -> maybe okay but note that time is below/above range
		//  -2 -> no OCurve set or no curve in OCurve [or failing assertion instead...]
		virtual int Eval(double t,double *result,EvalContext *) const;
		virtual int EvalD(double t,double *result,EvalContext *) const;
		virtual int EvalDD(double t,double *result,EvalContext *) const;
		
		// Interface for the time mapping function at create 
		// time. 
		// Get preferred number of interpolation segments; for splines, 
		// this is the number of spline curve pieces; curves which do 
		// not have a preference return -1. 
		virtual int TMapCreator_GetPreferredInterpolSegCount()
			{  return(-1);  }
		// Get internal curve time for n-th segment; need only be 
		// overridden if TMapCreator_GetPreferredInterpolSegCount() 
		// returned >=0. Must return it0 for n=0 and it1 for 
		// n=TMapCreator_GetPreferredInterpolSegCount(). 
		virtual double TMapCreator_GetInterpolSegITime(int n);
};


// T value map function: used for constant speed movement along 
// a spline and such things. 
class CurveTMapFunction : public Function_R_R
{
	private:
	public:  _CPP_OPERATORS
		CurveTMapFunction() : Function_R_R() {}
		virtual ~CurveTMapFunction() {}
		
		// Must pass the already-created curve on which this 
		// time mapping function shall operate. The passed curve 
		// is not stored internally; only needed to query some 
		// information needed by Create(). 
		// curve_t0,curve_t1 is the time range for the 
		// Return value: 
		//  0 -> OK
		//  1 -> error (reported); spline function not created
		virtual int Create(CurveFunctionBase *already_created_curve_to_map,
			double curve_t0,double curve_t1,EvalContext *);
		
		// Get first and second derivative of time mapping function: 
		virtual double diff(double x);
		virtual double ddiff(double x);
};


// A curve is a function f: [a..b] -> R^n, t -> curve(t). 
// Class OCurve is merely the container and manager for the actual 
// curve function. 
class OCurve : public CurveFunctionBase
{
	private:
		// This is the actual curve function, i.e. [interval] -> R^n. 
		CurveFunctionBase *curvefunc;
		
		// Time range: delta_t = t_range.b-t_range.a in the end. 
		// Before creation is complete, these contain as much info 
		// as we know. delta_t=NAN if unknown. 
		double delta_t;
		Range t_range;
		
		// Time mapping function implementing speed/length control 
		// ("moving along spline with constant speed"). 
		CurveTMapFunction *tmapfunc;
		
		// Internally used by Set_DT(), Set_T() to update 
		// delta_t/t_range by each other's values. 
		void _SetTime_DoFix();
	public:
		OCurve();
		~OCurve();
		
		// Get curve time range (curve time, not internal curve time 
		// [this is after applying the t map function] and not 
		// setting/object time): 
		double T0() const  {  return(t_range.val_a());  }
		double T1() const  {  return(t_range.val_b());  }
		
		
		//**** API for user parameters: ****
		
		// Set the position curve. 
		// This curve must have been allocated via operator new and 
		// is then passed to OCurve which will destroy it when no 
		// longer needed. 
		// Return value: 
		//   0 -> OK
		//   1 -> curve pos func was already set (in this case 
		//        the passed curvefunc was deleted to prevent 
		//        mem leaks) 
		int SetPosCurve(CurveFunctionBase *curvefunc);
		
		// Set time values: dt and t (range; may be open). 
		// Return value: 
		//   0 -> OK
		//   1 -> already set
		//   2 -> conflict
		int Set_DT(double dt);
		int Set_T(const Range &t);
		
		// Set curve 
		// Return value: 
		//   0 -> OK
		//   1 -> already set (passed function deleted to prevent 
		//        mem leak)
		int SetTMapFunction(CurveTMapFunction *tmapfunc);
		
		// [overriding virtual:]
		int Create(EvalContext *);
		
		// Evaluate function, first and second derivation, respectively. 
		// [overriding vituals]
		int Eval(double t,double *result,EvalContext *) const;
		int EvalD(double t,double *result,EvalContext *) const;
		int EvalDD(double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_CURVE_H_ */
