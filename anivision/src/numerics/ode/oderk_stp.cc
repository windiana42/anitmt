/*
 * numerics/ode/oderk_stp.cc
 * 
 * Numerical ODE (ordinary differential equation) solver: 
 * Runge-Kutta stepper implementations. 
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

const ODE_RK_Stepper_Euler::StepperParams ODE_RK_Stepper_Euler::sp=
{
	name:"Euler",  // should be .name="Euler",.m=1,.p=1
	m:1,
	p:1
};

int ODE_RK_Stepper_Euler::Step(Function_ODE &odefunc,
	double /*xn*/,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	
	for(int d=0; d<dim; d++)
		yn[d]+=h*dy_dx[d];
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_Heun::StepperParams ODE_RK_Stepper_Heun::sp=
{
	name:"Heun",
	m:2,
	p:2
};

int ODE_RK_Stepper_Heun::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k2[dim];
	double ytmp[dim];
	
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+h*dy_dx[d];
	
	odefunc(xn+h,ytmp,k2);
	
	double h2=0.5*h;
	for(int d=0; d<dim; d++)
		yn[d]+=h2*(dy_dx[d]+k2[d]);
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_Collatz::StepperParams ODE_RK_Stepper_Collatz::sp=
{
	name:"Collatz",
	m:2,
	p:2
};

int ODE_RK_Stepper_Collatz::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k[dim];
	double ytmp[dim];
	
	double h2=0.5*h;
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+h2*dy_dx[d];
	
	odefunc(xn+h2,ytmp,k);
	
	for(int d=0; d<dim; d++)
		yn[d]+=h*k[d];
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_Heun3::StepperParams ODE_RK_Stepper_Heun3::sp=
{
	name:"Heun3",
	m:3,
	p:3
};

int ODE_RK_Stepper_Heun3::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k[dim];
	double ytmp[dim];
	
	double hh=h/3.0;
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+hh*dy_dx[d];
	
	odefunc(xn+hh,ytmp,k);
	
	hh=2.0/3.0*h;
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+hh*k[d];
	
	odefunc(xn+hh,ytmp,k);  // overwrite k
	
	hh=0.25*h;
	for(int d=0; d<dim; d++)
		yn[d]+=hh*(dy_dx[d]+3.0*k[d]);
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_Kutta3::StepperParams ODE_RK_Stepper_Kutta3::sp=
{
	name:"Kutta3",
	m:3,
	p:3
};

int ODE_RK_Stepper_Kutta3::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k[2][dim];
	double ytmp[dim];
	
	double hh=0.5*h;
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+hh*dy_dx[d];
	
	odefunc(xn+hh,ytmp,k[0]);
	
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+h*(2.0*k[0][d]-dy_dx[d]);
	
	odefunc(xn+h,ytmp,k[1]);
	
	hh=h/6.0;
	for(int d=0; d<dim; d++)
		yn[d]+=hh*(dy_dx[d]+4.0*k[0][d]+k[1][d]);
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_RungeKutta3::StepperParams ODE_RK_Stepper_RungeKutta3::sp=
{
	name:"RungeKutta3",
	m:3,
	p:3
};

int ODE_RK_Stepper_RungeKutta3::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k[2][dim];
	double ytmp[dim];
	
	for(int d=0; d<dim; d++)
		ytmp[d]=yn[d]+h*dy_dx[d];
	
	odefunc(xn+h,ytmp,k[1]);
	
	double hh=0.25*h;
	for(int d=0; d<dim; d++)
	{
		k[0][d]=dy_dx[d]+k[1][d];
		ytmp[d]=yn[d]+hh*k[0][d];
	}
	
	odefunc(xn+0.5*h,ytmp,k[1]);  // overwrite k[1]
	
	hh=h/6.0;
	for(int d=0; d<dim; d++)
		yn[d]+=hh*(k[0][d]+4.0*k[1][d]);
	
	return(0);
}

//==============================================================================

const ODE_RK_Stepper_RungeKutta::StepperParams ODE_RK_Stepper_RungeKutta::sp=
{
	name:"RungeKutta (classic)",
	m:4,
	p:4
};

