/*
 * ani-parser/treereg.cc
 * 
 * Tree (identifier, setting, object...) registration. 
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
#include "treereg.h"


namespace ANI
{

AniGlue_ScopeBase *TRegistrationInfo::GetInnermostScope()
{
	AniGlue_ScopeBase *gsb;
	if(anonscope)
	{  gsb=(AniGlue_ScopeBase*)anonscope;  }
	else if(function)
	{  gsb=(AniGlue_ScopeBase*)function;  }
	else if(object)
	{  gsb=(AniGlue_ScopeBase*)object;  }
	else if(setting)
	{  gsb=(AniGlue_ScopeBase*)setting;  }
	else
	{  gsb=(AniGlue_ScopeBase*)animation;  }
	return(gsb);
}


TRegistrationInfo::TRegistrationInfo() : 
	special_idf_list()
{
	who=_TRL_LAST;
	
	animation=NULL;
	setting=NULL;
	object=NULL;
	function=NULL;
	anonscope=NULL;
	
	anidescstmt=NULL;
	
	n_errors=0;
	tn_opt_deleted=0;
	
	var_seq_num=-10;   // something <<0. 
}

TRegistrationInfo::~TRegistrationInfo()
{
	assert(special_idf_list.is_empty());
}

/******************************************************************************/

TRegistrationInfo::SpecialIdf::SpecialIdf() : 
	LinkedListBase<SpecialIdf>(),vtype()
{
	name=NULL;
	adb_idf_core_hook=NULL;
}

/******************************************************************************/

void TreeNode::DoRegistration(TRegistrationInfo * /*tri*/)
{
	// May not happen. All types subject to registration must 
	// override the virtual. 
	fprintf(stderr,"**DoRegistration(): type=%d\n",NType());
	assert(0);
}


void TNStatement::DoRegistration(TRegistrationInfo *tri)
{
	// Some statements do not need any registration. Simply 
	// pass on: 
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		if(i->NType()==TN_Statement || 
		   i->NType()==TN_Expression || 
		   i->NType()==TN_TypeSpecifier )
		{  i->DoRegistration(tri);  }
	}
}

void TNExpression::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who!=TRegistrationInfo::TRL_AllIdentifiers) return;
	
	// Some expressions do not need any registration. Simply 
	// pass on: 
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		if(i->NType()==TN_Expression || 
		   i->NType()==TN_Identifier || 
		   i->NType()==TN_TypeSpecifier )
		{  i->DoRegistration(tri);  }
	}
}

/******************************************************************************/

void TNIdentifier::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who!=TRegistrationInfo::TRL_AllIdentifiers) return;
	
	if(!ani_scope)
	{  ani_scope=tri->GetInnermostScope()->RegisterTN(this,tri);  }
}


void TNTypeSpecifier::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who!=TRegistrationInfo::TRL_AllIdentifiers) return;
	
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
}


void TNOperatorFunction::DoRegistration(TRegistrationInfo *tri)
{
	switch(func_id)
	{
		// Fall through switch. 
		case OF_PODCast:
		case OF_NewArray:
			if(tri->who==TRegistrationInfo::TRL_AllIdentifiers)
			{
				assert(down.first());
				
				for(TreeNode *i=down.first(); i; i=i->next)
				{
					if(i->NType()==TN_Expression || 
					   i->NType()==TN_Identifier || 
					   i->NType()==TN_TypeSpecifier || 
					   i->NType()==TN_NewArraySpecifier)
					{  i->DoRegistration(tri);  }
				}
				
				AniGlue_ScopeBase *gsb=tri->GetInnermostScope();
				gsb->RegisterTN(this,tri);
			}
			break;
		default:  TNExpression::DoRegistration(tri);
	}
}


void TNDeclarator::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who!=TRegistrationInfo::TRL_AllIdentifiers) return;
	
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
}


void TNNewArraySpecifier::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who!=TRegistrationInfo::TRL_AllIdentifiers) return;
	
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
}


// The code below is somewhat redundant. There is little chance 
// to avoid that while retaining readability here. 
// Anyways, the "interesting" part is done by the RegisterTN() 
// function...

void TNIterationStmt::DoRegistration(TRegistrationInfo *tri)
{
	// Just like default TNStatement implementation; just 
	// register children in the correct order: 
	if(for_init_stmt)  for_init_stmt->DoRegistration(tri);
	if(loop_cond)  loop_cond->DoRegistration(tri);
	if(for_inc_stmt)  for_inc_stmt->DoRegistration(tri);
	assert(loop_stmt);  loop_stmt->DoRegistration(tri);
}


