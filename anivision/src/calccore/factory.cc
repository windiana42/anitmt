/*
 * calccore/factory.cc
 * 
 * Calc core factory code: All factories need to "register" here. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "ccif.h"
#include "scc/factory.h"
#include "scc/ccif_simple.h"
#include "anitmt/factory.h"
#include "anitmt/ccif_anitmt.h"

#include <ani-parser/exprtf.h>

#include <assert.h>


namespace CC
{

// Initialize static data: 
int CCIF_Factory::n_factories=0;
CCIF_Factory **CCIF_Factory::factories=NULL;

void CCIF_Factory::InitFactories()
{
	assert(!n_factories);
	
	fprintf(stdout,"CC: Initializing calc cores:\n");
	
	n_factories=2;
	factories=(CCIF_Factory**)LMalloc(n_factories*sizeof(CCIF_Factory*));
	
	// Manually place an initailisation command for every calc core here: 
	int ci=0;
	factories[ci]=new CCIF_Factory_Simple(ci);   ++ci;
	factories[ci]=new CCIF_Factory_AniTMT(ci);   ++ci;
	
	assert(ci==n_factories);
	for(int i=0; i<n_factories; i++)
	{  assert(factories[i]->CoreIndex()==i);  }
}

void CCIF_Factory::CleanupFactories()
{
	fprintf(stdout,"CC: Cleaning up calc cores\n");
	
	for(int i=0; i<n_factories; i++)
		DELETE(factories[i]);
	factories=(CCIF_Factory**)LFree(factories);
}


CCIF_Factory *CCIF_Factory::GetFactoryByName(const char *name)
{
	assert(n_factories);
	
	// Use default if no name is specified: 
	if(!name)
	{  return(factories[0]);  }
	
	// Lookup factory by name: 
	for(int i=0; i<n_factories; i++)
	{
		if(!strcmp(factories[i]->name,name))
		{  return(factories[i]);  }
	}
	
	return(0);
}


CCIF_Setting **CCIF_Factory::CreateCCIF_SettingArray()
{
	CCIF_Setting **ccif=(CCIF_Setting **)
		LMalloc(n_factories*sizeof(CCIF_Setting*));
	for(int i=0; i<n_factories; i++)
	{
		ccif[i]=factories[i]->CreateCCIF_Setting();
	}
	return(ccif);
}

CCIF_Setting **CCIF_Factory::DestroyCCIF_SettingArray(CCIF_Setting **ccif)
{
	if(ccif)
	{
		for(int i=0; i<n_factories; i++)
			DELETE(ccif[i]);
		ccif=(CCIF_Setting **)LFree(ccif);
	}
	return(ccif);
}


CCIF_Object **CCIF_Factory::SetupCCIF_ObjectArray()
{
	CCIF_Object **ccif=(CCIF_Object **)
		LMalloc(n_factories*sizeof(CCIF_Object*));
	for(int i=0; i<n_factories; i++)
	{
		// We have lazy allocation here, so initialize with NULL: 
		ccif[i]=NULL;
	}
	return(ccif);
}

CCIF_Object **CCIF_Factory::DestroyCCIF_ObjectArray(CCIF_Object **ccif)
{
	if(ccif)
	{
		for(int i=0; i<n_factories; i++)
			DELETE(ccif[i]);
		ccif=(CCIF_Object **)LFree(ccif);
	}
	return(ccif);
}

//------------------------------------------------------------------------------

const char *_onf_str="OOPS: Called %s for >NULL< factory.\n";

CCIF_Setting *CCIF_Factory::CreateCCIF_Setting()
{
	fprintf(stderr,_onf_str,"CreateCCIF_Object()");
	return(NULL);
}

CCIF_Object *CCIF_Factory::CreateCCIF_Object(CCIF_Setting * /*setting*/,
	int /*vflags*/,AniObjectInstance *obj_inst)
{
	fprintf(stderr,_onf_str,"CreateCCIF_Object()");
	return(NULL);
}

CCIF_Value *CCIF_Factory::CreateCCIF_Value(CCIF_Setting * /*setting*/,
	int /*dimension*/)
{
	fprintf(stderr,_onf_str,"CreateCCIF_Value()");
	return(NULL);
}


void CCIF_Factory::DoADTNRegsistration(ADTNAniDescBlock *adtn,
	ANI::TRegistrationInfo *tri)
{
	// (default implementation)
	
	// Pass on to children. 
	for(ANI::TreeNode *tn=adtn->down.first(); tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
}

void CCIF_Factory::DoADTNRegsistration(ADTNTypedScope *adtn,
	ANI::TRegistrationInfo *tri)
{
	// (default implementation)
	
	// Pass on to children. 
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
}

void CCIF_Factory::DoADTNRegsistration(ADTNScopeEntry *adtn,
	ANI::TRegistrationInfo *tri)
{
	// (default implementation)
	
	// Pass on to value part. 
	adtn->down.last()->DoRegistration(tri);
}


int CCIF_Factory::DoADTNExprTF(ANI::TFInfo *ti,ADTNAniDescBlock *adtn)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	for(ANI::TreeNode *tn=adtn->down.first(); tn; tn=tn->next)
	{  if(tn->DoExprTF(ti))  ++fail;  }
	return(fail);
}

int CCIF_Factory::DoADTNExprTF(ANI::TFInfo *ti,ADTNTypedScope *adtn)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{
		if(tn->DoExprTF(ti,
			tn==adtn->first_entry ? 
				(ANI::TreeNode**)&adtn->first_entry : NULL))
		{  ++fail;  }
	}
	return(fail);
}

int CCIF_Factory::DoADTNExprTF(ANI::TFInfo *ti,ADTNScopeEntry *adtn)
{
	++ti->n_nodes_visited;
	
	return(adtn->down.last()->DoExprTF(ti));
}


CCIF_Factory::CCIF_Factory(const char *_name,int _core_index)
{
	name=_name;
	core_index=_core_index;
}

CCIF_Factory::~CCIF_Factory()
{
	/* empty */
}

}  // end of namespace CC
