#include "integrate.h"
 
 // gcc -O2 -fno-rtti -fno-exceptions test.cc -o test -I. -I.. -I../.. -I../../../../hlib-build/include/ ../../../../hlib-build/libhlib.a -lm ../../../../anivision-build/src/numerics/diff_int/libnumerics_diff_int.a ../../../../anivision-build/src/numerics/libnumerics.a
 
const int N=6;

struct MyFunc : NUM::Function_R_R
{
	int which;
	MyFunc() : which(0) {}
	~MyFunc() {}
	double function(double x)
	{
		switch(which)
		{
			case 0:  return(exp(-x*x));
			case 1:  return(1.1111111111111111*sin(x));
			case 2:  return(sin(x)/x);
			case 3:  return(1.0/(1+x*x));
			case 4:  return(1.0/x);
			case 5:  return(sqrt(1.0-0.6*x*x));   // 0..1
		}
		return(0.0);
	}
};


char *prg_name="integration test";

int main()
{
	double a[N]={ 0.2, 0.0,  0.01,  -0.8, 1.0, 0.0 };
	double b[N]={ 3.0, M_PI, M_PI,  2.5, 2.0, 1.0 };
	
	MyFunc funcA,funcB;
	//NUM::Integrator_R_R *intA=new NUM::Integrator_R_R_GaussCebyshev();
	//((NUM::Integrator_R_R_GaussCebyshev*)intA)->nintervals=4;
	//((NUM::Integrator_R_R_GaussCebyshev*)intA)->order=5;
	NUM::Integrator_R_R *intA=new NUM::Integrator_R_R_Romberg(1e-5,5);
	NUM::Integrator_R_R *intB=new NUM::Integrator_R_R_GaussLegendre();
	((NUM::Integrator_R_R_GaussLegendre*)intB)->nintervals=4;
	
	for(int i=0; i<N; i++)
	{
		funcA.which=funcB.which=i;
		funcA.ncalls=funcB.ncalls=0;
		double IA=intA->Int(funcA,a[i],b[i]);
		double IB=intB->Int(funcB,a[i],b[i]);
		printf(" %d> A: I=%.10g, n=%d  \tB: I=%.10g, n=%d  \tdelta=%g\n",
			i,IA,funcA.ncalls,IB,funcB.ncalls,fabs(IB-IA));
	}
	
	delete intA;
	delete intB;
	
	return(0);
}
