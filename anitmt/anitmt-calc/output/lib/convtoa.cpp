/*
 * convtoa.cpp 
 * Convert to ASCII 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs to > wwieser -a- gmx -*- de <
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   May 2001   started writing
 *
 */

#include <math.h>

#include "convtoa.hpp"

#include <val/val.hpp>


namespace output_io
{

inline int pow10i(int e)
{
	int r=1;
	while(e>0)
	{  r*=10;  --e;  }
	return(r);
}


// Writes the integer x in decimal notation to d which is assumed to be 
// long enough. 
// Return value: next free char; not '\0'-terminated. 
// On a PII-350, this function needs 38 seconds to write all integers 
// from 0...-1000000 (sprint(,"%d",i) needs 150 seconds). 
// This is nearly 4 times as fast as sprintf. 
char *Int2StrDec(char *d,int x)
{
	if(!x)
	{  *(d++)='0';  return(d);  }
	if(x<0)
	{  *(d++)='-';  x=-x;  }
	static const int int_maxdigit=int(log10(pow(2.0,double(sizeof(int)*8))));
	static const int int_startdiv=pow10i(int_maxdigit);
	if(x<int_startdiv)
	{
		int div=10;
		while(div<=x)
		{  div*=10;  ++d;  }
		div/=10;
	}
	else
	{  d+=int_maxdigit;  }
	char *end=++d;
	for(;;)
	{
		int r=x%10;
		*(--d)='0'+char(r);
		x/=10;
		if(!x)  break;
	}
	return(end);
}


#if 0  /* slower method, still faster than sprintf("%d",x); */
char *Int2StrDec(char *d,int x)
{
	if(!x)
	{  *(d++)='0';  return(d);  }
	if(x<0)
	{  *(d++)='-';  x=-x;  }
	static const int int_maxdigit=int(log10(pow(2.0,double(sizeof(int)*8))));
	static const int int_startdiv=pow10i(int_maxdigit);
	int div=int_startdiv;
	int r=x/div;
	while(!r)
	{  div/=10;  r=x/div;  }
	for(;;)
	{
		*(d++)='0'+char(r);
		x-=r*div;
		div/=10;
		if(!div)  break;
		r=x/div;
	}
	return(d);
}
#endif


// Writes the double value x in 1.1111e11-notation to d. 
// digits specifies the precision (the number of digits after the `.'). 
// d is assumed to be large enough. 
// NaN, -Inf and +Inf are handeled without problems. 
// The passed double value is rounded correctly. 
// Return value: next free char; not '\0'-terminated. 
// On my PII-350, Double2Str(,cal,5) is four times as fast as 
// sprintf(,"%.5e",val). 
// NOTE: a double with n digits needs up to at least (n+6) chars 
//       (1.nnnnne111)
char *Double2Str(char *d,double x,int ndigits)
{
	if(!finite(x))
	{
		if(isnan(x))
		{
			*(d++)='N';  *(d++)='a';  *(d++)='N';
			return(d);
		}
		int i=isinf(x);
		if(i<0)
		{  *(d++)='-';  }
		if(i)
		{
			*(d++)='I';  *(d++)='n';  *(d++)='f';
			return(d);
		}
		*(d++)='?';  *(d++)='?';  *(d++)='?';
		return(d);
	}
	
	if(x<0.0)
	{  *(d++)='-';  x=-x;  }
	int exp=0;
	if(x>=1.0e-100)
	{
		exp=int(log10(x));
		if(x<1.0)  --exp;
		x*=pow(10.0,double(-exp));
		#if 1   /* do rounding */
		x+=0.4999999999*pow(10.0,double(-ndigits));
		if(x>=10.0)
		{  x/=10.0;  ++exp;  }
		#endif
	}
	{
		int tmp=int(x);
		*(d++)='0'+char(tmp);
		x=(x-double(tmp))*10.0;
	}
	if(ndigits)
	{
		*(d++)='.';
		do
		{
			int tmp=int(x);
			*(d++)='0'+char(tmp);
			x=(x-double(tmp))*10.0;
		}
		while(--ndigits);
	}
	if(exp)
	{
		*(d++)='e';
		d=Int2StrDec(d,exp);
	}
	return(d);
}


// Converts a vector as ``<x,y,z>'' using Double2Str(). 
char *Vector2Str(char *d,const values::Vector &v,int ndigits)
{
	*(d++)='<';
	d=Double2Str(d,v[0],ndigits);
	*(d++)=',';
	d=Double2Str(d,v[1],ndigits);
	*(d++)=',';
	d=Double2Str(d,v[2],ndigits);
	*(d++)='>';
	return(d);
}


static const int i_ncode=10+26+26;
static const unsigned int ui_ncode=i_ncode;
static const char i_code[]=
	"0123456789"
	"abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// Really fast: translate an integer into some number-and-digit 
// ASCII output which is unique for each integer. 
// The first char is either `b' (negative) or `a' (positive). 
// (NOTE: The most significant digits are the last ones.)
// Return value: next free char; not '\0'-terminated. 
char *IntEncode(char *d,int x)
{
	if(x<0)  {  *(d++)='b';  x=-x;  }
	else     {  *(d++)='a';  }
	while(x)
	{
		*(d++)=i_code[x%i_ncode];
		x/=i_ncode;
	}
	return(d);
}

// Even faster than the version for signed integers. 
// (NOTE: The most significant digits are the last ones.)
// Return value: next free char; not '\0'-terminated. 
char *IntEncode(char *d,unsigned int x)
{
	do
	{
		*(d++)=i_code[x%ui_ncode];
		x/=ui_ncode;
	}
	while(x);
	return(d);
}


static const char *_obj_ident="_aniTMT_";

// Store an object identifier name: 
// ``_aniTMT_xxx_'' where xxx is the passed id. 
// Reserve 64 bytes for the string. 
char *ObjectIdent(char *d,unsigned int x)
{
	for(const char *s=_obj_ident; *s; s++)
	{  *(d++)=*s;  }
	d=IntEncode(d,x);
	*(d++)='_';
	return(d);
}


}  // namespace end 


#if 0
#include <stdio.h>
int main()
{
	char buf[1024];
	char *end=buf;
	
	#if 1
	for(int i=0; i<=10000000; i++)
	{
		end=buf;
		end=Int2StrDec(end,-i);  *(end++)='<';
		//end=IntEncode(end,(unsigned int)i);  *(end++)='<';
		//sprintf(buf,"%d",i);
		
		//*end='\0';  puts(buf);
	}
	#endif
	#if 0
	for(int i=0; i<=100000; i++)
	{
		double val=pow(i,i%100);
		end=buf;
		end=Double2Str(end,val,5);  *(end++)='<';
		//sprintf(buf,"%.5e",val);
	}
	#endif
	*(end++)='\n';
	*end='\0';
	
	puts(buf);
	
	return(0);
}
#endif
