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

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#include <netinet/in.h>


// Used as client/server internet address: 
class MyAddrInfo
{
	private:
		sockaddr_in a;
	public:  _CPP_OPERATORS_FF
		MyAddrInfo(int *failflag=NULL);
		~MyAddrInfo();
		
		// Simply calls listen(2): 
		static int listen(int sockfd,int backlog);
		
		// Simply calls close(2): 
		static int close(int sockfd)
			{  return(::close(sockfd));  }
		
		// *** Compare: ***
		// Compare address but not port: 
		inline bool same_address(const MyAddrInfo &b)
			{  return(a.sin_family==b.a.sin_family && 
			          a.sin_addr.s_addr==b.a.sin_addr.s_addr);  }
		// Compare port but not address: 
		inline bool same_port(const MyAddrInfo &b)
			{  return(a.sin_family==b.a.sin_family && 
			          a.sin_port==b.a.sin_port);  }
		// Compare address and port: 
		inline bool operator==(const MyAddrInfo &b)
			{  return(a.sin_family==b.a.sin_family && 
			          a.sin_addr.s_addr==b.a.sin_addr.s_addr &&
			          a.sin_port==b.a.sin_port);  }
		
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
		// Like SetAddress() but also report error if error happened. 
		// Return value: see above. 
		int SetAddressError(const char *host,int port);
		
		// *** Print Address ***
		// Get address string: with_port: include port name?
		RefString GetAddress(int with_port=1);
		
		// Get port number in host order (or -1): 
		int GetPort()
			{  return(a.sin_family==AF_INET ? int(ntohs(a.sin_port)) : (-1));  }
		
		// This currently only works for IPv4: 
		int IsInNet(MyAddrInfo &net_adr,u_int32_t netmask);
		// Checks if all bits where mask is 0 are 0 in this.a, too. 
		// If so, the test succeeds and returns 0, else 1. ONLY IPv4. 
		int CheckNetBitsZero(u_int32_t mask);
		
		// *** Actions: ***
		// Create a TCP socket (using socket(2)) with specified 
		// address family (AF_INET currently). 
		int socket();
		
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
