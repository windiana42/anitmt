/*
 * ani-parser/treemisc.cc
 * 
 * Misc routined for TreeNode-derived classes. 
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

#include "tree.h"
#include "coreglue.h"

namespace ANI
{

// First, here are all destructors. They are not inline because 
// they're all virtual. 

TNIdentifierSimple::~TNIdentifierSimple()
{
}


TNIdentifier::~TNIdentifier()
{
	DELETE(evaladrfunc);
	
	if(ani_scope)
	{  ani_scope->CG_DeleteNotify(this);  }
}


TNExpression::~TNExpression()
{
}


TNExpressionList::~TNExpressionList()
{
}


TNValue::~TNValue()
{
}


TNArray::~TNArray()
{
	_DeleteAssFunc();
}


TNTypeSpecifier::~TNTypeSpecifier()
{
}


TNOperatorFunction::~TNOperatorFunction()
{
	DELETE(evalfunc);
}


TNFCallArgument::~TNFCallArgument()
{
}


TNAssignment::~TNAssignment()
{
	DELETE(assfunc);
}


TNStatement::~TNStatement()
{
}


TNExpressionStmt::~TNExpressionStmt()
{
}


TNIterationStmt::~TNIterationStmt()
{
}


TNSelectionStmt::~TNSelectionStmt()
{
}


TNJumpStmt::~TNJumpStmt()
{
	DELETE(evalfunc);
	
	if(in_scope)
	{  in_scope->CG_DeleteNotify(this);  }
}


TNCompoundStmt::~TNCompoundStmt()
{
	if(anonscope)
	{  ((AniGlue_ScopeBase*)anonscope)->CG_DeleteNotify(this);  }
}


TNDeclarator::~TNDeclarator()
{
	_RemoveEvalFunc();
	DELETE(assfunc);
}


TNNewArraySpecifier::~TNNewArraySpecifier()
{
	DELETE(evalfunc);
}


TNDeclarationStmt::~TNDeclarationStmt()
{
}


TNAniDescStmt::~TNAniDescStmt()
{
	core_name=NULL;
	name_expr=NULL;
	adb_child=NULL;
	cc_factory=NULL;  // Be sure...
}


TNFuncDefArgDecl::~TNFuncDefArgDecl()
{
}


TNFunctionDef::~TNFunctionDef()
{
	if(function)
	{  ((AniGlue_ScopeBase*)function)->CG_DeleteNotify(this);  }
}


TNObject::~TNObject()
{
	if(object)
	{  ((AniGlue_ScopeBase*)object)->CG_DeleteNotify(this);  }
}


TNSetting::~TNSetting()
{
	if(setting)
	{  ((AniGlue_ScopeBase*)setting)->CG_DeleteNotify(this);  }
}


TNAnimation::~TNAnimation()
{
	if(animation)
	{  ((AniGlue_ScopeBase*)animation)->CG_DeleteNotify(this);  }
}

}  // end of namespace func
