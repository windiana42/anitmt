/*
 * hlib_id1.c
 *
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
#include <hlib/prototypes.h>


static void _SetSize(char *str,char id,int val)
{
	char *s0;
	char tmp[8];
	while(*str && *str!=id)  ++str;
	if(!(*str))  return;
	s0=str;
	do
	{  *(str++)=' ';  }
	while(*str==id);
	snprintf(tmp,8,"%d",val);
	if(str-s0<8)
	{  tmp[str-s0]='\0';  }
	strncpy(s0,tmp,strlen(tmp));
}


const char *HLIB_GetConfigString()
{
	static char str[]=  /* NON-const! */
		"hlib version " VERSION " (c) 1999--2004 by Wolfgang Wieser\n"
		"date: " HLIB_BUILD_DATE "\n"
		"build:  " HLIB_BUILD_SYSTEM "\n"
		"host:   " HLIB_HOST_SYSTEM "\n"
		"target: " HLIB_TARGET_SYSTEM "\n"
		"arch: " HLIB_BUILD_UNAME "\n"
		"size: size_t: AAA  ssize_t: BBB  void *: CCC  | float: GGG\n"
		"size: short: DDD   int: EEE      long: FFF    | double: HHH\n"

/*----------------------------------------------------------------------------*/
#if defined(__GNUC__)
		"GNU C/C++: yes"
#else
		"GNU C/C++: no "
#endif
		"     [config: " HLIB_CXXCOMPILER_VERSION "]\n"

#if HLIB_SIZE_OPT
		"SIZE_OPT: yes      "
#else
		"SIZE_OPT: no       "
#endif

#if !defined(TESTING)
		"TESTING: undefined       "
#elif !TESTING
		"TESTING: no              "
#else
		"TESTING: yes             "
#endif

#if defined(_GNU_SOURCE)
		"_GNU_SOURCE: yes\n"
#else
		"_GNU_SOURCE: no\n"
#endif

#if HAVE_POLL
		"poll: system       "
#else
		"poll: emulation    "  /* using select(2) */
#endif

#ifdef HLIB_CRIPPLED_SIGINFO_T
		"siginfo_t: replacement   "
#else
		"siginfo_t: system        "
#endif

#ifdef HLIB_PROCMAN_USE_LESS_SIGINFO_T
		"procman: wait-only\n"
#else
		"procman: combined\n"
#endif

		"functions:"
#if HAVE_GETLOADAVG
		" getloadavg"
#endif
#if HAVE_SIGACTION
		" sigaction"
#endif
#if HAVE_WORKING_FORK
		" fork"
#endif
#ifdef HLIB_DONT_USE_MALLOC_USABLE_SIZE
#else
		" malloc_usable_size"
#endif

		"\n"   /* end of functions */

/*----------------------------------------------------------------------------*/
	"\0";
	
	/* Set size entries: */
	static int must_set=1;
	if(must_set)
	{
		char *ss=strstr(str,"\nsize: ");
		if(ss)
		{
			_SetSize(ss,'A',sizeof(size_t));
			_SetSize(ss,'B',sizeof(ssize_t));
			_SetSize(ss,'C',sizeof(void *));
			_SetSize(ss,'D',sizeof(short int));
			_SetSize(ss,'E',sizeof(int));
			_SetSize(ss,'F',sizeof(long int));
			_SetSize(ss,'G',sizeof(float));
			_SetSize(ss,'H',sizeof(double));
		}
		must_set=0;
	}
	
	return(str);
}
