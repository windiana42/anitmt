/*
 * calccore/scc/factory.cc
 * 
 * Simple calc core interface factory. 
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

#include "factory.h"
#include "ccif_simple.h"
#include "scc_parse.h"

#include <ani-parser/treereg.h>
#include <ani-parser/exprtf.h>


namespace CC
{

CCIF_Setting *CCIF_Factory_Simple::CreateCCIF_Setting()
{
	S::CCSetting *ccs=new S::CCSetting(this);
	return(ccs);
}


CCIF_Object *CCIF_Factory_Simple::CreateCCIF_Object(CCIF_Setting *setting,
	int vflags,AniObjectInstance *obj_inst)
{
	S::CCObject *cco=new S::CCObject((S::CCSetting*)setting,vflags,obj_inst);
	return(cco);
}


CCIF_Value *CCIF_Factory_Simple::CreateCCIF_Value(CCIF_Setting *setting,
	int dimension)
{
	S::CCValue *ccv=new S::CCValue((S::CCSetting*)setting,dimension);
	return(ccv);
}


void CCIF_Factory_Simple::DoADTNRegsistration(ADTNAniDescBlock *adtn,
	ANI::TRegistrationInfo *tri)
{
	curr_depth=0;
	
	// Use default implementation. 
	CCIF_Factory::DoADTNRegsistration(adtn,tri);
	
	assert(!curr_depth);
	
	#if 0  /* Do NOT remove the #if 0; could remove this from code. */
	if(tri->who==ANI::TRegistrationInfo::TRL_AllIdentifiers)
	{
		// Perform lookup of identifier (this has to be done 
		// by the calc core which will interprete this ADB, i.e. US.) 
		
		adtn->scope_type_tok=toktrans.LookupTypedScopeName(
			adtn->scope_type->name);
		if(adtn->scope_type_tok<0)
		{
			Error(adtn->scope_type->GetLocationRange(),
				"unknown ADB scope type name \"%s\" (core: %s)\n",
				adtn->scope_type->name.str(),
				Name());
			++tri->n_errors;
		}
	}
	
	// Pass on to children. 
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
	#endif
}


