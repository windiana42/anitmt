/*
 * calccore/scc/basefunc.cc
 * 
 * Simple calc core basic object/value functionality. 
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

#include "ccif_simple.h"

#include <core/execthread.h>

#include <numerics/spline/cspline.h>

#include <objdesc/omatrix.h>


namespace CC { namespace S
{


int CCBaseFunctionality::InterpreteScopeEntries(
	ADTNTypedScope *tsc,ExecThread *et,
	const char *scope_name,void *dptr,int slot_id)
{
	InterpreteScopeEntries_IData *idata=
		(InterpreteScopeEntries_IData*)et->ess->popPtr(NULL);
	if(!idata)
	{  idata=new InterpreteScopeEntries_IData(tsc->first_entry);  }
	
	for(ANI::TreeNode *_tn=idata->curr_tn; _tn; _tn=_tn->next)
	{
		if(_tn->NType()!=ANI::TN_AniDesc) assert(0);
		if(((ADTreeNode*)_tn)->ADNType()!=ADTN_ScopeEntry) assert(0);
		
		ADTNScopeEntry *scent=(ADTNScopeEntry*)_tn;
		//ANI::TNIdentifier *name = (scent->down.first()==scent->down.last() ? 
		//	NULL : (ANI::TNIdentifier*)scent->down.first());
		ANI::TreeNode *value=scent->down.last();
		
		// Go back to the calling level via InterpreteScopeSlot(): 
		InterpreteScopeEntriesInfo info;
		info.tok_id=(AniDescBlockTokID)(
			(ADTNScopeEntry_CoreHook*)scent->ent_core_hook)->entry_name_tok;
		info.value=value;
		info.tsc=NULL;
		info.tsc_tok_id=TID_None;
		// We may need the type of the value part: 
		if(value)
		{
			if(value->NType()==ANI::TN_AniDesc && 
				((ADTreeNode*)value)->ADNType()==ADTN_TypedScope)
			{
				info.tsc=(ADTNTypedScope*)value;
				info.tsc_tok_id=(AniDescBlockTokID)info.tsc->scope_type_tok;
				assert(info.tsc_tok_id>=0);  // -1 -> lookup error -> we may not be here
			}
		}
		info.dptr=dptr;
		InterpreteScopeSlot_State rv=(InterpreteScopeSlot_State)
			InterpreteScopeSlot(slot_id,et,&info);
		switch(rv)
		{
			case ISS_Cont:  break;
			case ISS_OOPSError:
				// This may not happen. 
				assert(0);
				++n_errors;
				break;
			case ISS_CSwitch:
				// Context switch. 
				idata->curr_tn=_tn;
				et->ess->pushPtr(idata);
				return(1);
			case ISS_AlreadySet:
				Error(value->GetLocationRange(),
					"%s scope property \"%s\" was already set\n",
					scope_name,
					SCCTokenTranslator::Tok2Str((AniDescBlockTokID)
						((ADTNScopeEntry_CoreHook*)scent->ent_core_hook)->
							entry_name_tok));
				++n_errors;
				break;
			case ISS_TimeSpecConflict:
				Error(value->GetLocationRange(),
					"conflicting %s time specs\n",
					scope_name);
				++n_errors;
				break;
			case ISS_IllegalValue:
				Error(value->GetLocationRange(),
					"illegal value for \"%s\" property in %s scope\n",
					SCCTokenTranslator::Tok2Str((AniDescBlockTokID)
						((ADTNScopeEntry_CoreHook*)scent->ent_core_hook)->
							entry_name_tok),
					scope_name);
				++n_errors;
				break;
			default:  assert(0);
		}
	}
	
	delete idata;
	return(0);
}


int CCBaseFunctionality::InterpreteScopeSlot(
		int /*slot_id*/,ExecThread * /*et*/,InterpreteScopeEntriesInfo * /*info*/)
{
	// Must be overridden when used. 
	assert(0);
	return(ISS_OOPSError);
}


void CCBaseFunctionality::_UnknownProperty(ANI::TNIdentifier *name,
	ANI::TreeNode *value,const char *scope_name)
{
	if(name)
	{
		Error(name->GetLocationRange(),
			"unknown property \"%s\" in %s scope [ignored]\n",
			name->CompleteStr().str(),scope_name);
	}
	else
	{
		Error(value->GetLocationRange(),
			"anonymous property in %s scope [ignored]\n",
			scope_name);
	}
	++n_errors;
}


