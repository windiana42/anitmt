/*
 * core/fixinctype.cc
 * 
 * Fixup incomplete types. 
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

// Fix incomplete type specs. 
// This could be replaced with a more general constant folding 
// step which can then also handle things like "vector<2+3>" or 
//   const int nstars=1000;
//   vector starpos[nstars];
// Would probably require some dependency/solve however. 
// And you see: Even the most trivial approach is more than 
// a three-liner...
//      -- Wolfgang

#include "aniscope.h"
#include <limits.h>


using namespace ANI;


int AniScopeBase::_ConstExprEval(TNExpression *expr,int *ret_val,
	int non_negative,int short_val)
{
	// Only immediate types are allowed: 
	while(expr->ExprType()==TNE_Value)
	{
		TNValue *tnval=(TNValue*)expr;
		ExprValue *ev=&tnval->val;
		if(ev->GetExprValueTypeID()!=EVT_POD)  break;
		Value v;
		ev->GetPOD(&v);
		if(v.Type()==Value::VTInteger)
		{
			int i=v.GetIntegerPtr()->val();
			if(non_negative && i<0)
			{
				Error(expr->GetLocationRange(),
					"illegal negative value\n");
				return(1);
			}
			if(short_val && (i>SHRT_MAX) || (i<SHRT_MIN))
			{
				Error(expr->GetLocationRange(),
					"value %d too large (max %d)\n",
					i,i>0 ? SHRT_MAX : SHRT_MIN);
				return(1);
			}
			*ret_val=i;
			return(0);
		}
		Error(expr->GetLocationRange(),
			"value not an integer as expected\n");
		return(1);
	}
	Error(expr->GetLocationRange(),
		"expression not an immediate value\n");
	return(1);
}


int AniScopeBase::_DoFixIncompleteType(ExprValueType *evt,AniScopeBase *scope,
	TNTypeSpecifier *tspec,TNDeclarator *decl)
{
	// NOTE THAT decl MAY BE NULL (e.g. for function return types). 
	assert(tspec->NType()==TN_TypeSpecifier);
	assert(!decl || decl->NType()==TN_Declarator);
	
	// Okay, let's look closer: 
	const Value::CompleteType *pod=evt->PODType();
	if(pod)
	{
		// Yeah, it's a POD type. 
		Value::CompleteType mypod=*pod;
		if(pod->type==Value::VTVector && pod->n==-1)
		{
			// Aha... incomplete vector size. 
			assert(tspec->ev_type_id==EVT_POD && tspec->vtype==Value::VTVector);
			assert(tspec->down.count()==1);
			TNExpression *expr=(TNExpression*)tspec->down.first();
			if(_ConstExprEval(expr,&mypod.n,/*non_negative=*/1,/*short_val=*/0))
			{  return(1);  }
			evt->SetPOD(mypod);
		}
		else if(pod->type==Value::VTMatrix && (pod->r==-1 || pod->c==-1))
		{
			// Aha... incomplete matrix size. 
			assert(tspec->ev_type_id==EVT_POD && tspec->vtype==Value::VTMatrix);
			assert(tspec->down.count()==2);
			TNExpression *expr=(TNExpression*)tspec->down.first();
			int tmp;
			if(_ConstExprEval(expr,&tmp,/*non_negative=*/1,/*short_val=*/1))
			{  return(1);  }
			mypod.r=tmp;
			expr=(TNExpression*)tspec->down.last();
			if(_ConstExprEval(expr,&tmp,/*non_negative=*/1,/*short_val=*/1))
			{  return(1);  }
			mypod.c=tmp;
			evt->SetPOD(mypod);
		}
	}
	else if(evt->TypeID()==EVT_Array)
	{
		// NOTE: Array spec can be in TNTypeSpecifier and in TNDeclarator. 
		//       The one in TNTypeSpecifier is the major type and always 
		//       ArrSize_Unspecified (as opposed to ArrSize_Incomplete). 
		if(tspec->ev_type_id==EVT_Array)
		{
			ExprValueType tmp_ev=evt->ArrayType();
			if(_DoFixIncompleteType(&tmp_ev,scope,
				(TNTypeSpecifier*)tspec->down.first(),decl))
			{  return(1);  }
			evt->SetArray(tmp_ev);
		}
		else 
		{
			assert(decl);  // May not be NULL here. 
			// Skip initializer if present: 
			if(decl->GetDeclType()==TNDeclarator::DT_Initialize)
			{  decl=(TNDeclarator*)decl->down.first();  }
			assert(decl->GetDeclType()==TNDeclarator::DT_Array);
			
			// "int arr[17]" is not allowed, must use "int arr[]=new int[17]", 
			// so the array declarator may only have one child. 
			assert(decl->down.count()==1);
			
			// Maybe the next dimension or the underlaying type is 
			// incomplete. 
			ExprValueType tmp_ev=evt->ArrayType();
			if(_DoFixIncompleteType(&tmp_ev,scope,
				tspec,(TNDeclarator*)decl->down.first()))
			{  return(1);  }
			evt->SetArray(tmp_ev);
		}
	}
	
	assert(!evt->IsIncomplete());
	return(0);
}


int AniScopeBase::_FixEntriesInTypeFixList(
	LinkedList<AniScopeBase::TypeFixEntry> *list)
{
	int nerrors=0;
	for(;;)
	{
		AniScopeBase::TypeFixEntry *ent=list->popfirst();
		if(!ent)  break;
		
		nerrors+=_DoFixIncompleteType(ent->evt,this,ent->tspec,NULL);
		
		delete ent;
	}
	return(nerrors);
}