void CCIF_Factory_Simple::DoADTNRegsistration(ADTNTypedScope *adtn,
	ANI::TRegistrationInfo *tri)
{
	using namespace S;
	
	// We need to do the lookup before TRL_AllIdentifiers which is used 
	// by the grammar. 
	if(tri->who==ANI::TRegistrationInfo::TRL_AutoVars)
	{
		// Perform lookup of identifier (this has to be done 
		// by the calc core which will interprete this ADB, i.e. US.) 
		
		adtn->scope_type_tok=toktrans.LookupTypedScopeName(
			adtn->scope_type->name);
		//fprintf(stderr,"SCC: ADTN Lookup(%s)=%d\n",
		//	adtn->scope_type->name.str(),adtn->scope_type_tok);
		if(adtn->scope_type_tok<0)
		{
			Error(adtn->scope_type->GetLocationRange(),
				"unknown ADB scope type name \"%s\" (core: %s)\n",
				adtn->scope_type->name.str(),
				Name());
			++tri->n_errors;
		}
		
		// Pass on to children. 
		for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
		{  tn->DoRegistration(tri);  }
		
		return;
	}
	
	if(tri->who!=ANI::TRegistrationInfo::TRL_AllIdentifiers)
	{
		// Simply pass on to children. 
		for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
		{  tn->DoRegistration(tri);  }
		
		return;
	}
	
	if(!curr_depth)
	{
		// We're the typed scope of an entry in the toplevel ADB. 
		int rv;
		switch(adtn->scope_type_tok)
		{
			case S::TID_move:
				rv=stp->SetCurrentScope(
					S::SCC_ScopeTreeParser::SC_move);
				break;
			default:  rv=111111;  break;
		}
		switch(rv)
		{
			case 0:  break;
			case 111111:
				Error(adtn->scope_type->GetLocationRange(),
					"cannot use \"%s\" in toplevel ADB\n",
					S::SCCTokenTranslator::Tok2Str(
					(S::AniDescBlockTokID)adtn->scope_type_tok));
				++tri->n_errors;;
				break;
			case 1:  // fall through
			default:  assert(0);
		}
	}
	
	++curr_depth;
	
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{
		assert(tn->NType()==ANI::TN_AniDesc);
		assert(((ADTreeNode*)tn)->ADNType()==ADTN_ScopeEntry);
		ADTNScopeEntry *se=(ADTNScopeEntry*)tn;
		
		ADTNScopeEntry_CoreHook *se_ch=
			(ADTNScopeEntry_CoreHook*)se->ent_core_hook;
		assert(se_ch);
		
		bool is_scope;
		bool success=0;
		if(se->down.last()->NType()==ANI::TN_Expression)
		{
			int rv;
			if(!se_ch->entry_name_tok)
			{  rv=stp->ParseEntry(&se_ch->scc_expr_type_id,
				&se_ch->entry_name_tok);  }
			else
			{  rv=stp->ParseEntryN(se_ch->entry_name_tok,
				&se_ch->scc_expr_type_id);  }
			is_scope=0;
			
			switch(rv)
			{
				case 0:  success=1;  break;
				case 1:
					Error(se->down.first()->GetLocationRange(),
						"unknown property \"%s\" in %s\n",
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)se_ch->entry_name_tok),
						TopScopeName(adtn).str());
					++tri->n_errors;;
					break;
				case 2:  break;  // error in prev level (ignore)
				case 3:
					Error(se->down.first()->GetLocationRange(),
						"property \"%s\" in %s requires a scope "
						"(not an expression)\n",
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)se_ch->entry_name_tok),
						TopScopeName(adtn).str());
					++tri->n_errors;;
					break;
				case 4:
					Error(se->down.last()->GetLocationRange(),
						"invalid anonymous property in %s\n",
						TopScopeName(adtn).str());
					++tri->n_errors;;
			 		break;
				case -2:  break;  // Ignore!! (SetCurrentScope() failure)
				default:  assert(0);
			}
		}
		else if(se->down.last()->NType()==ANI::TN_AniDesc && 
			((ADTreeNode*)se->down.last())->ADNType()==ADTN_TypedScope)
		{
			ADTNTypedScope *child_sc=(ADTNTypedScope*)se->down.last();
			
			int rv;
			if(!se_ch->entry_name_tok)
			{  rv=stp->ParseEnterScope(child_sc->scope_type_tok,
				&se_ch->entry_name_tok);  }
			else
			{  rv=stp->ParseEnterScopeN(child_sc->scope_type_tok,
				se_ch->entry_name_tok);  }
			is_scope=1;
			
			switch(rv)
			{
				case 0:  success=1;  break;
				case 1:
					Error(se->down.first()->GetLocationRange(),
						"unknown property \"%s\" in %s\n",
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)se_ch->entry_name_tok),
						TopScopeName(adtn).str());
					++tri->n_errors;;
					break;
				case 2:  break;  // error in prev level (ignore)
				case 3:
					Error(se->down.first()->GetLocationRange(),
						"cannot use scope \"%s\" for property \"%s\" in %s\n",
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)child_sc->scope_type_tok),
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)se_ch->entry_name_tok),
						TopScopeName(adtn).str());
					++tri->n_errors;;
					break;
				case 4:
					Error(se->down.last()->GetLocationRange(),
						"invalid anonymous \"%s\" property in %s\n",
						S::SCCTokenTranslator::Tok2Str(
							(S::AniDescBlockTokID)child_sc->scope_type_tok),
						TopScopeName(adtn).str());
					++tri->n_errors;;
			 		break;
				case -2:  break;  // Ignore!! (SetCurrentScope() failure)
				default:  assert(0);
			}
		}
		else assert(0);
		
		// Pass on to children. 
		tn->DoRegistration(tri);
		
		if(is_scope)
		{
			int rv=stp->ParseLeaveScope();
			assert(rv==0);
		}
	}
	
	--curr_depth;
}


