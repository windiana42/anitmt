/*
 * numerics/spline/sl_base.cc
 * 
 * Numerics library spline base code. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "sl_base.h"

#include <stdio.h>

#include <numerics/diff_int/derive.h>


namespace NUM
{

// Static variables: 
const double *SplineBase::TVALS_DIST=(double*)0x1;
const double *SplineBase::TVALS_CPTS=(double*)0x2;


void SplineBase::CurveFunction::function(double x,double *result)
{
	spline->Eval(x,result);
}

double SplineBase::DevLenFunction::function(double x)
{
	if(!spline) return(0.0);
	int dim=spline->Dim();
	double tmp[dim];
	spline->EvalD(x,tmp);
	return(NormEuclidic(tmp,dim));
}

//------------------------------------------------------------------------------

int SplineBase::_TestAscentingOrder_LastCompo(
	const VectorArray<double> &cpoints) const
{
	if(!cpoints.NVectors()) return(0);
	int idx=cpoints.Dim()-1;
	if(idx<0) return(0);
	for(int i=1; i<n; i++)
		if(cpoints[i][idx]-cpoints[i-1][idx]<=tval_eps)
			return(i);
	return(0);
}

const SplineBase::Properties *SplineBase::SplineProperties() const
{
	return(NULL);
}


double SplineBase::_DoCalcLength(Integrator_R_R *integrator,
	int int_by_piece)
{
	DevLenFunction len_func(this);
	
	if(!int_by_piece)
	{  return(integrator->operator()(len_func,get_t(0),get_t(n)));  }
	
	double I=0.0;
	double a,b=get_t(0);
	for(int i=1; i<=n; i++)
	{
		a=b;
		b=get_t(i);
		I+=integrator->operator()(len_func,a,b);
	}
	return(I);
}


double SplineBase::CalcLength(Integrator_R_R *integrator,int int_by_piece)
{
	if(!n)  return(0.0);
	if(!integrator)
	{
		Integrator_R_R_NewtonCotes nc_int;
		nc_int.order=4;
		nc_int.nintervals=32;
		// Expect relative error to be <1e-5: 
		return(_DoCalcLength(&nc_int,1));
	}
	return(_DoCalcLength(integrator,int_by_piece));
}


void SplineBase::Clear()
{
	tv=FREE(tv);
	x=FREE(x);
	dim=0;
	n=0;
}


int SplineBase::Eval(double /*t*/,double * /*result*/) const
{
	// This should be overridden. After all, this function is the 
	// reason for the spline classes...
	assert(0);
	return(-2);
}


struct _ICurveComponentFunction : Function_R_R
{
	private:
		const SplineBase *spline;
	public:
		int compo;
		
		_ICurveComponentFunction(const SplineBase *_sb) : 
			Function_R_R()  {  spline=_sb;  }
		~_ICurveComponentFunction()  {}
		
		// [overriding virtual:]
		double function(double x)
		{
			int dim=spline->Dim();
			double tmp[dim];
			spline->Eval(x,tmp);
			return(tmp[compo]);
		}
};


int SplineBase::EvalD(double t,double *result) const
{
	if(!n)  return(-2);
	
	// Numerically compute first derivation: 
	_ICurveComponentFunction compo_func(this);
	for(int c=0; c<dim; c++)
	{
		compo_func.compo=c;
		result[c]=DiffCentral_GSL(compo_func,t);
	}
	
	if(t<get_t(0)-tval_eps)  return(-1);
	if(t>get_t(n)+tval_eps)  return(+1);
	return(0);
}


int SplineBase::EvalDD(double t,double *result) const
{
	if(!n)  return(-2);
	
	// Numerically compute second derivation: 
	_ICurveComponentFunction compo_func(this);
	for(int c=0; c<dim; c++)
	{
		compo_func.compo=c;
assert(0);  // Missing: Need good algorithm. 
		double h=1.0e-5;  // <-- WHAT TO USE?!
		result[c]=(compo_func(t+h)-2.0*compo_func(t)+compo_func(t-h))/(h*h);
	}
	
	if(t<get_t(0)-tval_eps)  return(-1);
	if(t>get_t(n)+tval_eps)  return(+1);
	return(0);
}


void SplineBase::_CopyPointsAndTVals(const VectorArray<double> &cpoints,
	const double *tvals)
{
	// Get rid of old stuff: 
	Clear();  // call the virtual function 
	
	// First, copy the control points: 
	n=cpoints.NVectors()-1;
	dim=cpoints.Dim();
	if(tvals==TVALS_CPTS)  --dim;
	x=ALLOC<double>(dim*(n+1));
	for(int i=0; i<=n; i++)
	{
		const double *src=cpoints[i];
		double *dest=xv(i);
		for(int j=0; j<dim; j++)
		{  *(dest++)=*(src++);  }
	}
	// Then, copy the t values, if present: 
	if(tvals)
	{
		tv=ALLOC<double>(n+1);
		if(tvals==TVALS_DIST)
		{
			// t-values according to distance. 
			tv[0]=0.0;
			double len=0.0;
			double *p0,*p1=xv(0);
			for(int i=1; i<=n; i++)
			{
				p0=p1;
				p1=xv(i);
				len+=DistEuclidic(p0,p1,dim);
				tv[i]=len;
			}
		}
		else if(tvals==TVALS_CPTS)
		{
			for(int i=0; i<=n; i++)
			{  tv[i]=cpoints[i][dim];  }
		}
		else if(tvals)
		{
			for(int i=0; i<=n; i++)
			{  tv[i]=tvals[i];  }
		}
	}
}


int SplineBase::_EvalFindIdx(double t,int *retval) const
{
	// Compute the index of the polynomial: 
	int idx;
	     if(t<get_t(0)-tval_eps)  {  idx=0;    *retval=-1;  }
	else if(t>get_t(n)+tval_eps)  {  idx=n-1;  *retval=+1;  }
	else
	{
		// Need to look up the apropriate interval using a binary 
		// search in case the t values were explicitly specified. 
		// But first ask the cache. 
		if(tv)
		{
			if(tv_cache_idx>=0 && tv_cache_idx<n && 
				tv[tv_cache_idx]<=t && t<tv[tv_cache_idx+1])
			{  idx=tv_cache_idx;  }
			else
			{
				idx=ArrayBinsearchInterval(tv,n+1,t);
				if(idx<0)  idx=0;
				if(idx>=n)  idx=n-1;
				((SplineBase*)this)->tv_cache_idx=idx; // cast away "const"
			}
		}
		else
		{
			idx = int(t);
			if(idx<0)  idx=0;
			if(idx>=n)  idx=n-1;
		}
		*retval=0;
	}
	return(idx);
}


SplineBase::SplineBase()
{
	n=0;
	dim=0;
	x=NULL;
	tv=NULL;
	
	tv_cache_idx=-1;
}

SplineBase::~SplineBase()
{
	// Be careful not to call the virtual function here. 
	SplineBase::Clear();
}

}  // end of namespace SL
