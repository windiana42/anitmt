/*
 * config.h 
 * 
 * Highly machine-dependent stuff is in here. 
 * 
 */

#ifndef _HLIB_CONFIG_H_
#define _HLIB_CONFIG_H_ 1

/* Cannot define _GNU_SOURCE here as this must be done earlier 
 * in the source code. */
#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#warning *** _GNU_SOURCE NOT DEFINED. Please pass -D_GNU_SOURCE to the compiler. ***
#endif


/*#warning (------- Reading config.h -------)*/

/* Need NULL, u_int32_t, etc. */
#include <sys/types.h>
#include <stddef.h>

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

#ifndef DirSepChar
#  define DirSepChar '/'
#endif

#ifndef HLIB_SIZE_OPT
#  define HLIB_SIZE_OPT 0
#endif

/* We might have to define these:  */
#ifdef __cplusplus
typedef signed long long int64_t;
typedef unsigned long long u_int64_t;
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
