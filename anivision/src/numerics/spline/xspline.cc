/*
 * numerics/spline/xspline.cc
 * 
 * Numerics library X spline (Blanc/Schlick). 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * Code based on implementation of X-Splines in XFig. 
 * XFig is (c) Brian Smith, the X-Spline implemntation in XFig was 
 * written by Blanc and Schlick. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "xspline.h"

// FIXME: What is wrong with these X-Splines? The t-values do not any 
// FIXME: influence because the original code used equally-spaced t intervals. 
// FIXME: Hence, when using t values, the first and second derivative will 
// FIXME: be discontinuous. 

namespace NUM
{

const SplineBase::Properties *X_Spline::SplineProperties() const
{
	static const Properties prop=
	{
		splinetype: ST_XSpline,
		name: "xspline",
		cont_order: 2
	};
	return(&prop);
}


static inline double f_blend(double numerator,double denominator)
{
	double p = 2.0 * denominator * denominator;
	double u = numerator / denominator;
	double u2 = u * u;

	//return(u * u2 * (10.0 - p + (2.0*p - 15.0)*u + (6.0 - p)*u2));
	return(u * u2 * (10.0 - p + u * (2.0*p - 15.0 + u * (6.0 - p))));
}

static inline double f_blendD(double numerator,double denominator)
{
	double p = 2.0 * denominator * denominator;
	double u = numerator / denominator;

	return(u * u * (30.0 - 3.0*p + u * (8.0*p - 60.0 + u*(30.0 - 5.0*p))) / 
		denominator );
}

static inline double g_blend(double u, double q)    /* p equals 2 */
{
	/*return(
	q *                             u 
	  + 2 * q *                     u*u
		+ (10 - 12 * q - p) *       u*u*u
		  + (2 * p + 14 * q - 15) * u*u*u*u
	    	+ (6 - 5*q - p) *       u*u*u*u*u );*/
	return(u*(q + u*(2.0*q + u*(8.0 - 12.0*q + 
		u*(14.0*q - 11.0 + u*(4.0 - 5.0*q))))));
}

static inline double g_blendD(double u, double q)    /* p equals 2 */
{
	return(q + u*(4.0*q + u*(24.0 - 36.0*q + 
		u*(56.0*q - 44.0 + u*(20.0 - 25.0*q)))));
}

static inline double h_blend(double u, double q)
{
	// q*u + 2.0*q*u^2 - 2.0*q*u^4 - q*u^5   (<- ?)
	return(u * (q + u * (2.0 * q + u*u * (-2.0*q - u*q))));
}

static inline double h_blendD(double u, double q)
{
	// q + 4.0*q*u - 8.0*q*u^3 - 5.0*q*u^4   (<- ?)
	return(q + u * (4.0 * q + u*u * (-8.0*q - 5.0*q*u)));
	
}

//----------------------------------------------------------------------

static inline void negative_s1_influence(double u,double s1,
	double *A0, double *A2)
{
	*A0 = h_blend(-u, -s1);
	*A2 = g_blend( u, -s1);
}

static inline void negative_s2_influence(double u,double s2,
	double *A1,double *A3)
{
	*A1 = g_blend(1.0-u, -s2);
	*A3 = h_blend(u-1.0, -s2);
}

static inline void positive_s1_influence(double u,
	double s1,double *A0,double *A2)
{
	*A0 = u<s1 ? f_blend(u-s1, -1.0-s1) : 0.0;
	*A2 = f_blend(u+s1, s1+1.0);
}

static inline void positive_s2_influence(double u,
	double s2,double *A1,double *A3)
{
	u-=1.0;  *A1 = f_blend(u-s2, -1.0-s2);
	u+=s2;   *A3 = u>0.0 ? f_blend(u, s2+1.0) : 0.0;
}

//----------------------------------------------------------------------

static inline void negative_s1_influenceD(double u,double s1,
	double *A0, double *A2)
{
	*A0 =-h_blendD(-u, -s1);
	*A2 = g_blendD( u, -s1);
}