void TNJumpStmt::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Functions)
	{
		assert(!in_scope);
		AniGlue_ScopeBase *gsb=tri->GetInnermostScope();
		in_scope=gsb->RegisterTN(this,tri);
	}
	
	// If we failed, do not look at our children: 
	if(!in_scope && tri->who!=TRegistrationInfo::TRL_Optimize)  return;
// NOTE: break currently has in_scope=NULL always but also no children. 
	
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
}


void TNCompoundStmt::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Functions)
	{
		assert(!anonscope);
		// Must be inside function; may be inside anonymous scope. 
		assert(tri->function);
		AniGlue_ScopeBase *gsb=(AniGlue_ScopeBase*)tri->anonscope;
		if(!gsb) gsb=(AniGlue_ScopeBase*)tri->function;
		anonscope=(AniAnonScope*)gsb->RegisterTN(this,tri);
	}
	
	// If we failed, do not look at our children: 
	if(!anonscope && tri->who!=TRegistrationInfo::TRL_Optimize)  return;
	
	// We may be inside an anonymous scope here. Save it. 
	AniAnonScope *save_scope=tri->anonscope;
	
	// Set this so that the children know it. 
	tri->anonscope=anonscope;
	
	// Pass on to children: 
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
	
	// Leave anonymous scope: 
	tri->anonscope=save_scope;
	
	if(tri->who==TRegistrationInfo::TRL_Optimize)
	{
		/*** This is ALSO done in TF step. ***/
		/*** Leave it activated here for the compound optimization below. ***/
		// Remove all empty statements in this compound stmt: 
		for(TreeNode *_tn=down.first(); _tn; )
		{
			assert(_tn->NType()==TN_Statement);
			TNStatement *stmt=(TNStatement*)_tn;
			_tn=_tn->next;
			
			if(stmt->StmtType()==TNS_ExpressionStmt && stmt->down.is_empty())
			{
				//fprintf(stderr,"%s: removing empty expression stmt.\n",
				//	stmt->GetLocationRange().PosRangeString().str());
				delete RemoveChild(stmt);
				++tri->tn_opt_deleted;
			}
		}
		
		/**** This SHOULD be done in TF step... */
		// If this TNCompoundStmt only contains one other compound stmt and 
		// nothing else, take over the children and remove the compound stmt: 
		if(!down.is_empty() && down.first()==down.last() && 
		   ((TNStatement*)down.first())->StmtType()==TNS_CompoundStmt)
		{
			//fprintf(stderr,"%s: removing empty compound stmt scope.\n",
			//		down.first()->GetLocationRange().PosRangeString().str());
			TransferChildrenFrom(down.first());
			delete RemoveChild(down.first());
			++tri->tn_opt_deleted;
		}
		
		#warning "enable me!"
		#if 0
		// If we contain a compound stmt and this compound stmt does 
		// not contain any declarations, take it over. 
		for(TreeNode *_tn=down.first(); _tn; )
		{
			assert(_tn->NType()==TN_Statement);
			TNStatement *stmt=(TNStatement*)_tn;
			_tn=_tn->next;
			
			if(stmt->StmtType()!=TNS_CompoundStmt) continue;
			
			bool can_optimize=1;
			for(TreeNode *i=stmt->down.first(); i; i=i->next)
			{
				TNStatement *s=(TNStatement*)i;
				if(s->StmtType()==TNS_DeclarationStmt)
				{  can_optimize=0;  break;  }
				// EEEE!! It is not THAt easy. We must check that no 
				// child TreeNode is a declaration unless it is 
				// inside its own (different) compound. 
				// E.g. we may not optimize "if(x) int val=6;" or 
				// "for(int i=0;;)" -- these contain decls but not 
				// as immediate children of us...
			}
			if(!can_optimize)  continue;
			
			// Take over children inserting them at the correct position...
			// FIXME: TODO. 
		}
		#endif
	}
}


