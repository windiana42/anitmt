/*
 * myaddrinfo.hpp
 * Network address class. 
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


#ifndef _RNDV_LIB_MYADDRINFO_H_
#define _RNDV_LIB_MYADDRINFO_H_ 1

#include <lib/prototypes.hpp>
#include <hlib/refstring.h>

#include <netinet/in.h>


// Used as client/server internet address: 
class MyAddrInfo
{
	private:
		sockaddr_in a;
	public:  _CPP_OPERATORS_FF
		MyAddrInfo(int *failflag=NULL);
		~MyAddrInfo();
		
		// Create a TCP socket (using socket(2)): 
		static int TCPSocket();
		
		// Simply calls listen(2): 
		static int listen(int sockfd,int backlog);
		
		// Simply calls close(2): 
		static int close(int sockfd)
			{  return(::close(sockfd));  }
		
		// *** Set address: ***
		// This is used for bind() so that INADDR_ANY is used. 
		// port: port in host order to bind to. 
		void SetPassiveAny(int port);
		
		// Set address passed in host. If host is specified numerically, 
		// it is parsed in, otherwise it is resolved (which may block some 
		// time). port is specified in host order. 
		// Return value: 
		//   0 -> OK
		//  -1 -> gethostbyname() failed (see h_errno)
		//  -2 -> not AF_INET
		int SetAddress(const char *host,int port);
		
		// *** Print Address ***
		// Get address string: 
		RefString GetAddress();
		
		// Get port number in host order (or -1): 
		int GetPort()
			{  return(a.sin_family==AF_INET ? int(ntohs(a.sin_port)) : (-1));  }
		
		// *** Actions: ***
		// Calls connect(2) and returns its return value: 
		// The connect address stored in *this is passed. 
		int connect(int sockfd);
		
		// Calls accept(2). The returned address is stored in *this. 
		// Return value: that of accept. 
		int accept(int sockfd);
		
		// Calls bind(2). The address to bind to is the current 
		// address in *this. Return value: see bind(2). 
		int bind(int sockfd);
};

#endif  /* _RNDV_LIB_MYADDRINFO_H_ */
