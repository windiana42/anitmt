/*
 * installsighandler.c
 * 
 * Copyright (c) 1999--2002 by Wolfgang Wieser (wwieser@gmx.de) 
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

#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#include <string.h>


/* Installs signal handler shandler for signal sig. 
 * Returns 0, if all went ok and -1 if sigaction() failed
 * (in this case, errno is set) 
 */
int InstallSignalHandler(int sig,void (*shandler)(int),int sa_flags)
{
	struct sigaction act;
	memset(&act,0,sizeof(act));
	sigemptyset(&act.sa_mask);
	act.sa_handler=shandler;
	act.sa_flags=sa_flags;

	return(sigaction(sig,&act,NULL));
}
