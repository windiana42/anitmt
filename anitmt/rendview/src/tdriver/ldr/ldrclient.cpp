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

#include <assert.h>



int test=TTR_Unset;


#ifndef TESTING
#define TESTING 1
#endif

#if TESTING
#warning TESTING switched on. 
#endif


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
	
	if(fail)
	{
		close(sock);
		return(-1);
	}
	
	return(already_connected);
}


int LDRClient::timernotify(TimerInfo *ti)
{
	assert(0 && ti);
	
	return(0);
}


LDRClient::LDRClient(TaskDriverInterface_LDR *_tdif,
	int *failflag=NULL) : 
	LinkedListBase<LDRClient>(),
	FDBase(failflag)
{
	int failed=0;
	
	tdif=_tdif;
	
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
	// Unrergister at TaskDriverInterface_LDR (-> task manager): 
	tdif->UnregisterLDRClient(this);
}

