#include <stdio.h>

#include "interpolate.h"
#include "derive.h"


using namespace NUM;

class MyFuncRR : public Function_R_R
{
	private:
		double (*func)(double);
		// [overriding virtual:]
		double function(double x)
		{  return((*func)(x));  }
	public:
		MyFuncRR(double (*foo)(double)) : Function_R_R() { func=foo; }
		~MyFuncRR() {}
};

static void TestDiff(
	const char *name,
	double (*func)(double),
	double (*funcD)(double),
	double (*funcDD)(double),
	double x,double eps,int n)
{
	printf("%s @x=%f, eps=%.2e, n=%d:\n",name,x,eps,n);
	
	MyFuncRR the_func(func);
	MyFuncRR the_funcD(funcD);
	MyFuncRR the_funcDD(funcDD);
	
	Interpolator_Chebyshev in_ts;
	in_ts.Interpolate(the_func,x-eps,x+eps,n);
	in_ts.PolyCalcCoeffs();
	Interpolator_Chebyshev in_tsD;
	in_tsD.Derive(in_ts);
	Interpolator_Chebyshev in_tsDD;
	in_tsDD.Derive(in_tsD);
	
	double exact=the_func(x);
	double exactD=the_funcD(x);
	double exactDD=the_funcDD(x);
	
	double itp=in_ts.Eval(x);
	
	double polyitp=in_ts.PolyEval(x);
	double polyitpD=in_ts.PolyEvalD(x);
	double polyitpDD=in_ts.PolyEvalDD(x);
	
	double gslD=DiffCentral_GSL(the_func,x);
	
	double tsD=in_tsD.Eval(x);
	double tsDD=in_tsDD.Eval(x);
	
	printf("  f  : poly=%+e  (%e) \titp=%+e (%e)  [d=%deps]\n",
		polyitp,(polyitp-exact)/exact,
		itp,(itp-exact)/exact,
		int((polyitp-itp)/macheps1+0.5) );
	printf("  f' : poly=%+e  (%g) \titp=%+e (%g)  [d=%deps]\n",
		polyitpD,(polyitpD-exactD)/exactD,
		tsD,(tsD-exactD)/exactD,
		int((polyitpD-tsD)/macheps1+0.5) );
	printf("       GSL= %+e  (%g)\n",
		gslD,(gslD-exactD)/exactD);
	printf("  f'': poly=%+e  (%e) \titp=%+e  (%g)  [d=%deps]\n",
		polyitpDD,(polyitpDD-exactDD)/exactDD,
		tsDD,(tsDD-exactDD)/exactDD,
		int((polyitpDD-tsDD)/macheps1+0.5) );
}

// CHOOSE A FUNCTION: 
#if 0
double eps=1e-2;
const char *myfunc_name="cos(x)";
static double myfunc(double x)
{  return(cos(x));  }
static double myfuncD(double x)
{  return(-sin(x));  }
static double myfuncDD(double x)
{  return(-cos(x));  }
#elif 0
double eps=1e-2;
const char *myfunc_name="log(x)";
static double myfunc(double x)
{  return(log(x));  }
static double myfuncD(double x)
{  return(1.0/x);  }
static double myfuncDD(double x)
{  return(-1.0/(x*x));  }
#elif 1
double eps=1e-3;
const char *myfunc_name="exp(-x^2)";
static double myfunc(double x)
{  return(exp(-x*x));  }
static double myfuncD(double x)
{  return(-2*x*exp(-x*x));  }
static double myfuncDD(double x)
{  return((4*x*x-2)*exp(-x*x));  }
#endif


char *prg_name="test_numerics";

int main()
{
	printf("macheps1=%g (%g)\n",macheps1,macheps1_prec);
	
	TestDiff(myfunc_name,&myfunc,&myfuncD,&myfuncDD, 0.3, eps, 4);
	TestDiff(myfunc_name,&myfunc,&myfuncD,&myfuncDD, 0.6, eps, 4);
	TestDiff(myfunc_name,&myfunc,&myfuncD,&myfuncDD, 1.5, eps, 4);
	TestDiff(myfunc_name,&myfunc,&myfuncD,&myfuncDD, 100.3, eps, 4);
	
	return(0);
}


#if 0
	MyFuncRR myfunc;
	MyFuncRR_D myfuncD;
	MyFuncRR_DD myfuncDD;
	Interpolator_Chebyshev in_ts;
	
	double a=100,b=108;
	//double a=-1,b=1;
	in_ts.Interpolate(myfunc,a,b,10);
	in_ts.PolyCalcCoeffs();
	
	for(int i=0; i<=20; i++)
	{
		double x = a + ((b-a)*i)/20;
		double exact = myfunc(x);
		double interp = in_ts.Eval(x);
		double polyitp = in_ts.PolyEval(x);
		printf("%+.1f -> %+.6f, %+.6f \trelerr=%+e; %g, delta=%d*eps",
			x,exact,
			interp,(interp-exact)/exact,
			polyitp,int((polyitp-interp)/macheps1+0.5));
		double exactD=myfuncD(x);
		double polyitpD=in_ts.PolyEvalD(x);
		//double polyitpD=DiffCentral_GSL(myfunc,x);  // <-- testing only
		printf("  \tdiff=%+g, %+g \trelerr=%+e",
			exactD,polyitpD,(polyitpD-exactD)/exactD);
		double exactDD=myfuncDD(x);
		double polyitpDD=in_ts.PolyEvalDD(x);
		printf("  \tDDiff=%+g, %+g \trelerr=%+e\n",
			exactDD,polyitpDD,(polyitpDD-exactDD)/exactDD);
	}
#endif
