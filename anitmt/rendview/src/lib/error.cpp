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
#ifdef DEBUG
		| VERBOSE_DBG    // comment out
		| VERBOSE_DBGV   // comment out
#endif
#warning !!! REMOVE THE VERBOSE_0 and fix all errors which show up then. !!!
		| VERBOSE_0
		;
	
	// These work on my Linux XTerm and (not that goog) Linux console. 
	rv_oparams.console_red_start="\33[0;31m";
	rv_oparams.console_red_end="\33[00m";
	rv_oparams.console_Red_start="\33[1;31m";
	rv_oparams.console_Red_end="\33[00m";
	rv_oparams.console_blue_start="\33[0;34m";
	rv_oparams.console_blue_end="\33[00m";
	rv_oparams.console_Blue_start="\33[1;34m";
	rv_oparams.console_Blue_end="\33[00m";
	
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


static inline int _vaError(const char *fmt,va_list ap)
{
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,rv_oparams.console_Red_start);  }
	int rv=vfprintf(stderr,fmt,ap);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,rv_oparams.console_Red_end);  }
	return(rv);
}
int vaError(const char *fmt,va_list ap)
{  return(_vaError(fmt,ap));  }

static inline int _vaWarning(const char *fmt,va_list ap)
{
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,rv_oparams.console_red_start);  }
	int rv=vfprintf(stderr,fmt,ap);
	if(rv_oparams.enable_color_stderr)
	{  fprintf(stderr,rv_oparams.console_red_end);  }
	return(rv);
}
int vaWarning(const char *fmt,va_list ap)
{  return(_vaWarning(fmt,ap));  }


int Error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=_vaError(fmt,ap);
	va_end(ap);
	return(rv);
}

int Warning(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv=_vaWarning(fmt,ap);
	va_end(ap);
	return(rv);
}

int _Verbose(int vspec,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	bool is_debug=(vspec & (VERBOSE_DBG|VERBOSE_DBGV));
	if(rv_oparams.enable_color_stdout && !is_debug)
	{  fprintf(stdout,rv_oparams.console_blue_start);  }
	int rv=vfprintf(stdout,fmt,ap);
	if(rv_oparams.enable_color_stdout && !is_debug)
	{  fprintf(stdout,rv_oparams.console_blue_end);  }
	va_end(ap);
	fflush(stdout);
	return(rv);
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
		fprintf(stdout,rv_oparams.console_Blue_start);   // <-- bold blue 
	}
	vfprintf(stdout,fmt,ap);
	if(rv_oparams.enable_color_stdout)
	{  fprintf(stdout,"%s\n",rv_oparams.console_Blue_end);  }
	else
	{  fprintf(stdout,"\n");  }
	va_end(ap);
	fflush(stdout);
}

