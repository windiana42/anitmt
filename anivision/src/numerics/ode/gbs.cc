
#include <numerics/function.h>
#include <numerics/interpolate/extr0poly.h>

#include "odesolver.h"


namespace NUM  // numerics
{

// Implements modified midpoint ODE solver step. 
// yn,dy_dx,yres are vectors of dimension odefunc.Dim(). 
// Takes current y values in yn[] and their derivations in dy_dx[] and 
// computes the step xn -> xn+htot using nsteps sub-steps. 
// Result is stored in yres[]. 
// (Note that it is valid to use yres=yn to get in-place updates to yn.) 
void ModifiedMidpoint(Function_ODE &odefunc,
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


//------------------------------------------------------------------------------

#define KMAXX 8   // Maximum row number used in the extrapolation.
#define IMAXX (KMAXX+1)
#define SAFE1 0.25   // Safety factors.
#define SAFE2 0.7
#define REDMAX 1.0e-5  // Maximum factor for stepsize reduction.
#define REDMIN 0.7     // Minimum factor for stepsize reduction.
#define TINY 1.0e-30   // Prevents division by zero.
#define SCALMX 0.1     // 1/SCALMX is the maximum factor by which a
                       // stepsize can be increased.


struct ODEStepParams
{
	double xn;      // Current value of independent variable ("time"). 
	                // Updated to xn+h_used. 
	double *yn;     // Current values of the dependent variables. 
	                // Changed to hold updated values. 
	double *dy_dx;  // Derivations dy/dx at xn, i.e. what odefunc(xn,yn,<..>) 
	                // returns; passed here to avoid unnecessary evaluations. 
	                // Updated to hold dy/dx at new xn. 
	double h;       // Initial step size guess; updated with new step size 
	                // guess to be used next time. 
	double h_used;  // Returns actually used step size. 
	int    nsteps;  // Number of steps performed (to gain used "solution"). 
	double epsilon; // <--
	double *yscale; // <--
};


// Bulirsch-Stoer step with monitoring of local truncation error to ensure
// accuracy and adjust stepsize. Input are the dependent variable vector y[1..nv]
// and its derivative dydx[1..nv] at the starting value of the independent
// variable x. Also input are the stepsize to be attempted htry, the required
// accuracy eps, and the vector yscal[1..nv] against which the error is scaled. On
// output, y and x are replaced by their new values, hdid is the stepsize that was
// actually accomplished, and hnext is the estimated next stepsize. derivs is the
// user-supplied routine that computes the right-hand side derivatives. Be sure to
// set htry on successive steps to the value of hnext returned from the
// previous step, as is the case if the routine is called by odeint.
void bsstep(Function_ODE &odefunc,
	double y[], double dydx[], int nv, double *xx, double htry, double eps,
	double yscal[], double *hdid, double *hnext)
{
	int i,iq,k,kk,km;
	static int first=1,kmax,kopt;
	static double epsold = -1.0,xnew;
	double eps1,errmax,fact,h,red,scale,work,wrkmin,xest;
	static double a[IMAXX+1];
	static double alf[KMAXX+1][KMAXX+1];
	static int nseq[IMAXX+1]={0,2,4,6,8,10,12,14,16,18};
	int reduct,exitflag=0;
	double err[KMAXX+1];
	double yerr[nv+1];
	double ysav[nv+1];
	double yseq[nv+1];
	
	Extrapolator_0E_Poly xxx(KMAXX,nv);
	
	if(eps != epsold) {  //A new tolerance, so reinitialize.
		*hnext = xnew = -1.0e29; // "Impossible" values.
		eps1=SAFE1*eps;
		a[1]=nseq[1]+1;   // Compute work coeffcients Ak.
		for(k=1;k<=KMAXX;k++) a[k+1]=a[k]+nseq[k+1];
		for(iq=2;iq<=KMAXX;iq++) {   // Compute ±(k, q).
			for(k=1;k<iq;k++)
				alf[k][iq]=pow(eps1,(a[k+1]-a[iq+1])/
					((a[iq+1]-a[1]+1.0)*(2*k+1)));
		}
		epsold=eps;
		for(kopt=2;kopt<KMAXX;kopt++) // Determine optimal row number for convergence. 
			if (a[kopt+1] > a[kopt]*alf[kopt-1][kopt]) break;
		kmax=kopt;
	}
	h=htry;
	for(i=1;i<=nv;i++) ysav[i]=y[i];   // Save the starting values.
	if (*xx != xnew || h != (*hnext)) {  // A new stepsize or a new integration: re-establish the order window. 
		first=1;
		kopt=kmax;
	}
	reduct=0;
	for (;;) {
		for (k=1;k<=kmax;k++) {  // Evaluate the sequence of modified midpoint integrations. 
			xnew=(*xx)+h;
			if(xnew == (*xx)) assert(!"step size underflow in bsstep");
			//mmid(ysav,dydx,nv,*xx,h,nseq[k],yseq,derivs);
			ModifiedMidpoint(odefunc,*xx,h,nseq[k],ysav+1,dydx+1,yseq+1);
			xest=SQR(h/nseq[k]);  // Squared, since error series is even.
	// Actually do the extrapolation: 
	if(k==1)
	{  xxx.FirstPoint(xest,yseq+1,y+1,yerr+1);  }
	else
	{  xxx.AddPoint(xest,yseq+1,y+1,yerr+1);  }
			if (k != 1) {   // Compute normalized error estimate epsilon(k). 
				errmax=TINY;
				for (i=1;i<=nv;i++) errmax=MAX(errmax,fabs(yerr[i]/yscal[i]));
				errmax /= eps;   // Scale error relative to tolerance.
				km=k-1;
				err[km]=pow(errmax/SAFE1,1.0/(2*km+1));
			}
			if (k != 1 && (k >= kopt-1 || first)) {  // In order window.
				if (errmax < 1.0) {  // Converged.
					exitflag=1;
					break;
				}
				if (k == kmax || k == kopt+1) {   // Check for possible stepsize reduction. 
					red=SAFE2/err[km];
					break;
				}
				else if (k == kopt && alf[kopt-1][kopt] < err[km]) {
					red=1.0/err[km];
					break;
				}
				else if (kopt == kmax && alf[km][kmax-1] < err[km]) {
					red=alf[km][kmax-1]*SAFE2/err[km];
					break;
				}
				else if (alf[km][kopt] < err[km]) {
					red=alf[km][kopt-1]/err[km];
					break;
				}
			}
		}
		if (exitflag) break;
		red=MIN(red,REDMIN);   // Reduce stepsize by at least REDMIN and at most REDMAX. 
		red=MAX(red,REDMAX);
		h *= red;
		reduct=1;
	}   // Try again.
	*xx=xnew;   // Successful step taken.
	*hdid=h;
	first=0;
	wrkmin=1.0e35;   // Compute optimal row for convergence and corresponding stepsize. 
	for (kk=1;kk<=km;kk++) {
		fact=MAX(err[kk],SCALMX);
		work=fact*a[kk+1];
		if (work < wrkmin) {
			scale=fact;
			wrkmin=work;
			kopt=kk+1;
		}
	}
	*hnext=h/scale;
	if (kopt >= k && kopt != kmax && !reduct) {
		// Check for possible order increase, but not if stepsize was just reduced.
		fact=MAX(scale/alf[kopt-1][kopt],SCALMX);
		if (a[kopt+1]*fact <= wrkmin) {
			*hnext=h/fact;
			kopt++;
		}
	}
}



const ODESolver_GBS::SolverParams ODESolver_GBS::sp={"GBS",2,2};

void ODESolver_GBS::ComputeStep(Function_ODE &odefunc,double xn,double h,double *yn)
{
	assert(0);
}

void ODESolver_GBS::AdaptiveStep(Function_ODE &odefunc,
	double &xn,double &h,double *yn)
{
	const int dim=odefunc.Dim();
	double dydx[dim];
	
	odefunc(xn,yn,dydx);
	
	#if 0
	h=1.0e-1;
	ModifiedMidpoint(odefunc,xn,h,30,yn,dydx,yn);
	xn+=h;
	#else
	
	double yscal[dim];
	for(int i=0; i<dim; i++)
		yscal[i] = fabs(yn[i])+fabs(dydx[i]*h)+TINY;
	
	double htry=h;
	double hdid;
	
	bsstep(odefunc,
		yn-1,dydx-1,dim, &xn, htry, /*eps=*/1e-13,
		yscal-1, &hdid, &h);
	#endif
}

}  // end of namespace NUM
