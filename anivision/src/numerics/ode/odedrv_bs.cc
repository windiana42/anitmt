/*
 * numerics/ode/odedrv_bs.cc
 * 
 * Numerical ODE (ordinary differential equation) solver driver 
 * for the Bulirsch-Stoer algoritm. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * Note also comments in file. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "odedrv_bs.h"
#include <stdio.h>
#include <assert.h>

#include <numerics/interpolate/extr0epoly.h>
#include <numerics/interpolate/extr0erat.h>
#include <numerics/linalg/linalg.h>


namespace NUM  // numerics
{

// Implements modified midpoint ODE solver step. 
// yn,dy_dx,yres are vectors of dimension odefunc.Dim(). 
// Takes current y values in yn[] and their derivations in dy_dx[] 
// and computes the step xn -> xn+htot using nsteps sub-steps. 
// Result is stored in yres[]. 
// (Note that it is valid to use yres=yn to get in-place updates to yn.) 
void ODESolver_BulirschStoer::ModifiedMidpoint(Function_ODE &odefunc,
	double xn,double htot,int nsteps,
	const double *yn,const double *dy_dx,double *yres)
{
	const int dim=odefunc.Dim();
	double yp[dim];  // "previous" yn values
	double yc[dim];  // "current" yn values
	
	double h=htot/nsteps;
	// Perfrom first step: 
	for(int d=0; d<dim; d++)
	{
		yp[d]=yn[d];
		yc[d]=yn[d]+h*dy_dx[d];
	}
	odefunc(xn+h,yc,yres);
	
	double h2=h+h;
	// Perform all successive steps: 
	for(int i=2; i<=nsteps; i++)
	{
		for(int d=0; d<dim; d++)
		{
			double tmp=h2*yres[d]+yp[d];
			yp[d]=yc[d];
			yc[d]=tmp;
		}
		odefunc(xn+i*h,yc,yres);
	}
	
	// Store result. 
	for(int d=0; d<dim; d++)
		yres[d] = 0.5*( yp[d] + yc[d] + h*yres[d] );
}


// Implements semi-implicit midpoint rule for ODE sovler step. 
// yn,dy_dx,yres are vectors of dimension odefunc.Dim(). 
// Takes current y values in yn[] and their derivations in dy_dx[] 
// and the Jacobi matrix df_dy (at xn) as well as the derivations df/dx. 
// Function computes the step xn -> xn+htot using nsteps sub-steps. 
// Result is stored in yres[]. 
void ODESolver_BulirschStoer::SIMidpoint(Function_ODE &odefunc,
	double xn,double htot,int nsteps,
	const double *yn,
	const double *dy_dx,const double *df_dx,const SMatrix<double> &df_dy,
	double *yres)
{
	int dim=odefunc.Dim();
	
	double diff[dim];
	double ytmp[dim];
	
	double h=htot/nsteps;
	
	// Initialize Matrix: 
	SMatrix<double> a(dim,dim);
	for(int i=0; i<dim; i++)
	{
		for(int j=0; j<dim; j++)
			a[i][j] = -h*df_dy[i][j];
		a[i][i]+=1.0;
	}
	
	// Now, LU decompose the matrix: 
	int p[dim];
	int rv=LUDecompose(a,p,NULL);
	assert(rv==0);
	
	// We will use yres[] to temporarily store values several times here. 
	for(int i=0; i<dim; i++)
		yres[i] = h * (h*df_dx[i] + dy_dx[i]);
	LUFwBackSubst(a,p,yres);
	
	// Perform first step: 
	for(int i=0; i<dim; i++)
	{
		diff[i]=yres[i];
		ytmp[i]=diff[i]+yn[i];
	}
	odefunc(xn+h,ytmp,yres);
	
	// Perform most successive steps: 
	for(int stp=1; stp<nsteps; stp++)
	{
		for(int i=0; i<dim; i++)
			yres[i] = h*yres[i] - diff[i];
		LUFwBackSubst(a,p,yres);
		
		for(int i=0; i<dim; i++)
		{
			diff[i]+=2.0*yres[i];
			ytmp[i]+=diff[i];
		}
		
		odefunc(xn+stp*h,ytmp,yres);
	}
	
	// Last step: 
	for(int i=0; i<dim; i++)
		yres[i] = h*yres[i]-diff[i];
	LUFwBackSubst(a,p,yres);
	for(int i=0; i<dim; i++)
		yres[i] += ytmp[i];
}

//------------------------------------------------------------------------------

// This is a combined step computation algorithm for the Bulirsch-Stoer- 
// algorithm. 
// The code below is heavily based upon the routines bsstep() and stifbs() 
// from "Numerical Recipes in C" by W.H. Press, S.A. Teukolsky, 
// W.T. Vetterling, B.P. Flannery. 
// Several modifications in the implementation were made but the 
// numerical calculations/calc results should be the same. 

//#define KMAXX 10 // WAS: 8   // Maximum row number used in the extrapolation.
//#define IMAXX (KMAXX+1)
#define SAFE1 0.25   // Safety factors.
#define SAFE2 0.7
#define REDMAX 1.0e-5  // Maximum factor for stepsize reduction.
#define REDMIN 0.7     // Minimum factor for stepsize reduction.
#define TINY 1.0e-30   // Prevents division by zero.
#define SCALMX 0.1     // 1/SCALMX is the maximum factor by which a
                       // stepsize can be increased.

// yscal may be NULL. 
ODESolver_BulirschStoer::StepState ODESolver_BulirschStoer::ComputeStep(
	Function_ODE &odefunc,ODESolver *ode,
	double *yscal,double *hdid,double *hnext)
{
	int dim=odefunc.Dim();
	
	double *y=ode->yn;
	double *dydx=ode->dy_dx;
	double *xx=&ode->xn;  // gets updated
	double htry=ode->h;
	double eps=ode->epsilon;
	
	// In case tolerance changed, we need to do some init again: 
	double eps1;
	if(eps != epsold || (_IsSemiImplicit() && dim!=nvold))
	{
		*hnext = xnew = -1.0e29; // "Impossible" values.
		eps1=SAFE1*eps;
		a[0]=nseq[0]+1;   // Compute work coeffcients Ak.
		for(int k=1; k<=KMAXX; k++)
			a[k]=a[k-1]+nseq[k];
		
		for(int iq=1; iq<KMAXX; iq++)  for(int k=0; k<iq; k++)
			alf[k][iq]=pow(eps1,
				(a[k+1]-a[iq+1]) / ((a[iq+1]-a[0]+1.0)*(k+k+3)));
		
		epsold=eps;
		if(_IsSemiImplicit())
		{
			nvold=dim;
			// Add cost of Jacobian evaluations to work coeffcients. 
			a[0]+=dim;
			for(int k=1; k<=KMAXX; k++)
				a[k]=a[k-1]+nseq[k];
		}
		
		// Determine optimal row number for convergence. 
		for(kopt=1; kopt<KMAXX-1; kopt++)
			if(a[kopt+1] > a[kopt]*alf[kopt-1][kopt]) break;
		kmax=kopt;
	}
	
	double err[KMAXX];
	double yerr[dim];
	double ysav[dim];
	double yseq[dim];
	for(int i=0; i<dim; i++)
		ysav[i]=y[i];   // Save the starting values.
	
	double h=htry;
	
	// These 2 variables are only needed for the semi-implicit method. 
	// Could use conditional alloca() for the second one. 
	SMatrix<double> &dfdy=*_dfdy;
	double dfdx[dim];
	if(_IsSemiImplicit())
	{  odefunc.Jacobian(*xx,y,dfdy,dfdx);  }
	
	if(*xx!=xnew || h!=(*hnext))
	{  // A new stepsize or a new integration: re-establish the order window. 
		first=1;
		kopt=kmax;
	}
	
	int k,km;
	double red,scale,work,wrkmin,xest;
	bool reduct=0;
	for(;;)
	{
		for(k=0; k<=kmax; k++)  // Evaluate the sequence of modified midpoint integrations. 
		{
			xnew=(*xx)+h;
			if(xnew == (*xx))
				return(SS_HUnderflow);
			
			if(_IsSemiImplicit())
			{  SIMidpoint(
				odefunc,*xx,h,nseq[k],ysav,dydx,dfdx,dfdy,yseq);  }
			else
			{  ModifiedMidpoint(
				odefunc,*xx,h,nseq[k],ysav,dydx,yseq);  }
			++ode->nsteps;
			
			xest=SQR(h/nseq[k]);  // Squared, since error series is even.
			
			if(!k)
			{
				xpol->FirstPoint(xest,yseq,y,yerr);
				continue;
			}
			
			// Actually do the extrapolation: 
			xpol->AddPoint(xest,yseq,y,yerr);
			
			// Compute normalized error estimate epsilon(k). 
			double errmax=TINY;
			if(yscal) for(int i=0; i<dim; i++)
			{
				double tmp=fabs(yerr[i]/yscal[i]);
				if(errmax<tmp)  errmax=tmp;
			}
			else for(int i=0; i<dim; i++)
			{
				double tmp=fabs(yerr[i]);
				if(errmax<tmp)  errmax=tmp;
			}
			errmax/=eps;   // Scale error relative to tolerance.
			km=k-1;
			err[km]=pow(errmax/SAFE1,1.0/(2*k+1));
			
			if(k >= kopt-1 || first)
			{  // In order window.
				if(errmax < 1.0)  goto converged;
				
				// Check for possible stepsize reduction. 
				if(k == kmax || k == kopt+1)
				{
					red=SAFE2/err[km];
					break;
				}
				if(k == kopt && alf[kopt-1][kopt] < err[km])
				{
					red=1.0/err[km];
					break;
				}
				if(kopt == kmax && alf[km][kmax-1] < err[km])
				{
					red=alf[km][kmax-1]*SAFE2/err[km];
					break;
				}
				if(alf[km][kopt] < err[km])
				{
					red=alf[km][kopt-1]/err[km];
					break;
				}
			}
		}
		
		// Reduce stepsize by at least REDMIN and at most REDMAX. 
		     if(red>REDMIN)  red=REDMIN;
		else if(red<REDMAX)  red=REDMAX;
		h *= red;
		reduct=1;
	}   // Try again.
	converged:;
	
	*xx=xnew;   // Successful step taken.
	*hdid=h;
	first=0;
	wrkmin=1.0e35;   // Compute optimal row for convergence and corresponding stepsize. 
	
	for(int kk=0; kk<=km; kk++)
	{
		double fact=MAX(err[kk],SCALMX);
		work=fact*a[kk+1];
		if(work < wrkmin)
		{
			scale=fact;
			wrkmin=work;
			kopt=kk+1;
		}
	}
	
	*hnext=h/scale;
	
	if(kopt>=k && kopt!=kmax && !reduct)
	{
		// Check for possible order increase, but not if stepsize was just reduced.
		double fact=MAX(scale/alf[kopt-1][kopt],SCALMX);
		if(a[kopt+1]*fact <= wrkmin)
		{
			*hnext=h/fact;
			++kopt;
		}
	}
	
	return(SS_OK);
}

//------------------------------------------------------------------------------


int ODESolver_BulirschStoer::Init(ODESolver *ode)
{
	// Do not touch: 
	first=1;
	//kmax=-1;
	//kopt=-1;
	nvold=-1;
	epsold=-1.0;
	//xnew
	
	FREE(a);
	a=ALLOC<double>(IMAXX+1);
	
	if(alf) for(int i=0; i<KMAXX+1; i++)
	{  FREE(alf[i]);  }
	FREE(alf);
	alf=ALLOC<double*>(KMAXX+1);
	for(int i=0; i<KMAXX+1; i++)
	{  alf[i]=ALLOC<double>(KMAXX+1);  }
	
	// Calculate seqence: 
	FREE((int*)nseq);
	int *nseq=ALLOC<int>(IMAXX);
	if(_IsSemiImplicit())
	{
		// Each value differs from the previous one by the smallest 
		// multiple of 4 which makes the ratio of successive terms <= 5/7. 
		// 2,6,10,14,22,34,50,70,98,138,194,274,386,542,...
		nseq[0]=2;
		for(int i=1; i<IMAXX; i++)
			nseq[i]=nseq[i-1]+((nseq[i-1]-1)/10+1)*4;
	}
	else
	{
		if((flags & S_Deuflhard))
		{
			// Deuflhard: 2,4,6,8,... every even number. 
			// [Works better with rational extrapolation here.]
			for(int i=0; i<IMAXX; i++)
				nseq[i]=2*(i+1);
		}
		else
		{
			// Burlisch & Stoer: 2,4,6, then: n_i = 2*n_{i-2}
			// 2,4,6,8,12,16,24,32,48,64,96,128,192,256,384,512,768,1024,...
			do {
				nseq[0]=2;  if(IMAXX<2)  break;
				nseq[1]=4;  if(IMAXX<3)  break;
				nseq[2]=6;
				for(int i=3; i<IMAXX; i++)
					nseq[i]=2*nseq[i-2];
			} while(0);
		}
	}
	this->nseq=nseq;
	
	// Set up extrapolator: 
	DELETE(xpol);
	if((flags & E_Rational))
	{  xpol=new Extrapolator_0E_Rational(KMAXX,ode->Dim());  }
	else
	{  xpol=new Extrapolator_0E_Poly(KMAXX,ode->Dim());  }
	
	DELETE(_dfdy);
	if(_IsSemiImplicit())
	{  _dfdy=new SMatrix<double>(ode->Dim(),ode->Dim());  }
	
	return(0);
}


ODESolver::StepState ODESolver_BulirschStoer::Step(
	ODESolver *ode,double *h_max)
{
	// Make sure we initialized...
	if(!xpol)
	{  Init(ode);  }
	
	const int dim=ode->Dim();
	
	// Check step size against upper limit. 
	StepState rv=_CheckUpperLimit_H(ode,h_max);
	
	// FIXME: h_min limit against underflow is not supported by this solver. 
	//        Needs to be implemented inside ComputeStep() if needed. 
	assert(ode->h_min<=1e-30);
	
	// Calculate y scale for error estimation. 
	if(ode->yscale)
	{  ode->GetODEFunc().CalcYScale(ode->xn,ode->yn,ode->dy_dx,
		ode->h,ode->yscale);  }
	
	// This will compute a Burlisch-Stoer step: 
	StepState tmp=ComputeStep(ode->GetODEFunc(),ode,
		ode->yscale,&ode->h_used,&ode->h);
	if(tmp)  rv=tmp;
	
	if(ode->used_h_min>ode->h_used)  ode->used_h_min=ode->h_used;
	if(ode->used_h_max<ode->h_used)  ode->used_h_max=ode->h_used;
	
	// Evaluate ODE to update dy_dx: 
	ode->GetODEFunc()(ode->xn,ode->yn,ode->dy_dx);
	
	return(rv);
}


RefString ODESolver_BulirschStoer::DrvName()
{
	RefString tmp;
	tmp.sprintf(0,"BulirschStoer (%s,%s)",
		_IsSemiImplicit() ? "semi-implicit" : "explicit",
		(flags & E_Rational) ? "rational" : "polynomial");
	return(tmp);
}


ODESolver_BulirschStoer::ODESolver_BulirschStoer(int _flags,int _k_max)
{
	flags=_flags;
	
	KMAXX=_k_max<0 ? (_IsSemiImplicit() ? 7 : 8) : _k_max;
	IMAXX=KMAXX+1;
	
	nseq=NULL;
	
	a=NULL;
	alf=NULL;
	
	_dfdy=NULL;
	xpol=NULL;
}

ODESolver_BulirschStoer::~ODESolver_BulirschStoer()
{
	a=FREE(a);
	if(alf) for(int i=0; i<KMAXX+1; i++)
	{  FREE(alf[i]);  }
	alf=FREE(alf);
	
	nseq=FREE((int*)nseq);
	
	DELETE(_dfdy);
	DELETE(xpol);
}

}  // end of namespace NUM
