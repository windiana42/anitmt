/*
 * numerics/ode/odedrv_rkf456.h
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the Runge-Kutta-Fehlberg method: This is a 6-stage RK method of 
 * order 5 with an embedded RK of order 4 for local error estimation. 
 * (Note: 5-stage RK methods of order 5 do not exist.)
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

#ifndef _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_FEHLBERG456_H_
#define _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_FEHLBERG456_H_

#include <numerics/ode/odesolver.h>


namespace NUM  // numerics
{

// ODE solver driver for an 6-stage order 5 Runge-Kutta algorithm which has 
// an order 4 RK embedded for local error estimation with little additional 
// overhead. 
// You will not want to use this class directly to solve an 
// ODE; see class ODESolver in <odesolver.h>. 
class ODESolver_RungeKutta_Fehlberg456 : public ODESolverDriver
{
	protected:
		// Interface to ODESolver: Do not use directly: 
		// [overriding virtuals]
		RefString DrvName();
		int Init(ODESolver *ode);
		
		// One adaptive step (involving 1 single "4-and-6-stage" - step) 
		// Changes h as necessary for specified maximum absolute 
		// error epsilon (and hence will actually perform more than 
		// 1 single step when step size has to be decreased) . 
		// [overriding virtual]
		StepState Step(ODESolver *ode,double *h_max);
	public:
		ODESolver_RungeKutta_Fehlberg456();
		~ODESolver_RungeKutta_Fehlberg456();
};

}  // end of namespace NUM

#endif  /* _WW_NUMERICS_ODE_DRIVER_RUNGEKUTTA_FEHLBERG456_H_ */
