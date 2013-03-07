/*
 * misc/lmalloc.h
 *
 * LMalloc (allocation limitation) include header. 
 *
 * Copyright (c) 1999--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_MISC_LMALLOC_H_
#define _HLIB_MISC_LMALLOC_H_ 1

/* include config */
#include <hlib/hconfig.h>


#ifdef __cplusplus
  extern "C" {
#endif

/* contains program name */
extern const char *prg_name;

/* limitmalloc.c: */
  struct LMallocUsage
  {
	size_t alloc_limit;    /* current limit; 0 for unlimited */
	size_t curr_used;      /* amount of currently used memory */
	size_t max_used;       /* max amount of mem used */
	size_t malloc_calls;   /* number of calls to LMalloc()... */
	size_t realloc_calls;  /* ...LRealloc(),... */
	size_t free_calls;     /* ...and LFree(). */
	int used_chunks;       /* incremented for alloc and decremented for free */
	int max_used_chunks;   /* max number of used chunks at a time */
	int real_failures;     /* how often allocation failed due to real failures */
	int limit_failures;    /* how often allocation failed due to alloc limit */
  };
  /* Get the LMallocUsage; pass a pointer where to store the values: */
  void LMallocGetUsage(struct LMallocUsage *dest);
  /* Limit feature: Sets memory usage limit (0 -> no limit) */
  void LMallocSetLimit(size_t limit);
  /* These functions allocate/reallocate/free memory using 
   * malloc()/realloc()/free(). More memory than set with LMallocSetLimit() 
   * cannote be allocated. LFree() is needed to keep internal statistics 
   * about the amount of memory currently malloc()ed right. 
   */
  extern void *LMalloc(size_t size);
  extern void *LRealloc(void *ptr,size_t size);
  extern void *LFree(void *ptr);

  /* In case of of the above (re)allocation functions fails (because of 
   + memory shortage or mem limitation), this function pointer is called 
   + if non-NULL. This function may: 
   * (a) do nothing (revert to original LMalloc behaviour)
   * (b) print a warning message (this is the reason why the size is passed)
   + (c) immediately abort execution
   + NOTE THAT IT MAY _NOT_ call any (de/re)allocation routines. 
   */
  extern void (*lmalloc_failure_handler)(size_t size_for_warning_message);

  /* Like strdup() but using LMalloc() instead of malloc(): 
   * Also returns NULL on failure. */
  static inline char *LStrDup(const char *str)
  { char *dest;
  	if( !str || !(dest=(char*)LMalloc(strlen(str)+1)) )  return(NULL);
	strcpy(dest,str);  return(dest);  }

  /* Like LStrDup() but only copies the first len bytes of the string. 
   * Returned copy is '\0'-terminated. */
  static inline char *LStrNDup(const char *str,size_t n)
  { char *dest;
    if( !str || !(dest=(char*)LMalloc(n+1)) )  return(NULL);
	strncpy(dest,str,n); dest[n]='\0';  return(dest);  }

#ifdef __cplusplus
  }   /* extern "C" */
#endif
#endif  /* _HLIB_MISC_LMALLOC_H_ */