void TNDeclarationStmt::DoRegistration(TRegistrationInfo *tri)
{
	int type = (tri->who==TRegistrationInfo::TRL_AutoVars) ? 
		1 : (tri->who==TRegistrationInfo::TRL_MemberVars ? 2 : 0);
	if(type)
	{
		AniGlue_ScopeBase *gsb=NULL;
		// NOTE: CANNOE USE GetInnermostScope() because of "type==1/2": 
		if(tri->anonscope)
		{  if(type==1) gsb=(AniGlue_ScopeBase*)tri->anonscope;  }
		else if(tri->function)
		{  if(type==1) gsb=(AniGlue_ScopeBase*)tri->function;  }
		else if(tri->object)
		{  if(type==2) gsb=(AniGlue_ScopeBase*)tri->object;  }
		else if(tri->setting)
		{  if(type==2) gsb=(AniGlue_ScopeBase*)tri->setting;  }
		else
		{  if(type==2) gsb=(AniGlue_ScopeBase*)tri->animation;  }
		// This may register more than one variable and will 
		// always return NULL. 
		if(gsb)
		{  gsb->RegisterTN(this,tri);  }
	}
	
	// No need to pass on to children here unless this is the final step: 
	// HOWEVER, we must keep var_seq_num in sync. 
	for(TreeNode *i=down.first(); i; i=i->next)
	{
		if(tri->who==TRegistrationInfo::TRL_AllIdentifiers)
		{  i->DoRegistration(tri);  }
		// Always execute that when here: 
		// NOTE: Actually, the registration is done by RegisterTN() 
		// above which will NOT modify var_seq_num but will INTERNALLY 
		// add the offset. 
		if(i!=down.first())  // ...which is the type specifier
		{
			assert(tri->var_seq_num>=0);
			// 0x7fffffff is special and means "see everything". 
			if(tri->var_seq_num!=0x7fffffff) ++tri->var_seq_num;
		}
	}
}


void TNAniDescStmt::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_AutoVars)
	{
		// In the one-before-last registration step...
		// (Note: TRegistrationInfo::TRL_AllIdentifiers is needed as well 
		//  since the core needs 2 steps. And we need to do the initial 
		//  registration if the TNAniDescStmt in the first one of these 
		//  two steps to have the cc_factory set.)
		// Must be inside function. 
		assert(tri->function);
		AniGlue_ScopeBase *gsb=(AniGlue_ScopeBase*)tri->function;
		gsb->RegisterTN(this,tri);  // Will always return NULL.
		
		// The RegisterTN() above also looked up the calc core. 
		// cc_factory is NULL here only in case of error. 
	}
	
	// Pass on to children. 
	assert(!tri->anidescstmt);
	tri->anidescstmt=this;
	if(name_expr)  name_expr->DoRegistration(tri);
	if(adb_child)  adb_child->DoRegistration(tri);
	tri->anidescstmt=NULL;
}


void TNFuncDefArgDecl::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_AutoVars)
	{
		// Must be inside function. 
		assert(tri->function);
		AniGlue_ScopeBase *gsb=(AniGlue_ScopeBase*)tri->function;
		gsb->RegisterTN(this,tri);
	}
	
	// No need to pass on to children here unless this is the final step: 
	if(tri->who==TRegistrationInfo::TRL_AllIdentifiers)
	{
		for(TreeNode *i=down.first(); i; i=i->next)
		{  i->DoRegistration(tri);  }
	}
	
	// Always execute that when here: 
	assert(tri->var_seq_num>=0);
	// 0x7fffffff is special and means "see everything". 
	if(tri->var_seq_num!=0x7fffffff) ++tri->var_seq_num;
}


void TNFunctionDef::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Functions)
	{
		assert(!function);
		AniGlue_ScopeBase *gsb;
		if(tri->object)
		{  gsb=(AniGlue_ScopeBase*)tri->object;  }
		else if(tri->setting)
		{  gsb=(AniGlue_ScopeBase*)tri->setting;  }
		else
		{  gsb=(AniGlue_ScopeBase*)tri->animation;  }
		function=(AniFunction*)gsb->RegisterTN(this,tri);
	}
	
	// If we failed, do not look at our children: 
	if(!function && tri->who!=TRegistrationInfo::TRL_Optimize)  return;
	
	// We may not be inside a function here. 
	assert(!tri->function);
	
	// Register the return value: 
	// (Before "entering" the function below.) 
	if(ret_type)  // NULL for destructor/animation spec. 
	{  ret_type->DoRegistration(tri);  }
	
	// Set this so that the children know it. 
	tri->function=function;
	int old_vsn=tri->var_seq_num;
	tri->var_seq_num=0;
	
	// Tell all our "children": 
	for(TreeNode *i=first_arg; i; i=i->next)
	{  i->DoRegistration(tri);  }
	// And...
	func_body->DoRegistration(tri);
	
	// Leave function: 
	tri->function=NULL;
	tri->var_seq_num=old_vsn;
}


