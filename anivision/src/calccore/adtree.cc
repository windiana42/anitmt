/*
 * calccore/adtree.cc
 * 
 * Animation description tree representation. 
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

#include "adtree.h"
#include <ani-parser/exprtf.h>
#include <ani-parser/treereg.h>
#include <calccore/ccif.h>


namespace CC
{

void ADTreeNode::DumpTree(StringTreeDump *d)
{
	assert(adntype==ADTN_None);
	
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{
		tn->DumpTree(d);
		d->Append("\n");
	}
	
	// We should never come here. This is ADTN_None!
	assert(0);
}

ANI::TreeNode *ADTreeNode::CloneTree()
{
	// May only be called if there is no derived class. 
	assert(adntype==ADTN_None);
	
	TreeNode *tn=new ADTreeNode(adntype);
	return(_CloneChildren(tn));
}


//------------------------------------------------------------------------------

void ADTNAniDescBlock::DumpTree(StringTreeDump *d)
{
	d->Append("{");
	
	if(!down.first())
	{  d->Append(" /*empty*/ ");  }
	else if(down.first()==down.last() && 
		!(down.first()->NType()==ANI::TN_AniDesc && 
		 ((ADTreeNode*)down.first())->ADNType()==ADTN_ScopeEntry ))
	{
		d->Append(" ");
		down.first()->DumpTree(d);
		d->Append(" ");
	}
	else
	{
		d->Append("\n");
		d->AddIndent();
		for(TreeNode *i=down.first(); i; i=i->next)
		{
			i->DumpTree(d);
			d->Append("\n");
		}
		d->SubIndent();
	}
	
	if(parent()->NType()!=ANI::TN_AniDesc)
	{  d->Append("}");  }  // NO newline, there's a ';' to come. 
	else
	{  d->Append("}\n");  }
}

ANI::TreeNode *ADTNAniDescBlock::CloneTree()
{
	TreeNode *tn=new ADTNAniDescBlock(loc0,loc1);
	return(_CloneChildren(tn));
}

ANI::TNLocation ADTNAniDescBlock::GetLocationRange() const
{
	ANI::TNLocation loc;
	
	loc.pos0=loc0.pos0;
	//if(down.last())
	//{  loc.pos1=down.last()->GetLocationRange().pos1;  }
	loc.pos1=loc1.pos1;
	
	return(loc);
}

void ADTNAniDescBlock::DoRegistration(ANI::TRegistrationInfo *tri)
{
	do {
		// TRL_AutoVars is special because we need 2 passes. 
		if(tri->who!=ANI::TRegistrationInfo::TRL_AllIdentifiers && 
		   tri->who!=ANI::TRegistrationInfo::TRL_AutoVars)  break;
		
		assert(tri->anidescstmt);
		
		// The cc_factory pointer may be NULL if an error in the 
		// calc core name lookup occured. 
		if(!tri->anidescstmt->cc_factory)  break;
		
		CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			tri->anidescstmt->cc_factory;
		ccfact->DoADTNRegsistration(this,tri);
		
		return;
	} while(0);
	
	// Important because the calc core does this part in the above case. 
	// Pass on to children. 
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
}

int ADTNAniDescBlock::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	// Need to get associated factory. 
	assert(ti->anidescstmt);
	CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			ti->anidescstmt->cc_factory;
	// ccfact may only be NULL if an error in registration step occured. 
	// But then we may not be here. 
	assert(ccfact);
	
	#if 0  /* Old version; we now know the factory from ti->anidescstmt. */
	do {
		if(!parent() || parent()->NType()!=ANI::TN_Statement)  break;
		ANI::TNStatement *stmt=(ANI::TNStatement*)parent();
		if(stmt->StmtType()!=ANI::TNS_AniDescStmt)  break;
		ccfact=(CC::CCIF_Factory*)((ANI::TNAniDescStmt*)stmt)->cc_factory;
	} while(0);
	// We must have a TNAniDescStmt parent here. 
	assert(ccfact);
	#endif
	
	return(ccfact->DoADTNExprTF(ti,this));
}

int ADTNAniDescBlock::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}

//------------------------------------------------------------------------------

void ADTNTypedScope::DumpTree(StringTreeDump *d)
{
//d->Append("TypedScope[ ");
	scope_type->DumpTree(d);
	
	if(scope_name)
	{
		d->Append(" ");
		scope_name->DumpTree(d);
	}
	d->Append(" {");
	
	if(!first_entry)
	{  d->Append(" /*empty*/ ");  }
	else if(!first_entry->next)
	{
		d->Append(" ");
		first_entry->DumpTree(d);
		d->Append(" ");
	}
	else
	{
		d->Append("\n");
		d->AddIndent();
		for(TreeNode *i=first_entry; i; i=i->next)
		{
			i->DumpTree(d);
			d->Append("\n");
		}
		d->SubIndent();
	}
	
	d->Append("}");
//d->Append("]");
}

