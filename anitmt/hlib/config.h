/*
 * config.h 
 * 
 * Highly machine-dependent stuff is in here. 
 * 
 */

#ifndef _HLIB_CONFIG_H_
#define _HLIB_CONFIG_H_ 1

/*#warning (Reading config.h)*/

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

#include <sys/types.h>   /* u_int32_t etc. */

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
