/*
 * core/a_anonscope.cc
 * 
 * ANI anonymous scope. 
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
#include "execthread.h"

using namespace ANI;


void AniAnonScope::CG_ExecEnterCompound(ExecThreadInfo *info)
{
	// Allocate stack frame (MUST be done even if there 
	// are 0 variables in it). 
	// The "this" pointer does not change. 
	info->stack->Allocate(iinfo->n_stack_vars,
		/*this_ptr=*/info->stack->GetThisPtr());
	// We do no longer have to initialize the variables with 
	// prototype values. 
	// The declatrator initializer uses special functions 
	// using a reference to make sure we can modify the 
	// ExprValue::iev...
	// NOTE: After the first ExprValue (external, non-real C++) 
	//       referencing, the Eval() code can no longer use 
	//       AN UNINITIALIZED ExprValue as lvalue (but an 
	//       initialized one with ExprValue::iev!=NULL has 
	//       no trouble). 
	
	//- ExecStack::Frame *sframe=info->stack->GetFrame(0);
	//- for(int i=0; i<iinfo->n_stack_vars; i++)
	//- {
	//- 	if(
	//- 	sframe->GetValue(i)=ExprValue(
	//- 		ExprValue::Prototype,iinfo->var_types[i]);
	//- }
}

void AniAnonScope::CG_ExecLeaveCompound(ExecThreadInfo *info)
{
	// Free stack frame: 
	info->stack->Free();
}


int AniAnonScope::InitStaticVariables(ExecThreadInfo *info,int flag)
{
	int nerrors=0;
	
	if(flag>0)
	{
		// Okay, so order is important here. 
		// (Not exactly here [as we only set prototype values] 
		// but generally for static var initialisation.)
		// First, init our variables: 
		nerrors+=variablelist.InitStaticVariables(info,flag);
		// Next, our children: 
		nerrors+=anonscopelist.InitStaticVariables(info,flag);
	}
	else if(flag<0)
	{
		// Cleanup goes the other way round than init. 
		// Note that also automatic static vals need to be cleaned up. 
		nerrors+=variablelist.InitStaticVariables(info,flag);
		nerrors+=anonscopelist.InitStaticVariables(info,flag);
	}
	else assert(0);
	
	return(nerrors);
}


int AniAnonScope::FixIncompleteTypes()
{
	int nerrors=0;
	nerrors+=anonscopelist.FixIncompleteTypes();
	nerrors+=variablelist.FixIncompleteTypes();
	//nerrors+=incompletelist.FixIncompleteTypes();
	nerrors+=_FixEntriesInTypeFixList(&typefix_list);
	return(nerrors);
}


AniScopeBase *AniAnonScope::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo *tri)
{
	// Function works recursively. Tell all our children: 
	anonscopelist.RegisterTN_DoFinalChecks(tri);
	variablelist.RegisterTN_DoFinalChecks(tri);
	//incompletelist.RegisterTN_DoFinalChecks(tri);
	
	// Perform checks:
	// -- nothing to do? (if yes: remember to implement forwarder for 
	//                    AniInternalFunc_POV)
	
	return(NULL);  // Always NULL; that is okay. 
}


int AniAnonScope::PerformCleanup(int cstep)
{
	int nerr=0;
	nerr+=anonscopelist.PerformCleanup(cstep);
	nerr+=variablelist.PerformCleanup(cstep);
	nerr+=incompletelist.PerformCleanup(cstep);
	
	switch(cstep)
	{
		case 1:
			while(!incompletelist.is_empty())
			{  delete incompletelist.first();  }  // NOT popfirst(). 
			assert(typefix_list.is_empty());
			break;
		case 2:
			iinfo=new InstanceInfo();
			FillInInstanceInfo(iinfo,&variablelist,/*vtype=*/1);
			break;
		default:  assert(0);
	}
	
	return(nerr);
}


void AniAnonScope::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	variablelist.LookupName(name,ret_list);
	// No need to ask the anonymous sub-scopes. 
	// NEVER try to look up in AniIncomplete -- that are no identifiers 
	// just placeholders from where to look up existing ones!
	if(query_parent && parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniAnonScope::QueueChild(AniScopeBase *asb,int do_queue)
{
	if(do_queue>0) switch(asb->ASType())
	{
		case AST_UserVar:     // fall through
		case AST_InternalVar: variablelist.append((AniVariable*)asb);     break;
		case AST_AnonScope:   anonscopelist.append((AniAnonScope*)asb);   break;
		case AST_Incomplete:  incompletelist.append((AniIncomplete*)asb); break;
		default: assert(0);
	}
	else if(do_queue<0) switch(asb->ASType())
	{
		case AST_UserVar:     // fall through
		case AST_InternalVar: variablelist.dequeue((AniVariable*)asb);    break;
		case AST_AnonScope:   anonscopelist.dequeue((AniAnonScope*)asb);  break;
		case AST_Incomplete:  incompletelist.dequeue((AniIncomplete*)asb); break;
		default: assert(0);
	}
}

int AniAnonScope::CountScopes(int &n_incomplete) const
{
	int n=1;
	n+=variablelist.CountScopes(n_incomplete);
	n+=anonscopelist.CountScopes(n_incomplete);
	n+=incompletelist.CountScopes(n_incomplete);
	return(n);
}


ANI::TNLocation AniAnonScope::NameLocation() const
{
	fprintf(stderr,"OOPS: AniAnonScope::NameLocation() called.\n");
	assert(0);
	return TNLocation();
}


LinkedList<AniScopeBase::TypeFixEntry> *AniAnonScope::GetTypeFixList()
{
	// Not inline as virtual. 
	return(&typefix_list);
}


AniScopeBase::InstanceInfo *AniAnonScope::GetInstanceInfo()
{
	// Not inline as virtual. 
	return(iinfo);
}


const char *AniAnonScope::CG_AniScopeTypeStr() const
{
	return("[anonscope]");
}


void AniAnonScope::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append("\n");
	
	d->AddIndent();
	variablelist.AniTreeDump(d);
	anonscopelist.AniTreeDump(d);
	incompletelist.AniTreeDump(d);
	d->SubIndent();
}


AniAnonScope::AniAnonScope(TreeNode *_treenode,AniScopeBase *_parent) : 
	AniScopeBase(AST_AnonScope,_treenode,RefString(),_parent)
{
	scopetype=AS_AnonScope;
	
	iinfo=NULL;
	
	if(parent)
	{
		// parent->ASType()==AST_InternalFunc will happen for 
		// the $$povhook. 
		if(parent->ASType()==AST_UserFunc || 
		   parent->ASType()==AST_InternalFunc )
		{  scopetype=AS_FunctionScope;  }
	}
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniAnonScope::~AniAnonScope()
{
	variablelist.ClearList();
	anonscopelist.ClearList();
	incompletelist.ClearList();
	
	while(!typefix_list.is_empty())
	{  delete typefix_list.popfirst();  }
	
	DELETE(iinfo);
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/-1);  }
}
