/*
 * calccore/scc/objeval.cc
 * 
 * Simple calc core object evauation routines. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "ccif_simple.h"

#include <objdesc/omatrix.h>

#include <objdesc/curve_fixed.h>
#include <objdesc/curve_lspline.h>
#include <objdesc/curve_cspline.h>
#include "curve_function.h"

#include <objdesc/curvetmap_none.h>
#include <objdesc/curvetmap_linear.h>

#include <objdesc/frontspec_speed.h>

#include <objdesc/upspec_gravity.h>


namespace CC { namespace S
{

// Exclusively used by EvalAniDescBlock(). 
int CCObject::_DoEvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et)
{
	for(ANI::TreeNode *_tn=et->ess->popTN(adb->down.first()); 
		_tn; _tn=_tn->next)
	{
		if(_tn->NType()!=ANI::TN_AniDesc) assert(0);
		if(((ADTreeNode*)_tn)->ADNType()!=ADTN_ScopeEntry) assert(0);
		
		ADTNScopeEntry *scent=(ADTNScopeEntry*)_tn;
		//ANI::TNIdentifier *name = (scent->down.first()==scent->down.last() ? 
		//	NULL : (ANI::TNIdentifier*)scent->down.first());
		ANI::TreeNode *value=scent->down.last();
		
		// This is checked in ADTN's DoExprTF():  
		assert(!scent->entry_name_tok);  // 0 -> not set
		
		if(value->NType()==ANI::TN_AniDesc && 
			((ADTreeNode*)value)->ADNType()==ADTN_TypedScope)
		{
			ADTNTypedScope *tsc=(ADTNTypedScope*)value;
			
			//RefString tsc_type(tsc->scope_type->name);
			switch(tsc->scope_type_tok)
			{
				case TID_move:
				{
					NUM::OMovement *movement=NULL;
					if(_InterpreteMoveScope(tsc,et,&movement))
					{
						// Context switch. 
						et->ess->pushTN(_tn);
						return(1);
					}
					
					// Got a new movement; add to list. 
					movements.append(movement);
					
					// Finally create the movement: 
					// This will generate all the internal data and also 
					// try to achieve continuacy,... 
					int rv=movement->Create(setting->CurrentSettingTime());
					if(rv)
					{  ++n_errors;  }
					
					// FIXME: Need to check if successive movements overlap 
					// in time (using movement->{T0,T1}(). 
				}  break;
				default:
					// This should have been caught during ADTN's DoExprTF(). 
					assert(0);
					++n_errors;
			}
		}
		else
		{
			// This should have been caught during ADTN's DoExprTF(). 
			assert(0);
			++n_errors;  continue;
		}
	}
	
	return(0);
}


int CCObject::EvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et)
{
// We must make sure that only one thread at a time executes an 
// ani desc block. 
fprintf(stderr,"Why the hell?!\n");  // -> check n_errors and the like
if(AquireADBLock(adb,et))
{  return(0);  }  // <-- No context switch (could use 1 for waiting, too). 
	
	fprintf(stdout,"CC: Interpreting ADB @%s\n",
		adb->GetLocationRange().PosRangeString().str());
	
	int rv=_DoEvalAniDescBlock(adb,et);
	
	if(rv)
	{
		// Context switch. 
		if(et->schedule_status==ExecThread::SSRequest)
		{
			Error(adb->GetLocationRange(),
				"OOPS: Cloning from inside ADB forbidden\n");
			abort();
		}
		
		return(1);
	}
	
	// If we come here, there is regular exit, i.e. no context switch. 
LoseADBLock();  // Does NOT clear n_errors. 
	return(n_errors ? -1 : 0);
}

}}  // end of namespace CC::S
