/*
 * core/i_object.cc
 * 
 * Animation object instanc. 
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

#include "aniinstance.h"
#include "animation.h"
#include <calccore/ccif.h>

using namespace ANI;


ExprValue &AniObjectInstance::GetInternalVarRef(/*BuiltinVarIndex*/int bvi)
{
	assert(bvi>=0 && bvi<_BVI_LAST);
	return(ivar_ev[bvi]);
}


int AniObjectInstance::ComputeInternalVar(/*BuiltinVarIndex*/int bvi,
	ExprValue &store_val,ExecThread *et)
{
	char vtype='\0';  // 'd'ouble, '3'd vector, 'i'nteger
	CC::CCIF_Object::OVal oval;
	switch(bvi)
	{
		case BVI_Time:   vtype='d'; oval=CC::CCIF_Object::ValTime;   break;
		case BVI_Pos:    vtype='3'; oval=CC::CCIF_Object::ValPos;    break;
		case BVI_Front:  vtype='3'; oval=CC::CCIF_Object::ValFront;  break;
		case BVI_Up:     vtype='3'; oval=CC::CCIF_Object::ValUp;     break;
		case BVI_Speed:  vtype='3'; oval=CC::CCIF_Object::ValSpeed;  break;
		case BVI_Accel:  vtype='3'; oval=CC::CCIF_Object::ValAccel;  break;
		case BVI_Active: vtype='i'; oval=CC::CCIF_Object::ValActive; break;
		default: assert(0);
	}
	
	int rv=-2;
	do {
		CC::CCIF_Object *accif=GetActiveCCIF();
		if(!accif)  break;  // rv stays -2
		switch(vtype)
		{
			case 'i':
			{
				int tmp;
				rv=accif->GetVal(oval,&tmp,/*CCIF_ExecThread=*/et);
				if(rv) break;
				store_val.assign(tmp);
			}	break;
			case 'd':
			{
				double tmp;
				rv=accif->GetVal(oval,&tmp,/*CCIF_ExecThread=*/et);
				if(rv) break;
				store_val.assign(tmp);
			}	break;
			case '3':
			{
				// This is a little hack...
				Vector tmp(Vector::NoInit,3);
				rv=accif->GetVal(oval,&tmp[0],/*CCIF_ExecThread=*/et);  // tmp[3] returns ref. 
				if(rv) break;
				store_val.assign(tmp);
			}	break;
			default: assert(0);
		}
	} while(0);
	
	// Store null values in case of error. 
	if(rv) switch(vtype)
	{
		case 'i':  store_val.assign((int)0);  break;
		case 'd':  store_val.assign((double)0.0);  break;
		case '3':  store_val.assign(Vector(Vector::Null,3));  break;
		default: assert(0);
	}
	
	return(rv);
}


void AniObjectInstance::vn_change(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	// So... some internal variable changed. 
	// Tell the CCIF. 
	if(active_core_index>=0)
	{
		assert(!"!implemented");
		//ccif[active_core_index]->InternalVarChanged(bvi);
	}
}

void AniObjectInstance::vn_delete(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	// This may not happen. 
	assert(0);
}


CC::CCIF_Object *AniObjectInstance::GetOrCreateCCIF(
	CC::CCIF_Factory *cc_factory,ExecThread *et)
{
	int idx=cc_factory->CoreIndex();
	
	if(!ccif[idx])
	{
		// Must allocate one: 
		ccif[idx]=cc_factory->CreateCCIF_Object(et->Context()->ccif[idx],
			/*vflags=*/CC::CCIF_Object::VF_TimeLocAct,this);
		
		if(active_core_index<0)
		{
			fprintf(stderr,"HACK! setting active_core_index to AniTMT core.\n");
			active_core_index=1;
		}
	}
	
	return(ccif[idx]);
}


AniAnimation *AniObjectInstance::_GetAnimation()
{
	#warning "Could be replaced by O(1) implementation."
	for(AniScopeBase *i=asb; i; i=i->Parent())
	{
		if(i->ASType()==AniScopeBase::AST_Animation)
		{  return((AniAnimation*)i);  }
	}
	assert(0);  // Must have animation in hierarchy. 
	return(NULL);
}


TNCompoundStmt *AniObjectInstance::_GetPOVCompound()
{
	return( ((TNFunctionDef*)(
			((AniObject*)asb)->pov_attach_detach->GetTreeNode()
		))->func_body );
}


void AniObjectInstance::RegisterDeleteNotifier(
	NotifyHandler<AniObjectInstance> *dn)
{
	int rv=delete_notifier_list.Append(dn);
	assert(rv!=-2);
}

void AniObjectInstance::UnregisterDeleteNotifier(
	NotifyHandler<AniObjectInstance> *dn)
{
	int rv=delete_notifier_list.Remove(dn);
	assert(rv!=-2);
}


AniObjectInstance::AniObjectInstance(AniScopeBase *_asb) : 
	AniScopeInstanceBase(_asb),
	ValueNotifyHandler(),
	delete_notifier_list()
{
	// Currently, no calc core is active: 
	active_core_index=-1;
	
	// Initialize the built-in variables: 
	int _count=0;
	Vector v3(0.0,0.0,0.0);
	ivar_ev[BVI_Time].assign(0.0/*double*/);  ++_count;
	ivar_ev[BVI_Pos].assign(v3);    ++_count;
	ivar_ev[BVI_Front].assign(v3);  ++_count;
	ivar_ev[BVI_Up].assign(v3);     ++_count;
	ivar_ev[BVI_Speed].assign(v3);  ++_count;
	ivar_ev[BVI_Accel].assign(v3);  ++_count;
	ivar_ev[BVI_Active].assign(0/*integer*/);  ++_count;
	assert(_count==_BVI_LAST);
	
	// Register change notifiers for all of them: 
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNInstall(ivar_ev[i],&ivar_ev[i]);
		assert(!rv);
	}
	
	// In previous versions, it was not possible to create an object 
	// outside animation context. Now, this is no longer the case because 
	// the ccif gets created when needed. 
	ccif=CC::CCIF_Factory::SetupCCIF_ObjectArray();  // <-- sets up ccif[]={NULL,NULL,...}
}

AniObjectInstance::~AniObjectInstance()
{
	// Call the delete notifiers: 
	// These notifiers MAY but do not have to delete itself: 
	delete_notifier_list.CallNotifiers(0,this);
	
	ccif=CC::CCIF_Factory::DestroyCCIF_ObjectArray(ccif);
	
	// Unregister the change notifiers: 
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNUninstall(ivar_ev[i]);
		assert(!rv);
	}
	
	active_core_index=-1;  // be sure...
}

