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
/*#include <hlib/hconfig.h>  <--- done by <hlib/lmalloc.h> */
#include <hlib/lmalloc.h>


/* some forward decls: */
struct timeval;


#ifdef __cplusplus
  extern "C" {
#endif

/* contains program name */
extern char *prg_name;

/* limitmalloc.c: */
  /* See lmalloc.h. */

/* checkmalloc.c: */
  /* returns ptr; exits with error, if ptr==NULL */
  extern void *CheckMalloc(void *ptr);
  static inline void *Free(void *ptr)
    {  if(ptr)  free(ptr);  return(NULL);  }

/* getloadavg.c: */
  /* This function returns the 1-minute load average multiplied with 100. 
   * Negative return values indicate error. 
   * If it failed once, failure is memorized and all subsequent calls will 
   * return failure. 
   */
  extern int GetLoadAverage();

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
  
/* hlib_id?.c: */
  /* Get version string ("hlib version xyz"): */
  extern const char *HLIB_GetVersionString();
  /* Get config string (version, arch, build date, poll emulation?, 
   * size opt... */
  extern const char *HLIB_GetConfigString();

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

/* reuseaddr.c: */
  /* Set SO_REUSEADDR on socket. 
   * Return value: 
   *   0 -> success
   *  -1 -> setsockopt(SOL_SOCKET,SO_REUSEADDR) failed
   */
  extern int SocketReUseAddr(int fd);

/* sockerror.c: */
  /* Get socket's pending error code. 
   * Return value: 
   *   0 -> no error
   *  >0 -> error (see errno)
   *  -1 -> getsockopt(SOL_SOCKET,SO_ERROR) failed
   */
  extern int GetSocketError(int fd);

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
