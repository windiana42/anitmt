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


int LDRClient::ConnectTo(ClientParam *)
{
	assert(0);
	
	return(1);
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