// Eval scalar expression. 
// Return value: 
//   0 -> OK, value read and stored in *ret_val
//   1 -> context switch
//  -1 -> error: message written, n_errors incremented and *ret_val 
//        not modified
int CCBaseFunctionality::_GetScalarValue(double &ret_val,ANI::TreeNode *tn,
	ExecThread *et)
{
	do {
		if(tn->NType()!=ANI::TN_Expression)  break;
		
		ANI::TNExpression *expr=(ANI::TNExpression*)tn;
		ANI::ExprValue tmp;
		if(expr->Eval(tmp,et))
		{  return(1);  }
		
		if(tmp.GetExprValueTypeID()!=ANI::EVT_POD)  break;
		
		Value valtmp;
		tmp.GetPOD(&valtmp);
		if(valtmp.Type()!=Value::VTScalar && 
		   valtmp.Type()!=Value::VTInteger)
		{  break;  }
		
		ret_val=valtmp.GetScalar().val();  // Will convert int->double if needed. 
		return(0);
	} while(0);
	
	//tn->DumpTree(stdout);
	Error(tn->GetLocationRange(),
		"invalid argument: (scalar) expression expected\n");
	++n_errors;
	return(-1);
}


// Like _GetScalarValue for strings. 
int CCBaseFunctionality::_GetStringValue(String &ret_val,ANI::TreeNode *tn,
	ExecThread *et)
{
	do {
		if(tn->NType()!=ANI::TN_Expression)  break;
		
		ANI::TNExpression *expr=(ANI::TNExpression*)tn;
		ANI::ExprValue tmp;
		if(expr->Eval(tmp,et))
		{  return(1);  }
		
		if(tmp.GetExprValueTypeID()!=ANI::EVT_POD)  break;
		
		Value valtmp;
		tmp.GetPOD(&valtmp);
		if(valtmp.Type()!=Value::VTString)
		{  break;  }
		
		ret_val=valtmp.GetString();
		return(0);
	} while(0);
	
	//tn->DumpTree(stdout);
	Error(tn->GetLocationRange(),
		"invalid argument: (string) expression expected\n");
	++n_errors;
	return(-1);
}


// Like _GetScalarValue for range. 
int CCBaseFunctionality::_GetRangeValue(Range &ret_val,ANI::TreeNode *tn,
	ExecThread *et)
{
	do {
		if(tn->NType()!=ANI::TN_Expression)  break;
		
		ANI::TNExpression *expr=(ANI::TNExpression*)tn;
		ANI::ExprValue tmp;
		if(expr->Eval(tmp,et))
		{  return(1);  }
		
		if(tmp.GetExprValueTypeID()!=ANI::EVT_POD)  break;
		
		Value valtmp;
		tmp.GetPOD(&valtmp);
		if(valtmp.Type()!=Value::VTRange)
		{  break;  }
		
		ret_val=valtmp.GetRange();
		return(0);
	} while(0);
	
	//tn->DumpTree(stdout);
	Error(tn->GetLocationRange(),
		"invalid argument: (range) expression expected\n");
	++n_errors;
	return(-1);
}


// Like _GetScalarValue for vector<3>. 
int CCBaseFunctionality::_GetVector3Value(double *vector,ANI::TreeNode *tn,
	ExecThread *et)
{
	do {
		if(tn->NType()!=ANI::TN_Expression)  break;
		
		ANI::TNExpression *expr=(ANI::TNExpression*)tn;
		ANI::ExprValue tmp;
		if(expr->Eval(tmp,et))
		{  return(1);  }
		
		if(tmp.GetExprValueTypeID()!=ANI::EVT_POD)  break;
		
		Value valtmp;
		tmp.GetPOD(&valtmp);
		if(valtmp.Type()==Value::VTScalar || 
		   valtmp.Type()==Value::VTInteger)
		{
			// Will convert int->double if needed. 
			double tmp=valtmp.GetScalar().val();
			// Promote vector: 
			for(int i=0; i<3; i++)
				vector[i]=tmp;
			return(0);
		}
		if(valtmp.Type()==Value::VTVector)
		{
			const Vector *vect=valtmp.GetPtr<Vector>();
			assert(vect->dim()==3);
			for(int i=0; i<3; i++)
				vector[i]=(*vect)[i];
			return(0);
		}
	} while(0);
	
	//tn->DumpTree(stdout);
	Error(tn->GetLocationRange(),
		"invalid argument: (vector<3>) expression expected\n");
	++n_errors;
	return(-1);
}


