/*
 * numerics/ode/odedrv_rkf456.cc
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the "Runge-Kutta-Fehlberg-456 method. 
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

#include "odedrv_rkf456.h"
#include <stdio.h>
#include <assert.h>


namespace NUM  // numerics
{


int ODESolver_RungeKutta_Fehlberg456::Init(ODESolver * /*ode*/)
{
	return(0);
}


ODESolver::StepState ODESolver_RungeKutta_Fehlberg456::Step(
	ODESolver *ode,double *h_max)
{
	int dim=ode->Dim();
	
	// Check step size against upper limit. 
	StepState rv=_CheckUpperLimit_H(ode,h_max);
	
	double k[5][dim];
	double ytmp[dim];
	
	// This is for convenience: 
	const double *k1=ode->dy_dx;
	double *k2=k[0];
	double *k3=k[1];
	double *k4=k[2];
	double *k5=k[3];
	double *k6=k[4];
	
	double *yn=ode->yn;
	double xn=ode->xn;
	
	for(;;)
	{
		double h=ode->h;
		
		// Compute k1: Done. 
		
		// Compute k2: 
		double htmp1=2.0/9.0*h;
		for(int d=0; d<dim; d++)
			ytmp[d]=yn[d]+htmp1*k1[d];
		ode->GetODEFunc().operator()(xn+htmp1,ytmp,k2);
		
		// Compute k3: 
		htmp1=1.0/12.0*h;
		double htmp2=0.25*h;
		for(int d=0; d<dim; d++)
			ytmp[d]=yn[d]+htmp1*k1[d]+htmp2*k2[d];
		ode->GetODEFunc().operator()(xn+h/3.0,ytmp,k3);
		
		// Compute k4: 
		htmp1=69.0/128.0*h;
		htmp2=-243.0/128.0*h;
		double htmp3=135.0/64.0*h;
		for(int d=0; d<dim; d++)
			ytmp[d]=yn[d]+htmp1*k1[d]+htmp2*k2[d]+htmp3*k3[d];
		ode->GetODEFunc().operator()(xn+4.0/3.0*h,ytmp,k4);
		
		// Compute k5: 
		htmp1=-17.0/12.0*h;
		htmp2=27.0/4.0*h;
		htmp3=-27.0/5.0*h;
		double htmp4=16.0/15.0*h;
		for(int d=0; d<dim; d++)
			ytmp[d]=yn[d]+htmp1*k1[d]+htmp2*k2[d]+htmp3*k3[d]+htmp4*k4[d];
		ode->GetODEFunc().operator()(xn+h,ytmp,k5);
		
		// Compute k6: 
		htmp1=65.0/432.0*h;
		htmp2=-5.0/16.0*h;
		htmp3=13.0/16.0*h;
		htmp4=4.0/27.0*h;
		double htmp5=5.0/144.0*h;
		for(int d=0; d<dim; d++)
			ytmp[d]=yn[d]+htmp1*k1[d]+htmp2*k2[d]+htmp3*k3[d]+htmp4*k4[d]+
				htmp5*k5[d];
		ode->GetODEFunc().operator()(xn+5.0/6.0*h,ytmp,k6);
		
		// Compute local error estimate: 
		double delta;
		if(!dim)
		{  delta=0.0;  }
		else if(ode->yscale)
		{
			// Calculate the yscale values: 
			ode->GetODEFunc().CalcYScale(ode->xn,ode->yn,ode->dy_dx,
				ode->h,ode->yscale);
			
			delta=fabs((-2.0*k1[0]+9.0*k3[0]-64.0*k4[0]-15.0*k5[0]+72.0*k6[0])/
				ode->yscale[0]);
			for(int d=1; d<dim; d++)
			{
				double diff=fabs(
					(-2.0*k1[d]+9.0*k3[d]-64.0*k4[d]-15.0*k5[d]+72.0*k6[d])/
						ode->yscale[d]);
				if(delta<diff)  delta=diff;
			}
		}
		else // no yscale
		{
			delta=fabs(-2.0*k1[0]+9.0*k3[0]-64.0*k4[0]-15.0*k5[0]+72.0*k6[0]);
			for(int d=1; d<dim; d++)
			{
				double diff=fabs(-2.0*k1[d]+9.0*k3[d]-64.0*k4[d]-15.0*k5[d]+
					72.0*k6[d]);
				if(delta<diff)  delta=diff;
			}
		}
		delta*=fabs(h)/300.0;
		
		++ode->nsteps;
		
		// Calc optimum step size: 
		// The exponent 0.2 is 1/(p+1) with order of consistency 
		// p for the first and p+1 for the second method used. 
		// I use order 4 here, hence 0.2. 
		double h_opt = h*pow(ode->epsilon/delta,0.2);
		const double safety_fact=0.9;
		bool last=0;
		if(fabs(h)<safety_fact*fabs(h_opt))
		{
			// Accept result. 
			last=1;
			
			// See if step size increase should be tried: 
			assert(0);
			if(0/*FIXME*/)
			{
				//ode->h=FIXME; // beware h<0.
				// Maximum limit is checked when we next enter. 
			}
		}
		else
		{
			// Decrease step size: 
			ode->h=h*safety_fact;
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
			// Go on with reduced step size. 
		}
		
		if(last)
		{
			ode->h_used=h;
			
			// Update xn and yn: 
			ode->xn+=h;
			for(int d=0; d<dim; d++)
				ode->yn[d]+=h*(47.0/450.0*k1[d]+12.0/25.0*k3[d]+
					32.0/225.0*k4[d]+1.0/30.0*k5[d]+6.0/25.0*k6[d]);
			
			// Update dy_dx: 
			ode->GetODEFunc()(ode->xn,ode->yn,ode->dy_dx);
			
			break;
		}
	}
	
	return(rv);
}


RefString ODESolver_RungeKutta_Fehlberg456::DrvName()
{
	RefString tmp;
	tmp.sprintf(0,"RungeKuttaFehlberg456");
	return(tmp);
}


ODESolver_RungeKutta_Fehlberg456::ODESolver_RungeKutta_Fehlberg456() : 
	ODESolverDriver()
{
}

ODESolver_RungeKutta_Fehlberg456::~ODESolver_RungeKutta_Fehlberg456()
{
}

}  // end of namespace NUM