static inline void negative_s2_influenceD(double u,double s2,
	double *A1,double *A3)
{
	*A1 =-g_blendD(1.0-u, -s2);
	*A3 = h_blendD(u-1.0, -s2);
}

static inline void positive_s1_influenceD(double u,
	double s1,double *A0,double *A2)
{
	*A0 = u<s1 ? f_blendD(u-s1, -1.0-s1) : 0.0;
	*A2 = f_blendD(u+s1, s1+1.0);
}

static inline void positive_s2_influenceD(double u,
	double s2,double *A1,double *A3)
{
	u-=1.0;  *A1 = f_blendD(u-s2, -1.0-s2);
	u+=s2;   *A3 = u>0.0 ? f_blendD(u, s2+1.0) : 0.0;
}

//----------------------------------------------------------------------


// Actually compute the spline: 
// t is in range 0..h. 
// p0..p4 are arrays of the point coordinates; the array size 
//    is the dimension dim (normally 3 for free space)
// result: array (vector) just as p0..p4
// seg: segment number being calculated (0..np-1)
// s1,s2: s values ("weights") for p1 and p2. 
void X_Spline::_DoEval(double t,double h,double *result,int idx,
	const double *p0,const double *p1,const double *p2,const double *p3) const
{
	double s1,s2;
	if(sv)  {  s1=sv[idx];  s2=sv[idx+1];  }
	else    {  s1=-1.0;  s2=-1.0;  }
	
	double u=t/h;
	
	// I love it... just stripped down the original code by a factor of 
	// 5 again... Seems these guys did not have a lot of programming 
	// experience (note that xsplines were added to XFig by Blanc/Schlick 
	// and not by Brian Smith). 
	double A_blend[4];
	if(s1<0.0)  negative_s1_influence(u,s1,&A_blend[0],&A_blend[2]);
	else        positive_s1_influence(u,s1,&A_blend[0],&A_blend[2]);
	if(s2<0.0)  negative_s2_influence(u,s2,&A_blend[1],&A_blend[3]);
	else        positive_s2_influence(u,s2,&A_blend[1],&A_blend[3]);
	
	double weights_sum=A_blend[0]+A_blend[1]+A_blend[2]+A_blend[3];
	for(int i=0; i<dim; i++)
		result[i]=(A_blend[0]*p0[i] + A_blend[1]*p1[i] + 
			A_blend[2]*p2[i] + A_blend[3]*p3[i]) / weights_sum;
}

// ...first derivation...
void X_Spline::_DoEvalD(double t,double h,double *result,int idx,
	const double *p0,const double *p1,const double *p2,const double *p3) const
{
	double s1,s2;
	if(sv)  {  s1=sv[idx];  s2=sv[idx+1];  }
	else    {  s1=-1.0;  s2=-1.0;  }
	
	double u=t/h;
	double A_blend[4];
	double A_blendD[4];
	if(s1<0.0) { negative_s1_influence (u,s1,&A_blend [0],&A_blend [2]);
	             negative_s1_influenceD(u,s1,&A_blendD[0],&A_blendD[2]); }
	else       { positive_s1_influence (u,s1,&A_blend [0],&A_blend [2]);
	             positive_s1_influenceD(u,s1,&A_blendD[0],&A_blendD[2]); }
	if(s2<0.0) { negative_s2_influence (u,s2,&A_blend [1],&A_blend [3]);
	             negative_s2_influenceD(u,s2,&A_blendD[1],&A_blendD[3]); }
	else       { positive_s2_influence (u,s2,&A_blend [1],&A_blend [3]);
	             positive_s2_influenceD(u,s2,&A_blendD[1],&A_blendD[3]); }
	
	double weights_sum= A_blend[0]+ A_blend[1]+ A_blend[2]+ A_blend[3];
	double weights_sumD=A_blendD[0]+A_blendD[1]+A_blendD[2]+A_blendD[3];
	for(int i=0; i<dim; i++)
		result[i]=
			( (A_blendD[0]*p0[i] + A_blendD[1]*p1[i] + 
				A_blendD[2]*p2[i] + A_blendD[3]*p3[i]) * weights_sum - 
			  (A_blend[0]*p0[i] + A_blend[1]*p1[i] + 
				A_blend[2]*p2[i] + A_blend[3]*p3[i]) * weights_sumD ) / 
			(weights_sum * weights_sum * h);
}


