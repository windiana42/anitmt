/*
 * numerics/diff_int/integrate.h
 * 
 * Numerical integration header. 
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

#ifndef _WW_NUMERICS_INTEGRATE_H_
#define _WW_NUMERICS_INTEGRATE_H_

#include <numerics/function.h>


namespace NUM  // numerics
{

// Integrator for R->R functions (base class): 
// In order to do some actual integration, you need to use a derived 
// integrator class; see below. 
class Integrator_R_R
{
	public:
		// Use int64_t if you need REALLY large numbers here. 
		// (But note that in such a case you're probably doing 
		// something really wrong!)
		typedef int iter_t;
	protected:
		// The actual integration function: 
		// This must be overridden 
		virtual double Integrate(Function_R_R &f,double a,double b);
		
		// Must use derived class, hence protected. 
		Integrator_R_R() {}
	public:  _CPP_OPERATORS
		virtual ~Integrator_R_R() {}
		
		// Perform integration over f, from a to b. 
		inline double Int(Function_R_R &f,double a,double b)
			{  return(Integrate(f,a,b));  }
		inline double operator()(Function_R_R &f,double a,double b)
			{  return(Int(f,a,b));  }
};


// Simple non-adaptive trapeziod (triangle) integration using 
// a specified fixed number of equidistant evaluation points. 
class Integrator_R_R_Trapezoid : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		// Number of equally-sized intervals: 
		iter_t nintervals;
		
		Integrator_R_R_Trapezoid(iter_t _nintervals=64) : 
			nintervals(_nintervals) {}
		~Integrator_R_R_Trapezoid() {}
};


// Simple non-adaptive Simpson-rule integration using a 
// specified fixed number of equidistant evaluation points. 
class Integrator_R_R_Simpson : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		// Number of equally-sized intervals: 
		iter_t nintervals;
		
		Integrator_R_R_Simpson(iter_t _nintervals=64) : 
			nintervals(_nintervals) {}
		~Integrator_R_R_Simpson() {}
};


#warning "Should use a base class for all adaptive/refining integrators."

// Trapezoid integration. 
// This is the best integration algorithm if f(x) is periodic, analytic 
// (holomorph) on R and the integration is over one complete period. 
class Integrator_R_R_TrapezoidRefining : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		// User-tunable parameters: 
		// Specify relative error in epsilon and max. number of 
		// refinements in maxdepth (0 to disable limitation [unwise]). 
		// The number of function evaluations after k refinements 
		// is 2^k + 1 (this is NOT an adaptive algorithm). 
		// Note: epsilon is just a rough guess for the relative error 
		//       estimated by comparing the results from rectangle and 
		//       triangle (trapezoid) integrals. For smooth functions, 
		//       the real error is most likely smaller. 
		// NOTE: If you use this function to integrate over one period 
		//       of an analytic periodic function, use sqrt(eps) instead 
		//       eps (as value for epsilon) when calling the integrator 
		//       function. 
		double epsilon;
		int maxdepth;
		
		Integrator_R_R_TrapezoidRefining(
			double _epsilon=1.0e-5,int _maxdepth=12) : 
			epsilon(_epsilon),maxdepth(_maxdepth) {}
		~Integrator_R_R_TrapezoidRefining() {}
};


// Integrator: 
// Use adaptive simpson formular to integrate f from x=a to x=b. 
class Integrator_R_R_SimpsonAdaptive : public Integrator_R_R
{
	private:
		// Used to pass data between the recursive calls: 
		Function_R_R *f;
		int depth;
		
		// Internally used recursive integration function: 
		double _RecInt(double a,double b,double fa,double fb);
		
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		// User-tunable parameters: 
		// Specify relative error in epsilon and max. recursion depth in 
		// maxdepth (0 to disable limitation [unwise]). 
		// (Max. 2^maxdepth+1 function evaluations are performed.)
		// Note: Minimum number is 9 evaluations (min depth 3) to prevent 
		//       too early termination. 
		// Note: epsilon is just a rough guess for the relative error 
		//       estimated by comparing the results from triangle and 
		//       simpson-rule integrals. For smooth functions, the 
		//       real error is most likely much smaller. 
		double epsilon;
		int maxdepth;
		
		Integrator_R_R_SimpsonAdaptive(
			double _epsilon=1.0e-5,int _maxdepth=32) : 
			f(NULL),depth(-1),epsilon(_epsilon),maxdepth(_maxdepth) {}
		~Integrator_R_R_SimpsonAdaptive() {}
};


// Generally quite good; needs few function evaluations: 
class Integrator_R_R_GaussLegendre : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		//-- User-tunable parameters: 
		// Divide range a..b into this many intervals and integrate 
		// them sequentially. 
		iter_t nintervals;
		// Order of the Gauss/Legendre integration (determines order of 
		// interpolation Legendre polynomial); 
		// Implemented values: 3, 5, 10
		int order;
		
		Integrator_R_R_GaussLegendre(int _nintervals=1,int _order=10) : 
			nintervals(_nintervals),order(_order) {}
		~Integrator_R_R_GaussLegendre() {}
};


// FUBAR! Do not use ATM. 
class Integrator_R_R_GaussCebyshev : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		//-- User-tunable parameters: 
		// Divide range a..b into this many intervals and integrate 
		// them sequentially. 
		iter_t nintervals;
		// Order of the Gauss/Cebyshev integration (determines order of 
		// interpolation Cebyshev polynomial); 
		// Implemented values: 3, 5, 10
		int order;
		
		Integrator_R_R_GaussCebyshev(int _nintervals=1,int _order=10) : 
			nintervals(_nintervals),order(_order) {}
		~Integrator_R_R_GaussCebyshev() {}
};


class Integrator_R_R_NewtonCotes : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		//-- User-tunable parameters: 
		// Divide range a..b into this many intervals and integrate 
		// them sequentially. 
		iter_t nintervals;
		// Order of the Newton/Cotes integration (determines order of 
		// interpolation polynomial); 
		// Implemented values: 2, 4
		int order;
		
		Integrator_R_R_NewtonCotes(int _nintervals=1,int _order=4) : 
			nintervals(_nintervals),order(_order) {}
		~Integrator_R_R_NewtonCotes() {}
};


// Romberg integration using trapesoid (triangle) method and extrapolation 
// to step size 0. 
class Integrator_R_R_Romberg : public Integrator_R_R
{
	protected:
		// The actual integration function: [overriding a virtual]
		double Integrate(Function_R_R &f,double a,double b);
		
	public:
		//-- User-tunable parameters: 
		// epsilon: Relative error estimate. 
		// maxdepth: Max number of refinements; number of evaluations 
		//           is 2^depth+1; maxdepth may not be larger than 15. 
		double epsilon;
		int maxdepth;
		
		Integrator_R_R_Romberg(double _epsilon=1.0e-5,int _maxdepth=12) : 
			epsilon(_epsilon),maxdepth(_maxdepth) {}
		~Integrator_R_R_Romberg() {}
};


}  // end of namespace NUM

#endif  /* _WW_NUMERICS_INTEGRATE_H_ */
