/*
 * numerics/ode/odedrv_rk.cc
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the Runge-Kutta steppers. 
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

#include "odedrv_rk.h"
#include <stdio.h>
#include <assert.h>


namespace NUM  // numerics
{

void ODESolver_RungeKutta::_DoRecalcLimits(ODESolver *ode)
{
	if(!stepper)  return;
	
	// Get order of consistency: 
	int p=stepper->GetStepperParams()->p;
	
	adapt_limit1 = (1.0-pow(2.0,-p)) * ode->epsilon;
	adapt_limit0 = adapt_limit1 * pow(2.0,p+1);
	// Add some safety margin to prevent up-down-up-down-up - switches. 
	// This should normally not be necessary. 
	adapt_limit1 *= 0.9;
	
	need_recalc_limits=0;	
}


int ODESolver_RungeKutta::Init(ODESolver * /*ode*/)
{
	need_recalc_limits=1;
	
	return(0);
}


ODESolver::StepState ODESolver_RungeKutta::Step(ODESolver *ode,double *h_max)
{
	if(need_recalc_limits)
	{  _DoRecalcLimits(ode);  }
	
	int dim=ode->Dim();
	double ynF[dim],ynH[dim],ynHH[dim];
	
	// Check step size against upper limit. 
	StepState rv=_CheckUpperLimit_H(ode,h_max);
	
	double ah=fabs(ode->h);
	for(bool first=1,last=(ah<=ode->h_min);;first=0)
	{
		if(first)
		{
			for(int d=0; d<dim; d++)
			{  ynF[d]=ode->yn[d];  ynH[d]=ode->yn[d];  }
			
			// Do the full step: 
			stepper->Step(ode->GetODEFunc(),ode->xn,ode->h,ynF,ode->dy_dx);
			++ode->nsteps;
		}
		else
		{
			// Use saved half step from last time as full step: 
			for(int d=0; d<dim; d++)
			{  ynF[d]=ynHH[d];  ynH[d]=ode->yn[d];  }
		}
		
		if(ode->used_h_min>ah)  ode->used_h_min=ah;
		if(ode->used_h_max<ah)  ode->used_h_max=ah;
		
		// Do the two half steps: 
		double h2=0.5*ode->h;
		
		// Check for h underflow: 
		{
			volatile double x_tmp=ode->xn;
			volatile double xh2_tmp=ode->xn+h2;
			if(x_tmp==xh2_tmp)
			{  return(SS_HUnderflow);  }
		}
		
		// First half step: 
		stepper->Step(ode->GetODEFunc(),ode->xn,h2,ynH,ode->dy_dx);
		++ode->nsteps;
		
		// Save that in case we need to reduce the step size: 
		if(!last)  for(int d=0; d<dim; d++)  ynHH[d]=ynH[d];
		
		// Second half step: 
		{
			double m_dy_dx[dim];
			ode->GetODEFunc()(ode->xn+h2,ynH,m_dy_dx);
			stepper->Step(ode->GetODEFunc(),ode->xn+h2,h2,ynH,m_dy_dx);
			++ode->nsteps;
		}
		
		// Compute the difference: 
		// (maximum norm of full_step minus two_half_steps) 
		double delta;
		if(!dim)
		{  delta=0.0;  }
		else if(ode->yscale)
		{
			// Calculate the yscale values: 
			ode->GetODEFunc().CalcYScale(ode->xn,ode->yn,ode->dy_dx,
				ode->h,ode->yscale);
			
			delta=fabs((ynF[0]-ynH[0])/ode->yscale[0]);
			for(int d=1; d<dim; d++)
			{
				double diff=fabs((ynF[d]-ynH[d])/ode->yscale[d]);
				if(delta<diff)  delta=diff;
			}
		}
		else // no yscale
		{
			delta=fabs(ynF[0]-ynH[0]);
			for(int d=1; d<dim; d++)
			{
				double diff=fabs(ynF[d]-ynH[d]);
				if(delta<diff)  delta=diff;
			}
		}
		
		if(delta<=adapt_limit0)
		{
			// Accept result. 
			ode->xn+=ode->h;
			ode->h_used=ode->h;
			
			if(delta<=adapt_limit1)
			{
				// Even try larger step size next time: 
				ode->h+=ode->h;
				// Maximum limit is checked when we next enter. 
			}
			
			break;
		}
		
		// Must reduce step size. 
		if(last)
		{  // Already smallest step size. 
			ode->xn+=ode->h;
			ode->h_used=ode->h;
			break;
		}
		
		ode->h=h2;
		if(ode->h>0.0)
		{
			if(ode->h<ode->h_min)
			{  ode->h=ode->h_min;  last=1;  rv=SS_HMinLim;  }
		}
		else
		{
			if(ode->h>-ode->h_min)
			{  ode->h=-ode->h_min;  last=1;  rv=SS_HMinLim;  }
		}
	}
	
	// Update yn: 
	for(int d=0; d<dim; d++)
		ode->yn[d]=ynH[d];
	
	// Update dy_dx: 
	ode->GetODEFunc()(ode->xn,ode->yn,ode->dy_dx);
	
	return(rv);
}


RefString ODESolver_RungeKutta::DrvName()
{
	RefString tmp;
	tmp.sprintf(0,"RungeKutta (%s stepper)",
		stepper ? stepper->GetStepperParams()->name : "no");
	return(tmp);
}


int ODESolver_RungeKutta::SetStepper(ODESolver_RK_Stepper *rk_stepper)
{
	DELETE(stepper);
	stepper=rk_stepper;
	need_recalc_limits=(stepper ? 1 : 0);
	return(0);
}


ODESolver_RungeKutta::ODESolver_RungeKutta(ODESolver_RK_Stepper *rk_stepper) : 
	ODESolverDriver()
{
	stepper=rk_stepper;
	
	adapt_limit0=-1.0;
	adapt_limit1=-1.0;
	
	need_recalc_limits=(stepper ? 1 : 0);
}

ODESolver_RungeKutta::~ODESolver_RungeKutta()
{
	DELETE(stepper);
}


//------------------------------------------------------------------------------

const ODESolver_RK_Stepper::StepperParams *
	ODESolver_RK_Stepper::GetStepperParams()
{
	static const StepperParams sp=
	{
		name: "[none]",
		m: 0,
		p: 0,
	};
	
	return(&sp);
}


int ODESolver_RK_Stepper::Step(Function_ODE & /*odefunc*/,
	double /*xn*/,double /*h*/,double * /*yn*/,
	double * /*dy_dx*/)
{
	return(-1);
}


ODESolver_RK_Stepper::ODESolver_RK_Stepper()
{
}

ODESolver_RK_Stepper::~ODESolver_RK_Stepper()
{
}

}  // end of namespace NUM
