/*
 * reuseaddr.c 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

/* Set SO_REUSEADDR on socket. 
 * Return value: 
 *   0 -> success
 *  -1 -> setsockopt(SOL_SOCKET,SO_REUSEADDR) failed
 */
int SocketReUseAddr(int fd)
{
	int x=1;
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&x,sizeof(x)) == -1)
	{  return(-1);  }
	return(0);
}
