/*
 * consfunc.cc
 * 
 * Implementation of console print function redirector. 
 * 
 * Copyright (c) 2003 -- 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#define HLIB_IN_HLIB 1
#include <hlib/misc/prototypes.h>

#include <string.h>

#include "parmanager.h"


namespace par
{

int ParameterManager::cerr_printf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv = cerr_printf_func ? 
		((*cerr_printf_func)(fmt,ap)) : vfprintf(stderr,fmt,ap);
	va_end(ap);
	return(rv);
}

int ParameterManager::cwarn_printf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv = cwarn_printf_func ? 
		((*cwarn_printf_func)(fmt,ap)) : vfprintf(stderr,fmt,ap);
	va_end(ap);
	return(rv);
}

int ParameterManager::cinfo_printf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	int rv = cinfo_printf_func ? 
		((*cinfo_printf_func)(fmt,ap)) : vfprintf(stdout,fmt,ap);
	va_end(ap);
	return(rv);
}

}  // namespace end 