void CCIF_Factory_Simple::DoADTNRegsistration(ADTNScopeEntry *adtn,
	ANI::TRegistrationInfo *tri)
{
	using namespace S;
	
	// We need to do the lookup before TRL_AllIdentifiers which is used 
	// by the grammar. 
	if(tri->who==ANI::TRegistrationInfo::TRL_AutoVars)
	{
		// Attach a core hook to the entry (if there is none): 
		if(!adtn->ent_core_hook)
		{  adtn->ent_core_hook=new ADTNScopeEntry_CoreHook();  }
		ADTNScopeEntry_CoreHook *ch=
			(ADTNScopeEntry_CoreHook*)adtn->ent_core_hook;
		
		// Lookup name part if any: 
		// Perform lookup of identifier (this has to be done 
		// by the calc core which will interprete this ADB, i.e. US.) 
		
		RefString ename;
		ANI::TNIdentifier *idf=NULL;
		if(adtn->down.last()!=adtn->down.first())
		{
			idf=(ANI::TNIdentifier*)adtn->down.first();
			assert(idf->NType()==ANI::TN_Identifier);
			ename=idf->CompleteStr();
		}
		// else: ename stays NULL ref. 
		
		ch->entry_name_tok=toktrans.LookupScopeEntryName(ename);
		if(ch->entry_name_tok<0)
		{
			if(ename && idf)
			{  Error(idf->GetLocationRange(),
				"unknown ADB entry name \"%s\" (core: %s)\n",
				ename.str(),Name());  }
			else assert(0);

			++tri->n_errors;
		}
		
		// Pass on to value part. 
		adtn->down.last()->DoRegistration(tri);
		
		return;
	}
	
	if(tri->who!=ANI::TRegistrationInfo::TRL_AllIdentifiers)
	{
		// Use default implementation. 
		// (We currently will not get here anyways...)
		CCIF_Factory::DoADTNRegsistration(adtn,tri);
		return;
	}
	
	if(!curr_depth)
	{
		// We're an entry in the toplevel ADB. 
		
		if(adtn->down.last()->NType()==ANI::TN_Expression)
		{
			Error(adtn->down.last()->GetLocationRange(),
				"cannot use expressions in toplevel ADB\n");
			++tri->n_errors;
		}
		else if(adtn->down.last()->NType()==ANI::TN_AniDesc && 
			((ADTreeNode*)adtn->down.last())->ADNType()==ADTN_TypedScope)
		{
			ADTNTypedScope *sc=(ADTNTypedScope*)adtn->down.last();
			assert(sc->scope_type_tok>0);  // not =0 (IMO)
			
			// This is okay... but currently no "lvalues", i.e. left names 
			// are allowed. 
			ADTNScopeEntry_CoreHook *ch=
				(ADTNScopeEntry_CoreHook*)adtn->ent_core_hook;
			assert(ch);
			assert(ch->entry_name_tok>=0);
			if(ch->entry_name_tok!=0)
			{
				Error(adtn->down.first()->GetLocationRange(),
					"cannot use named properties (like \"%s\") in "
					"toplevel ADB\n",
					S::SCCTokenTranslator::Tok2Str(
						(S::AniDescBlockTokID)ch->entry_name_tok));
				++tri->n_errors;
			}
		}
		else assert(0);
	}
	
	//---<lookup and grammar done>---
	
	// FIXME: All this should be put into some extra interface which 
	//        can handle several parameters etc. 
	// Check if we need to attach special identifier table (for things 
	// like function {$t}). 
	// This is a hack because this is currently only possible for 
	// function{ pos=... }. 
	// Note that all the entry_name_tok's are already looked up and 
	// the not specified name parts ones were replaced by the ones 
	// expected from the order. 
	int n_entries_attached=0;
	if(adtn->parent()->NType()==ANI::TN_AniDesc && 
	   ((ADTreeNode*)adtn->parent())->ADNType()==ADTN_TypedScope)
	{
		ADTNScopeEntry_CoreHook *ch=
			(ADTNScopeEntry_CoreHook*)adtn->ent_core_hook;
		assert(ch);
		
		ADTNTypedScope *sc=(ADTNTypedScope*)adtn->parent();
		if(sc->scope_type_tok==TID_function)
		{
			if(ch->entry_name_tok==TID_pos)
			{
				// This is the special core hook which is used to be able 
				// to supply the value of the $t for evaluation. 
				CCIF_SpecialIdf_CoreHook_T *special_ch=
					new CCIF_SpecialIdf_CoreHook_T();
				// Attach the special_ch to the ADTNScopeEntry_CoreHook: 
				ch->func_val_t=special_ch;
				special_ch->aqref();
				
				// Need to attach DynIdf: ADD AT THE END!
				// Things like function { pos=function {... }} (although 
				// illegal in this case) are not a problem because the 
				// lookup goes reverse in the list. 
				DynIdf *di=new DynIdf();
				di->name="$t";
				di->vtype=ANI::ExprValueType::RScalarType();
				di->adb_idf_core_hook=special_ch;
				// NO special_ch->aqref(); here; see detach below. 
				
				tri->special_idf_list.append(di);
				++n_entries_attached;
			}
		}
	}
	
	// Pass on to value part. 
	adtn->down.last()->DoRegistration(tri);
	
	// Detach those entries which were just attached: 
	int n_entries_unused=0;
	for(int _i=0; _i<n_entries_attached; _i++)
	{
		// REMOVE FROM THE END!
		DynIdf *di=(DynIdf*)tri->special_idf_list.poplast();
		assert(di);
		if(di->adb_idf_core_hook && 
			di->adb_idf_core_hook->core_hook_used==1)  // <-- CORRECT!
		{
			// The attached special identifier was not used to create 
			// an AniInternalVariable_ADB. 
			++n_entries_unused;
			// NOTE: We do NOT delete the hook sice it is still attached 
			//       to the ADTNScopeEntry_CoreHook. This is also the 
			//       reason for the check against ==1 above. 
		}
		DELETE(di);
	}
	
	if(n_entries_attached && n_entries_unused==n_entries_attached)
	{
		// This warning occurs if you write a constant expression 
		// inside a function{} block. 
		Warning(adtn->down.last()->GetLocationRange(),
			"expression in function{} block completely independent "
			"of variables\n");
	}
}