// Eval vector array expression. 
// Return value: see _GetScalarValue(). 
int CCBaseFunctionality::_GetVectorArray(NUM::VectorArray<double> &array,
	ANI::TreeNode *tn,ExecThread *et,int vec_dim)
{
	do {
		if(tn->NType()!=ANI::TN_Expression)  break;
		
		ANI::TNExpression *expr=(ANI::TNExpression*)tn;
		ANI::ExprValue tmp;
		if(expr->Eval(tmp,et))
		{  return(1);  }
		
		ANI::ExprValueType evt;
		tmp.GetExprValueType(&evt);
		//fprintf(stderr,"TYPE=%s\n",evt.TypeString().str());
		const Value::CompleteType *pod_elem_type=evt.ArrayType().PODType();
		if(!pod_elem_type || pod_elem_type->type!=Value::VTVector)  break;
		
		ANI::ExprValueArray tmparr;
		tmp.GetArray(&tmparr);
		ssize_t arrsize=tmparr.GetSize();
		assert(arrsize>=0);
		
		const int vdim=pod_elem_type->n;
		assert(vdim>=1);
		if(vec_dim>=0 && vdim!=vec_dim)  break;
		array.Resize(arrsize,vdim);
	
		ANI::ExprValue elemtmp;
		Value vecttmp;
		for(ssize_t idx=0; idx<arrsize; idx++)
		{
			int rv=tmparr.IndexGet(idx,&elemtmp);
			assert(!rv);
			elemtmp.GetPOD(&vecttmp);
			const Vector *vect=vecttmp.GetPtr<Vector>();
			assert(vect->dim()==vdim);
			for(int i=0; i<vdim; i++)
			{  array[idx][i]=(*vect)[i];  }
		}
		
		return(0);
	} while(0);
	
	//tn->DumpTree(stdout);
	char tmpstr[24];
	if(vec_dim<0)
	{  *tmpstr='\0';  }
	else
	{  snprintf(tmpstr,24,"<%d>",vec_dim);  }
	Error(tn->GetLocationRange(),
		"invalid argument: (vector%s[]) expression expected\n",tmpstr);
	++n_errors;
	return(-1);
}


int CCBaseFunctionality::AquireADBLock(ADTNAniDescBlock *adb,ExecThread *et)
{
	if(this->lock_thread!=et->thread_id || this->lock_adb!=adb)
	{
		// The object is locked by another thread. 
		// First, see if this other thread still exists or if it 
		// died in the meantime. 
		#warning FIXME FIXME FIXME FIXME FIXME
		// FIXME: Need better locking mechanism. thread id may be re-used 
		//        and is only unique on a per-context basis. 
		// NOTE: Objects can be shared between different contexts (via 
		//       static pointers). 
		// Problem: Threads suspended in ADBs during context switch, e.g. 
		// between two settings. 
		// Und was ist überhaupt mit dem CCObject, wenn es von mehreren 
		// contexts bearbeitet wird...? Scheiße. 
		// Nö... keine Scheiße: der ADB bleibt eben dann gelockt, bis der 
		// ursprüngliche context wieder dran ist. 
		
		// NOTE: need to make sure that we lose the lock when the thread 
		// is killed (hence the TestThreadExist() below). 
		
		// PROBABLY we do not need any locking at all (???) if we 
		// put n_errors into some info structure like 
		// InterpreteScopeEntries_IData *idata. 
		
		// ...and of course we need an "incomplete" flag for the movements. 
		
		static int warned=0;
		if(!warned)
		{  fprintf(stderr,"TODO: Correct thread lock issue!! (basefunc.cc)\n");
			++warned;  }
		
		if(!et->TestThreadExist(this->lock_thread))
		{  LoseADBLock();  }
		else
		{
			Error(adb->GetLocationRange(),
				"ADB cannot be executed by thread [%d] because...\n",
				this->lock_thread);
			Error(this->lock_adb->GetLocationRange(),
				"...thread [%d] currently holds ADB lock [skipped]\n",
				this->lock_thread);
			return(1);
		}
	}
	
	if(this->lock_thread<0)
	{
		assert(!lock_adb);
		this->lock_thread=et->thread_id;
		this->lock_adb=adb;
		assert(et->thread_id>0);  // thread 0 is non-ani context
		n_errors=0;
	}
	
	return(0);
}


void CCBaseFunctionality::LoseADBLock()
{
	this->lock_thread=-1;
	this->lock_adb=NULL;
	// n_errors not yet cleared. 
}


CCBaseFunctionality::CCBaseFunctionality()
{
	lock_thread=-1;
	lock_adb=NULL;
	n_errors=0;
}

CCBaseFunctionality::~CCBaseFunctionality()
{
	//assert(lock_thread<0 && !lock_adb); <-- NO; thread may have been killed
}

}}  // end of namespace CC::S
