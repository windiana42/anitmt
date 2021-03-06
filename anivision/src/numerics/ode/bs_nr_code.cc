#if 0
		void stifbs(Function_ODE &odefunc,
			double y[], double dydx[], int nv, double *xx, double htry, double eps,
			double yscal[], double *hdid, double *hnext);
		void bsstep(Function_ODE &odefunc,
			double y[], double dydx[], int nv, double *xx, double htry, double eps,
			double yscal[], double *hdid, double *hnext);


...


	if(flags & X_XX)
	{
		StepState tmp=ComputeStep(ode->GetODEFunc(),
			ode->yn,ode->dy_dx,&ode->xn,htry,/*eps=*/ode->epsilon,
			yscal,&ode->h_used,&ode->h);
		if(tmp)  rv=tmp;
	}
	else
	{
		if(_IsSemiImplicit())
		{
			stifbs(ode->GetODEFunc(),
				ode->yn-1,ode->dy_dx-1,dim, &ode->xn, htry, /*eps=*/ode->epsilon,
				yscal-1, &ode->h_used, &ode->h);
		}
		else
		{
			bsstep(ode->GetODEFunc(),
				ode->yn-1,ode->dy_dx-1,dim, &ode->xn, htry, /*eps=*/ode->epsilon,
				yscal-1, &ode->h_used, &ode->h);
		}
	}
