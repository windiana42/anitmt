/*
 * error.cpp
 * 
 * Implementation of Error(), Warning(), Verbose(). 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "prototypes.hpp"

#if HAVE_STDARG_H
#  include <stdarg.h>
#endif
#include <string.h>


// Global params: 
RVOutputParams rv_oparams;

void InitRVOutputParams(int &argc,char **argv,char ** /*envp*/)
{
	// First, set up defaults: 
	rv_oparams.enable_color_stdout=0;
	rv_oparams.enable_color_stderr=0;
	rv_oparams.vlevel_field = 0
		// | VERBOSE_BasicInit
		| VERBOSE_MiscInfo
		| VERBOSE_TDI
		| VERBOSE_TDR
		| VERBOSE_TSI
		| VERBOSE_TSP
		//| VERBOSE_TSR0
		| VERBOSE_TSR1
		| VERBOSE_TSLR
		| VERBOSE_TSLLR
#warning !!! REMOVE THE VERBOSE_0 and fix all errors which show up then. !!!
		| VERBOSE_0
		;
	
	// Check cmd line: 
	int color_arg=0;
	for(int i=1; i<argc; i++)
	{
		const char *a=argv[i];
		if(*a!='-')  continue;
		++a;  while(*a=='-')  ++a;
		if(*a=='n' && !strcmp(a,"nocolor"))
		{
			color_arg=-1;
			for(int j=i+1; j<argc; j++)
			{  argv[j-1]=argv[j];  }
			--i; --argc;
		}
		else if(*a=='c' && !strcmp(a,"color"))
		{
			if(!color_arg)  color_arg=+1;
			for(int j=i+1; j<argc; j++)
			{  argv[j-1]=argv[j];  }
			--i; --argc;
		}
	}
	if(!color_arg)
	{
		if(GetTerminalSize(fileno(stdout),NULL,NULL)==0)
		{  rv_oparams.enable_color_stdout=1;  }
		if(GetTerminalSize(fileno(stderr),NULL,NULL)==0)
		{  rv_oparams.enable_color_stderr=1;  }
	}
	else if(color_arg==1)
	{
		rv_oparams.enable_color_stdout=1;
		rv_oparams.enable_color_stderr=1;
	}
}


void Error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,"\33[1;31m");  }
	vfprintf(stderr,fmt,ap);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,"\33[00m");  }
	va_end(ap);
}

void Warning(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,"\33[0;31m");  }
	vfprintf(stderr,fmt,ap);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,"\33[00m");  }
	va_end(ap);
}

void _Verbose(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(rv_oparams.enable_color_stdout)
	{  fprintf(stdout,"\33[0;34m");  }
	vfprintf(stdout,fmt,ap);
	if(rv_oparams.enable_color_stdout)
	{  fprintf(stdout,"\33[00m");  }
	va_end(ap);
	fflush(stdout);
}

void VerboseSpecial(const char *fmt,...)
{
	// NOTE: VerboseSpecial() adds a newline by itself because of 
	//       the green beackground...
	va_list ap;
	va_start(ap,fmt);
	if(rv_oparams.enable_color_stdout)
	{
		//fprintf(stdout,"\33[0;42m");   // <-- green background
		fprintf(stdout,"\33[1;34m");   // <-- bold blue 
	}
	vfprintf(stdout,fmt,ap);
	if(rv_oparams.enable_color_stdout)
	{  fprintf(stdout,"\33[00m\n");  }
	else
	{  fprintf(stdout,"\n");  }
	va_end(ap);
	fflush(stdout);
}

