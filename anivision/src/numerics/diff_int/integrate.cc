/*
 * numerics/diff_int/integrate.cc
 * 
 * Numerical integration method implementation. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "integrate.h"
#include <stdio.h>
#include <assert.h>

namespace NUM  // numerics
{

double Integrator_R_R::Integrate(Function_R_R &/*f*/,double /*a*/,double /*b*/)
{
	// Should be overridden. At least that's the purpose 
	// of the class...
	assert(0);
	return(0.0);
}


//------------------------------------------------------------------------------

double Integrator_R_R_Trapezoid::Integrate(Function_R_R &f,double a,double b)
{
	//iter_t N=(iter_t)ceil((b-a)/h);
	double r=0.5*(f(a)+f(b));
	double bma=b-a;
	for(iter_t i=1; i<nintervals; i++)
		r+=f(a+bma*i/nintervals);
	return(r*bma/nintervals);
}


//------------------------------------------------------------------------------

double Integrator_R_R_Simpson::Integrate(Function_R_R &f,double a,double b)
{
	//iter_t N=(iter_t)ceil((b-a)/h);
	//if(N%2) ++N;  // make n even
	if(nintervals%2) ++nintervals;  // make nintervals even
	double bma=b-a;
	double r=0.0;
	for(iter_t i=1; i<nintervals; i++)
	{
		r+=2.0*f(a+i*bma/nintervals);
		++i;
		r+=f(a+i*bma/nintervals);
	}
	r+=r;
	r+=f(a)+f(b);
	return(r*bma/(3*nintervals));
}


//------------------------------------------------------------------------------

double Integrator_R_R_TrapezoidRefining::Integrate(
	Function_R_R &f,double a,double b)
{
	double bma=b-a;
	double h=bma;
	double T=0.5*h*(f(a)+f(b));
	iter_t n=1;
	for(int k=1; !maxdepth || k<=maxdepth; k++,n+=n)
	{
		double a0=a+0.5*bma/n;
		double M=0.0;
		for(iter_t j=0; j<n; j++)  // n=2^k
			M+=f(a0+double(j)/n*bma);
		M*=h;
		T=0.5*(T+M);
		if(fabs(T-M)<epsilon*fabs(T) ||(T==0.0 && M==0.0))  break;
		h*=0.5;
	}
	return(T);
}


//------------------------------------------------------------------------------

// Integrate (sub-)interval from a to b with f(a)=fa and f(b)=fb 
// precomputed for speed increase. 
double Integrator_R_R_SimpsonAdaptive::_RecInt(
	double a,double b,double fa,double fb)
{
	double m=0.5*(a+b);
	double h=b-a;
	double I1=(fa+fb)*h*0.5;
	double fm=(*f)(m);
	double I2=(I1+fm*h*2.0)/3.0;
	
	const int min_n=3;   // Prevent too early termination. 
	
	if(depth>=min_n)
	{
		// It's not clear whether to use macheps1_prec or macheps1 here. 
		// On my AthlonXP using gcc-3.3, macheps1_prec shows the desired 
		// results and accuracy. 
		double IS=epsilon*I2/macheps1_prec;
		if(I1+IS==I2+IS || (maxdepth && depth>=maxdepth))
		{  return(I2);  }
	}
	
	// Need to recurse: 
	++depth;
	I1 = _RecInt(a,m,fa,fm) + _RecInt(m,b,fm,fb);
	--depth;
	
	return(I1);
}


double Integrator_R_R_SimpsonAdaptive::Integrate(
	Function_R_R &_f,double a,double b)
{
	f=&_f;
	depth=1;  // correct
	double I=_RecInt(a,b,(*f)(a),(*f)(b));
	depth=-1;
	return(I);
}


//------------------------------------------------------------------------------

