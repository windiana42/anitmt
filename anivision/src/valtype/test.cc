/*
 * valtype/test.cc
 * 
 * Value test code. 
 * This is part of the AniVision project. 
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

#include <stdio.h>
#include <stdlib.h>
#include "valtypes.h"

static void PrintSizes()
{
	printf("sizeof(ValueBase)=%d\n",sizeof(ValueBase));
	printf("sizeof(Integer)=%d\n",sizeof(Integer));
	printf("sizeof(Scalar)=%d\n",sizeof(Scalar));
	printf("sizeof(Range)=%d\n",sizeof(Range));
	printf("sizeof(Vector)=%d\n",sizeof(Vector));
	printf("sizeof(Matrix)=%d\n",sizeof(Matrix));
	printf("sizeof(String)=%d\n",sizeof(String));
}

static void PrintVector(const Vector &v)
{
	printf("<");
	for(int i=0; i<v.dim(); i++)
	{  printf("%g%s",v[i],i+1<v.dim() ? "," : "");  }
	printf(">");
}


static void MandelTest()
{
	// Just for fun: Compute Mandelbrot fractal using the isomorphismus 
	// between complex values and matrices: 
	//                 /  a   b \      ...
	//  a + b*i  <==>  |        |
	//                 \ -b   a /
	// The difference compared with pure FP arith is about factor 50++ 
	// when using standard LMalloc(). One time-critical part is 
	// memory (de)allocation. 
	int w=100,h=66;
	for(int y=0; y<h; y++)
	{
		for(int x=0; x<w; x++)
		{
			#if 0
			// The Matrix method (time: 50)
			Matrix mx(Matrix::Null,2,2);
			Matrix mc(Matrix::NoInit,2,2);
			mc[0][0]=mc[1][1]=(x-w/2)/double(w/2);
			mc[0][1]=(y-h/2)/double(h/2);
			mc[1][0]=-mc[0][1];
			int i;
			for(i=0; i<256; i++)
			{
				mx=mx*mx+mc;
				if(mx[0][0]*mx[0][0]+mx[0][1]*mx[0][1]>4.0) break;
			}
			#elif 0
			// Use Vector... (time: 25)
			Vector vx(0.0,0.0);
			Vector vc((x-w/2)/double(w/2),(y-h/2)/double(h/2));
			int i;
			for(i=0; i<256; i++)
			{
				Vector _x(vx[0]*vx[0]-vx[1]*vx[1]+vc[0]);
				vx[1]=2.0*vx[0]*vx[1]+vc[1];
				vx[0]=_x[0];
				if(vx[0]*vx[0]+vx[1]*vx[1]>4.0) break;
			}
			#elif 1
			// Use Scalar for the arith: (time: 2)
			Scalar xx=0.0,yy=0.0;
			Scalar a=(x-w/2)/double(w/2),b=(y-h/2)/double(h/2);
			int i;
			for(i=0; i<256; i++)
			{
				Scalar _x=xx*xx-yy*yy+a;
				yy=2.0*xx*yy+b;
				xx=_x;
				if(xx*xx+yy*yy>4.0) break;
			}
			#else
			// Pure floating point: (time: 1)
			double xx=0.0,yy=0.0;
			double a=(x-w/2)/double(w/2),b=(y-h/2)/double(h/2);
			int i;
			for(i=0; i<256; i++)
			{
				double _x=xx*xx-yy*yy+a;
				yy=2.0*xx*yy+b;
				xx=_x;
				if(xx*xx+yy*yy>4.0) break;
			}
			#endif
		}
		fprintf(stderr,"\b\b\b\b\b\b\b\b%4d",y);
	}
}


extern String getstring();
static void BUGTEST()
{
	fprintf(stderr,"null=%s\n",getstring().str());
}
String getstring()
{  return String();  }


char *prg_name="test-val";

extern void ValAllocPrintStat();

int main()
{
	PrintSizes();
	
	{
	
	BUGTEST();
	
	Integer iv0(1),iv1=10;
	Scalar sc0=1.3;
	printf("iv0=%d, iv1=%d,  sc0=%g\n",iv0.val(),iv1.val(),sc0.val());
	
	Vector v3(1.1,2.3,4.8);
	Vector vb;
	vb=v3;
	v3=(v3+vb+Vector(1,2,3))/iv1*0.1;
	v3[1]=-11;
	PrintVector(v3);
	
	Value mv;
	mv=v3;
	
	{
		Value s=Integer(3);
		Scalar ss(s.GetScalar());
		fprintf(stderr,"<%g>\n",ss.val());
	}
	
	{
		String s1("hallo");
		String s2=s1;
		printf("%s %s\n",s1.str(),s2.str());
		String s3=s1+s2;
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
		s3.trunc(8);
		s2.trunc(2);
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
		s2=s1;
		s2.skip(2);
		s3.skip(1);
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
		s2+=s3;
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
		s2+="xxx";
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
		s2=s1;
		s2="uru"+s1+"oro";
		s3=s2+"ullo";
		printf(">%s< >%s< >%s<\n",s1.str(),s2.str(),s3.str());
	}
	
	MandelTest();
	
	printf("\n");
	}
	
	ValAllocPrintStat();
	return(0);
}
