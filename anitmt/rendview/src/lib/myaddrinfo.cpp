/*
 * myaddrinfo.cpp
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

#include "myaddrinfo.hpp"

#include <netdb.h>
#include <arpa/inet.h>

#include <assert.h>


int MyAddrInfo::listen(int sockfd,int backlog)
{
	return(::listen(sockfd,backlog));
}


void MyAddrInfo::SetPassiveAny(int port)
{
	a.sin_family=AF_INET;
	a.sin_port=htons(port);
	a.sin_addr.s_addr=INADDR_ANY;
}


int MyAddrInfo::SetAddress(const char *host,int port)
{
	do {
		if(inet_aton(host,&a.sin_addr))  break;
		
		hostent *he=gethostbyname(host);
		if(!he)  return(-1);
		if(he->h_addrtype!=AF_INET)  return(-2);
		assert(he->h_length==sizeof(a.sin_addr));
		memcpy(&a.sin_addr,he->h_addr_list[0],he->h_length);
	} while(0);
	a.sin_family=AF_INET;
	a.sin_port=htons(port);
	return(0);
}

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


RefString MyAddrInfo::GetAddress(int with_port)
{
	RefString s;
	if(a.sin_family!=AF_INET)
	{  s.set("[none]");  }
	else if(with_port)
	{  s.sprintf(0,"%s:%u",
		inet_ntoa(a.sin_addr),(unsigned int)ntohs(a.sin_port));  }
	else
	{  s.set(inet_ntoa(a.sin_addr));  }
	return(s);
}


int MyAddrInfo::IsInNet(MyAddrInfo &net_adr,u_int32_t netmask)
{
	// When using IPv4 and IPv6, we should probably return 0 if 
	// a.sin_family!=net_adr.sin_family. 
	assert(a.sin_family==AF_INET && net_adr.a.sin_family==AF_INET);
	
	u_int32_t thisaddr=a.sin_addr.s_addr;
	u_int32_t netaddr=net_adr.a.sin_addr.s_addr;
	
	return((thisaddr & netmask) == (netaddr & netmask));
}

int MyAddrInfo::CheckNetBitsZero(u_int32_t mask)
{
	assert(a.sin_family==AF_INET);
	
	u_int32_t thisaddr=a.sin_addr.s_addr;
	fprintf(stderr,"<<0x%08x 0x%08x>>\n",thisaddr,mask);
	return((thisaddr & ~mask) ? 1 : 0);
}


int MyAddrInfo::socket()
{
	return(::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP));
}


int MyAddrInfo::connect(int sockfd)
{
	int rv=::connect(sockfd,(sockaddr*)&a,sizeof(a));
	return(rv);
}


int MyAddrInfo::accept(int sockfd)
{
	memset(&a,0,sizeof(a));
	socklen_t alen=sizeof(a);
	int rv=::accept(sockfd,(sockaddr*)&a,&alen);
	assert(alen<=sizeof(a));
	assert(a.sin_family==AF_INET);
	return(rv);
}


int MyAddrInfo::bind(int sockfd)
{
	int rv=::bind(sockfd,(sockaddr*)&a,sizeof(a));
	// Does not help against TIME_WAIT and socket already in use: 
	SocketReUseAddr(sockfd);
	return(rv);
}


MyAddrInfo::MyAddrInfo(int * /*failflag*/)
{
	memset(&a,0,sizeof(a));
}

MyAddrInfo::~MyAddrInfo()
{

}