double Integrator_R_R_NewtonCotes::Integrate(
	Function_R_R &f,double a,double b)
{
	double h=(b-a)/double(order*nintervals);
	if(order==2)
	{
		double I2=0.0,x2k;
		for(iter_t k=1; k<nintervals; k++)
		{
			x2k=a+double(k)*(b-a)/double(nintervals);
			I2+=f(x2k)+2.0*f(x2k+h);
		}
		I2=(I2*2.0 + f(a)+4.0*f(a+h)+f(b))*h/3.0;
		return(I2);
	}
	else if(order==4)
	{
		double I4=0.0,x4k;
		for(iter_t k=1; k<nintervals; k++)
		{
			x4k=a+double(k)*(b-a)/double(nintervals);
			I4+=14.0*f(x4k);
			x4k+=h;
			I4+=32.0*(f(x4k)+f(x4k+h+h))+12.0*f(x4k+h);
		}
		I4=(I4 + 7.0*(f(a)+f(b)) + 32.0*(f(a+h)+f(a+3.0*h)) + 
			12.0*f(a+h+h) )*h/22.5;
		return(I4);
	}
	else
	{  fprintf(stderr,"Integrator_R_R_NewtonCotes: Unsupported order=%d\n",
		order);  }
	return(0.0);
}


//------------------------------------------------------------------------------

double Integrator_R_R_GaussLegendre::Integrate(Function_R_R &f,
	double a,double b)
{
	#warning "Need more precise values for the weights[] and xvals[]."
	
	const double *weight,*xval;
	switch(order)
	{
		case 3:
		{
			const double _weight[3]=
				{ 0.277777777778, 0.444444444444, 0.277777777778 };
			const double _xval[3]=
				{ 0.112701665379, 0.5, 0.887298334621 };
			weight=_weight;
			xval=_xval;
		} break;
		case 5:
		{
			const double _weight[5]=
				{ 0.118463442528, 0.23931433525, 0.284444444444,
				  0.23931433525, 0.118463442528 };
			const double _xval[5]=
				{ 0.0469100770307, 0.230765344947, 0.5,
				  0.769234655053, 0.953089922969 };
			weight=_weight;
			xval=_xval;
		} break;
		case 10:
		{
			const double _weight[10]=
				{ 0.0333356721543, 0.0747256745753, 0.109543181258,
				  0.134633359655, 0.147762112357, 0.147762112357,
				  0.134633359655, 0.109543181258, 0.0747256745753,
				  0.0333356721543 };
			const double _xval[10]=
				{ 0.0130467357414, 0.0674683166555, 0.16029521585,
				  0.283302302935, 0.425562830509, 0.574437169491,
				  0.716697697065, 0.83970478415, 0.93253168334,
				  0.986953264259 };
			weight=_weight;
			xval=_xval;
		} break;
		default:
			fprintf(stderr,"Integrator_R_R_GaussLegendre: "
				"Unsupported order=%d\n",order);
			return(0.0);
	}
	
	// Now,. let's come to the actual integration: 
	
	double I=0.0;
	double bma=(b-a)/nintervals;
	for(iter_t i=0; i<nintervals; i++)
	{
		double a0=a+i*(b-a)/nintervals;
		for(int w=0; w<order; w++)
			I+=weight[w]*f(xval[w]*bma+a0);
	}	
	
	return(I*bma);
}


//------------------------------------------------------------------------------

double Integrator_R_R_GaussCebyshev::Integrate(Function_R_R &f,
	double a,double b)
{
	#warning "Need more precise values for the weights[] and xvals[]."
	
	const double *weight,*xval;
	switch(order)
	{
		case 3:
		{
			const double _weight[3]=
				{ 0.261799387799, 0.523598775598, 0.261799387799 };
			const double _xval[3]=
				{ 0.933012701892, 0.5, 0.0669872981078 };
			weight=_weight;
			xval=_xval;
		} break;
		case 5:
		{
			const double _weight[5]=
				{ 0.0970805519363, 0.254160184616, 0.314159265359,
				  0.254160184616, 0.0970805519363 };
			const double _xval[5]=
				{ 0.975528258148, 0.793892626146, 0.5,
				  0.206107373854, 0.0244717418524 };
			weight=_weight;
			xval=_xval;
		} break;
		case 10:
		{
			const double _weight[10]=
				{ 0.0245726683069, 0.0713126609391, 0.111072073454,
				  0.139958977535, 0.155145721742, 0.155145721742,
				  0.139958977535, 0.111072073454, 0.0713126609391,
				  0.0245726683069 };
			const double _xval[10]=
				{ 0.993844170298, 0.945503262094, 0.853553390593,
				  0.72699524987, 0.57821723252, 0.42178276748,
				  0.27300475013, 0.146446609407, 0.0544967379058,
				  0.00615582970243 };
			weight=_weight;
			xval=_xval;
		} break;
		default:
			fprintf(stderr,"Integrator_R_R_GaussCebyshev: "
				"Unsupported order=%d\n",order);
			return(0.0);
	}
	
	// Now,. let's come to the actual integration: 
	assert(0);  // <-- Algorithm seems to be FUBAR. Maybe the weights are incorrect. 
	
	double I=0.0;
	double bma=(b-a)/nintervals;
	for(iter_t i=0; i<nintervals; i++)
	{
		double a0=a+i*(b-a)/nintervals;
		for(int w=0; w<order; w++)
			I+=weight[w]*f(xval[w]*bma+a0);
	}	
	
	return(I*bma);
}


