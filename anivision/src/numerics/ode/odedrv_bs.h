/*
 * numerics/ode/odedrv_bs.h
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the Bulirsch-Stoer algorithm. 
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

#ifndef _WW_NUMERICS_ODE_DRIVER_BULIRSCHSTOER_H_
#define _WW_NUMERICS_ODE_DRIVER_BULIRSCHSTOER_H_

#include <numerics/ode/odesolver.h>


namespace NUM  // numerics
{

// See <numerics/interpolate/extr0ebase.h>
class Extrapolator_0E_Base;

// ODE solver driver for the Bulirsch-Stoer algorithm. 
// You will not want to use this class directly to solve an 
// ODE; see class ODESolver in <odesolver.h>. 
class ODESolver_BulirschStoer : public ODESolverDriver
{
	public:
		enum  // <-- For the flags to be passed to the constructor. 
		{
			// Select ODE solving method: Explicit or SemiImplicit. 
			// Defaults to explicit method; 
			// NOTE: The semi-implicit version is meant for stiff ODEs. 
			//       The semi-implicit algorithm needs the Jacobi 
			//       matrix of the ODE, i.e. you must provide 
			//       Function_ODE::jacobian(). 
			M_Explicit=    0x00,    // <-- default
			M_SemiImplicit=0x01,
			
			// Select extrapolation method to use: 
			// Polynomial or Rational extrapolation. The preferred 
			// one depends on the problem; try out. 
			E_Polynomial=  0x00,    // <-- default
			E_Rational=    0x02,
			
			// Select sequence of steps to use for the explicit 
			// method (the semi-implicit method has a fixed 
			// sequence: 2,6,10,14,22,34,50,...): 
			//   Deuflhard:      2,4,6,8,10,12,14,...
			//   Burlisch-Stoer: 2,4,6,8,12,16,24,...
			S_BurlischStoer=0x00,    // <-- default
			S_Deuflhard=    0x04,
		};
	private:
		// As passed to the constructor: 
		int flags;
		
		// Max. number of extrapolation steps to use: 
		// Defaults to 8 for explicit and 7 for semi-implicit method. 
		int KMAXX;
		
		// Do not touch these; some of them are to be eliminated: 
		int IMAXX;
		int first,kmax,kopt,nvold;
		double epsold,xnew;
		double *a,**alf;
		SMatrix<double> *_dfdy;
		
		// nseq[i] is the number of steps for the i-th 
		// approximation/extrapolation step. 
		const int *nseq;
		
		// Extrapolator to use: 
		Extrapolator_0E_Base *xpol;
		
		// Quick access to the flags: 
		bool _IsExplicit() const
			{  return(!(flags&M_SemiImplicit));  }
		bool _IsSemiImplicit() const
			{  return((flags&M_SemiImplicit));  }
		
		// Details: See implementation. 
		static void ModifiedMidpoint(Function_ODE &odefunc,
			double xn,double htot,int nsteps,
			const double *yn,const double *dy_dx,double *yres);
		static void SIMidpoint(Function_ODE &odefunc,
			double xn,double htot,int nsteps,
			const double *yn,
			const double *dy_dx,const double *df_dx,const SMatrix<double> &df_dy,
			double *yres);
		
		// Here the real work is done :)
		StepState ComputeStep(Function_ODE &odefunc,ODESolver *ode,
			double *yscal,double *hdid,double *hnext);
		
	protected:
		// Interface to ODESolver: Do not use directly: 
		// [overriding virtuals]
		RefString DrvName();
		int Init(ODESolver *ode);
		
		// One adaptive step which can involve several substeps using 
		// the modified midpoint rule or the semi-implicit midpoint rule. 
		// For details, see the implementation. 
		// [overriding virtual]
		StepState Step(ODESolver *ode,double *h_max);
	public:
		// See the enum at the beginninfg of this class for values 
		// to pass here (OR'ed together): 
		// k_max: Max number of approximation/extrapolation steps to 
		//        do; use values <0 for a default value. 
		ODESolver_BulirschStoer(int flags=0,int k_max=-1);
		~ODESolver_BulirschStoer();
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_ODE_DRIVER_BULIRSCHSTOER_H_ */