void TNObject::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Objects)
	{
		assert(!object);
		AniGlue_ScopeBase *gsb;
		if(tri->setting)
		{  gsb=(AniGlue_ScopeBase*)tri->setting;  }
		else
		{  gsb=(AniGlue_ScopeBase*)tri->animation;  }
		object=(AniAniSetObj*)gsb->RegisterTN(this,tri);
	}
	
	// If we failed, do not look at our children: 
	if(!object && tri->who!=TRegistrationInfo::TRL_Optimize)  return;
	
	// We may not be inside an object here. 
	assert(!tri->object);
	
	// Set this so that the children know it. 
	tri->object=object;
	int old_vsn=tri->var_seq_num;
	tri->var_seq_num=0;
	
	// Tell all our children: 
	// (down.first() is our name)
	for(TreeNode *i=down.first()->next; i; i=i->next)
	{  i->DoRegistration(tri);  }
	
	// Leave object: 
	tri->object=NULL;
	tri->var_seq_num=old_vsn;
}


void TNSetting::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Settings)
	{
		assert(!setting);
		AniGlue_ScopeBase *gsb=(AniGlue_ScopeBase*)tri->animation;
		setting=(AniAniSetObj*)gsb->RegisterTN(this,tri);
	}
	
	// If we failed, do not look at our children: 
	if(!setting && tri->who!=TRegistrationInfo::TRL_Optimize)  return;
	
	// We may not be inside a setting here. 
	assert(!tri->setting);
	
	// Set this so that the children know it. 
	tri->setting=setting;
	int old_vsn=tri->var_seq_num;
	tri->var_seq_num=0;
	
	// Tell all our children: 
	// (down.first() is our name)
	for(TreeNode *i=down.first()->next; i; i=i->next)
	{  i->DoRegistration(tri);  }
	
	// Leave setting: 
	tri->setting=NULL;
	tri->var_seq_num=old_vsn;
}


void TNAnimation::DoRegistration(TRegistrationInfo *tri)
{
	if(tri->who==TRegistrationInfo::TRL_Animation)
	{
		assert(!animation);
		animation=tri->animation;
	}
	
	// Not much to do here, just pass on. 
	tri->animation=animation;
	int old_vsn=tri->var_seq_num;
	tri->var_seq_num=0;
	
	for(TreeNode *i=down.first(); i; i=i->next)
	{  i->DoRegistration(tri);  }
	
	tri->animation=NULL;
	tri->var_seq_num=old_vsn;
	
	// If the last registration step was performed completely, 
	// tell the core: 
	if(tri->who==TRegistrationInfo::_TRL_LAST-1)
	{
		AniGlue_ScopeBase *gsb=(AniGlue_ScopeBase*)animation;
		gsb->RegisterTN(this,tri); /* Will always return NULL. */
	}
}

/******************************************************************************/


int TreeNode::CheckAniScopePresence() const
{
	// Could make this function virtual -- but this is (just) a 
	// check "hack" against internal errors. 
	int fail=0;
	
	switch(NType())
	{
		case TN_Identifier:
			if(up->NType()==TN_AniDesc || 
			   (up->NType()==TN_Statement && 
			     ((TNStatement*)up)->StmtType()==TNS_AniDescStmt))  break;
			fail=((TNIdentifier*)this)->ani_scope==NULL;
			// POV identifiers (in pov name space) may not have 
			// ani_scope attached. 
			if(parent() && parent()->NType()==TN_POV)
			{  assert(fail==1);  fail=0;  }
			break;
		case TN_Statement:
			if(((TNStatement*)this)->StmtType()==TNS_CompoundStmt)
			{  fail=((TNCompoundStmt*)this)->anonscope==NULL;  }
			break;
		case TN_FunctionDef:
			fail=((TNFunctionDef*)this)->function==NULL;
			break;
		case TN_Object:
			fail=((TNObject*)this)->object==NULL;
			break;
		case TN_Setting:
			fail=((TNSetting*)this)->setting==NULL;
			break;
		case TN_Animation:
			fail=((TNAnimation*)this)->animation==NULL;
			break;
		// All the rest: Doesn't matter, fail stays 0. 
	}
	
	if(fail)
	{  fprintf(stderr,"** Internal error: AniScope missing for node "
		"type %d @%s.\n",NType(),GetLocationRange().PosRangeString().str());  }
	
	// Pass on to children. 
	for(const TreeNode *tn=down.first(); tn; tn=tn->next)
	{  fail+=tn->CheckAniScopePresence();  }
	return(fail);
}

}  // end of namespace ANI
