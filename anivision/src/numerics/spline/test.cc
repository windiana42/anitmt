#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "cspline.h"
#include "lspline.h"
#include "xspline.h"

#include <QTX/xpainter.h>

#include <numerics/diff_int/integrate.h>

char *prg_name="splinelib-test";

//------------------------------------------------------------------------------

// "Function" to calculate spline length: 
struct SplineLengthFunction : NUM::Function_R_R
{
	// The spline to use; only EvalD() is needed. 
	NUM::SplineBase *spline;
	// The function: calculates euclidic norm of first derivation 
	// of spline function.  [overriding virtual]
	double function(double x);
	
	SplineLengthFunction(NUM::SplineBase *_spline)
		{  spline=_spline;  }
	~SplineLengthFunction()
		{  spline=NULL;  }
};

double SplineLengthFunction::function(double x)
{
	int dim=spline->Dim();
	double tmp[dim];
	spline->EvalD(x,tmp);
	double res=0.0;
	for(int i=0; i<dim; i++)
	{  res+=NUM::SQR(tmp[i]);  }
	return(sqrt(res));
}


//------------------------------------------------------------------------------

static void TestCalcLength(NUM::SplineBase *spline)
{
	SplineLengthFunction len_func(spline);
	double a=spline->GetT(0);
	double b=spline->GetT(spline->N());
	fprintf(stderr,"Calc: %d points %g..%g: complete integral\n",
		spline->NPoints(),a,b);
	NUM::Integrator_R_R_SimpsonAdaptive saintegrator;
	// First method: integrate complete spline. 
	for(double epsilon=1.0e5; epsilon>1.0e-7; epsilon/=10.0)
	{
		saintegrator.epsilon=epsilon;
		len_func.ncalls=0;
		double I=saintegrator(len_func,a,b);
		fprintf(stderr,"  I=%.15g, eps=%g  (%u calls)\n",
			I,saintegrator.epsilon,len_func.ncalls);
	}
	
	// Next method: integrate between two interpolation points 
	// and sum up: 
	// Result: For small epsilon, performance is about the same 
	//         as above. For large epsilon, (>=0.01), this method 
	//         has better results because it will use more function 
	//         evaluations (Worst case: 3 per curve piece instead 
	//         3 for the complete curve in the above algo). 
	fprintf(stderr,"Integrating pieces:\n");
	for(double epsilon=1.0e5; epsilon>1.0e-7; epsilon/=10.0)
	{
		double I=0.0;
		
		saintegrator.epsilon=epsilon;
		len_func.ncalls=0;
		
		b=spline->GetT(0);
		for(int i=1; i<=spline->N(); i++)
		{
			a=b;
			b=spline->GetT(i);
			I+=saintegrator(len_func,a,b);
		}
		
		fprintf(stderr,"  I=%.15g, eps=%g  (%u calls)\n",
			I,saintegrator.epsilon,len_func.ncalls);
	}
	
	// Finally, do Newton-Cotes integral, piece-wise: 
	fprintf(stderr,"Integrating pieces (Newton-Cotes, order 2):\n");
	NUM::Integrator_R_R_NewtonCotes ncintegrator;
	for(ncintegrator.nintervals=1; ncintegrator.nintervals<512; 
		ncintegrator.nintervals*=2)
	{
		double I=0.0;
		
		ncintegrator.order=2;
		len_func.ncalls=0;
		
		b=spline->GetT(0);
		for(int i=1; i<=spline->N(); i++)
		{
			a=b;
			b=spline->GetT(i);
			I+=ncintegrator(len_func,a,b);
		}
		
		fprintf(stderr,"  I=%.15g, nintervals=%d  (%u calls)\n",
			I,ncintegrator.nintervals,len_func.ncalls);
	}
	
	fprintf(stderr,"Integrating pieces (Newton-Cotes, order 4):\n");
	for(ncintegrator.nintervals=1; ncintegrator.nintervals<512; 
		ncintegrator.nintervals*=2)
	{
		double I=0.0;
		
		ncintegrator.order=4;
		len_func.ncalls=0;
		
		b=spline->GetT(0);
		for(int i=1; i<=spline->N(); i++)
		{
			a=b;
			b=spline->GetT(i);
			I+=ncintegrator(len_func,a,b);
		}
		
		fprintf(stderr,"  I=%.15g, nintervals=%d  (%u calls)\n",
			I,ncintegrator.nintervals,len_func.ncalls);
	}
}

