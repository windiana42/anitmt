/*
 * numerics/spline/cspline.cc
 * 
 * Numerics library cubic poly spline. 
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

#include "cspline.h"
#include <linalg/linalg.h>


namespace NUM
{

const SplineBase::Properties *CubicPolySpline::SplineProperties() const
{
	static const Properties prop=
	{
		splinetype: ST_CSpline,
		name: "cspline",
		cont_order: 2
	};
	return(&prop);
}


void CubicPolySpline::Clear()
{
	p=FREE(p);
	SplineBase::Clear();
}


int CubicPolySpline::Eval(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the spline: 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	for(int j=0; j<dim; j++)
	{
		result[j] = t/6.0*( 
			(pv(idx+1)[j]-pv(idx)[j])*t/h*t - 
			  (pv(idx+1)[j]+2.0*pv(idx)[j])*h ) + 
			( 0.5*pv(idx)[j]*t + (xv(idx+1)[j]-xv(idx)[j])/h )*t + 
			xv(idx)[j] ;
	}
	
	return(retval);
}

int CubicPolySpline::EvalD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the spline: 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	for(int j=0; j<dim; j++)
	{
		result[j] = 0.5*(pv(idx+1)[j]-pv(idx)[j])*t/h*t + pv(idx)[j]*t - 
			(pv(idx+1)[j]+2.0*pv(idx)[j])*h/6 + 
			(xv(idx+1)[j]-xv(idx)[j])/h;
	}
	
	return(retval);
}

int CubicPolySpline::EvalDD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the spline: 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	for(int j=0; j<dim; j++)
	{  result[j] = (pv(idx+1)[j]-pv(idx)[j])*t/h + pv(idx)[j];  }
	
	return(retval);
}


int CubicPolySpline::Create(
	const VectorArray<double> &cpoints,const double *tvals,
	SplineBoundaryCondition sbc,
	const double *paramA,const double *paramB)
{
	if(cpoints.NVectors()<2 || cpoints.Dim()<(tvals==TVALS_CPTS ? 2 : 1))
	{  return(-2);  }
	
	bool is_ok=0;
	static const int x=(SBC_SlopeA   |SBC_SlopeB);
	switch(sbc)
	{
		case      SBC_Natural:                 is_ok = 1;                break;
		case      SBC_Hermite:                 is_ok = paramA && paramB; break;
		case (int)SBC_SlopeA   |SBC_MomentumA: is_ok = paramA;           break;
		case (int)SBC_SlopeB   |SBC_MomentumB: is_ok = paramA;           break;
		case (int)SBC_SlopeA   |SBC_MomentumB: is_ok = paramA;           break;
		case (int)SBC_MomentumA|SBC_SlopeB:    is_ok =           paramB; break;
		case      SBC_Periodic:                is_ok = 1;                break;
		//default: is_ok stays 0. 
	}
	if(!is_ok)
	{  return(-3);  }
	
	// Check t values if specified: 
	if(tvals && tvals!=TVALS_DIST && 
		( (tvals==TVALS_CPTS) ? 
			_TestAscentingOrder_LastCompo(cpoints) : 
			_TestAscentingOrder(tvals,cpoints.NVectors()) ) )
	{  return(-4);  }
	
	// Get rid of old stuff...
	// ...and copy the control points and t values, if present: 
	_CopyPointsAndTVals(cpoints,tvals);  // Calls Clear(). 
	
	// Finally, allocate the momenta before calculating them: 
	p=ALLOC<double>(dim*(n+1));
	
	// Okay, now we're ready to actually compute the spline's momenta: 
	double *b=ALLOC<double>(n+1);
	double *diag=ALLOC<double>(n+1);
	
	// Linear system start/end coos; the actual system has 
	// the size (lsi1-lsi0+1)^2. 
	int lsi0,lsin;
	
	if(sbc==(SBC_SlopeA|SBC_MomentumA))
	{
		// This is easy; no need to solve a linear system, we simply 
		// compute the momenta from the beginning to the end. 
		for(int j=0; j<dim; j++)
		{
			double s=paramA[j];   // slope
			double m=paramB[j];   // momentum
			pv(0)[j]=m;
			for(int i=0; i<n; i++)
			{
				double h = tv ? (tv[i+1]-tv[i]) : 1.0;
				double tmp = (xv(i+1)[j]-xv(i)[j])/h;
				double _m = 6.0*(tmp-s)/h - 2.0*m;
				s = 3.0*tmp - 0.5*m*h - 2.0*s;
				m=_m;
				pv(i+1)[j]=m;
			}
		}
	}
	else if(sbc==(SBC_SlopeB|SBC_MomentumB))
	{
		// This is nearly as easy as above; just need to go the 
		// other way round. 
		for(int j=0; j<dim; j++)
		{
			double s=paramA[j];   // slope
			double m=paramB[j];   // momentum
			pv(n)[j]=m;
			for(int i=n; i>0; i--)
			{
				double h = tv ? (tv[i]-tv[i-1]) : 1.0;
				double tmp = (xv(i)[j]-xv(i-1)[j])/h;
				double _m = 6.0*(s-tmp)/h - 2.0*m;
				s = 3.0*tmp + 0.5*m*h - 2.0*s;
				m=_m;
				pv(i-1)[j]=m;
			}
		}
	}
	else if(sbc==SBC_Periodic)
	{
		double *ou=ALLOC<double>(n-1);   // n-1 is CORRECT. 
		
		// Note: xv(0)[j] must equal xv(n)[j] here; the caller 
		//       has to make that sure. 
		for(int j=0; j<dim; j++)
		{
			// Set up linear system: (It is n*n in size.) 
			double hi;
			if(tv)
			{
				hi=tv[1]-tv[0];
				double him1=tv[n]-tv[n-1];
				b[0] = 6.0*( (xv(1)[j]-xv(0  )[j])/hi - 
				             (xv(0)[j]-xv(n-1)[j])/him1 );
				diag[0] = 2.0*(him1+hi);
				for(int i=1; i<n; i++)
				{
					him1=hi;  hi=tv[i+1]-tv[i];
					b[i] = 6.0*( (xv(i+1)[j]-xv(i  )[j])/hi - 
				                 (xv(i  )[j]-xv(i-1)[j])/him1 );
					diag[i] = 2.0*(him1+hi);
					ou[i-1]=him1;
				}
			}
			else
			{
				b[0] = 6.0*( (xv(1)[j]-xv(0  )[j]) - (xv(0)[j]-xv(n-1)[j]) );
				diag[0]=4.0;
				for(int i=1; i<n; i++)
				{
					b[i] = 6.0*( (xv(i+1)[j]-xv(i  )[j]) - 
				                 (xv(i  )[j]-xv(i-1)[j]) );
					diag[i]=4.0;
					ou[i-1]=1.0;
				}
				hi=1.0;
			}
			
			CholeskySolveDiagDominant(diag,ou,hi,n,b,b);
			
			for(int i=0; i<n; i++)
			{  pv(i)[j]=b[i];  }
			pv(n)[j]=b[0];
		}
		
		FREE(ou);
	}
	else if(!tv) for(int j=0; j<dim; j++)
	{
		// Set up linear system: 
		for(int i=1; i<n; i++)
		{
			b[i] = 6.0*(xv(i-1)[j] - 2.0*xv(i)[j] + xv(i+1)[j]);
			diag[i]=4.0;
		}
		
		// Check boundary condition at the beginning: 
		switch(sbc&SBC_MaskA)
		{
			case SBC_SlopeA:  // Slope at the beginning. 
				b[0]=6.0*(xv(1)[j]-xv(0)[j]-paramA[j]);
				diag[0]=2.0;
				lsi0=0;
				break;
			case SBC_MomentumA:  // Momentum at the beginning. 
				pv(0)[j]=paramA ? paramA[j] : 0.0;
				lsi0=1;
				break;
			default:  nc_assert(0);
		}
		
		// Check boundary condition at the end: 
		switch(sbc&SBC_MaskB)
		{
			case SBC_SlopeB:  // Slope at the end. 
				b[n]=6.0*(paramB[j]-(xv(n)[j]-xv(n-1)[j]));
				diag[n]=2.0;
				lsin=n;
				break;
			case SBC_MomentumB:  // Momentum at the end. 
				pv(n)[j] = paramB ? paramB[j] : 0.0;
				lsin=n-1;
				break;
			default:  nc_assert(0);
		}
		
		// This must be done here to be sure that b[] is set up. 
		if((sbc&SBC_MaskA)==SBC_MomentumA && paramA)
		{  b[1]-=paramA[j];  }
		if((sbc&SBC_MaskB)==SBC_MomentumB && paramB)
		{  b[n-1]-=paramB[j];  }
		
		// Now solve the diag-stuff * p[] = b[]: 
		if(lsi0<=lsin)
		{
			for(int i=lsi0; i<lsin; i++)
			{
				b[i+1]-=b[i]/diag[i];
				diag[i+1]-=1.0/diag[i];
			}
			pv(lsin)[j]=b[lsin]/diag[lsin];
			for(int i=lsin-1; i>=lsi0; i--)
			{  pv(i)[j]=(b[i]-pv(i+1)[j])/diag[i];  }
		}
	}
	else for(int j=0; j<dim; j++)
	{
		double *ou=ALLOC<double>(n);  // n is CORRECT. 
		
		//#warning "Do better matrix conditioning by div through h(j-1)+h(j). -> Werner p.171"
		// We then need o[] and u[] separately instead of ou[]. 
		
		// Set up linear system: 
		ou[0]=tv[1]-tv[0];
		for(int i=1; i<n; i++)
		{
			b[i] = 6.0*(
				(xv(i+1)[j]-xv(i)[j])/(tv[i+1]-tv[i]) - 
				(xv(i)[j]-xv(i-1)[j])/(tv[i]-tv[i-1]) );
			diag[i]=2.0*(tv[i+1]-tv[i-1]);
			ou[i]=tv[i+1]-tv[i];
		}
		
		// Check boundary condition at the beginning: 
		switch(sbc&SBC_MaskA)
		{
			case SBC_SlopeA:  // Slope at the beginning. 
				b[0]=6.0*( (xv(1)[j]-xv(0)[j])/(tv[1]-tv[0]) - paramA[j] );
				diag[0]=2.0*(tv[1]-tv[0]);
				lsi0=0;
				break;
			case SBC_MomentumA:  // Momentum at the beginning. 
				pv(0)[j]=paramA ? paramA[j] : 0.0;
				lsi0=1;
				break;
			default:  nc_assert(0);
		}
		
		// Check boundary condition at the end: 
		switch(sbc&SBC_MaskB)
		{
			case SBC_SlopeB:  // Slope at the end. 
				b[n]=6.0*( paramB[j] - (xv(n)[j]-xv(n-1)[j])/(tv[n]-tv[n-1]) );
				diag[n]=2.0*(tv[n]-tv[n-1]);
				lsin=n;
				break;
			case SBC_MomentumB:  // Momentum at the end. 
				pv(n)[j] = paramB ? paramB[j] : 0.0;
				lsin=n-1;
				break;
			default:  nc_assert(0);
		}
		
		// This must be done here to be sure that b[] is set up. 
		if((sbc&SBC_MaskA)==SBC_MomentumA && paramA)
		{  b[1]-=paramA[j]*(tv[1]-tv[0]);  }
		if((sbc&SBC_MaskB)==SBC_MomentumB && paramB)
		{  b[n-1]-=paramB[j]*(tv[n]-tv[n-1]);  }
		
		// Now solve the diag-stuff * p[] = b[]: 
		if(lsi0<=lsin)
		{
			for(int i=lsi0; i<lsin; i++)
			{
				double tmp=ou[i]/diag[i];
				b[i+1]-=b[i]*tmp;
				diag[i+1]-=ou[i]*tmp;
			}
			pv(lsin)[j]=b[lsin]/diag[lsin];
			for(int i=lsin-1; i>=lsi0; i--)
			{  pv(i)[j]=(b[i]-ou[i]*pv(i+1)[j])/diag[i];  }
		}
		
		FREE(ou);
	}
	
	FREE(diag);
	FREE(b);
	
	return(0);
}


CubicPolySpline::CubicPolySpline() : 
	SplineBase()
{
	p=NULL;
}

CubicPolySpline::~CubicPolySpline()
{
	Clear();
}

}  // end of namespace NUM
