/*
 * daemonize.c 
 * Put process into background as daemon and detach from controlling tty. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <fcntl.h>


/* Longer description in prototypes.h. */
int Daemonize(int flags)
{
	/* Do as much as we can before forking. */
	if((flags & DAEMONIZE_CHDIR) && chdir("/")<0)
	{  return(-3);  }
	
	if(flags & DAEMONIZE_CLOSE)
	{
		/* Close stderr last. */
		/* NOTE: We may get fooled if /dev/null is not the 
		 *       device we think it is. */
		int fd=open("/dev/null",O_RDWR);
		int xfd;
		if(fd<0)  return(-4);
		
		/* I depend on stdin,out,err to be properly set up here. */
		/* One could also use fd=0,1,2 directly. */
		if(flags & DAEMONIZE_CLOSE_IN)
		{
			xfd=fileno(stdin);
			if(xfd>=0 && (close(xfd)<0 || dup2(fd,xfd)<0))  return(-6);
		}
		
		if(flags & DAEMONIZE_CLOSE_OUT)
		{
			xfd=fileno(stdout);
			if(xfd>=0 && (close(xfd)<0 || dup2(fd,xfd)<0))  return(-7);
		}
		
		if(flags & DAEMONIZE_CLOSE_ERR)
		{
			xfd=fileno(stderr);
			if(xfd>=0 && (close(xfd)<0 || dup2(fd,xfd)<0))  return(-8);
		}
		
		/* This is needed; otherwise we keep one unnecessary FD open. */
		if(close(fd)<0)  return(-5);
	}
	
	{
		pid_t pid=fork();
		if(pid<0)
		{  return(-1);  }
		if(pid>0)  /* Parent thread: */
		{  _exit(0);  }
	}
	
	/* Child thread: */
	if(setsid()<0)
	{  return(-2);  }
	
	return(0);
}
