/*
 * genpoly.cpp
 * 
 * Polinon creation utility. Algorithm works well in my tests 
 * (but is not the best one ever invented). 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>

// gcc genpoly.cpp -o genpoly -lm -fno-rtti -fno-exceptions -O2

class GenPoly
{
	private:
		int n;  // number of infos and equations; 
		int iidx;
		struct Equation
		{
			double *f;  // size: n
			double r;
		} *eq;
		
	public:
		// poly_n is the degree of the polynom. 
		// n (the number of infos) is poly_n+1. 
		// (make sure n>=0)
		GenPoly(int poly_n);
		~GenPoly();
		
		// Return value: 
		//  0 -> success
		// -1 -> too much info (max n infos)
		// -2 -> a<0 or a>=n
		int AddInfo(double px,double py,int a);
		
		// Actally generate the polynom (after having called AddInfo())
		// coeff must be an array capable of storing n=poly_n+1 
		// double values. 
		// Return value: 
		//  0 -> OK
		// -1 -> too few information (need n infos)
		//  1 -> matrix inversion errors (illegal info)
		int Generate(double *coeff);
};

#define SUB(array,C,c,r)  array[c*C+r]

// rm must be set up to the identity matrix. 
// rm and m are n*n-matrices. 
// The content of m is changed. 
static void InvertMatrix(double *rm,double *m,int n)
{
	// Implementing the Gauss-Jordan algorithm. 
	// m is the identiy matrix. 
	for(int k=0; k<n; k++)
	{
		double tmp=SUB(m,n,k,k);
		for(int j=0; j<n; j++)
		{
			SUB(m, n,j,k)/=tmp;
			SUB(rm,n,j,k)/=tmp;
		}
		for(int i=0; i<n; i++)  if(i!=k)
		{
			double tmp=SUB(m,n,k,i);
			for(int j=0; j<n; j++)
			{
				SUB(m, n,j,i) -= tmp * SUB(m, n,j,k);
				SUB(rm,n,j,i) -= tmp * SUB(rm,n,j,k);
			}
		}
	}
}


// calculates (n! / d!)
static inline double fac_frac(int n,int d)
{
	double res=1.0;
	for(int i=d+1; i<=n; i++)
	{  res*=double(i);  }
	return(res);
}


// Mult n*n-matrix m with n-vector v; result stored in n-vector rv. 
static void MatrixMulVect(double *rv,double *m,double *v,int n)
{
	for(int r=0; r<n; r++)
	{
		double s=0.0;
		for(int c=0; c<n; c++)
		{  s += SUB(m,n,c,r) * v[c];  }
		rv[r]=s;
	}
}


static void *Malloc(size_t size)
{
	void *ptr=malloc(size);
	if(!ptr)
	{  fprintf(stderr,"malloc() failed\n");  exit(1);  }
	return(ptr);
}


int GenPoly::AddInfo(double px,double py,int a)
{
	if(iidx>=n)  return(-1);
	if(a<0 || a>=n)  return(-2);
	
	Equation *e=&eq[iidx];
	int i=0;
	for(; i<a; i++)
	{  e->f[i]=0.0;  }
	for(; i<n; i++)
	{  e->f[i]=fac_frac(i,i-a)*pow(px,double(i-a));  }
	e->r=py;
	++iidx;
	
	return(0);
}

int GenPoly::Generate(double *coeff)
{
	if(iidx<n)  return(-1);
	
	// This represents the main matrix: 
	double mat[n][n];
	double inv_mat[n][n];
	// Result and coefficient vector: 
	double res[n];
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			mat[i][j]=eq[j].f[i];
			inv_mat[i][j]=((i==j) ? 1.0 : 0.0);  // identity matrix
		}
		res[i]=eq[i].r;
		coeff[i]=0.0;  // just set to some sane value
	}
	
	#if 0
	for(int r=0; r<n; r++)
	{
		for(int c=0; c<n; c++)
			printf("%f ",mat[c][r]);
		printf("= %f\n",res[r]);
	}
	#endif
	
	// Now we have to solve the equation 
	// coeff * mat = res
	// with coeff being unknown. 
	// I use the Gauss-Jordan algorithm to invert the matrix: 
	// inv_mat = invert(mat)
	// Then we can calculate coeff: 
	// coeff = inv_mat * res
	
	// Matrix inversion: 
	{
		double tmp[n][n];
		for(int i=0; i<n; i++)  for(int j=0; j<n; j++)  tmp[i][j]=mat[i][j];
		InvertMatrix(inv_mat[0],tmp[0],n);
	}
	
	// Check if inversion went okay: 
	for(int i=0; i<n; i++)
	{
		for(int j=0; j<n; j++)
		{
			if(!finite(inv_mat[i][j]))
			{  return(1);  }
		}
	}
	
	// Matrix-vector multiplication: 
	MatrixMulVect(coeff,inv_mat[0],res,n);
	
	#if 0
	// Check...
	double tmp[n];
	MatrixMulVect(tmp,mat[0],coeff,n);
	for(int i=0; i<n; i++)
	{
		if(fabs(tmp[i]-res[i])>0.00001)
		{  fprintf(stderr,"oops...\n");  }
	}
	#endif
	
	return(0);
}


GenPoly::GenPoly(int _n)
{
	n=_n+1;
	iidx=0;
	eq=(Equation*)Malloc(sizeof(Equation)*n);
	for(int i=0; i<n; i++)
	{  eq[i].f=(double*)Malloc(sizeof(double)*n);  }
}

GenPoly::~GenPoly()
{
	for(int i=0; i<n; i++)
	{  if(eq[i].f)  free(eq[i].f);  }
	free(eq);  eq=NULL;
}

// Return 1 on error
static int ParseDouble(char **str,double *rv)
{
	int errors=0;
	char *end;
	*rv=strtod(*str,&end);
	if(end==*str)
	{  fprintf(stderr,"Float parse error.\n");  ++errors;  }
	*str=end;
	return(errors);
}
static int ParseInt(char **str,int *rv)
{
	int errors=0;
	char *end;
	*rv=int(strtol(*str,&end,10));
	if(end==*str)
	{  fprintf(stderr,"Integer parse error.\n");  ++errors;  }
	*str=end;
	return(errors);
}


// genpoly [-options] [info...]
// INFO: 
//  specify point: p(x,y) 
//  specify ascent: a(x,dy)
//  specify extremum: e(x), e(x,y)
//  specify wending point: w(x), w(x,y)
//  rest: i(x,y,a) (a-th derivation at position x is y)

static void PrintHelp()
{
	printf("Usage: genpoly [--help] [-acpq] [info...]\n"
	  "  -c  print coefficients only\n"
	  "  -p  use pow(x,y) instead of x^y in output\n"
	  "  -a  use x**y instead of x^y in output\n"
	  "  -q  quiet operation\n"
	  "polynom info:\n"
	  "  p(x,y)   point at x,y\n"
	  "  a(x,m)   ascent m at position x\n"
	  "  e(x)     extremum at specified x-position\n"
	  "  e(x,y)   extremum at position x,y\n"
	  "  w(x)     wending point at specified x-position\n"
	  "  w(x,y)   wending point at position x,y\n"
	  "  i(x,y,a) a-th derivation of polynom at pos x is y\n");
}

struct Info
{
	double px,py;
	int a;
};
int main(int argc,char **arg)
{
	int info_dim=0;
	int quiet=0;
	int pmode='\0';
	for(int i=1; i<argc; i++)
	{
		if(*arg[i]=='-')
		{
			if(!strcmp(arg[i],"--help"))
			{  PrintHelp();  return(0);  }
			else
			{
				for(char *c=arg[i]+1; *c; c++)
				{
					switch(*c)
					{
						case 'q': ++quiet; break;
						case 'c': // fallthrough
						case 'a':
						case 'p': pmode=*c; break;
						default: fprintf(stderr,"Illegal option \'%c\' "
							"in arg \"%s\".\n",*c,arg[i]);  exit(1);
					}
				}
			}
		}
		else
		{  info_dim+=2;  }
	}
	
	Info info[info_dim];
	int ninfo=0;
	int errors=0;
	
	for(int i=1; i<argc; i++)
	{
		if(*arg[i]=='-')  continue;
		char *str=arg[i];
		assert(ninfo<info_dim);
		Info *in=&info[ninfo];
		char itype=*str;
		int maxprm=0,minprm=0,nprm=0;
		if(!itype)  goto error;
		switch(itype)
		{
			case 'p': case 'P': maxprm=2; minprm=2; break;
			case 'a': case 'A': maxprm=2; minprm=2; break;
			case 'e': case 'E': maxprm=2; minprm=1; break;
			case 'w': case 'W': maxprm=2; minprm=1; break;
			case 'i': case 'I': maxprm=3; minprm=3; break;
			default: fprintf(stderr,"Illegal info type \'%c\'.\n",
				itype);  goto err_cont;
		}
		++str;
		if(*str!='(')  goto error;
		++str;
		if(ParseDouble(&str,&in->px))  goto err_cont;
		++nprm;
		if(*str==',')
		{
			if(nprm>=maxprm)
			{  fprintf(stderr,"Too many params for info type \'%c\'.\n",
				itype);  goto err_cont;  }
			++str;
			if(ParseDouble(&str,&in->py))  goto err_cont;
			++nprm;
		}
		if(*str==',')
		{
			if(nprm>=maxprm)
			{  fprintf(stderr,"Too many params for info type \'%c\'.\n",
				itype);  goto err_cont;  }
			++str;
			if(ParseInt(&str,&in->a))  continue;
			++nprm;
		}
		if(*str!=')')  goto error;
		++str;
		if(*str)  goto error;
		if(nprm<minprm)
		{  fprintf(stderr,"Too few params for info type \'%c\'.\n",
			itype);  goto err_cont;  }
		
		switch(itype)
		{
			case 'p': case 'P': in->a=0; ++ninfo; break;
			case 'a': case 'A': in->a=1; ++ninfo; break;
			case 'e': case 'E':
			case 'w': case 'W':
			{
				in->a=((itype=='e' || itype=='E') ? 1 : 2);
				double tmp=in->py;
				in->py=0.0;
				if(nprm==2)  // coos specified -> extra info
				{
					++ninfo;
					assert(ninfo<info_dim);
					in[1].px=in->px;
					in[1].py=tmp;
					in[1].a=0;
				}
				++ninfo;
			}  break;
			case 'i': case 'I': ++ninfo; break;
		}
		
		continue;
		error:  fprintf(stderr,"Illegal info spec \"%s\".\n",arg[i]);
		err_cont:;
		++errors;
	}
	
	if(errors)  exit(1);
	
	if(!ninfo)
	{  fprintf(stderr,"No info specified. Try genpoly --help.\n");
		exit(1);  }
	if(ninfo<2)
	{  fprintf(stderr,"Need at least 2 infos.\n");  exit(1);  }
	
	int poly_n=ninfo-1;
	
	if(!quiet)
	{  printf("Generating polynom of degree %d.\n",poly_n);  }
	
	GenPoly gp(poly_n);
	
	for(int i=0; i<ninfo; i++)
	{
		int rv=gp.AddInfo(info[i].px,info[i].py,info[i].a);
		switch(rv)
		{
			case 0:  break;
			case -2:  fprintf(stderr,"Cannot use derivation %d "
				"with a polynom of degree %d.\n",info[i].a,poly_n);
				exit(1);
			case -1:  // fallthrough
			default:  fprintf(stderr,"Oops: internal error AddInfo()=%d\n",
				rv);  exit(1);  break;
		}
	}
	
	double cf[poly_n+1];
	
	int rv=gp.Generate(cf);
	switch(rv)
	{
		case 0:  break;
		case 1:  fprintf(stderr,"You specified illegal information.\n");
			exit(1);  break;
		case -1:   // fallthrough
		default:  fprintf(stderr,"Oops: internal error Generate()=%d\n",
			rv);  exit(1);  break;
	}
	
	char tmp[256];
	int first=1;
	for(int i=poly_n; i>=0; i--)
	{
		char *s=tmp,*se=tmp+255;  // yes, 255
		if(pmode=='c')
		{
			if(!first)  *(s++)=' ';
			snprintf(s,se-s,"%g",cf[i]);  s+=strlen(s);
		}
		else
		{
			if(fabs(cf[i])<=0.0000000001)  continue;
			if(cf[i]<0.0)  *(s++)='-';
			else if(!first)  *(s++)='+';
			double dv=fabs(cf[i]);
			if(fabs(dv-1.0)<=0.0000000001)   // dv==1
			{
				if(!i)
				{  snprintf(s,se-s,"%g",dv);  s+=strlen(s);  }
			}
			else  // dv!=1
			{
				snprintf(s,se-s,"%g",dv);  s+=strlen(s);
				if(i)
				{  snprintf(s,se-s,"*");  s+=strlen(s);  }
			}
			if(i==1)
			{  snprintf(s,se-s,"x");  s+=strlen(s);  }
			else if(i>1)
			{
				if(pmode=='p')
				{  snprintf(s,se-s,"pow(x,%d)",i);  s+=strlen(s);  }
				else
				{  snprintf(s,se-s,"x%s%d",
					(pmode=='a') ? "**" : "^",i);  s+=strlen(s);  }
			}
		}
		
		*s='\0';
		printf("%s",tmp);
		first=0;
	}
	printf("\n");
	
	return(0);
}