#endif

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
void ODESolver_BulirschStoer::bsstep(Function_ODE &odefunc,
	double y[], double dydx[], int nv, double *xx, double htry, double eps,
	double yscal[], double *hdid, double *hnext)
{
const int *nseq=this->nseq-1;
	
	// Deuflhard: 2,4,6,8,... every even number. 
	// [Works better with rational extrapolation here.]
	//static const int nseq[IMAXX+1]={0,2,4,6,8,10,12,14,16,18,20,22};  <--
	
	// Burlisch & Stoer: 2,4,6, then: n_i = 2*n_{i-2}
	// 2,4,6,8,12,16,24,32,48,64,96,128,192,256,384,512,768,1024,...
	//static const int nseq[IMAXX+1]={0,2,4,6,8,12,16,24,32,48,64,96};
	
	//static int first=1,kmax,kopt;
	//static double epsold = -1.0,xnew;
	//static double a[IMAXX+1];
	//static double alf[KMAXX+1][KMAXX+1];
	
	int i,iq,k,kk,km;
	double eps1,errmax,fact,h,red,scale,work,wrkmin,xest;
	int reduct,exitflag=0;
	double err[KMAXX+1];
	double yerr[nv+1];
	double ysav[nv+1];
	double yseq[nv+1];
	
	//Extrapolator_0E_Poly xxx(KMAXX,nv);
	//Extrapolator_0E_Rational xxx(KMAXX,nv);  <--
	
	if(eps != epsold) {  //A new tolerance, so reinitialize.
		*hnext = xnew = -1.0e29; // "Impossible" values.
		eps1=SAFE1*eps;
		a[1]=nseq[1]+1;   // Compute work coeffcients Ak.
		for(k=1;k<=KMAXX;k++) a[k+1]=a[k]+nseq[k+1];
		for(iq=2;iq<=KMAXX;iq++) {   // Compute �(k, q).
			for(k=1;k<iq;k++)
				alf[k][iq]=pow(eps1,(a[k+1]-a[iq+1])/
					((a[iq+1]-a[1]+1.0)*(k+k+1)));
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
			ModifiedMidpoint(
				odefunc,*xx,h,nseq[k],ysav+1,dydx+1,yseq+1);
			xest=SQR(h/nseq[k]);  // Squared, since error series is even.
	// Actually do the extrapolation: 
	if(k==1)
	{  xpol->FirstPoint(xest,yseq+1,y+1,yerr+1);  }
	else
	{  xpol->AddPoint(xest,yseq+1,y+1,yerr+1);  }
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

//------------------------------------------------------------------------------

//#undef KMAXX
//#undef IMAXX

//#define KMAXX 8 // WAS: 7
//#define IMAXX (KMAXX+1)

// #define SAFE1 0.25
// #define SAFE2 0.7
// #define REDMAX 1.0e-5
// #define REDMIN 0.7
// #define TINY 1.0e-30
// #define SCALMX 0.1

// Semi-implicit extrapolation step for integrating sti o .d.e.s, with
// monitoring of local truncation error to adjust stepsize. Input are the
// dependent variable vector y[1..nv] and its derivative dydx[1..nv] at the
// starting value of the independent variable x. Also input are the stepsize to
// be attempted htry, the required accuracy eps, and the vector yscal[1..nv]
// against which the error is scaled. On output, y and x are replaced by their
// new values, hdid is the stepsize that was actually accomplished, and hnext
// is the estimated next stepsize. derivs is a user-supplied routine that
// computes the derivatives of the right-hand side with respect to x, while
// jacobn (a fixed name) is a user-supplied routine that computes the Jacobi
// matrix of derivatives of the right-hand side with respect to the components
// of y. Be sure to set htry on successive steps to the value of hnext returned
// from the previous step, as is the case if the routine is called by odeint.
void ODESolver_BulirschStoer::stifbs(Function_ODE &odefunc,
	double y[], double dydx[], int nv, double *xx, double htry, double eps,
	double yscal[], double *hdid, double *hnext)
{
const int *nseq=this->nseq-1;
	
	// Each member differs from its predecessor by the smallest multiple 
	// of 4 that makes the ratio of successive terms be <= 5/7. 
	// 2,6,10,14,22,34,50,70,98,138,194,274,386,542,...
	//static const int nseq[IMAXX+1]={0,2,6,10,14,22,34,50,70,98};  <--
	
	//static int first=1,kmax,kopt,nvold = -1;
	//static double epsold = -1.0,xnew;
	//static double a[IMAXX+1];
	//static double alf[KMAXX+1][KMAXX+1];
	
	int i,iq,k,kk,km;
	double eps1,errmax,fact,h,red,scale,work,wrkmin,xest;
	int reduct,exitflag=0;
	double dfdx[nv+1];
	//SMatrix<double> dfdy(nv,nv);
	SMatrix<double> &dfdy=*_dfdy;
	double err[KMAXX+1];
	double yerr[nv+1];
	double ysav[nv+1];
	double yseq[nv+1];
	
	//Extrapolator_0E_Poly xxx(KMAXX,nv);   <--
	//Extrapolator_0E_Rational xxx(KMAXX,nv);
	
	if(eps != epsold || nv != nvold) { // Reinitialize also if nv has changed.
		*hnext = xnew = -1.0e29;
		eps1=SAFE1*eps;
		a[1]=nseq[1]+1;
		for (k=1;k<=KMAXX;k++) a[k+1]=a[k]+nseq[k+1];
		for (iq=2;iq<=KMAXX;iq++) {
			for (k=1;k<iq;k++)
				alf[k][iq]=pow(eps1,((a[k+1]-a[iq+1])/
					((a[iq+1]-a[1]+1.0)*(2*k+1))));
		}
		epsold=eps;
		nvold=nv;   // Save nv.
		a[1] += nv;   // Add cost of Jacobian evaluations to work coeffcients. 
		for (k=1;k<=KMAXX;k++) a[k+1]=a[k]+nseq[k+1];
		for (kopt=2;kopt<KMAXX;kopt++)
			if (a[kopt+1] > a[kopt]*alf[kopt-1][kopt]) break;
		kmax=kopt;
	}
	h=htry;
	for (i=1;i<=nv;i++) ysav[i]=y[i];
	//jacobn(*xx,y,dfdx,dfdy,nv);  // Evaluate Jacobian.
	odefunc.Jacobian(*xx,y,dfdy,dfdx+1);
	if (*xx != xnew || h != (*hnext)) {
		first=1;
		kopt=kmax;
	}
	reduct=0;
	for (;;) {
		for (k=1;k<=kmax;k++) {
			xnew=(*xx)+h;
			if (xnew == (*xx)) assert(!"step size underflow in stifbs");
			// Semi-implicit midpoint rule.
			//simpr(ysav,dydx,dfdx,dfdy,nv,*xx,h,nseq[k],yseq,derivs);
			SIMidpoint(
				odefunc,*xx,h,nseq[k],ysav+1,dydx+1,dfdx+1,dfdy,yseq+1);
			xest=SQR(h/nseq[k]); // The rest of the routine is identical to bsstep. 
	// Actually do the extrapolation: 
	// pzextr(k,xest,yseq,y,yerr,nv);
	if(k==1)
	{  xpol->FirstPoint(xest,yseq+1,y+1,yerr+1);  }
	else
	{  xpol->AddPoint(xest,yseq+1,y+1,yerr+1);  }
			if (k != 1) {
				errmax=TINY;
				for (i=1;i<=nv;i++) errmax=MAX(errmax,fabs(yerr[i]/yscal[i]));
				errmax /= eps;
				km=k-1;
				err[km]=pow(errmax/SAFE1,1.0/(2*km+1));
			}
			if (k != 1 && (k >= kopt-1 || first)) {
				if (errmax < 1.0) {
					exitflag=1;
					break;
				}
				if (k == kmax || k == kopt+1) {
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
		red=MIN(red,REDMIN);
		red=MAX(red,REDMAX);
		h *= red;
		reduct=1;
	}
	*xx=xnew;
	*hdid=h;
	first=0;
	wrkmin=1.0e35;
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
		fact=MAX(scale/alf[kopt-1][kopt],SCALMX);
		if (a[kopt+1]*fact <= wrkmin) {
			*hnext=h/fact;
			kopt++;
		}
	}
}

