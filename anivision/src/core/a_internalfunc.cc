/*
 * core/a_interalfunc.cc
 * 
 * ANI internal functions (base). 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "animation.h"
#include "execthread.h"

#warning "Fix all $$#'s."

using namespace ANI;


void AniInternalFunction::RegisterInternalFunctions(AniAniSetObj *parent)
{
	AniInternalFunc_Simple::_RegisterInternalFunctions(parent);
	AniInternalFunc_POV::_RegisterInternalFunctions(parent);
	AniInternalFunc_Wait::_RegisterInternalFunctions(parent);
}


const char *AniInternalFunction::CG_AniScopeTypeStr() const
{
	return("internal function");
}


void AniInternalFunction::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	// Search the argument list...: 
	//for(int i=0; i<nargs; i++)
	{
		// Hmm... seems that it is illegal if we come here. 
		// At least we cannot return a parameter to a built-in function 
		// as an ASB in the ret_list. 
		assert(0);
	}
	
	if(query_parent && parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


ANI::TNLocation AniInternalFunction::NameLocation() const
{
	ANI::TNLocation loc;
	//loc.pos0=<fill in "[built-in]">  ##$
	loc.pos1=loc.pos0;
	return(loc);
}


AniScopeBase *AniInternalFunction::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo * /*tri*/)
{
	// Nothing to do. 
	return(NULL);  // Always NULL; that is okay. 
}


int AniInternalFunction::FixIncompleteTypes()
{
	// Nothing to do. 
	return(0);
}


int AniInternalFunction::InitStaticVariables(ExecThreadInfo * /*info*/,int flag)
{
	int nerrors=0;
	
	if(flag>0)
	{
		// Init: Nothing to do. 
	}
	else if(flag<0)
	{
		// Cleanup: Nothing to do. 
	}
	else assert(0);
	
	return(nerrors);
}


int AniInternalFunction::PerformCleanup(int cstep)
{
	int nerr=0;
	
	switch(cstep)
	{
		case 1:  break;  // Nothing to do here. 
		case 2:
			// Create instance info... No need for one. 
			break;
		default:  assert(0);
	}
	
	return(nerr);
}


void AniInternalFunction::QueueChild(AniScopeBase * /*asb*/,int)
{
	// May not be called. AniInternalFunc_Simple has no children. 
	assert(0);
}

int AniInternalFunction::CountScopes(int & /*n_incomplete*/) const
{
	return(1);
}


LinkedList<AniScopeBase::TypeFixEntry> *AniInternalFunction::GetTypeFixList()
{
	// Not inline as virtual. 
	return(NULL);  // There is not typefix list. 
}


AniScopeBase::InstanceInfo *AniInternalFunction::GetInstanceInfo()
{
	// Not inline as virtual. 
	assert(0);  // No need for instance info
	return(NULL);
}


AniInternalFunction::AniInternalFunction(const RefString &name,
	AniAniSetObj *_parent,int _attrib,TreeNode *_treenode) : 
	AniFunction(AST_InternalFunc,name,_treenode,_parent,_attrib)
{
	// empty
}

AniInternalFunction::~AniInternalFunction()
{
	// empty
}

