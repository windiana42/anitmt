/*
 * hlib_id1.c
 *
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

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
	static char str[]=
		"hlib version " VERSION " (c) 1999--2002 by Wolfgang Wieser\n"
		"arch: " HLIB_BUILD_SYSTEM "\n"
		"built: " HLIB_BUILD_DATE "\n"
		"size: size_t: AAA  ssize_t: BBB  void *: CCC  | float: GGG\n"
		"size: short: DDD   int: EEE      long: FFF    | double: HHH\n"

/*----------------------------------------------------------------------------*/
#if defined(__GNUC__)
		"GNU C/C++: yes\n"
#else
		"GNU C/C++: no\n"
#endif

#if HLIB_SIZE_OPT
		"SIZE_OPT: yes\t\t"
#else
		"SIZE_OPT: no\t\t"
#endif

#if !defined(TESTING)
		"TESTING: undefined\n"
#elif !TESTING
		"TESTING: no\n"
#else
		"TESTING: yes\n"
#endif

#if HAVE_POLL
		"poll: system\n"
#else
		"poll: emulation using select(2)\n"
#endif

#ifdef HLIB_CRIPPLED_SIGINFO_T
		"siginfo_t: crippled replacement\n"
#else
		"siginfo_t: system\n"
#endif

#ifdef HLIB_PROCMAN_USE_LESS_SIGINFO_T
		"procman: wait-only\n"
#else
		"procman: combined (wait & siginfo)\n"
#endif

#ifdef HLIB_DONT_USE_MALLOC_USABLE_SIZE
		"malloc_usable_size: no\n"
#else
		"malloc_usable_size: yes\n"
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
