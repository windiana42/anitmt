/*
 * numerics/ode/odesolver.cc
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

#include "odesolver.h"
#include <stdio.h>
#include <assert.h>


namespace NUM  // numerics
{

void ODESolver::_DriverInit()
{
	assert(drv && _odefunc);
	
	// First, compute current dy_dx: 
	odefunc(xn,yn,dy_dx);
	
	int rv=drv->Init(this);
	assert(rv==0);
}


ODESolver::StepState ODESolver::Step(double *h_max)
{
	if(!drv) return(SS_NoDriver);
	StepState rv=drv->Step(this,h_max);
	return(rv);
}


int ODESolver::SetSolver(ODESolverDriver *_drv)
{
	DELETE(drv);
	drv=_drv;
	
	// If a function is already set, initialize: 
	if(_odefunc && drv)
	{  _DriverInit();  }
	
	return(0);
}


int ODESolver::Init(Function_ODE &odefunc,
	double x0,const double *y0,double h0,double _epsilon,
	bool need_yscale)
{
	// This will do nothing if the dimensions match: 
	_ParamRealloc(odefunc.Dim(),need_yscale);
	
	// Internally store pointer to the function: 
	_odefunc=&odefunc;
	
	xn=x0;
	for(int d=0; d<dim; d++)  yn[d]=y0[d];
	h=h0;
	epsilon=_epsilon;
	
	// Compute dy_dx. yscale (if needed) is computed by lower level. 
	if(drv)
	{  _DriverInit();  }
	
	return(0);
}


void ODESolver::_ParamRealloc(int newdim,bool need_yscale)
{
	if(dim==newdim)
	{
		// Fast path. 
		if(!yscale && need_yscale)
		{  yscale=ALLOC<double>(dim);  }
		else if(yscale && !need_yscale)
		{  yscale=FREE(yscale);  }
		
		return;
	}
	
	dim=newdim;
	
	yn=FREE(yn);
	dy_dx=FREE(dy_dx);
	yscale=FREE(yscale);
	
	if(!dim)  return;
	
	yn=ALLOC<double>(dim);
	dy_dx=ALLOC<double>(dim);
	if(need_yscale)
	{  yscale=ALLOC<double>(dim);  }
}


ODESolver::ODESolver()
{
	drv=NULL;
	dim=0;
	_odefunc=NULL;
	
	xn=0.0;
	yn=NULL;
	dy_dx=NULL;
	h=1e-3;
	h_used=0.0;
	nsteps=0;
	epsilon=1e-5;
	yscale=NULL;
	
	h_min=0.0;
	h_max=1e30;
	
	used_h_min=1e100;
	used_h_max=-1.0;
}


ODESolver::~ODESolver()
{
	DELETE(drv);
	
	_ParamRealloc(0,0);
	
	_odefunc=NULL;   // we will not delete this
}


//------------------------------------------------------------------------------

#define TINY 1.0e-30   // Prevents division by zero.

void ODESolverDriver::DefaultYScale(int dim,double /*xn*/,const double *yn,
	const double *dy_dx,double h,double *yscale)
{
	// Calculate y scale for error estimation. 
	// This is a general purpose formula suggested in 
	// "Numerical Recipes in C" by W.H. Press, S.A. Teukolsky, 
	// W.T. Vetterling, B.P. Flannery. 
	for(int i=0; i<dim; i++)
		yscale[i] = fabs(yn[i])+fabs(dy_dx[i]*h)+TINY;
}

#undef TINY


ODESolver::StepState ODESolverDriver::_CheckUpperLimit_H(ODESolver *ode,
	double *h_max)
{
	StepState rv=SS_OK;
	double ahmax=fabs(ode->h_max);
	StepState which=SS_HMaxLim;
	if(h_max)
	{
		double tmp=fabs(*h_max);
		if(ahmax>=tmp)  // Leave >= here. 
		{  ahmax=tmp;  which=SS_ShortStep;  }
	}
	if(ode->h>=0.0)
	{
		if(ode->h>ahmax)
		{  ode->h=ahmax;  rv=which;  }
	}
	else
	{
		if(ode->h<-ahmax)
		{  ode->h=-ahmax;  rv=which;  }
	}
	return(rv);
}


RefString ODESolverDriver::DrvName()
{
	RefString tmp;
	tmp.set("[none]");
	return(tmp);
}


int ODESolverDriver::Init(ODESolver * /*ode*/)
{
	return(0);
}


ODESolver::StepState ODESolverDriver::Step(ODESolver * /*ode*/,double * /*h_max*/)
{
	return(SS_NoDriver);
}


ODESolverDriver::ODESolverDriver()
{
}

ODESolverDriver::~ODESolverDriver()
{
}

}  // end of namespace NUM
