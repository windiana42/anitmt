/*
 * tcpnodelay.c 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>

#include <sys/socket.h>
#include <netinet/tcp.h>

/* Disable Nagle algorithm on TCP sockets.  
 * Return value: 
 *   0 -> success
 *  -1 -> setsockopt(SOL_TCP,TCP_NODELAY) failed
 */
int SetTcpNoDelay(int fd)
{
	int x=1;
	if(setsockopt(fd,SOL_TCP,TCP_NODELAY,&x,sizeof(x)) == -1)
	{  return(-1);  }
	return(0);
}
