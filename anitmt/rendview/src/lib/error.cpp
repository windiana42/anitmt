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
#include <ctype.h>


// Global params: 
RVOutputParams rv_oparams;

const _ConstStrings cstrings=
{
	/*allocfail:*/ "allocation failure"
};


int InitRVOutputParams(int &argc,char **argv,char ** /*envp*/)
{
	rv_oparams.enable_color_stdout=0;
	rv_oparams.enable_color_stderr=0;
	// First, set up defaults: 
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
	
	// Check if we are on a terminal and enable color output by default:
	if(GetTerminalSize(fileno(stdout),NULL,NULL)==0)
	{  rv_oparams.enable_color_stdout=1;  }
	if(GetTerminalSize(fileno(stderr),NULL,NULL)==0)
	{  rv_oparams.enable_color_stderr=1;  }
	
	// These work on my Linux XTerm and (not that good) on Linux console. 
	rv_oparams.console_red_start="\33[0;31m";
	rv_oparams.console_red_end="\33[00m";
	rv_oparams.console_Red_start="\33[1;31m";
	rv_oparams.console_Red_end="\33[00m";
	rv_oparams.console_blue_start="\33[0;34m";
	rv_oparams.console_blue_end="\33[00m";
	rv_oparams.console_Blue_start="\33[1;34m";
	rv_oparams.console_Blue_end="\33[00m";
	
	// Check cmd line: 
	int errors=0;
	for(int i=1; i<argc; i++)
	{
		int remove_arg=CheckParseOutputParam(
			argv[i],(i+1<argc) ? argv[i+1] : NULL,&errors,i);
		
		// We actually remove the arg from the cmd line...
		// That's a bit hacky but you won't notice in /proc because I 
		// only change the argv pointers. 
		if(remove_arg)
		{
			for(int j=i+remove_arg; j<argc; j++)
			{  argv[j-remove_arg]=argv[j];  }
			i-=remove_arg;
			argc-=remove_arg;
		}
	}
	
	return(errors ? 1 : 0);
}

// *errors: error counter
// Return value: 
//  0 -> no output param
//  1 -> arg0 was used
//  2 -> arg0 and arg1 were used
int CheckParseOutputParam(const char *arg0,const char *arg1,
	int *errors,int argidx_for_error)
{
	int color_arg=0;
	
	const char *a=arg0;
	bool optdash=(*a=='-');
	if(*a!='-' && !strchr(a,'='))  return(0);
	
	while(*a=='-')  ++a;
	int remove_arg=0;
	if(optdash && *a=='n' && 
		(!strcmp(a,"nocolor") || !strcmp(a,"no-color")) )
	{
		color_arg=-1;
		remove_arg=1;
	}
	else if(optdash && *a=='c' && !strcmp(a,"color"))
	{
		if(!color_arg)  color_arg=+1;
		remove_arg=1;
	}
	else if(*a=='v' && !strncmp(a,"verbose",7))
	{
		const char *parseme=NULL;
		if(a[7]=='=')
		{
			parseme=&a[8];
			remove_arg=1;
		}
		else if(optdash && a[7]=='\0')
		{
			if(arg1 && *arg1!='-' && !strchr(arg1,'=') && !strchr(arg1,'/'))
			{
				parseme=arg1;
				remove_arg=2;
			}
			else
			{
				fprintf(stderr,"%s: [cmdline]:%d: required arg for "
					"-verbose missing.\n",prg_name,argidx_for_error);
				++(*errors);
			}
		}
		// else: this is not a verbose option but something else 
		// (lile --verboseblah)
		if(parseme)
		{
			char prefix[32];
			snprintf(prefix,32,"[cmdline]:%d",argidx_for_error);
			(*errors)+=ParseVerbosSpec(parseme,prefix);
		}
	}
	
	// See if we have to force color on or off. 
	if(color_arg==1)
	{
		rv_oparams.enable_color_stdout=1;
		rv_oparams.enable_color_stderr=1;
	}
	else if(color_arg==-1)
	{
		rv_oparams.enable_color_stdout=0;
		rv_oparams.enable_color_stderr=0;
	}
	
	return(remove_arg);
}


static const struct 
{
	const char *name;
	int namelen;
	int flag;
} _vdesc[]=
{
	{ "misc",  4, VERBOSE_MiscInfo },
	{ "tdi",   3, VERBOSE_TDI },
	{ "tdr",   3, VERBOSE_TDR },
	{ "tsi",   3, VERBOSE_TSI },
	{ "tsp",   3, VERBOSE_TSP },
	{ "tsr0",  4, VERBOSE_TSR0 },
	{ "tsr1",  4, VERBOSE_TSR1 },
	{ "tslr",  4, VERBOSE_TSLR },
	{ "tsllr", 5, VERBOSE_TSLLR },
	{ "dbg",   3, VERBOSE_DBG },
	{ "dbgv",  4, VERBOSE_DBGV },
	{ "all",   3, _VERBOSE_ALL },
	{ NULL,-1,0 }  // <-- LAST entry
};

// String: {+/-}spec{+/-}spec
int ParseVerbosSpec(const char *str,const char *err_prefix)
{
	for(const char *c=str; *c; )
	{
		int mode=(*c=='+' ? (+1) : ( *c=='-' ? (-1) : 0 ));
		if(!mode)
		{
			print_error:;
			fprintf(stderr,"%s: %s: illegal verbose spec (-> \"%.*s\"). "
				"Try %s --help .\n",
				prg_name,err_prefix,10,c,prg_name);
			return(1);
		}
		++c;
		const char *start=c;
		while(isalnum(*c))  ++c;
		int namelen=c-start;
		for(int i=0; _vdesc[i].name; i++)
		{
			if(_vdesc[i].namelen!=namelen)  continue;
			if(strncasecmp(_vdesc[i].name,start,namelen))  continue;
			if(mode>0)
			{  rv_oparams.vlevel_field|=_vdesc[i].flag;  }
			else
			{  rv_oparams.vlevel_field&=~_vdesc[i].flag;  }
			goto found;
		}
		c=start;  // for error
		goto print_error;
		found:;
	}
	//fprintf(stderr,"verbose flags: 0x%x\n",rv_oparams.vlevel_field);
	return(0);
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

