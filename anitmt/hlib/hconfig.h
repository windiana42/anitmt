/*
 * hconfig.h 
 * 
 * Some hlib config & tweaking / debuging stuff. 
 * Also includes config.h. 
 * 
 * Copyright (c) 1999--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _HLIB_CONFIG_H_
#define _HLIB_CONFIG_H_ 1

/*#warning (------- Reading hconfig.h -------)*/

/* THIS HERE... are the first lines of any source code file in hlib 
 * seen by the compiler. So this is the right place to make some 
 * important definitions. 
 */
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#if !defined(__GNUC__)
#define __attribute__(x)
#endif

#if !defined(__cplusplus) && \
  (defined(__CPLUSPLUS) || defined(c_plusplus) || defined(C_PLUSPLUS))
#define __cplusplus 1
#endif

/* Include the configuration header generated by configure script: */
#include "hlib-config.h"

#if (defined(HAVE_MALLOC_USABLE_SIZE) && !HAVE_MALLOC_USABLE_SIZE) || \
	!defined(HAVE_MALLOC_USABLE_SIZE)
# ifndef HLIB_DONT_USE_MALLOC_USABLE_SIZE
#  define HLIB_DONT_USE_MALLOC_USABLE_SIZE
# endif
#endif


#ifdef __CYGWIN__
# define __rtems__
# include <sys/signal.h>
/*# ifndef HLIB_PROCMAN_USE_LESS_SIGINFO_T 
#  define HLIB_PROCMAN_USE_LESS_SIGINFO_T 
# endif
# ifndef HLIB_DONT_USE_MALLOC_USABLE_SIZE
#  define HLIB_DONT_USE_MALLOC_USABLE_SIZE
# endif*/
#endif



/* Need NULL, u_int32_t, etc. */
/* This is copied from autoconf manual and meant 
 * to save a lot of trouble... */
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if !defined(HAVE_WORKING_FORK) || (defined(HAVE_WORKING_FORK) && !HAVE_WORKING_FORK)
#error You lack a working fork(). 
#endif

#if !defined(HAVE_SSIZE_T) || (defined(HAVE_SSIZE_T) && !HAVE_SSIZE_T)
typedef signed int ssize_t;
#endif


#ifdef __GNUC__
# if !defined(HAVE_INT8_T)  || (defined(HAVE_INT8_T) && !HAVE_INT8_T)
  typedef signed int int8_t  __attribute__((__mode__(__QI__)));
# endif
# if !defined(HAVE_U_INT8_T)  || (defined(HAVE_U_INT8_T) && !HAVE_U_INT8_T)
  typedef unsigned int u_int8_t  __attribute__((__mode__(__QI__)));
# endif
# if !defined(HAVE_INT16_T)  || (defined(HAVE_INT16_T) && !HAVE_INT16_T)
  typedef signed int int16_t __attribute__((__mode__(__HI__)));
# endif
# if !defined(HAVE_U_INT16_T)  || (defined(HAVE_U_INT16_T) && !HAVE_U_INT16_T)
  typedef unsigned int u_int16_t __attribute__((__mode__(__HI__)));
# endif
# if !defined(HAVE_INT32_T)  || (defined(HAVE_INT32_T) && !HAVE_INT32_T)
  typedef signed int int32_t __attribute__((__mode__(__SI__)));
# endif
# if !defined(HAVE_U_INT32_T)  || (defined(HAVE_U_INT32_T) && !HAVE_U_INT32_T)
  typedef unsigned int u_int32_t __attribute__((__mode__(__SI__)));
# endif
# if !defined(HAVE_INT64_T)  || (defined(HAVE_INT64_T) && !HAVE_INT64_T)
  typedef signed int int64_t __attribute__((__mode__(__DI__)));
# endif
# if !defined(HAVE_U_INT64_T)  || (defined(HAVE_U_INT64_T) && !HAVE_U_INT64_T)
  typedef unsigned int u_int64_t __attribute__((__mode__(__DI__)));
# endif
#else  /* !__GNUC__ */
# if !defined(HAVE_INT8_T)  || (defined(HAVE_INT8_T) && !HAVE_INT8_T)
  typedef signed char int8_t;
# endif
# if !defined(HAVE_U_INT8_T)  || (defined(HAVE_U_INT8_T) && !HAVE_U_INT8_T)
  typedef unsigned char u_int8_t;
# endif
# if !defined(HAVE_INT16_T)  || (defined(HAVE_INT16_T) && !HAVE_INT16_T)
#  error You are lacking int16_t. 
# endif
# if !defined(HAVE_U_INT16_T)  || (defined(HAVE_U_INT16_T) && !HAVE_U_INT16_T)
#  error You are lacking u_int16_t. 
# endif
# if !defined(HAVE_INT32_T)  || (defined(HAVE_INT32_T) && !HAVE_INT32_T)
#  error You are lacking int32_t. 
# endif
# if !defined(HAVE_U_INT32_T)  || (defined(HAVE_U_INT32_T) && !HAVE_U_INT32_T)
#  error You are lacking u_int32_t. 
# endif
# if !defined(HAVE_INT64_T)  || (defined(HAVE_INT64_T) && !HAVE_INT64_T)
#  error You are lacking int64_t. 
# endif
# if !defined(HAVE_U_INT64_T)  || (defined(HAVE_U_INT64_T) && !HAVE_U_INT64_T)
#  error You are lacking u_int64_t. 
# endif
#endif  /* __GNUC__ */


#if !defined(HAVE_SIGINFO_T) || (defined(HAVE_SIGINFO_T) && !HAVE_SIGINFO_T)
# ifndef HLIB_CRIPPLED_SIGINFO_T
#  define HLIB_CRIPPLED_SIGINFO_T
# endif
# ifndef HLIB_PROCMAN_USE_LESS_SIGINFO_T
#  define HLIB_PROCMAN_USE_LESS_SIGINFO_T
# endif
/* Must emulate siginfo_t with really basic struct... */
typedef struct siginfo
{
	int si_signo;   /* signal number */
} siginfo_t;
#endif  /* HAVE_SIGINFO_T */


/*--------<hack helping in allocation debugging if needed>----------
#ifdef __cplusplus
  extern "C" {
#endif
  extern void free(void *);
  #warning NO LIMIT MALLOC!!!!!!!
  #define LMalloc malloc
  static inline void *_LFree(void *p)
  {  if(p)  free(p);  return(NULL);  }
  #define LFree  _LFree 
#ifdef __cplusplus
  }
#endif
--------------------------------------------------------------------*/

#ifndef HLIB_SIZE_OPT
#  define HLIB_SIZE_OPT 0
#endif

#ifdef __cplusplus
#include <hlib/cplusplus.h>
#endif

#if !HLIB_SIZE_OPT  /* may be passed as argument to the compiler */
#define HL_PureVirt(x) =0;
#else
#define HL_PureVirt(x)  { return x; }
#endif

#endif  /* _HLIB_CONFIG_H_ */
