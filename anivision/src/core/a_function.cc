/*
 * core/a_function.cc
 * 
 * ANI function scope (base). 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
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

#include <ani-parser/treereg.h>
#include "execthread.h"


using namespace ANI;


int AniFunction::Match_FunctionArguments(LookupFunctionInfo * /*lfi*/,
	int /*store_evalhandler*/)
{
	// Must be overridden. 
	assert(0);
}


String AniFunction::PrettyCompleteNameString() const
{
	// Must be overridden. 
	assert(0);
}


AniFunction::AniFunction(AniScopeType _func_t,const RefString &_name,
	TreeNode *_treenode,AniAniSetObj *_parent,int _attrib) : 
	AniScopeBase(_func_t,_treenode,_name,_parent)
{
	attrib=_attrib;
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniFunction::~AniFunction()
{
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/-1);  }
}