String CCIF_Factory_Simple::TopScopeName(ADTNTypedScope *tn)
{
	String str;
	
	// This is more or less a quick hack and could be a bit more fancy. 
	if(tn->ADNType()==ADTN_ScopeEntry)
	{  tn=(ADTNTypedScope*)tn->parent();  }
	if(tn->ADNType()==ADTN_TypedScope)
	{
		str+=S::SCCTokenTranslator::Tok2Str((S::AniDescBlockTokID)
			((ADTNTypedScope*)tn)->scope_type_tok);
	}
	else assert(0);
	
	return(str);
}


int CCIF_Factory_Simple::DoADTNExprTF(ANI::TFInfo *ti,ADTNAniDescBlock *adtn)
{
	++ti->n_nodes_visited;
	
	curr_depth=0;
	
	int fail=0;
	for(ANI::TreeNode *tn=adtn->down.first(); tn; tn=tn->next)
	{
		if(tn->DoExprTF(ti))  ++fail;
	}
	
	assert(!curr_depth);
	
	return(fail);
}

int CCIF_Factory_Simple::DoADTNExprTF(ANI::TFInfo *ti,ADTNTypedScope *adtn)
{
	using namespace S;
	
	++ti->n_nodes_visited;
	
	++curr_depth;
	
	int fail=0;
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{
		assert(tn->NType()==ANI::TN_AniDesc);
		assert(((ADTreeNode*)tn)->ADNType()==ADTN_ScopeEntry);
		ADTNScopeEntry *se=(ADTNScopeEntry*)tn;
		
		bool is_scope;
		if(se->down.last()->NType()==ANI::TN_Expression)
		{  is_scope=0;  }
		else if(se->down.last()->NType()==ANI::TN_AniDesc && 
			((ADTreeNode*)se->down.last())->ADNType()==ADTN_TypedScope)
		{  is_scope=1;  }
		else assert(0);
		
		// Pass on to children: 
		if(tn->DoExprTF(ti,
			tn==adtn->first_entry ? 
				(ANI::TreeNode**)&adtn->first_entry : NULL))
		{  ++fail;  }
		
		if(!is_scope)
		{
			// Check expr_type_id. 
			ADTNScopeEntry_CoreHook *se_ch=
				(ADTNScopeEntry_CoreHook*)se->ent_core_hook;
			assert(se_ch->scc_expr_type_id>=0);
			switch(stp->CheckType(ti->tp.sevt,se_ch->scc_expr_type_id))
			{
				case 0:  break;
				case 1:
					Error(se->down.last()->GetLocationRange(),
						"invalid type (%s instead of %s) for "
						"property \"%s\" in %s\n",
						ti->tp.sevt.TypeString().str(),
						stp->TypeID2Str((S::SCC_ScopeTreeParser::TypeID)
							se_ch->scc_expr_type_id),
						S::SCCTokenTranslator::Tok2Str(
							(AniDescBlockTokID)(se_ch->entry_name_tok)),
						TopScopeName(adtn).str());
					++fail;
					break;
				default:  assert(0);
			}
		}
	}
	
	--curr_depth;
	
	return(fail);
}

int CCIF_Factory_Simple::DoADTNExprTF(ANI::TFInfo *ti,ADTNScopeEntry *adtn)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	// NOTE: The entry lookup + grammar is now done in the 
	//       registration step. 
	// Pass on to value part: 
	if(adtn->down.last()->DoExprTF(ti))  ++fail;
	
	return(fail);
}


CCIF_Factory_Simple::CCIF_Factory_Simple(int _core_index) : 
	CCIF_Factory("scc",_core_index),
	toktrans()
{
	stp=new S::SCC_ScopeTreeParser();
	curr_depth=0;
	
	fprintf(stdout,"SCC: Simple calc core (scc) initialized.\n");
}

CCIF_Factory_Simple::~CCIF_Factory_Simple()
{
	DELETE(stp);
}

//------------------------------------------------------------------------------

namespace S
{

ANI::ExprValue CCIF_SpecialIdf_CoreHook_T::Eval(ANI::TNIdentifier *idf,
	ANI::ExecThreadInfo *info)
{
	// Make sure the value is not a NULL ref. 
	// In case it is we either forgot to attach a value or we're 
	// here due to illegal reason. 
	assert(!!t_val);
	return(t_val);
}

//------------------------------------------------------------------------------

S::ADTNScopeEntry_CoreHook::~ADTNScopeEntry_CoreHook()
{
	// Dereference: 
	if(func_val_t)
	{
		func_val_t->deref();
		func_val_t=NULL;
	}
	entry_name_tok=-1;
	scc_expr_type_id=-1;
}

}  // end of namespace S

}  // end of namespace CC
