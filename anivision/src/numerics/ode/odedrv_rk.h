/*
 * numerics/ode/odedrv_rk.h
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the Runge-Kutta steppers 
 *   -and-
 * several associated Runge-Kutta steppers. 
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

#ifndef _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_H_
#define _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_H_

#include <numerics/ode/odesolver.h>


namespace NUM  // numerics
{

// Runge-Kutta stepper as used by the Runge-Kutta driver(s). 
// This is just a base class; the actual stepper must be derived 
// from this class. 
class ODESolver_RK_Stepper
{
	public:
		struct StepperParams
		{
			// Name of the algorithm: 
			const char *name;
			
			// Order of the algorithm: 
			int m;
			
			// Order of consistency. 
			// This is never larger than the order of the algorithm (m). 
			// An algorithm has the order of consistency p, if the 
			// local discretisation error scales like h^(p+1) 
			// where h is the step size. 
			int p;
		};
	public:  _CPP_OPERATORS
		ODESolver_RK_Stepper();
		virtual ~ODESolver_RK_Stepper();
		
		// This must be overridden by the derived stepper class to 
		// supply some information about the algorithm. 
		virtual const StepperParams *GetStepperParams();
		
		// Compute a single step using the passed information. 
		// dy_dx is odefunc at (xn,yn) [i.e. odefunc(xn,yn,dy_dx)]. 
		// Return value: 
		//   0 -> OK
		virtual int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
};


// ODE solver driver for the Runge-Kutta family of steppers. 
// There may be an alternative Runge-Kutta driver available 
// which is using the same steppers but using different 
// error estimation and/or adaptive step size control. 
// You will not want to use this class directly to solve an 
// ODE; see class ODESolver in <odesolver.h>. 
class ODESolver_RungeKutta : public ODESolverDriver
{
	protected:
		// Runge-Kutta stepper to use: 
		ODESolver_RK_Stepper *stepper;
		
		// Internal parameter used for adaptive step control: 
		double adapt_limit0;
		double adapt_limit1;
		
		// Re-calc the adaptive limits: 
		bool need_recalc_limits;
		void _DoRecalcLimits(ODESolver *ode);
		
	protected:
		// Interface to ODESolver: Do not use directly: 
		// [overriding virtuals]
		RefString DrvName();
		int Init(ODESolver *ode);
		
		// One adaptive step (involving 3 single steps: two with 
		// step size h/2 and one with h for error estimation). 
		// Computes steps xn -> xn+h/2 -> xn+h. 
		// Changes h as necessary for specified maximum absolute 
		// error epsilon (and hence will actually perform more than 
		// 3 single steps when step size has to be decreased). 
		// [overriding virtual]
		StepState Step(ODESolver *ode,double *h_max);
	public:
		// NOTE: Pass stepper to use as parameter, allocated using 
		//       operator new. 
		//       You may change the stepper using SetStepper() any 
		//       time you want. The ODESolver_RungeKutta will 
		//       delete the passed stepper upon destruction or 
		//       when a new one is set. 
		ODESolver_RungeKutta(ODESolver_RK_Stepper *rk_stepper=NULL);
		~ODESolver_RungeKutta();
		
		// -----<User interface:>-----
		// Change Runge Kutta stepper; must have been allocated 
		// using operator new; NULL for no stepper. 
		// Will delete the currently set stepper. 
		// Return value: 
		//   0 -> OK
		int SetStepper(ODESolver_RK_Stepper *rk_stepper);
};


// Lots of different Runge-Kutta steppers: 

// Order of consistency: 1. Generally the worst solver. 
class ODE_RK_Stepper_Euler : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_Euler() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_Euler() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// Order of consistency: 2. Generally medium-quality. 
class ODE_RK_Stepper_Heun : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_Heun() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_Heun() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// Order of consistency: 2. Generally medium-quality. 
class ODE_RK_Stepper_Collatz : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_Collatz() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_Collatz() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODE_RK_Stepper_Heun3 : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_Heun3() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_Heun3() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODE_RK_Stepper_Kutta3 : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_Kutta3() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_Kutta3() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODE_RK_Stepper_RungeKutta3 : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_RungeKutta3() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_RungeKutta3() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// One of the "classical" Runge-Kutta algorithms. 
// (THE "classical" one because this one is the oODEst.) 
// Order of consistency: 4. Normally quite good solver 
// (generally best quality in this file). 
class ODE_RK_Stepper_RungeKutta : public ODESolver_RK_Stepper
{
	private:
		static const StepperParams sp;
	public:
		ODE_RK_Stepper_RungeKutta() : ODESolver_RK_Stepper() {}
		~ODE_RK_Stepper_RungeKutta() {}
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

// General explicit Runge-Kutta solver for any order of algorithms. 
// You just have to supply a list of coefficients and weights; see 
// lengthy comment in class. 
// DEFAULT: An order 4 Runge-Kutta-Algorithm applying the analogon 
// to the 3/8th integration rule (normally a quite good solver, 
// generally best quality in this file). 
class ODE_RK_Stepper_GeneralRungeKutta : public ODESolver_RK_Stepper
{
	protected:
		// This stores order of algorithm and order of consistency. 
		StepperParams sp;
		
		// Coefficiants for the general Runge Kutta algorithm. 
		// gamma: Weights when summing up final solution 
		//        (solution = y delta for this step). 
		//        Weight gamma[i] for eval result in sub-step [i]. 
		// alpha: Eval point distances in h-sized units for sub-step [i]. 
		//        (=Weight for calculating x-value for ODE eval in 
		//        sub-step [i].) 
		// _beta: Weights when calculating y-value for ODE eval in 
		//        sub-steps. As this implements the explicit Runge Kutta 
		//        algorithm only, these weights can be specified for 
		//        all _previous_ sub-step evaluation results when 
		//        calculating the current sub-step y values. 
		//        Hence, in sub-step i, the y value is calculated as 
		//         y[i] = \sum_{s=0}^{i-1} beta[i,s] * k[s]
		//        where k[s] (s<i) is the ODE eval result in the 
		//        previously done sub-step s and 
		//        beta[i,s]=_beta[i*(i-1)/2+s]. 
		// Note, that you can write down a table like the following one 
		// as an example for m=4 with a:=alpha, g:=gamma and b:=beta ...
		// 
		//          a[0] |
		//          a[1] | b[1,0]
		//          a[2] | b[2,0]  b[2,1]
		//          a[3] | b[3,0]  b[3,1]  b[3,2]
		//         ------|------------------------------
		//               |  g[0]    g[1]   g[2]   g[3]
		// 
		// ...and then simply use: 
		//         _beta[]={<line by line as they appear>}
		// 
		// For example, one of the classical Runge-Kutta looks like that: 
		// 
		//          0.0 |
		//          0.5 |  0.5
		//          0.5 |  0.0   0.5
		//          1.0 |  0.0   0.0   1.0
		//         -----|-------------------------
		//              |  1/6   1/3   1/3   1/6
		// 
		// Hence we have: alpha[4]={ 0.0, 0.5, 0.5, 1.0 }
		//                gamma[4]={ 1./6., 1./3., 1./3., 1./6. };
		//                _beta[6]={ 0.5, 
		//                           0.0, 0.5, 
		//                           0.0, 0.0, 1.0 }
		// 
		// Another example, which is not implemented as separate (optimized) 
		// class above and therefore the default algorithm when creating 
		// this class: An order 4 Runge-Kutta-Algorithm applying the analogon 
		// to the 3/8th integration rule: 
		// 
		//          0.0 |
		//          1/3 |  1/3
		//          2/3 | -1/3   1.0
		//          1.0 |  1.0  -1.0   1.0
		//         -----|-------------------------
		//              |  1/8   3/8   3/8   1/8
		// 
		const double *gamma;  // array of size [m]
		const double *alpha;  // array of size [m]
		const double *_beta;  // array of size [m*(m-1)/2]
		
		// Returns beta[i,s] (check above comment on the meaning of 
		// this notation). 
		// Valid input range: i=1..m-1, s=0..m-2
		inline double beta(int i,int s) const
			{  return(_beta[i*(i-1)/2+s]);  }
	public:
		ODE_RK_Stepper_GeneralRungeKutta();
		~ODE_RK_Stepper_GeneralRungeKutta() {}
		
		// NOTE: Functions for setting gamma, alpha, beta, sp.n, sp.p 
		//       must be implemented when actually needed. 
		
		// [overriding virtuals]
		int Step(Function_ODE &odefunc,
			double xn,double h,double *yn,
			double *dy_dx);
		const StepperParams *GetStepperParams()
			{  return(&sp);  }
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_H_ */
