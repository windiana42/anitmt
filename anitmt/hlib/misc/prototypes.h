/*
 * misc/prototypes.h
 *
 * Prototypes of all (non-member) functions of this directory. 
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

#ifndef _HLIB_MISC_PROTOTYPES_H_
#define _HLIB_MISC_PROTOTYPES_H_ 1

/* include config */
#include <hlib/hconfig.h>

#include <stdio.h>
#include <stdlib.h>


#ifdef __cplusplus
  extern "C" {
#endif

/* contains program name */
extern char *prg_name;

/* limitmalloc.c: */
  struct LMallocUsage
  {
	size_t alloc_limit;    /* current limit; 0 for unlimited */
	size_t curr_used;      /* amount of currently used memory */
	size_t max_used;       /* max amount of mem used */
	size_t malloc_calls;   /* number of calls to LMalloc()... */
	size_t realloc_calls;  /* ...LRealloc(),... */
	size_t free_calls;     /* ...and LFree(). */
	int used_chunks;  /* incremented for alloc and decremented for free */
	int real_failures;   /* how often allocation failed due to real failures */
	int limit_failures;  /* how often allocation failed due to alloc limit */
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

/* checkmalloc.c: */
  /* returns ptr; exits with error, if ptr==NULL */
  extern void *CheckMalloc(void *ptr);
  static inline void *Free(void *ptr)
    {  if(ptr)  free(ptr);  return(NULL);  }

/* getprgname.c: */
  /* Used to get the program name from argv[0]. Will return "???" in
   * case arg[0] is NULL or an empty string.
   */
  extern char *GetPrgName(const char *arg0);

/* gettermsize.c: */
  /* Get the size of a terminal. 
   * fd: the fd of the teriminal (1 for stdout)
   * ret_row, ret_col: size is stored here (set to NULL if not needed)
   * Return value: 0 -> Okay; 
   *               1 -> not supported on this arch 
   *              -1 -> ioctl() failed; fd is not a TTY
   *              -2 -> ioctl() failed, other reason. 
   */
  extern int GetTerminalSize(int fd,int *ret_row,int *ret_col);
  
/* installsighandler.c: */
  /* Installs signal handler shandler for signal sig. 
   * Returns 0, if all went ok and -1 if sigaction() failed
   * (in this case, errno is set) 
   */
  extern int InstallSignalHandler(int sig,void (*shandler)(int),int sa_flags);

/* memfind.c: */
  /* like strstr(); searches for tofind ('\0'-ternimated) in buf (len bytes)
   */
  extern char *memfind(const char *buf,const char *tofind,size_t len);

/* msecelapsed.c: */
  /* Calc the time between *old and *current in msec. 
   * Current may be NULL (then it is queried from the system). 
   * Result truncated after division (see also msec_elapsed_r()). 
   */
  extern long msec_elapsed(const struct timeval *old,const struct timeval *current);

/* msecelapsedr.c: */
  /* Calc the time between *old and *current in msec. 
   * Current may be NULL (then it is queried from the system). 
   * Result rounded at division (see also msec_elapsed()). 
   */
  extern long msec_elapsed_r(const struct timeval *old,const struct timeval *current);

/* llongstr.c, ullongstr.c: */
  /* Convert the passed argument to a string. The statically allocated 
   * string is returned. 
   */
  extern char *Int64ToStr(int64_t val);
  extern char *UInt64ToStr(u_int64_t val);

/* nonblock.c: */
  /* Set fd nonblocking. 
   * Return value: 
   *   0 -> success
   *  -1 -> fcntl(F_GETFL) failed
   *  -2 -> fcntl(F_SETFL) failed 
   */
  extern int SetNonblocking(int fd);

/* tcpnodelay.c: */
  /* Disable Nagle algorithm on TCP sockets.
   * Return value:
   *   0 -> success
   *  -1 -> setsockopt(SOL_TCP,TCP_NODELAY) failed
   */
  extern int SetTcpNoDelay(int fd);


#ifdef __cplusplus
  }   /* extern "C" */
#endif
#endif  /* _HLIB_MISC_PROTOTYPES_H_ */
