/*
 * core/aniinstance.h
 * 
 * Scope (and object in particular) instance. 
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

#ifndef _ANIVISION_CORE_ANIINSTANCE_H_
#define _ANIVISION_CORE_ANIINSTANCE_H_ 1

#include <core/aniscope.h>
#include <core/a_variable.h>
//#include <core/animation.h>

// For attached POV objects: 
#include <pov-core/povparser.h>

#include <hlib/notifierlist.h>


// Include the calculation core interface: 
//#include <calccore/ccif.h>
namespace CC
{
	class CCIF_Setting;
	class CCIF_Object;
	class CCIF_Value;
	class CCIF_Factory;
}

// Non-instances (i.e. (proto)types): 
class AniAniSetObj;
class AniAnimation;
class AniSetting;
class AniObject;

class ExecThread;
class Animation_Context;

// This is the base class for ani scope instances, i.e. 
// animation, setting, object, function, anonymous scope. 
// All the associated (member/...) variables are stored in 
// here and accessed O(1) via array subscript. 
// See below for derived types. 
// NOTE: 
//  - This is an AniObjectInstance if the attached AniScopeBase *asb 
//    refers to an object. 
//  - This is a plain AniScopeInstanceBase otherwise. 
// NOTE: (08/2004) ?!?! Animation, setting, function and anon scope do 
//       not have instances; they all exist statically, don't they!?
class AniScopeInstanceBase : public ANI::AniGlue_ScopeInstanceBase
{
	public:
		static AniScopeInstanceBase *CreateAniInstance(AniScopeBase *asb);
		
	protected:
		// Pointer to associated AniScopeBase: 
		AniScopeBase *asb;
		
		// Non-static, non-automatic regular member variables 
		// (i.e. all which are registered in the ani scope tree). 
		// Array of size n_memb_vars. 
		int n_memb_vars;
		ANI::ExprValue *memb_vars;
		
		// NEVER CALL DIRECTLY. USE CreateAniInstance(). 
		// Constructor. Will set asb an n_memb_vars and allocate 
		// memb_vars[]. Setting up memb vars is task of derived 
		// class. 
		AniScopeInstanceBase(AniScopeBase *asb);
	public:  _CPP_OPERATORS
		// Destructor: free memb_vars[]. 
		~AniScopeInstanceBase();
		
		// Get type... (of attached ASB): 
		AniScopeBase::AniScopeType ASType() const
			{  return(asb->ASType());  }
		// ...and ASB: 
		AniScopeBase *GetASB() const
			{  return(asb);  }
		
		// Get member variable associated with passed 
		// addressation index. O(1) and lvalue-capable: 
		// NOTE: Returns a true C++ reference. 
		ANI::ExprValue &GetMemberVar(int idx) const;
		
		// See derived class for more information. 
		virtual ANI::ExprValue &GetInternalVarRef(int bvi);
		virtual int ComputeInternalVar(int bvi,ANI::ExprValue &store_val,
			ExecThread *et);
		
		// [overriding virtuals from AniGlue_ScopeInstanceBase:] 
		String ToString() const;
		void GetExprValueType(ANI::ExprValueType *store_here) const;
};


// NOTE: NOT C++ save. Use ANI::ObjectInstance for a ref counting container. 
class AniObjectInstance : 
	public AniScopeInstanceBase,
	public ANI::ValueNotifyHandler
{
	public:
		// This is the built-in variable index in the ivar_ev array below. 
		enum BuiltinVarIndex
		{
			BVI_Time=0,  // <-- First MUST be 0. 
			BVI_Pos,
			BVI_Front,
			BVI_Up,
			BVI_Speed,
			BVI_Accel,
			BVI_Active,
			
			_BVI_LAST
		};
		
		// There may be classes which need to know when a certain 
		// object instance is deleted (notably the POV attachment 
		// code). For EACH AniObjectInstance you monitor, create 
		// ONE NotifyHandler<AniObjectInstance>-derived class and 
		// register it using RegisterDeleteNotifier(). 
		// When the object is deleted, the handlenotify() function 
		// (NOT deletenotify()) will be called. 
		// NOTE: You need ONE NotifyHandler<AniObjectInstance> for EACH 
		//       AniObjectInstance but an AniObjectInstance can have any 
		//       number of NotifyHandler<AniObjectInstance> registered. 
		
	private:
		// The list of attached POV objects is (now) in the 
		// animation context. 
		
		// This is a list of attached delete notifiers. 
		// See above for more info. 
		NotifierList<AniObjectInstance> delete_notifier_list;
		
		// Calculation Core InterFaces for Objects: 
		// This is an array with one entry per calc core. 
		// Note that initially all entries are NULL and they are created 
		// when needed. 
		CC::CCIF_Object **ccif;
		// This is the index (in the above array and matching 
		// CCIF_Factory::CoreIndex()) of the calc core which is currently 
		// managing the object. 
		int active_core_index;
		
		// These are the internal variables of each object. 
		// These variables represent the current state and are also lvalues. 
		// Change notifiers are used to notify the CCIF about changes in 
		// the internal variables. 
		ANI::ExprValue ivar_ev[_BVI_LAST];
		
		// Get AniAnimation pointer: 
		AniAnimation *_GetAnimation();
		
		// [overriding virtuals from ValueNotifyHandler:]
		void vn_change(void *hook);
		void vn_delete(void *hook);
	public:  _CPP_OPERATORS
		AniObjectInstance(AniScopeBase *asb);
		~AniObjectInstance();
		
		// Internally used by AniAnimation for POV object attach/detach. 
		ANI::TNCompoundStmt *_GetPOVCompound();
		
		// Get calc core interface pointer for the currently active 
		// context (or NULL): 
		CC::CCIF_Object *GetActiveCCIF() const
			{  return(active_core_index<0 ? NULL : ccif[active_core_index]);  }
		
		// Get object CCIF associated with passed calc core (factory) 
		// (i.e. the proper element of ccif[] array) or create a new 
		// one (if the entry in the ccif[] array is still NULL). 
		CC::CCIF_Object *GetOrCreateCCIF(CC::CCIF_Factory *cc_factory,
			ExecThread *et);
		
		// Get internal variable (this will return the current state 
		// and will not trigger a computation by the calc core): 
		// Returns a true reference of the internal variable with 
		// specified index. Use copy constructor once if you do not 
		// need a true lvalue. 
		// [overriding virtual from AniScopeInstanceBase]
		ANI::ExprValue &GetInternalVarRef(/*BuiltinVarIndex*/int bvi);
		
		// This is like GetInternalVar() but will trigger a computation by 
		// the calc core. Hence, no real reference is returned (no lvalue). 
		// Return value: 
		//    0 -> OK
		//  1,2 -> see CCIF_Object::GetVal()
		//   -2 -> active ccif=NULL: no (active) interface
		// In case of error, object instances store 0 in the passed var 
		// other instances abort immediately. 
		// [overriding virtual from AniScopeInstanceBase]
		int ComputeInternalVar(/*BuiltinVarIndex*/int bvi,
			ANI::ExprValue &store_val,ExecThread *et);
		
		// See above...
		void RegisterDeleteNotifier(NotifyHandler<AniObjectInstance> *dn);
		void UnregisterDeleteNotifier(NotifyHandler<AniObjectInstance> *dn);
};

#endif  /* _ANIVISION_CORE_ANIINSTANCE_H_ */