void CalcSplineLength(NUM::SplineBase *spline,QTX::XPainter *Xpaint)
{
	if(!spline || spline->NPoints()<2)  return;
	
	//TestCalcLength(spline);
	
	//------------------------------------------------------------------
	
	// Constant moving stuff. 
	// Number of control points per spline curve piece. 
	int ppsp=12;
	
	NUM::SplineBase::DevLenFunction len_func(spline);
	NUM::Integrator_R_R_NewtonCotes ncintegrator;
	ncintegrator.nintervals=3; // decrease if ppsp increase 
	ncintegrator.order=4;
	len_func.ncalls=0;
	
	// Calc number of control points: 
	int nctlpts=ppsp*spline->N()+1;
	NUM::VectorArray<double> tv(nctlpts,1);
	NUM::VectorArray<double> sv(nctlpts,1);
	double ti0,ti1=spline->GetT(0);
	double len=0.0;
	// Compute control points for the inverse arc length 
	// function: 
	*tv[0]=ti1;
	*sv[0]=0.0;
	int vidx=1;
	for(int i=0; i<spline->N(); i++)
	{
		ti0=ti1;
		ti1=spline->GetT(i+1);
		double dt=(ti1-ti0)/double(ppsp);
		double t0,t1=ti0;
		for(int j=0; j<ppsp; j++,vidx++)
		{
			t0=t1;
			t1+=dt;
			// Calculate curve length until t. 
			// I.e. add length of curve in t0..t1 (=l) to 
			//      length ti0..t0 (=len). 
			double l=ncintegrator(len_func,t0,t1);
			assert(l>0.0);
			len+=l;
			*sv[vidx]=len;
			*tv[vidx]=t1;
		}
	}
	assert(vidx==nctlpts);
	fprintf(stderr,"totlen=%.15g; (%d calls)\n",*sv[nctlpts-1],len_func.ncalls);
	
	// Create inverse arc length function: 
	// l^{-1}(s_i)=t_i  (s_i=sv[i]; t_i=tv[i])
	// #FIXME##: Should probably use Hermite boundary condition. 
	NUM::CubicPolySpline ialf;
	int rv=ialf.Create(
		/*cpoints=*/tv, /*tvals=*/sv.GetArrayBase(),
		NUM::SBC_MomentumA|NUM::SBC_MomentumB,NULL,NULL);  // <-- currently "natutal"
	assert(rv==0);
	
	// ialf: [0..len] -> t value for spline. 
	{
		int nsamples=100;  // For display. 
		double tv,tv_prev;
		double min_speed,max_speed,avg_speed=0.0,speed_sqs=0.0;
		for(int i=0; i<=nsamples; i++)
		{
			tv_prev=tv;
			#if 1  // 0 -> No constant speed correction. 
			double where=len*i/nsamples;
			ialf.Eval(where,&tv);
			#else
			tv=spline->GetT(0)+spline->GetTRangeLength()*i/nsamples;
			#endif
			double pnt[spline->Dim()];
			spline->Eval(tv,pnt);
			Xpaint->FillArc(
				int(pnt[0]+0.5)-2,int(pnt[1]+0.5)-2, 
				5,5, 0,360*64);
			
			if(i)
			{
				double dl=ncintegrator(len_func,tv_prev,tv);
				double speed=dl/(len/nsamples);
				if(i==1)
				{  min_speed=max_speed=speed;  }
				else if(min_speed>speed)
				{  min_speed=speed;  }
				else if(max_speed<speed)
				{  max_speed=speed;  }
				avg_speed+=speed;
				speed_sqs+=NUM::SQR(speed);
			}
		}
		avg_speed/=nsamples;
		speed_sqs/=nsamples;
		fprintf(stderr,"speed: min=%g, max=%g, avg=%g, stdev=%g (npts=%d)\n",
			min_speed,max_speed,avg_speed,
			sqrt(speed_sqs-NUM::SQR(avg_speed)),
			ialf.NPoints());
	}
}


void DOTEST(NUM::SplineBase *spline,QTX::XPainter *pnt)
{
	CalcSplineLength(spline,pnt);
}