int ODE_RK_Stepper_RungeKutta::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double k[dim];
	double ydelta[dim];
	double ytmp[dim];
	
	double h2=0.5*h;
	for(int d=0; d<dim; d++)
	{
		ydelta[d]=dy_dx[d];
		ytmp[d]=yn[d]+h2*dy_dx[d];
	}
	
	odefunc(xn+h2,ytmp,k);
	
	for(int d=0; d<dim; d++)
	{
		ydelta[d]+=2.0*k[d];
		ytmp[d]=yn[d]+h2*k[d];
	}
	
	odefunc(xn+h2,ytmp,k);
	
	for(int d=0; d<dim; d++)
	{
		ydelta[d]+=2.0*k[d];
		ytmp[d]=yn[d]+h*k[d];
	}
	
	odefunc(xn+h,ytmp,k);
	
	h2=h/6.0;
	for(int d=0; d<dim; d++)
		yn[d]+=h2*(ydelta[d]+k[d]);
	
	return(0);
}

//==============================================================================

int ODE_RK_Stepper_GeneralRungeKutta::Step(Function_ODE &odefunc,
	double xn,double h,double *yn,
	double *dy_dx)
{
	int dim=odefunc.Dim();
	double _k[sp.m-1][dim];
	double *k[sp.m];
	double ytmp[dim];
	
	k[0]=dy_dx;
	for(int i=1; i<sp.m; i++)
		k[i]=_k[i-1];
	
	// First evaluation: dy_dx already known. 
	
	// All the other evaluations: 
	for(int i=1; i<sp.m; i++)
	{
		for(int d=0; d<dim; d++)
		{
			double tmp=0.0;
			for(int s=0; s<i; s++)
				tmp+=beta(i,s)*k[s][d];
			ytmp[d]=yn[d]+h*tmp;
		}
		
		odefunc(xn+alpha[i]*h,ytmp,k[i]);
	}
	
	// Update values: 
	for(int d=0; d<dim; d++)
	{
		double delta=0.0;
		for(int s=0; s<sp.m; s++)
			delta+=gamma[s]*k[s][d];
		yn[d]+=h*delta;
	}
	
	return(0);
}


ODE_RK_Stepper_GeneralRungeKutta::ODE_RK_Stepper_GeneralRungeKutta() : 
	ODESolver_RK_Stepper()
{
	#if 1
	// Runge-Kutta analogon to the 3/8th integration rule: 
	static const int def_m=4,def_p=4;
	static const double def_gamma[def_m]={ 1./8., 3./8., 3./8., 1./8. };
	static const double def_alpha[def_m]={ 0.0, 1./3., 2./3., 1.0 };
	static const double def_beta[def_m*(def_m-1)/2]=
	{
		 1./3.,
		-1./3.,  1.0,
		 1.0,   -1.0,  1.0
	};
	#elif 0
	// [One of] the classical Runge-Kutta algorithms: 
	static const int def_m=4,def_p=4;
	static const double def_gamma[def_m]={ 1./6., 1./3., 1./3., 1./6. };
	static const double def_alpha[def_m]={ 0.0, 0.5, 0.5, 1.0 };
	static const double def_beta[def_m*(def_m-1)/2]=
	{
		0.5,
		0.0, 0.5,
		0.0, 0.0, 1.0
	};
	#elif 0
	// Heun: 
	static const int def_m=2,def_p=2;
	static const double def_gamma[def_m]={ 0.5,  0.5 };
	static const double def_alpha[def_m]={ 0., 1. };
	static const double def_beta[def_m*(def_m-1)/2]={ 1.0 };
	#elif 0
	// Euler: 
	static const int def_m=1,def_p=1;
	static const double def_gamma[def_m]={ 1.0 };
	static const double def_alpha[def_m]={ 0.0 };
	static const double def_beta[def_m*(def_m-1)/2]={};
	#endif
	
	// Set defaults: 
	sp.name="RungeKutta (general)";
	sp.m=def_m;
	sp.p=def_p;
	gamma=def_gamma;
	alpha=def_alpha;
	_beta=def_beta;
	
	//assert(beta(1,0)==0.5 && beta(2,0)==0.0 && beta(3,2)==1.0);
}

}  // end of namespace NUM
