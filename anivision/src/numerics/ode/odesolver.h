/*
 * numerics/ode/odesolver.h
 * 
 * Numerical ODE (ordinary differential equation) solver. 
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

#ifndef _WW_NUMERICS_ODE_SOLVER_H_
#define _WW_NUMERICS_ODE_SOLVER_H_

#include <hlib/refstring.h>

#include <numerics/function.h>


namespace NUM  // numerics
{

// ---STOP---
// 
// You should read this in order to get a clue on how things are 
// meant to work: 
// For actual ODE solving, create an object ODESolver. 
// This is not a real solver but merely stores all the state info 
// current time, step size, etc. and also the currently used 
// solver algorithm. 
// The actual solving is done by some class derived from 
// ODESolverDriver. You need to create such one and pass it to 
// the ODESolver to do the actual solving. Note that ODESolver 
// will destory the attached ODESolverDriver when you attach a 
// new one or destory the ODESolver. 

class ODESolver;
class ODESolverDriver;


// Internal use only: 
struct _ODESolver_Namespace
{
	enum StepState
	{
		SS_OK=0,       // OK
		SS_ShortStep,  // OK; needed to reduce step size 
			           // to match passed h_max.
		SS_HMaxLim,    // OK; needed to reduce step size 
			           // to match this->h_max. 
		SS_HMinLim,    // OK; needed to increase step size 
			           // to match this->h_min. 
			           // NOTE: This means that the desired accuracy 
			           //       (epsilon) is NO LONGER guaranteed. 
		SS_NoDriver=-1,    // No solver driver specified. 
		SS_HUnderflow=-2,  // OOPS: step size underflow; cannot go on 
		//Anything different < 0  -> Severe error in solver driver
	};
};

class ODESolver : public _ODESolver_Namespace
{
	private:
		// ODE solver driver -- the "actual solver" (not the 
		// stepper used by the solver). 
		ODESolverDriver *drv;
		
		int    dim;  // Dimension of the linear differentinal equation 
		             // (i.e. the number of linear equations); taken 
		             // from the ODE function passed. 
		
		// Pointer to the function to be solved. 
		// (No reference so that we can change it at runtime.) 
		// Caller is responsible for this; we will not delete it. 
		Function_ODE *_odefunc;
		inline void odefunc(double x,const double *y,double *result)
			{  (*_odefunc)(x,y,result);  }
		
		// Free/Alloc yn,dy_dx,yscale arrays (=vectors): 
		// Also sets dim to the passed value. 
		// Use newdim=0 to free the memory. 
		void _ParamRealloc(int newdim,bool need_yscale);
		
		// Do driver init and also calculate current dy_dx: 
		void _DriverInit();
		
	public:
		// Note: People using this class are assumed to know what we're 
		//       doing here. Hence, I chose to make most vars public for  
		//       easy access. 
		double xn;      // Current value of independent variable ("time"). 
	               		// Updated to xn+h_used by stepper. 
		double *yn;     // Current values of the dependent variables. 
	               		// Changed to hold updated values. 
		double *dy_dx;  // Derivations dy/dx at xn, i.e. what odefunc(xn,yn,<..>) 
	               		// returns; passed here to avoid unnecessary evaluations. 
	               		// Updated to hold dy/dx at new xn. 
		double h;       // Initial step size guess; updated with new step size 
	               		// guess to be used next time. 
		double h_used;  // Returns actually used step size. 
		int    nsteps;  // Number of steps performed (to gain used "solution"). 
		double epsilon; // Accuracy to be achieved. 
		                // NOTE: Must call drv->Init() when changing it. 
		double *yscale; // Scaling factors for error estimate. These get updated 
		                // by the solver during stepping; NULL if need_yscale=0. 
		
		double h_min,h_max;  // Hard min and max for the step size for 
		                     // adaptive algorithms. Default: 0 and 1e30. 
		
		// Some statistics: 
		// Smallest and largest step size used: 
		double used_h_min,used_h_max;
		
	public:  _CPP_OPERATORS
		ODESolver();
		~ODESolver();
		
		// Get dim of the linear differentinal equation: 
		int Dim() const
			{  return(dim);  }
		
		// Get currently used ODE function: 
		// Be careul -- NULL deref in case it was not set. 
		Function_ODE &GetODEFunc()
			{  return(*_odefunc);  }
		
		// Set the actual ODE solver (driver). 
		// Most people will choose ODESolver_RungeKutta. 
		// However, Gragg Bulirsch Stör should be used for 
		// superior quality; use ODESolver_GBS for that. 
		// NOTE: The solver must have been allocated using 
		//       operator new and will be destoryed on 
		//       ODESolver destruction. If there was already 
		//       a solver set, the old one is deleted and 
		//       the new one replaces the old one. 
		// Return value: 
		//   0 -> OK
		int SetSolver(ODESolverDriver *drv);
		
		// Get the currently used solver or NULL. 
		// Be careful with the returned pointer. 
		ODESolverDriver *GetSolver()
			{  return(drv);  }
		
		// After you have set the solver, you should set the inital 
		// conditions. However, note that you can also set the inital 
		// conditions first and set the solver lateron. You  may 
		// even change the solver (driver) during solving. 
		// The value need_yscale specifies if odefunc.CalcYScale() 
		// shall be used to calculate the scale factors for error 
		// estimation. This is generally advisable. You may consider 
		// calling (static) ODESolverDriver::DefaultYScale() from within 
		// (virtual) odefunc.calcyscale() to use a general-purpose formula. 
		// (If you set this to 0, all scale factors will be 1.) 
		// A default yscale is used in case you pass NULL. 
		// NOTE: A pointer to the passed Function_ODE is stored 
		//       internally so make sure that it exists as long 
		//       as the ODESolver is "living". ODESolver will NOT 
		//       desotroy the passed odefunc at any time. 
		// Return value: 
		//    0 -> OK
		// NOTE: You may consider also setting h_min and h_max for 
		//       hard step size limits. 
		int Init(Function_ODE &odefunc,
			double x0,const double *y0,double h0,double epsilon,
			bool need_yscale);
		
		// Compute one step. 
		// If the pointer is non-NULL, do not make a step 
		// larger than the passed value. (Useful if you 
		// want to step up to a specified time.)
		// Return value: 
		//   See enum StepState above. 
		StepState Step(double *h_max=NULL);
};


// Base class for different ODE solver drivers (not steppers): 
// There are different drivers available, see for example 
// <odedrv_rk.h> and <odedrv_gbs.h>. 
class ODESolverDriver : public _ODESolver_Namespace
{
	public:
		// This is a general-purpose error estimate scale factor 
		// calculation formula. To be called e.g. from within 
		// Function_ODE::calcyscale(). No side effects. 
		// Stores restult in yscale[]. 
		static void DefaultYScale(int dim,double xn,const double *yn,
			const double *dy_dx,double h,double *yscale);
	protected:
		// Some logic to adjust ode->h so that it is smaller than 
		// ode->h_max and smaller than *h_max if non-NULL. 
		// Returns SS_HMaxLim or SS_ShortStep in case h was adjusted; 
		// otherwise SS_OK is returned. 
		StepState _CheckUpperLimit_H(ODESolver *ode,double *h_max);
		
	public:  _CPP_OPERATORS
		ODESolverDriver();
		virtual ~ODESolverDriver();
		
		// Return driver name: 
		virtual RefString DrvName();
		
		// Initalize driver before first Step(). 
		// Or, because epsilon was changed. 
		// Return value: 
		//   0 -> OK
		int Init(ODESolver *ode);
		
		// Description: See ODESolver::Step(). 
		virtual StepState Step(ODESolver *ode,double *h_max=NULL);
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_ODE_SOLVER_H_ */
