/*
 * installsighandler.cc
 * 
 * Copyright (c) 1999 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <signal.h>

/* Installs signal handler shandler for signal sig. 
 * Returns 0, if all went ok and -1 if sigaction() failed
 * (in this case, errno is set) 
 */
int InstallSignalHandler(int sig,void (*shandler)(int),int sa_flags)
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler=shandler;
    act.sa_flags=sa_flags;
    act.sa_restorer=NULL;
    
    return(sigaction(sig,&act,NULL));
}
