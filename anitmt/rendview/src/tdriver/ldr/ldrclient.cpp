/*
 * ldrclient.cpp
 * 
 * LDR task driver stuff: LDR client representation on server side. 
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


#include "../../taskmanager.hpp"
#include "dif_ldr.hpp"
#include "dif_param.hpp"
#include "ldrclient.hpp"

#include <ctype.h>
#include <assert.h>



int test=TTR_Unset;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif

#warning IMPORTANT: Remove potentially dangerous asserts in destructors (like assert(fd<0))

#warning do: use getsockopt(2) to read the SO_ERROR \
option at level  SOL_SOCKET  to  determine  whether \
connect  completed  successfully (SO_ERROR is zero) 

// Return value: 
//   0 -> connecting...
//   1 -> connected successfully without delay
//  -1 -> error
int LDRClient::ConnectTo(ClientParam *cp)
{
	Verbose("  Client %s (%s): ",cp->name.str(),cp->addr.GetAddress().str());
	
	const char *failure="FAILURE\n";
	
	int sock=cp->addr.socket();
	if(sock<0)
	{
		Verbose(failure);
		Error("Failed to create inet socket: %s\n",strerror(errno));
		return(-1);
	}
	
	int fail=1;
	int already_connected=0;
	do {
		if(SetNonblocking(sock)<0)
		{
			Verbose(failure);
			Error("Failed to set socket non-blocking: %s\n",strerror(errno));
			break;
		}
		
		int rv;
		do
		{  rv=cp->addr.connect(sock);  }
		while(rv<0 && errno==EINTR);
		if(rv<0)
		{
			if(errno!=EINPROGRESS)
			{
				Verbose(failure);
				Error("Failed to connect to %s: %s\n",
					cp->addr.GetAddress().str(),strerror(errno));
				break;
			}
			// Okay, must poll for it to see when connection estblishes. 
			Verbose("[in progress]\n");
		}
		else if(!rv)
		{
			// Oh! That was fast. We're already connected. 
			already_connected=1;
			Verbose("[connected]\n");
		}
		else assert(rv<=0);
		
		fail=0;
	} while(0);
	
	if(!fail)
	{
		pollid=tdif->PollFD_Init(this,sock);  // does POLLIN. 
		if(!pollid)
		{  Error("PollFD failed.\n");  fail=1;  }
	}
	
	if(fail)
	{
		close(sock);
		return(-1);
	}
	
	this->sock_fd=sock;
	assert(pollid);  // otherwise great internal error
	
	connected_state=already_connected+1;
	// Okay, now we're doing POLLIN and waiting for answer. 
	
	this->cp=cp;
	
	return(already_connected);
}


// Finish up connect(2). 
// Return value: 
//  0 -> OK, now connected. 
//  1 -> failure; remove client
int LDRClient::_DoFinishConnect(FDBase::FDInfo *fdi)
{
	assert(connected_state==1);
	
	Verbose("XXX>>> ");
	
	// Okay, see if we have POLLIN. (DO THAT FIRST; YES!!)
	if(fdi->revents & POLLIN)
	{
		int errval;
		socklen_t errval_len=sizeof(errval);
		if(getsockopt(sock_fd,SOL_SOCKET,SO_ERROR,&errval,&errval_len)<0)
		{
			Error("Client %s: getsockopt failed: %s\n",
				_ClientName().str(),strerror(errno));
			return(1);
		}
		if(errval)
		{
			Error("Failed to connect to %s: %s\n",
				_ClientName().str(),strerror(errval));
			return(1);
		}
	}
	else
	{
		Error("Client %s: no POLLIN after connect. Removing client.\n",
			_ClientName().str());
		return(1);
	}
	
	// Seems that we may connect without problems. 
	// See if other flags are set: 
	if(fdi->revents & (POLLERR | POLLHUP | POLLNVAL))
	{
		Error("Client %s: strange poll revents%s%s%s. Removing client.\n",
			_ClientName().str(),
			(fdi->revents & POLLERR) ? " ERR" : "",
			(fdi->revents & POLLHUP) ? " HUP" : "",
			(fdi->revents & POLLNVAL) ? " NVAL" : "");
		return(1);
	}
	
	// Okay, we're connected. 
	connected_state=2;
	Verbose("Okay, connected to %s. Waiting for challenge...\n",
		_ClientName().str());
	return(0);
}


// Called via TaskDriverInterface_LDR: 
void LDRClient::fdnotify(FDBase::FDInfo *fdi)
{
	assert(fdi->fd==sock_fd && fdi->pollid==pollid);
	assert(connected_state);  /* otherwise: we may not be here; we have no fd */
	if(connected_state==1)  // waiting for response to connect(2)
	{
		if(_DoFinishConnect(fdi))
		{
			// Failed; we are not connected; give up. 
			connected_state=0;
			UnpollFD();
			assert(pollid==NULL);
			tdif->FailedToConnect(this);  // This will delete us. 
			return;
		}
		return;
	}
	
	// We're connected. 
	Verbose("ugh ");
}


RefString LDRClient::_ClientName()
{
	RefString s;
	if(!cp || !cp->name.str())
	{  s.set("???");  }
	else if(isdigit(cp->name.str()[0]))
	{  s=cp->addr.GetAddress();  }
	else
	{  s.sprintf(128,"%s (%s)",cp->name.str(),cp->addr.GetAddress().str());  }
	return(s);
}


LDRClient::LDRClient(TaskDriverInterface_LDR *_tdif,
	int *failflag=NULL) : 
	LinkedListBase<LDRClient>()
{
	int failed=0;
	
	tdif=_tdif;
	cp=NULL;
	
	sock_fd=-1;
	
	connected_state=0;
	
	// Register at TaskDriverInterface_LDR (-> task manager): 
	assert(component_db()->taskmanager());
	if(tdif->RegisterLDRClient(this))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("LDRClient");  }
}

LDRClient::~LDRClient()
{
	ShutdownFD();  // be sure...
	
	// Unrergister at TaskDriverInterface_LDR (-> task manager): 
	tdif->UnregisterLDRClient(this);
	
	assert(!cp);  // TaskDriverInterface_LDR must have cleaned up. 
}

