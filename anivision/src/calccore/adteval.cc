/*
 * calccore/adteval.cc
 * 
 * Animation description tree eval code. 
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
#include <core/execthread.h>
#include <core/aniinstance.h>
#include <core/anicontext.h>

#include <calccore/ccif.h>


namespace CC
{

int ADTNAniDescBlock::Eval(ANI::ExprValue &nvalue,ANI::ExecThreadInfo *info)
{
	assert(info);
	ExecThread *et=(ExecThread*)info;
	
	if(!et->Context())
	{
		Error(GetLocationRange(),
			"attempt to execute ani desc stmt while not in animation "
			"context\n");
		abort();
	}
	
	// We're normally inside an TNAniDescStmt and need to know which 
	// calc core to use. 
	CCIF_Factory *cc_factory=NULL;
	do {
		if(!parent() || 
			parent()->NType()!=ANI::TN_Statement || 
			((ANI::TNStatement*)parent())->StmtType()!=ANI::TNS_AniDescStmt)
		{  break;  }
		cc_factory=(CCIF_Factory*)((ANI::TNAniDescStmt*)parent())->cc_factory;
	} while(0);
	// If the factory is not set, then this is 
	// - either due to an error in registration step 
	// - or because we have a different parent node. 
	// Both is currently illegal here. 
	assert(cc_factory);
	
	// NOTE!!
	// - nvalue (as passed) is NOT the return value but stores the 
	//   value we're animating. 
	// - nvalue is an lvalue (or an object)
	//   [i.e. if it is an object, it does not need to be an lvalue]
	switch(nvalue.GetExprValueTypeID())
	{
		case ANI::EVT_AniScope:
		{
			ANI::ScopeInstance _inst;
			nvalue.GetAniScope(&_inst);
			AniObjectInstance *inst=(AniObjectInstance*)_inst.GetASIB();
			assert(inst->ASType()==AniScopeBase::AST_Object);
			
			CCIF_Object *o_ccif=inst->GetOrCreateCCIF(cc_factory,et);
			assert(o_ccif);
			int rv=o_ccif->EvalAniDescBlock(this,et);
			switch(rv)
			{
				case 1:  return(1);  // thread switch
				case 0:  return(0);  // regular return
				case -1:  // (non-thread-switch) return with error
					#warning "Accumulate errors... FIXME"
					return(0);
				default: assert(0);
			}
		} break;
		case ANI::EVT_POD:
		{
			//ANI::ExprValueType evt;
			//nvalue.GetExprValueType(&evt);
			//const ANI::Value::CompleteType *pod=evt.PODType();
			//assert(pod);
			
			//CCIF_Setting *s_ccif=et->Context()->ccif;
			//assert(s_ccif);
			fprintf(stderr,"CC: IMPLEMENT ME: Eval ani desc block "
				"for POD (calccore)\n");
		} break;
		default: assert(0);
	}
	
	return(0);
}


}  // end of namespace CC