//------------------------------------------------------------------------------

double Integrator_R_R_Romberg::Integrate(Function_R_R &f,double a,double b)
{
	const int min_n=3;   // Prevent too early termination. 3 -> min 2^3+1=9 evals
	const int max_n=15;  // Otherwise overflow in calculation of "pw". 
	
	// NOTE: This algorithm works absolutely correct. 
	//       It has been tested by me using a second extrapolator 
	//       and using an example (namely 1/x: over 1..2) found 
	//       in H.R. Schwarz, "Numerische Mathematik". 
	
	if(maxdepth>max_n) maxdepth=max_n;
	
	// This function has a Neville extrapolator built-in for 
	// efficiency (can be optimized due to the fact that the 
	// step sizes are halfed each time). The alternative using 
	// the class Extrapolator_R_0_Poly is commented out using 
	// the "//X:" comments. 
	
	double bma=b-a,h=bma;
	double r[maxdepth+1];
	r[0]=0.5*h*(f(a)+f(b));
	//X: Extrapolator_R_0_Poly nve(maxdepth+1);
	//X: double rj=0.5*h*(f(a)+f(b));
	//X: nve.FirstPoint(h*h,rj);
	
	int j=0;
	for(int m=1; j<maxdepth; m+=m)
	{
		// Refinement: 
		h*=0.5;
		++j;
		
		// Trapezoid (triangle) integration: 
		double a0=a+h;
		double sum=0.0;
		for(int k=0; k<m; k++)
			sum+=f(a0+double(k)/m*bma);
		
		//X: double error=rj;
		//X: rj=nve.AddPoint(h*h,h*sum+0.5*r[0]);
		//X: <substitute rj for r[j]; error is r[j-1] as used below, too. 
		
		// r[j-1] is the previous estimate (extrapolation) for the 
		// value of the integral. 
		double error=r[j-1];   // <-- This is not yet the error but see below. 
		// Neville interpolation. 
		// Only works for successive halving of step size h (hence 
		// the value 2^2 = 4 in the pw calculation). [optimized]
		// Update the integral result using just as in the trapezoid 
		// refinement algorithm. Here, h was already halfed, so it is 
		//   new_int = h*sum + 0.5*old_int
		// rather than 
		//   new_int = 0.5*(h*sum + old_int)
		// Note that r[0] is the integral value ("h*sum") from the last 
		// iteration. There are some tricky temporary variables (pr, tmp) 
		// so that the complete updated extrapolation can be done with a 
		// single array of size [maxdepth]. 
		double pr=h*sum + 0.5*r[0];
		for(int k=0,pw=4; k<j; k++,pw*=4)
		{
			double tmp=pr-r[k];
			r[k]=pr;
			pr=r[k]+tmp/(pw-1);
		}
		r[j]=pr;
		
		if(j>=min_n)
		{
			// Estimete error by using the difference between the 
			// previous extrapolation and this one. 
			error-=r[j];
			if(fabs(error)<=epsilon*fabs(r[j]))  break;
		}
	}
	
	return(r[j]);
}

}  // end of namespace NUM
