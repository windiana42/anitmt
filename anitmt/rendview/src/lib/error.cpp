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
#include <stdio.h>
#include <stdarg.h>


int do_colored_output_stdout=0;
int do_colored_output_stderr=0;

void Error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(do_colored_output_stderr)
	{  fprintf(stderr,"\33[1;31m");  }
	vfprintf(stderr,fmt,ap);
	if(do_colored_output_stderr)
	{  fprintf(stderr,"\33[00m");  }
	va_end(ap);
}

void Warning(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(do_colored_output_stderr)
	{  fprintf(stderr,"\33[0;31m");  }
	vfprintf(stderr,fmt,ap);
	if(do_colored_output_stderr)
	{  fprintf(stderr,"\33[00m");  }
	va_end(ap);
}

void Verbose(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	if(do_colored_output_stdout)
	{  fprintf(stdout,"\33[0;34m");  }
	vfprintf(stdout,fmt,ap);
	if(do_colored_output_stdout)
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
	if(do_colored_output_stdout)
	{
		//fprintf(stdout,"\33[0;42m");   // <-- green background
		fprintf(stdout,"\33[1;34m");   // <-- bold blue 
	}
	vfprintf(stdout,fmt,ap);
	if(do_colored_output_stdout)
	{  fprintf(stdout,"\33[00m\n");  }
	va_end(ap);
	fflush(stdout);
}

