/*
 * gettermsize.c 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_IN_HLIB 1
#include <hlib/prototypes.h>

#if HAVE_TERMIOS_H
# include <termios.h>
#endif

#if GWINSZ_IN_SYS_IOCTL
# include <sys/ioctl.h>
#endif


/* Get the size of a terminal. 
 * fd: the fd of the teriminal (1 for stdout)
 * ret_row, ret_col: size is stored here (set to NULL if not needed)
 * Return value: 0 -> Okay; 
 *               1 -> not supported on this arch 
 *              -1 -> ioctl() failed; fd is not a TTY
 *              -2 -> ioctl() failed, other reason. 
 */
int GetTerminalSize(int fd,int *ret_row,int *ret_col)
{
	#ifdef TIOCGWINSZ
		// Use ioctl: 
		struct winsize wsz;
		if(ioctl(fd,TIOCGWINSZ,&wsz))
		{  return((errno==ENOTTY) ? (-1) : (-2));  }
		
		kjdf:;
		if(ret_row)  *ret_row=wsz.ws_row;
		if(ret_col)  *ret_col=wsz.ws_col;
		return(0);
	#else
		#warning NOTE: GetTerminalSize() will be unavailable. 
		return(1);
	#endif
}
