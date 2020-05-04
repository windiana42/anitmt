/*
 * ani-parser/treeclone.cc
 * 
 * Clone TreeNode trees. 
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

#include <stdio.h>


namespace ANI
{


TreeNode *TreeNode::_CloneChildren(TreeNode *new_parent)
{
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  new_parent->AddChild(tn->CloneTree());  }
	return(new_parent);
}


TreeNode *TreeNode::CloneTree()
{
	assert(ntype==TN_None);
	// TN_None type. Otherwise we may not be here. 
	
	TreeNode *tn=new TreeNode(TN_None);
	return(_CloneChildren(tn));
}


TreeNode *TNIdentifierSimple::CloneTree()
{
	TreeNode *tn=new TNIdentifierSimple(loc,name);
	return(_CloneChildren(tn));
}


TreeNode *TNIdentifier::CloneTree()
{
	assert(!ani_scope && !evaladrfunc);
	TreeNode *tn=new TNIdentifier();
	return(_CloneChildren(tn));
}


TreeNode *TNExpression::CloneTree()
{
	// In case there's a derived class, we may not be here. 
	assert(expr_type==TNE_Expression);
	
	TNExpression *tn=new TNExpression(expr_type);
	return(_CloneChildren(tn));
}


TreeNode *TNExpressionList::CloneTree()
{
	TreeNode *tn=new TNExpressionList();
	return(_CloneChildren(tn));
}


TreeNode *TNValue::CloneTree()
{
	TreeNode *tn=new TNValue(loc,val);
	return(_CloneChildren(tn));
}


TreeNode *TNArray::CloneTree()
{
	TNArray *tn=new TNArray();
	tn->loc0=loc0;
	tn->loc1=loc1;
	tn->extra_comma_start=extra_comma_start;
	tn->extra_comma_end=extra_comma_end;
	tn->elem_type=elem_type;
	tn->nelem=nelem;
	assert(!assfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNTypeSpecifier::CloneTree()
{
	TNTypeSpecifier *tn=new TNTypeSpecifier(loc,(TNIdentifier*)NULL);
	tn->ev_type_id=ev_type_id;
	tn->vtype=vtype;
	return(_CloneChildren(tn));
}


TreeNode *TNOperatorFunction::CloneTree()
{
	TNOperatorFunction *tn=new TNOperatorFunction(func_id,loc0,loc1);
	tn->ms_is_subscript=ms_is_subscript;
	tn->new_type=new_type;
	assert(!evalfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNFCallArgument::CloneTree()
{
	TNFCallArgument *tn=new TNFCallArgument(NULL);
	tn->loc=loc;
	return(_CloneChildren(tn));
}


TreeNode *TNAssignment::CloneTree()
{
	TNAssignment *tn=new TNAssignment(ass_op,loc,NULL,NULL);
	assert(!assfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNStatement::CloneTree()
{
	// We may not be here...
	assert(0);
}


TreeNode *TNExpressionStmt::CloneTree()
{
	TreeNode *tn=new TNExpressionStmt(Sloc);
	return(_CloneChildren(tn));
}


TreeNode *TNIterationStmt::CloneTree()
{
	int nclone=0;
	TNIterationStmt *tn=new TNIterationStmt(itertype,loc0,loc1,
		loop_stmt ? (++nclone,(TNStatement*)loop_stmt->CloneTree()) : NULL,
		loop_cond ? (++nclone,(TNExpression*)loop_cond->CloneTree()) : NULL);
	tn->SetForLoopStmts(
		for_init_stmt ? (++nclone,(TNStatement*)for_init_stmt->CloneTree()) : NULL,
		for_inc_stmt ? (++nclone,(TNStatement*)for_inc_stmt->CloneTree()) : NULL);
	assert(nclone==down.count());
	return(tn);  // NOT _CloneChildren(tn). 
}


TreeNode *TNSelectionStmt::CloneTree()
{
	int nclone=0;
	TNSelectionStmt *tn=new TNSelectionStmt(loc0,
		cond_expr ? (++nclone,(TNExpression*)cond_expr->CloneTree()) : NULL,
		if_stmt ? (++nclone,(TNStatement*)if_stmt->CloneTree()) : NULL,
		loc1,
		else_stmt ? (++nclone,(TNStatement*)else_stmt->CloneTree()) : NULL);
	assert(nclone==down.count());
	return(tn);  // NOT _CloneChildren(tn). 
}


TreeNode *TNJumpStmt::CloneTree()
{
	TreeNode *tn=new TNJumpStmt(loc,jumptype,Sloc);
	assert(!in_scope && !evalfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNCompoundStmt::CloneTree()
{
	TreeNode *tn=new TNCompoundStmt(loc0,loc1);
	assert(!anonscope);
	return(_CloneChildren(tn));
}


TreeNode *TNDeclarator::CloneTree()
{
	TNDeclarator *tn=new TNDeclarator(NULL);
	tn->decl_type=decl_type;
	assert(!evalfunc && !assfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNNewArraySpecifier::CloneTree()
{
	TreeNode *tn=new TNNewArraySpecifier(loc);
	assert(!evalfunc);
	return(_CloneChildren(tn));
}


TreeNode *TNDeclarationStmt::CloneTree()
{
	TreeNode *tn=new TNDeclarationStmt(NULL,attrib,Sloc,aloc);
	return(_CloneChildren(tn));
}


TreeNode *TNAniDescStmt::CloneTree()
{
	int nclone=0;
	TreeNode *tn=new TNAniDescStmt(loc0,Sloc,
		adb_child ? (++nclone,(TNExpression*)adb_child->CloneTree()) : NULL,
		name_expr ? (++nclone,(TNExpression*)name_expr->CloneTree()) : NULL,
		core_name ? (++nclone,(TNIdentifier*)core_name->CloneTree()) : NULL);
	assert(nclone==down.count());
	return(tn);  // NOT _CloneChildren(tn). 
}


TreeNode *TNFuncDefArgDecl::CloneTree()
{
	TreeNode *tn=new TNFuncDefArgDecl(NULL,NULL);
	return(_CloneChildren(tn));
}


TreeNode *TNFunctionDef::CloneTree()
{
	int nclone=0;
	TNFunctionDef *tn=new TNFunctionDef(
		func_name ? (++nclone,(TNIdentifier*)func_name->CloneTree()) : NULL,
		ret_type ? (++nclone,(TNTypeSpecifier*)ret_type->CloneTree()) : NULL,
		func_body ? (++nclone,(TNCompoundStmt*)func_body->CloneTree()) : NULL);
	tn->aloc0=aloc0;
	tn->aloc1=aloc1;
	tn->attrib=attrib;
	assert(!function);
	for(TreeNode *i=first_arg; i; i=i->next,++nclone)
	{  tn->AddChild(i->CloneTree());  }
	assert(nclone==down.count());
	return(tn);  // NOT _CloneChildren(tn). 
}


TreeNode *TNObject::CloneTree()
{
	TreeNode *tn=new TNObject(NULL,loc0,loc1);
	assert(!object);
	return(_CloneChildren(tn));
}


TreeNode *TNSetting::CloneTree()
{
	TreeNode *tn=new TNSetting(NULL,loc0,loc1);
	assert(!setting);
	return(_CloneChildren(tn));
}


TreeNode *TNAnimation::CloneTree()
{
	TreeNode *tn=new TNAnimation();
	assert(!animation);
	return(_CloneChildren(tn));
}

}  // end of namespace func
