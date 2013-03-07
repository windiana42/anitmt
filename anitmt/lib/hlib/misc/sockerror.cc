/*
 * sockerror.c 
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
#include "prototypes.h"

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif


/* Get socket's pending error code. 
 * Return value: 
 *   0 -> no error
 *  >0 -> error (see errno)
 *  -1 -> getsockopt(SOL_SOCKET,SO_ERROR) failed
 */
int GetSocketError(int fd)
{
#if HAVE_SYS_SOCKET_H
	int errval;
	socklen_t errval_len=sizeof(errval);
	if(getsockopt(fd,SOL_SOCKET,SO_ERROR,&errval,&errval_len)<0)
	{  return(-1);  }
	return(errval);
#else
	return 0;
#endif
}
