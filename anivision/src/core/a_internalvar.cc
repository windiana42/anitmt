/*
 * core/a_internalvar.cc
 * 
 * ANI internal variables (base). 
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
//#include "execthread.h"

#warning "Fix all $$#'s."

using namespace ANI;


const char *AniInternalVariable::CG_AniScopeTypeStr() const
{
	return("internal variable");
}


ANI::TNLocation AniInternalVariable::NameLocation() const
{
	ANI::TNLocation loc;
	//loc.pos0=<fill in "[built-in]">  ##$
	loc.pos1=loc.pos0;
	return(loc);
}


AniScopeBase *AniInternalVariable::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo *tri)
{
	// Nothing to do. 
	return(NULL);  // Always NULL; that is okay. 
}


int AniInternalVariable::FixIncompleteTypes()
{
	// Nothing to do. 
	return(0);
}


int AniInternalVariable::PerformCleanup(int cstep)
{
	int nerr=0;
	
	switch(cstep)
	{
		case 1:  break;  // Nothing to do. 
		case 2:  break;  // Nothing to do for InstanceInfo creation step. 
		default:  assert(0);
	}
	
	return(nerr);
}


int AniInternalVariable::InitStaticVariables(ANI::ExecThreadInfo *info,int flag)
{
	// Default implementation: Do nothing (not static). 
	return(0);
}


void AniInternalVariable::RegisterInternalVariables(AniAniSetObj *parent)
{
	AniInternalVariable_Object::_RegisterInternalVariables(parent);
	AniInternalVariable_AniSet::_RegisterInternalVariables(parent);
}


AniInternalVariable::AniInternalVariable(const RefString &_name,
	ANI::TreeNode *_treenode,AniScopeBase *_parent) : 
	AniVariable(AST_InternalVar,_name,_treenode,_parent)
{
}

AniInternalVariable::~AniInternalVariable()
{
}
