/*
 * numerics/interpolate/cebyshev.cc
 * 
 * Cebyshev interpolation functions. 
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

#include "cebyshev.h"


// NOTE: The Chebyshev polynomials are: 
// T0(x) = 1
// T1(x) = x
// T2(x) = 2*x^2 -1
// T3(x) = 4*x^3 -3*x
// T4(x) = 8*x^4 -8*x^2 +1
// ...
// The coeffs of these polys can be stored as 32 bit integer up to 
// degree 26 and as 64 bit integer up to degree 52. 
// The coeffs of these polys can be stored as 32 bit integer up to 
// degree 26 and as 64 bit integer up to degree 52. 
// (Especially note that the first coefficient is not always the 
// largest one.) 


namespace NUM  // numerics
{

void Interpolator_Chebyshev::_Reset(int new_n)
{
	coeff=FREE(coeff);
	if(new_n!=n)
	{
		gamma=FREE(gamma);
		if(new_n!=-1)
		{  gamma=ALLOC<double>(new_n+1);  }
		n=new_n;
	}
}

int Interpolator_Chebyshev::Interpolate(Function_R_R &f,
	double _a,double _b,int _n)
{
	if(_n<1) return(-2);
	
	_Reset(_n);
	a=_a;
	b=_b;
	
	// Temporarily store function values here: 
	double func[n+1];
	
	// This works as follows: we first evaluate the function 
	// n+1 times and store the results in func[]; lateron we 
	// calculate gamma[] (the actual coefficients) from func[]. 
	// This is to avoid unecessary repetitions of function 
	// evaluations. 
	double apb2=0.5*(a+b);
	double bma2=0.5*(b-a);
	//double tmp=M_PI/(n+n+2);
	//for(int i=0; i<=n; i++)
	//{  func[i]=f( cos(double(i+i+1)*tmp) * bma2 + apb2 );  }
	for(int i=0; i<=n; i++)
	{  func[i]=f( cos(M_PI*double(i+i+1)/double(n+n+2)) * bma2 + apb2 );  }	
	
	// Actually calc the gamma values: 
	double tmp2=2.0/(n+1);
	for(int k=0; k<=n; k++)
	{
		double g=0.0;
		//for(int i=0; i<=n; i++)
		//{  g+=func[i]*cos( double(k*(i+i+1))*tmp );  }
		for(int i=0; i<=n; i++)
		{  g+=func[i]*cos( M_PI*double(k*(i+i+1))/double(n+n+2) );  }
		gamma[k]=g*tmp2;
	}
	
	return(0);
}


double Interpolator_Chebyshev::Eval(double xx)
{
	if(n<1) return(NAN);
	
	// This implements the "Clamshaw recursion" which is 
	// numerically stable. 
	double x=(xx+xx-a-b)/(b-a);
	double dk2=0.0,dk1=gamma[n];
	double y=x+x;
	for(int k=n-1; k>0; k--)
	{
		double tmp=dk1;
		dk1=y*dk1-dk2+gamma[k];
		dk2=tmp;
	}
	// The iteration for k=0 is extracted to this point: 
	return(x*dk1-dk2+0.5*gamma[0]);
	
	//-- Previous algorithm using an array: 
	// double x=(xx+xx-a-b)/(b-a);
	// double d[n+1];
	// d[n]=gamma[n];
	// double y=x+x;
	// d[n-1]=gamma[n-1]+y*d[n];  // (d[n]=gamma[n] here)
	// for(int k=n-2; k>=0; k--)
	// {  d[k]=gamma[k]+y*d[k+1]-d[k+2];  }
	// return(0.5*(d[0]-d[2]));
}


int Interpolator_Chebyshev::Derive(const Interpolator_Chebyshev &src)
{
	if(src.n<2)  return(-2);
	
	int sn=src.n;
	_Reset(sn-1);
	a=src.a;
	b=src.b;
	
	// Due to test above, n>=1 here. 
	// Derivation algorithm based on gsl_cheb_calc_deriv from GSL. 
	// Error in derivation is at least 2*(src.n+1)*src.gamma[src.n]. 
	gamma[n]=(sn+sn)*src.gamma[sn];
	gamma[n-1]=(n+n)*src.gamma[n];
	for(int i=n-1; i>0; i--)
	{  gamma[i-1] = gamma[i+1] + (i+i)*src.gamma[i];  }
	
	// "Nachdifferenzieren": 
	double fact=2.0/(b-a);
	for(int i=0; i<=n; i++)
	{  gamma[i]*=fact;  }
	
	return(0);
}


int Interpolator_Chebyshev::PolyCalcCoeffs()
{
	if(n<1) return(-2);
	
	FREE(coeff);
	coeff=ALLOC<double>(n+1);
	
	coeff[0]=0.5*gamma[0];
	coeff[1]=gamma[1];
	for(int i=2; i<=n; i++) coeff[i]=0.0;
	
	// State array for Chebyshev polynomial coefficient calculation. 
	double tp_state[n+1];
	tp_state[0]=1;
	tp_state[1]=1;
	for(int i=2; i<=n; i++) tp_state[i]=0.0;
	
	for(int k=2; k<=n; k++)
	{
		// Calculate coefficient for Chebyshev polynomial of degree k: 
		// Tricky Wieser-algorithm based on recursive T-poly def. It only 
		// needs one state array because every second coeff is 0 and 
		// hence can be used to store the coeffs of the previous order. 
		for(int i=k; i>0; i-=2)
		{  tp_state[i]=2*tp_state[i-1]-tp_state[i];  }
		tp_state[0]=(k/2)%2 ? -1 : 1;
		
		// Sum up poly coeffs: 
		// (Every second coeff is 0 so we can leave that out. 
		// But NOTE that tp_state is NOT 0 at these positions but saves 
		// the coeffs from the previous order instead, see above.) 
		double g=gamma[k];
		for(int i=k; i>=0; i-=2)
		{  coeff[i]+=g*tp_state[i];  }
	}
	
	return(0);
}


double Interpolator_Chebyshev::PolyEval(double xx)
{
	if(!coeff)  return(NAN);
	
	double x=(xx+xx-a-b)/(b-a);
	double r=coeff[n];
	for(int i=n-1; i>=0; i--)
	{  r=coeff[i]+x*r;  }
	return(r);
}

double Interpolator_Chebyshev::PolyEvalD(double xx)
{
	if(!coeff)  return(NAN);
	
	double x=(xx+xx-a-b)/(b-a);
	double r=n*coeff[n];
	for(int i=n-1; i>0; i--)
	{  r=i*coeff[i]+x*r;  }
	return( (r+r)/(b-a) );  // "Nachdifferenzieren"
}

double Interpolator_Chebyshev::PolyEvalDD(double xx)
{
	if(!coeff)  return(NAN);
	
	double x=(xx+xx-a-b)/(b-a);
	double r=(n*(n-1))*coeff[n];
	for(int i=n-1; i>1; i--)
	{  r=((i-1)*i)*coeff[i]+x*r;  }
	double fact=2.0/(b-a);
	return( r * fact * fact );
}


Interpolator_Chebyshev::Interpolator_Chebyshev()
{
	n=0;
	//a=b=0.0;
	gamma=NULL;
	coeff=NULL;
}

Interpolator_Chebyshev::~Interpolator_Chebyshev()
{
	_Reset(-1);
}

}  // end of namespace NUM
