/*
 * valtype/tostring.cc
 * 
 * Generate string representation of values. 
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

#include "valtypes.h"


String Integer::ToString() const
{
	String s;
	s.sprintf("%d",x);
	return(s);
}


String Scalar::ToString() const
{
	String s;
	s.sprintf("%g",x);
	return(s);
}


String Range::ToString() const
{
	String s;
	switch(valid)
	{
		case RInvalid:  s="?..?";  break;
		case RValidA:   s.sprintf("%g..?",a);  break;
		case RValidB:   s.sprintf("?..%g",b);  break;
		case RValidAB:  s.sprintf("%g..%g",a,b);  break;
	}
	return(s);
}


String Vector::ToString() const
{
	// We pre-allocate 10 bytes per entry to avoid 
	// reallocations. 
	String s(n*10+1);
	s="<";
	char tmp[32];
	for(int i=0; i<n; i++)
	{
		snprintf(tmp,32,"%g%s",xp[i],(i+1<n) ? "," : "");
		s+=tmp;
	}
	s+=">";
	return(s);
}


String Matrix::ToString() const
{
	// We pre-allocate 10 bytes per entry to avoid 
	// reallocations. 
	String s(size()*10+1);
	char tmp[32];
	s="[";
	for(short int r=0; r<nr; r++)
	{
		s+="[";
		for(short int c=0; c<nc; c++)
		{
			snprintf(tmp,32,"%g%s",x(r,c),(c+1<nc) ? "," : "");
			s+=tmp;
		}
		s+=(r+1<nr) ? "]," : "]";
	}
	s+="]";
	return(s);
}


static const char *transstr(char c)
{
	switch(c)
	{
		case '\"':  return("\\\"");
		case '\'':  return("\\\'");
		case '\\':  return("\\\\");
		case 127:   return("\\x7f");
		case '\a':  return("\\a");
		case '\b':  return("\\b");
		case '\f':  return("\\f");
		case '\n':  return("\\n");
		case '\r':  return("\\r");
		case '\t':  return("\\t");
		case '\v':  return("\\v");
		//default: below
	}
	static char tmp[5];
	snprintf(tmp,5,"\\x%02x",(unsigned int)c);
	return(tmp);
}

String String::ToString() const
{
	// Prealloc 10% too much: 
	String s(len()+len()/10+3);
	s="\"";
	const char *b=str();
	for(const char *c=b; *c; c++)
	{
		if(*c>=32 && *c!=127 && *c!='\"' && *c!='\'' && *c!='\\') continue;
		if(c>b) s.append(b,c-b);
		s.append(transstr(*c));
		b=c+1;
	}
	if(*b) s.append(b);
	s.append("\"");
	return(s);
}
