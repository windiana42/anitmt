/*
 * myaddrinfo2.cpp
 * Network address class. 
 *
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "myaddrinfo.hpp"

#include <netdb.h>
#include <arpa/inet.h>

#include <assert.h>


// Those parts which need additional RendView-specific interfaces. 

int MyAddrInfo::SetAddressError(const char *host,int port)
{
	int rv=SetAddress(host,port);
	if(rv==-1)
	{  Error("Failed to resolve host \"%s\": %s\n",host,hstrerror(h_errno));  }
	else if(rv==-2)
	{  Error("gethostbyname() did not return AF_INET for \"%s\".\n",host);  }
	else assert(!rv);
	return(rv);
}
