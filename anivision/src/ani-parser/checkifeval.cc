/*
 * ani-parser/checkifeval.cc
 * 
 * Tree evaluation check. Well, it does not do a lot but is necessary. 
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

#include "tree.h"
#include "coreglue.h"
#include "opfunc.h"

#include <stdio.h>

// NOTE: 
// These routines do basically the same as the CheckIfEvaluable() and Exec() routines 
// just that they check if we actually CAN eval, i.e. the eval funcs 
// are set etc. 
// MAKE SURE TO KEEP THEM IN SYNC WITH CheckIfEvaluable() AND Exec(). 

namespace ANI
{

int TreeNode::CheckIfEvaluable(CheckIfEvalInfo *)
{
	// Must be overridden if called. 
	assert(0);
}

int TNStatement::CheckIfEvaluable(CheckIfEvalInfo *)
{
	// Must be overridden by all TNStatement-derived classes. 
	assert(0);
}

/*----------------------------------------------------------------------------*/

int TNIdentifier::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	if(!evaladrfunc)
	{
		Error(GetLocationRange(),
			"invalid use of \"%s\"\n",
			CompleteStr().str());
		++fail;
	}
	else
	{  fail+=evaladrfunc->CheckIfEvaluable(ci);  }
	return(fail);
}


int TNExpression::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	// Special case as usual :)
	// I mean: TNExpression is a base class for different expressions 
	// the only special case of no derived class is an expression 
	// holding an identifier. 
	if(!down.first() || down.first()->NType()!=TN_Identifier || 
		down.first()!=down.last())
	{  assert(0);  }
	
	return(down.first()->CheckIfEvaluable(ci));
}


int TNExpressionList::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		fail+=i->CheckIfEvaluable(ci);
	}
	return(fail);
}


int TNValue::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	++ci->number_of_values;
	return(0);
}


int TNArray::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	assert(assfunc);
	
	int fail=0;
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		fail+=i->CheckIfEvaluable(ci);
	}
	return(fail);
}


int TNFCallArgument::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	// This is simple as the identifer mapping was already done. 
	// Simply evaluate the expresion: 
	return( down.last()->CheckIfEvaluable(ci) );
}


int TNAssignment::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	// If this assert fails and it is due to user error 
	// which did not get caught yet, replace it by an error 
	// message like the one in TNIdentifier. 
	assert(assfunc);
	
	fail+=down.last()->CheckIfEvaluable(ci);
	fail+=down.first()->CheckIfEvaluable(ci);
	
	return(fail);
}


int TNOperatorFunction::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	switch(func_id)
	{
		case OF_PODCast:
			fail+=down.last()->CheckIfEvaluable(ci);
			break;
		case OF_MembSel:
		case OF_Mapping:
			fail+=down.first()->CheckIfEvaluable(ci);
			// last() is only evaluated if we're not POD-indexing. 
			if(!ms_is_subscript)
			{  fail+=down.last()->CheckIfEvaluable(ci);  }
			break;
		case OF_NewArray:
			fail+=down.first()->next->CheckIfEvaluable(ci);
			break;
		case OF_Delete:
			// Not yet implemented. 
			Error(GetLocationRange(),
				"operator delete not yet implemented\n");
			abort();
			break;
		case OF_PostIncrement:  // fall through.......
		case OF_PostDecrement:
		case OF_PreIncrement:
		case OF_PreDecrement:
		case OF_FCall:
		case OF_Subscript:
		case OF_New:
		default:
			for(TreeNode *tn=down.first(); tn; tn=tn->next)
			{  fail+=tn->CheckIfEvaluable(ci);  }
			break;
	}
	
	// If this assert fails and it is due to user error 
	// which did not get caught yet, replace it by an error 
	// message like the one in TNIdentifier. 
	assert(evalfunc);
	
	return(fail);
}


int TNDeclarator::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	// If this assert fails and it is due to user error 
	// which did not get caught yet, replace it by an error 
	// message like the one in TNIdentifier. 
	assert(evalfunc);
	
	// This is non-recursively, so this check tests if the 
	// decl has an initializer or not: 
	if(decltype==DT_Initialize)
	{
		assert(assfunc);  // Only needed if DT_Initialize. 
		fail+=down.last()->CheckIfEvaluable(ci);
	}
	
	// Check the identifier (find DT_Name): 
	TreeNode *tn=this;
	for(; tn->NType()==TN_Declarator; tn=tn->down.first());
	TNIdentifier *idf=(TNIdentifier*)tn;
	assert(idf->NType()==TN_Identifier);
	fail+=idf->CheckIfEvaluable(ci);
	
	return(fail);
}


int TNNewArraySpecifier::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	assert(!down.is_empty());
	assert(evalfunc);
	
	// This is special because we walk left-to-right instead 
	// of top-to-bottom. 
	if(next && !next->down.is_empty())
	{  fail+=next->CheckIfEvaluable(ci);  }
	
	return(fail);
}


int TNExpressionStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	if(down.first())
	{
		fail+=down.first()->CheckIfEvaluable(ci);
	}
	return(fail);
}


int TNIterationStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	if(for_init_stmt)
	{  fail+=for_init_stmt->CheckIfEvaluable(ci);  }
	if(loop_cond)
	{  fail+=loop_cond->CheckIfEvaluable(ci);  }
	assert(loop_stmt);
	{  fail+=loop_stmt->CheckIfEvaluable(ci);  }
	if(for_inc_stmt)
	{  fail+=for_inc_stmt->CheckIfEvaluable(ci);  }
	
	return(fail);
}


int TNSelectionStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	assert(cond_expr);
	assert(if_stmt);
	
	fail+=cond_expr->CheckIfEvaluable(ci);
	fail+=if_stmt->CheckIfEvaluable(ci);
	if(else_stmt)
	{  fail+=else_stmt->CheckIfEvaluable(ci);  }
	
	return(fail);
}


int TNJumpStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	switch(jumptype)
	{
		case JT_Break:
			break;
		case JT_Return:
			assert(evalfunc);
			if(down.first())
			{  fail+=down.first()->CheckIfEvaluable(ci);  }
			break;
		case JT_None:  // fall through
		default: assert(0);
	}
	
	return(fail);
}


int TNCompoundStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{
		fail+=tn->CheckIfEvaluable(ci);
	}
	
	return(fail);
}


int TNDeclarationStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	// Process all declarators: 
	for(TreeNode *tn=down.first()->next; tn; tn=tn->next)
	{
		fail+=tn->CheckIfEvaluable(ci);
	}
	
	return(fail);
}


int TNAniDescStmt::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	if(name_expr)  fail+=name_expr->CheckIfEvaluable(ci);
	if(adb_child)  fail+=adb_child->CheckIfEvaluable(ci);
	return(fail);
}


int TNFuncDefArgDecl::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	// Check the delarator: 
	return( down.last()->CheckIfEvaluable(ci) );
}


//------------------------------------------------------------------------------

int TNFunctionDef::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	
	//fprintf(stderr,"CHECKING: function %s\n",
	//	((TNIdentifier*)down.first())->CompleteStr().str());
	
	//fail+=func_name->CheckIfEvaluable();  NO! (No need for eval; this is DEF not CALL.)
	for(TreeNode *tn=first_arg; tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	fail+=func_body->CheckIfEvaluable(ci);
	
	return(fail);
}


int TNObject::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *tn=down.first()->next; tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}


int TNSetting::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *tn=down.first()->next; tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}


int TNAnimation::CheckIfEvaluable(CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}

}  // end of namespace func
