/*
 * core/a_object.cc
 * 
 * ANI tree object scope. 
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


AniObject::AniObject(const RefString &name,ANI::TreeNode *_treenode,
	AniAniSetObj *_parent) : 
	AniAniSetObj(AST_Object,name,_treenode,_parent)
{
	pov_attach_detach=NULL;
	
	// Register internal functions (atan(),sin(),$pov(),...) 
	// and variables ($time,...): 
	AniInternalFunction::RegisterInternalFunctions(this);
	AniInternalVariable::RegisterInternalVariables(this);
}

AniObject::~AniObject()
{
	pov_attach_detach=NULL;
}
