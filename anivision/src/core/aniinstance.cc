/*
 * core/aniinstance.cc
 * 
 * General var instance implementation. 
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

#include "aniinstance.h"
#include "aniscope.h"


// The "lookup" is O(1) due to addressation index. 
// NOTE: Returns a true C++ reference. 
ANI::ExprValue &AniScopeInstanceBase::GetMemberVar(int idx) const
{
	assert(idx>=0);
	assert(idx<n_memb_vars);
	return(memb_vars[idx]);
}


ANI::ExprValue &AniScopeInstanceBase::GetInternalVarRef(int /*bvi*/)
{
	// The Object instance has an own version. 
	// Reaching here is a bug. 
	assert(0);
}

int AniScopeInstanceBase::ComputeInternalVar(int /*bvi*/,
	ANI::ExprValue & /*store_val*/,ExecThread * /*et*/)
{
	// The Object instance has an own version. 
	// Reaching here is a bug. 
	assert(0);
	return(-2);
}


String AniScopeInstanceBase::ToString() const
{
	String tmp;
	
	tmp=asb->CompleteName();
	tmp+=" { ";
	
	// Could write a nicer version which uses asb->variablelist 
	// (need to cast depending on asb->ASType()) and writes 
	// the var NAMES with values as well as the static vars. 
	for(int i=0; i<n_memb_vars; i++)
	{
		tmp+=memb_vars[i].ToString();
		if(i+1<n_memb_vars)  tmp+=", ";
	}
	
	tmp+=" }";
	
	return(tmp);
}


void AniScopeInstanceBase::GetExprValueType(
	ANI::ExprValueType *store_here) const
{
	// Not inline because virtual. 
	store_here->SetAniScope(asb);
}


AniScopeInstanceBase *AniScopeInstanceBase::CreateAniInstance(
	AniScopeBase *asb)
{
	switch(asb->ASType())
	{
		case AniScopeBase::AST_Object:  return(new AniObjectInstance(asb));
		default:  assert(0);
	}
	// Not reached. 
	return(NULL);
}


AniScopeInstanceBase::AniScopeInstanceBase(AniScopeBase *_asb) : 
	ANI::AniGlue_ScopeInstanceBase()
{
	asb=_asb;
	
	AniScopeBase::InstanceInfo *iinfo=asb->GetInstanceInfo();
	assert(iinfo);
	
	// Update instance counter: 
	++iinfo->instance_counter;
	//fprintf(stderr,"** Creating instance of %s %s [now %d]\n",
	//	asb->AniScopeTypeStr(),asb->CompleteName().str(),
	//	iinfo->instance_counter);
	
	// Allocate member vars. 
	n_memb_vars=iinfo->n_stack_vars;
	memb_vars=n_memb_vars ? new ANI::ExprValue[n_memb_vars] : NULL;
	
	// We MUST set up the (nonstatic) member variables because 
	// they are currently NULL refs which may not be used as 
	// rvalue (but are okay as lvalue). 
	for(int i=0; i<n_memb_vars; i++)
	{
		memb_vars[i]=ANI::ExprValue(ANI::ExprValue::Prototype,
			iinfo->var_types[i]);
	}
}

AniScopeInstanceBase::~AniScopeInstanceBase()
{
	// Deallocate variables: 
	if(memb_vars)
	{  delete[] memb_vars;  memb_vars=NULL;  }
	n_memb_vars=0;
	
	// Update instance counter: 
	AniScopeBase::InstanceInfo *iinfo=asb->GetInstanceInfo();
	assert(iinfo);
	--iinfo->instance_counter;
	
	//fprintf(stderr,"** Deleting instance of %s %s [left %d]\n",
	//	asb->AniScopeTypeStr(),asb->CompleteName().str(),
	//	iinfo->instance_counter);
	
	asb=NULL;
}
