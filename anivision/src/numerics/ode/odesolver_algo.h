/*
 * numerics/ode/odesolver_algo.h
 * 
 * Numerical ODE (ordinary differential equation) solving algorithms. 
 * NOTE: This file is included by odesolver.h, so there is normally 
 *       no need to include it manually. 
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

#ifndef _WW_NUMERICS_ODE_SOLVER_ALGO_H_
#define _WW_NUMERICS_ODE_SOLVER_ALGO_H_

#error "THIS FILE IS UNUSED AND SUPERSEDED. DELETE IT."


namespace NUM  // numerics
{

class ODESolver_GBS : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_GBS() : ODESolver() {}
		~ODESolver_GBS() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
		
		// [overriding a virtual]
		void AdaptiveStep(Function_ODE &odefunc,
			double &xn,double &h,double *yn);
};


// Order of consistency: 1. Generally the worst solver. 
class ODESolver_Euler : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_Euler() : ODESolver() {}
		~ODESolver_Euler() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// Order of consistency: 2. Generally medium-quality. 
class ODESolver_Heun : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_Heun() : ODESolver() {}
		~ODESolver_Heun() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// Order of consistency: 2. Generally medium-quality. 
class ODESolver_Collatz : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_Collatz() : ODESolver() {}
		~ODESolver_Collatz() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODESolver_Heun3 : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_Heun3() : ODESolver() {}
		~ODESolver_Heun3() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODESolver_Kutta3 : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_Kutta3() : ODESolver() {}
		~ODESolver_Kutta3() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// Order of consistency: 3. Generally higher medium-quality. 
class ODESolver_RungeKutta3 : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_RungeKutta3() : ODESolver() {}
		~ODESolver_RungeKutta3() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// One of the "classical" Runge-Kutta algorithms. 
// (THE "classical" one because this one is the oODEst.) 
// Order of consistency: 4. Normally quite good solver 
// (generally best quality in this file). 
class ODESolver_RungeKutta : public ODESolver
{
	private:
		static const SolverParams sp;
	public:
		ODESolver_RungeKutta() : ODESolver() {}
		~ODESolver_RungeKutta() {}
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

// General explicit Runge-Kutta solver for any order of algorithms. 
// You just have to supply a list of coefficients and weights; see 
// lengthy comment in class. 
// DEFAULT: An order 4 Runge-Kutta-Algorithm applying the analogon 
// to the 3/8th integration rule (normally a quite good solver, 
// generally best quality in this file). 
class ODESolver_GeneralRungeKutta : public ODESolver
{
	protected:
		// This stores order of algorithm and order of consistency. 
		SolverParams sp;
		
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
		ODESolver_GeneralRungeKutta();
		~ODESolver_GeneralRungeKutta() {}
		
		// NOTE: Functions for setting gamma, alpha, beta, sp.n, sp.p 
		//       must be implemented when actually needed. 
		
		// Do single step computation of the ODE solve algorithm. 
		// [overriding a virtual]
		void ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn);
		// [overriding a virtual]
		const SolverParams *GetSolverParams()
			{  return(&sp);  }
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_ODE_SOLVER_ALGO_H_ */
