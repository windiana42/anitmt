/*
 * nonblock.c 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/prototypes.h>

#include <fcntl.h>

/* Set fd nonblocking. 
 * Return value: 
 *   0 -> success
 *  -1 -> fcntl(F_GETFL) failed
 *  -2 -> fcntl(F_SETFL) failed 
 */
int SetNonblocking(int fd)
{
	int flags=fcntl(fd,F_GETFL);
	if(flags<0)
	{  return(-1);  }
	if(!(flags & O_NONBLOCK))
	{
		if(fcntl(fd,F_SETFL,flags | O_NONBLOCK))
		{  return(-2);  }
	}
	return(0);
}