void X_Spline::Clear()
{
	sv=FREE(sv);
	SplineBase::Clear();
}


int X_Spline::Eval(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	static int warn=10;
	if(warn)
	{  fprintf(stderr,"X_Spline::Eval: not working correctly.\n");  --warn;  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the spline: 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	if(idx==0)
	{
		// First control point is needed twice for the first segment. 
		// np=1 -> only 2 points
		_DoEval(t,h,result,idx,
			xv(0),xv(0),xv(1),xv(n==1 ? 1 : 2));
	}
	else if(idx+1<=n)
	{
		// Last control point is needed twice (only) for the last segment. 
		_DoEval(t,h,result,idx,
			xv(idx-1),xv(idx),xv(idx+1),xv(idx+1>=n ? idx+1 : idx+2));
	}
	
	return(retval);
}

int X_Spline::EvalD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	static int warn=10;
	if(warn)
	{  fprintf(stderr,"X_Spline::EvalD: not working correctly.\n");  --warn;  }
	
	// Compute the index of the polynomial: 
	int retval;
	int idx=_EvalFindIdx(t,&retval);
	
	// Normalize t: 
	t-=get_t(idx);
	
	// Evaluate the spline: 
	double h = tv ? (tv[idx+1]-tv[idx]) : 1.0;
	if(idx==0)
	{
		// First control point is needed twice for the first segment. 
		// np=1 -> only 2 points
		_DoEvalD(t,h,result,idx,
			xv(0),xv(0),xv(1),xv(n==1 ? 1 : 2));
	}
	else if(idx+1<=n)
	{
		// Last control point is needed twice (only) for the last segment. 
		_DoEvalD(t,h,result,idx,
			xv(idx-1),xv(idx),xv(idx+1),xv(idx+1>=n ? idx+1 : idx+2));
	}
	
	return(retval);
}

int X_Spline::EvalDD(double t,double *result) const
{
	if(!n)
	{  for(int j=0; j<dim; j++)  result[j]=0.0;  return(-2);  }
	
	// I suggest doing this numerically...
	
	static int warn=10;
	if(warn)
	{  fprintf(stderr,"X_Spline::EvalDD: not implemented.\n");  --warn;  }
	
	// This is a very quick hack: 
	double y0[dim],y1[dim];
	double h=1e-6;
	int rv=EvalD(t-h,y0);
	EvalD(t+h,y1);
	h+=h;
	for(int i=0; i<dim; i++)
		result[i]=(y1[i]-y0[i])/h;
	return(0);
}


int X_Spline::Create(
	const VectorArray<double> &cpoints,const double *tvals,
	const double *weights)
{
	if(cpoints.NVectors()<2 || cpoints.Dim()<(tvals==TVALS_CPTS ? 2 : 1))
	{  return(-2);  }
	
	// Check t values if specified: 
	if(tvals && tvals!=TVALS_DIST && 
		( (tvals==TVALS_CPTS) ? 
			_TestAscentingOrder_LastCompo(cpoints) : 
			_TestAscentingOrder(tvals,cpoints.NVectors()) ) )
	{  return(-4);  }
	
	// Get rid of old stuff...
	// ...and copy the control points and t values, if present: 
	_CopyPointsAndTVals(cpoints,tvals);  // calls Clear(). 
	
	// If there were weights, copy them: 
	sv=FREE(sv);
	if(weights)
	{
		sv=ALLOC<double>(n+1);
		for(int i=0; i<=n; i++)
		{  sv[i]=weights[i];  }
	}
	
	return(0);
}


X_Spline::X_Spline() : 
	SplineBase()
{
	sv=NULL;
}

X_Spline::~X_Spline()
{
	Clear();
}

}  // end of namespace NUM
