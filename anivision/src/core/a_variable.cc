/*
 * core/a_variable.cc
 * 
 * ANI tree variable. 
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
#include "aniinstance.h"
#include "execthread.h"

using namespace ANI;


void AniVariable::_NullScopeExplicitMemberSelect(TNIdentifier *idf)
{
	// The "variable" which we want to access using the member select: 
	AniScopeBase *vasb=(AniScopeBase*)idf->ani_scope;
	// The member select base: 
	AniScopeBase *basb=vasb->Parent();
	Error(idf->GetLocationRange(),
		"OOPS: Member select for %s %s inside NULL %s %s\n",
		vasb ? vasb->AniScopeTypeStr() : "[variable?]",
		vasb ? vasb->GetName().str() : "???",
		basb ? basb->AniScopeTypeStr() : "[object?]",
		basb ? basb->CompleteName().str() : "???");
	abort();
}


ExprValueType AniVariable::GetType() const
{
	// Must be overridden. 
	assert(0);
}


int AniVariable::GetAttrib() const
{
	// Must be overridden if ever called. 
	assert(0);
}


ANI::AniGlue_ScopeBase::EvalAddressationFunc *AniVariable::GetVarEvalAdrFuncFunc(
	ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc)
{
	// Must be overridden if ever used. 
	assert(0);
}


void AniVariable::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	//fprintf(stderr,"AniVariable::LookupName(\"%s\")\n",name.str());
	
	// There are no children, so we cannot look up anything: 
	if(parent && query_parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniVariable::QueueChild(AniScopeBase * /*asb*/,int /*do_queue*/)
{
	// AniUserVariable has no children. 
	assert(0);
}


int AniVariable::CountScopes(int & /*n_incomplete*/) const
{
	return(1);
}


AniVariable::AniVariable(AniScopeType _astype,const RefString &_name,
	ANI::TreeNode *_treenode,AniScopeBase *_parent) : 
	AniScopeBase(_astype,_treenode,_name,_parent)
{
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniVariable::~AniVariable()
{
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/-1);  }
}