ANI::TreeNode *ADTNTypedScope::CloneTree()
{
	int nclone=0;
	ADTNTypedScope *tn=new ADTNTypedScope(
		scope_type ? (++nclone,(ANI::TNIdentifierSimple*)scope_type->CloneTree()) : NULL,
		loc0,loc1,
		scope_name ? (++nclone,(ANI::TNIdentifier*)scope_name->CloneTree()) : NULL);
	tn->scope_type_tok=scope_type_tok;
	for(TreeNode *i=first_entry; i; i=i->next,++nclone)
	{  tn->AddChild(i->CloneTree());  }
	assert(nclone==down.count());
	return(tn);  // NOT _CloneChildren(tn). 
}

ANI::TNLocation ADTNTypedScope::GetLocationRange() const
{
	ANI::TNLocation loc;
	
	loc.pos0=scope_type->GetLocationRange().pos0;
	loc.pos1=loc1.pos1;
	
	return(loc);
}

void ADTNTypedScope::DoRegistration(ANI::TRegistrationInfo *tri)
{
	do {
		// TRL_AutoVars is special because we need 2 passes. 
		if(tri->who!=ANI::TRegistrationInfo::TRL_AllIdentifiers && 
		   tri->who!=ANI::TRegistrationInfo::TRL_AutoVars)  break;
		
		assert(tri->anidescstmt);
		
		// The cc_factory pointer may be NULL if an error in the 
		// calc core name lookup occured. 
		if(!tri->anidescstmt->cc_factory)  break;
		
		CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			tri->anidescstmt->cc_factory;
		ccfact->DoADTNRegsistration(this,tri);
		
		return;
	} while(0);
	
	// Important because the calc core does this part in the above case. 
	// Pass on to children. 
	for(TreeNode *tn=first_entry; tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
}

int ADTNTypedScope::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	// Need to get associated factory. 
	assert(ti->anidescstmt);
	CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			ti->anidescstmt->cc_factory;
	// ccfact may only be NULL if an error in registration step occured. 
	// But then we may not be here. 
	assert(ccfact);
	
	return(ccfact->DoADTNExprTF(ti,this));
}

int ADTNTypedScope::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	assert(scope_type_tok>=0);
	
	int fail=0;
	for(TreeNode *tn=first_entry; tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}

//------------------------------------------------------------------------------

void ADTNScopeEntry::DumpTree(StringTreeDump *d)
{
	if(down.first()!=down.last())
	{
		assert(down.count()==2);
		down.first()->DumpTree(d);
		d->Append("=");
	}
	
	down.last()->DumpTree(d);
	
	if(down.last()->NType()==ANI::TN_Expression)
	{  d->Append(";");  }
}

ANI::TreeNode *ADTNScopeEntry::CloneTree()
{
	ADTNScopeEntry *tn=new ADTNScopeEntry((ADTNTypedScope*)NULL,NULL,&loc0);
	// Currently, we may only clone trees which do not yet have 
	// a core hook attached. If it is needed to clone other trees as 
	// well the corresponding code needs to be implemented. 
	assert(!tn->ent_core_hook);
	return(_CloneChildren(tn));
}

void ADTNScopeEntry::DoRegistration(ANI::TRegistrationInfo *tri)
{
	do {
		// TRL_AutoVars is special because we need 2 passes. 
		if(tri->who!=ANI::TRegistrationInfo::TRL_AllIdentifiers && 
		   tri->who!=ANI::TRegistrationInfo::TRL_AutoVars)  break;
		
		assert(tri->anidescstmt);
		
		// The cc_factory pointer may be NULL if an error in the 
		// calc core name lookup occured. 
		if(!tri->anidescstmt->cc_factory)  break;
		
		CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			tri->anidescstmt->cc_factory;
		ccfact->DoADTNRegsistration(this,tri);
		
		return;
	} while(0);
	
	// Important because the calc core does this part in the above case. 
	// Pass on to value part. 
	down.last()->DoRegistration(tri);
}

int ADTNScopeEntry::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	// Need to get associated factory. 
	assert(ti->anidescstmt);
	CC::CCIF_Factory *ccfact=(CC::CCIF_Factory*)
			ti->anidescstmt->cc_factory;
	// ccfact may only be NULL if an error in registration step occured. 
	// But then we may not be here. 
	assert(ccfact);
	
	return(ccfact->DoADTNExprTF(ti,this));
}

int ADTNScopeEntry::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	return(down.last()->CheckIfEvaluable(ci));
}

}  // end of namespace CC
