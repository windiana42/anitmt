/*
 * calccore/anitmt/factory.cc
 * 
 * AniTMT calc core interface factory. 
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

#include <ani-parser/treereg.h>
#include <ani-parser/exprtf.h>

#include "factory.h"
#include "ccif_anitmt.h"

#include <functionality/make_avail.hpp>

// FIXME: HACK ALERT
namespace anitmt { namespace adlparser { int yydebug=0; } }

namespace CC
{


CCIF_Setting *CCIF_Factory_AniTMT::CreateCCIF_Setting()
{
	AniTMT::CCSetting *ccs=new AniTMT::CCSetting(this);
	return(ccs);
}


CCIF_Object *CCIF_Factory_AniTMT::CreateCCIF_Object(CCIF_Setting *setting,
	int vflags,AniObjectInstance *obj_inst)
{
	AniTMT::CCObject *cco=new AniTMT::CCObject((AniTMT::CCSetting*)setting,vflags,obj_inst);
	return(cco);
}


CCIF_Value *CCIF_Factory_AniTMT::CreateCCIF_Value(CCIF_Setting *setting,
	int dimension)
{
	assert(0); // not implemented.
	//AniTMT::CCValue *ccv=new AniTMT::CCValue((AniTMT::CCSetting*)setting,dimension);
	//return(ccv);
	return(NULL);
}


void CCIF_Factory_AniTMT::DoADTNRegsistration(ADTNAniDescBlock *adtn,
	ANI::TRegistrationInfo *tri)
{
	curr_depth=0;
	
	// Use default implementation. 
	CCIF_Factory::DoADTNRegsistration(adtn,tri);
	
	assert(!curr_depth);
}


void CCIF_Factory_AniTMT::DoADTNRegsistration(ADTNTypedScope *adtn,
	ANI::TRegistrationInfo *tri)
{
	using namespace AniTMT;
	
	// Use default implementation. 
	CCIF_Factory::DoADTNRegsistration(adtn,tri);
	
	// We do not need it (yet) but make the eval check happy...
	adtn->scope_type_tok=0;
}


void CCIF_Factory_AniTMT::DoADTNRegsistration(ADTNScopeEntry *adtn,
	ANI::TRegistrationInfo *tri)
{
	using namespace AniTMT;
	
	// Use default implementation: 
	CCIF_Factory::DoADTNRegsistration(adtn,tri);
}


String CCIF_Factory_AniTMT::TopScopeName(ADTNTypedScope *tn)
{
	String str;
	
	str+="implement me: TopScopeName";
	
	#if 0
	// This is more or less a quick hack and could be a bit more fancy. 
	if(tn->ADNType()==ADTN_ScopeEntry)
	{  tn=(ADTNTypedScope*)tn->parent();  }
	if(tn->ADNType()==ADTN_TypedScope)
	{
		str+=S::SCCTokenTranslator::Tok2Str((S::AniDescBlockTokID)
			((ADTNTypedScope*)tn)->scope_type_tok);
	}
	else assert(0);
	#endif
	
	return(str);
}


int CCIF_Factory_AniTMT::DoADTNExprTF(ANI::TFInfo *ti,ADTNAniDescBlock *adtn)
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

int CCIF_Factory_AniTMT::DoADTNExprTF(ANI::TFInfo *ti,ADTNTypedScope *adtn)
{
	using namespace AniTMT;
	
	++ti->n_nodes_visited;
	
	++curr_depth;
	
	int fail=0;
	for(ANI::TreeNode *tn=adtn->first_entry; tn; tn=tn->next)
	{
		#if 0
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
		#endif
		
		// Pass on to children: 
		if(tn->DoExprTF(ti,
			tn==adtn->first_entry ? 
				(ANI::TreeNode**)&adtn->first_entry : NULL))
		{  ++fail;  }
		
		#if 0
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
		#endif
	}
	
	--curr_depth;
	
	return(fail);
}

int CCIF_Factory_AniTMT::DoADTNExprTF(ANI::TFInfo *ti,ADTNScopeEntry *adtn)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	// NOTE: The entry lookup + grammar is now done in the 
	//       registration step. 
	// Pass on to value part: 
	if(adtn->down.last()->DoExprTF(ti))  ++fail;
	
	return(fail);
}


CCIF_Factory_AniTMT::CCIF_Factory_AniTMT(int _core_index) : 
	CCIF_Factory("anitmt",_core_index),
	msg_handler(std::cerr,std::cout,std::cout),
	msg_manager(&msg_handler),
	msg_consultant(&msg_manager,0),
	semi_global(/*param::Parameter_Manager *parameter_manager=*/NULL,
		&msg_consultant),
	priority_system(),
	pt_tree_info(&priority_system,&semi_global)
{
	curr_depth=0;
	
	// register animation tree nodes
	functionality::make_nodes_availible();
	
	fprintf(stdout,"AniTMT: AniTMT calc core (anitmt) initialized.\n");
}

CCIF_Factory_AniTMT::~CCIF_Factory_AniTMT()
{
	
}

}  // end of namespace CC
