/*
 * calccore/ccif.h
 * 
 * Calc core interface to animation core. 
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

#ifndef _ANIVISION_CALCCORE_CCIF_H_
#define _ANIVISION_CALCCORE_CCIF_H_

#include <hlib/refstring.h>
#include <valtype/valtypes.h>


// -*- MAIN CALCULAION CORE INTERFACE SPECIFICATION -*-

namespace NUM
{
	// See omatrix.h: 
	class ObjectMatrix;
}

class ExecThread;
class AniObjectInstance;

namespace ANI
{
	struct TRegistrationInfo;
	struct TFInfo;
}

namespace CC   // calculation core namespace
{

// See "adtree.h": 
class ADTNAniDescBlock;
class ADTNTypedScope;
class ADTNScopeEntry;


// NOTE: All the types in here are NOT C++-save. Always use 
//       pointers to them. 

// Internal bug abort routine; pass line number as argument.  
extern void _ccif_failassert(int line);


// NOTE: To create a CCIF_* object, you need to ask the 
//       CCIF_Factory. This allows for different core 
//       implementations. 
class CCIF_Factory;

// This is a data class to be passed to the GetValue() calls which 
// contains the required exec thread etc. (needed for user function 
// calls). 
typedef ExecThread CCIF_ExecThread;


// Calculation Core InterFace class for SETTING:
// Meant to be attached to a setting. Just a base for 
// the actual class provided by some deeper implementetion. 
class CCIF_Setting
{
	friend class CCIF_Factory;
	protected:
		// Current setting time. 
		// This is set by ComputeAll() and SetTimeRange(). 
		double current_time;
		// Associated factory: 
		CCIF_Factory *factory;
		
		CCIF_Setting(CCIF_Factory *_factory);  // Use factory to create. 
	private:
		CCIF_Setting &operator=(const CCIF_Setting &)
			{  _ccif_failassert(__LINE__);  return(*this);  }
		CCIF_Setting(const CCIF_Setting &)
			{  _ccif_failassert(__LINE__);  }
	public:  _CPP_OPERATORS
		virtual ~CCIF_Setting();
		
		// Get associated calc core factory: 
		CCIF_Factory *CoreFactory() const
			{  return(factory);  }
		
		// Get current setting time: 
		double CurrentSettingTime() const
			{  return(current_time);  }
		
		// Set the time range for the animation of this setting. 
		// Return value: 
		//  0 -> OK
		virtual int SetTimeRange(const Range &tr);
		
		// Compute all values and objects for the current time. 
		virtual int ComputeAll();
		
		// Advance time. Returns 1 if the animation now ends, 
		// 0 if to continue. 
		virtual int TimeStep();
};


// Calculation Core InterFace class for OBJECT: 
// This class is meant to be attached to an object to represent 
// the object's orientation in space and its time. 
// Just a base for the actual class provided by some deeper 
// implementetion. 
class CCIF_Object
{
	friend class CCIF_Factory;
	public:
		// Value flags: 
		enum
		{
			VF_None=    0x00,  // no flag
			VF_Time=    0x01,  // time
			VF_Location=0x02,  // pos, front, up, speed, accel
			VF_Active=  0x04,  // active flag (integer, not bool)
			VF_TimeLocAct=VF_Time|VF_Location|VF_Active,
		};
		
		// Provided variables for the object, depending on the 
		// variable flags. 
		enum OVal  // "object value"
		{
			ValNone=0,     // void
			// VF_Location
			ValPos,        // 3d vector
			ValFront,      // 3d vector
			ValUp,         // 3d vector
			ValSpeed,      // 3d vector
			ValAccel,      // 3d vector
			ValMatrix,     // position and orientation as matrix -> ObjectMatrix
			// VF_Time: 
			ValTime,       // double
			// VF_Active: 
			ValActive,     // int
		};
		
	protected:
		// Associated object. 
		// NOTE: As soon as this object gets deleted, we're deleted, too. 
		//       (I.e. CCIF_Object gets deleted when associated object gets 
		//       deleted.)
		AniObjectInstance *ani_object_inst;
		
		// Associated setting interface: 
		CCIF_Setting *setting;
		// Present VF_* flags: 
		int vflags;
		
		CCIF_Object(CCIF_Setting *_setting,int _vflags,
			AniObjectInstance *ani_object);  // Use factory to create. 
	private:
		// This gets called when the associated object is deleted. 
		// Reaction is to unregister the delete handler and call 
		// CC_ObjectDeleteNotify(). 
		// [overriding virtual]
		void handlenotify(int ntype,AniObjectInstance *obj_inst);
		
		// Do not use these: 
		CCIF_Object &operator=(const CCIF_Object &)
			{  _ccif_failassert(__LINE__);  return(*this);  }
		CCIF_Object(const CCIF_Object &)
			{  _ccif_failassert(__LINE__);  }
	public:  _CPP_OPERATORS
		virtual ~CCIF_Object();
		
		// Get associated calc core factory: 
		CCIF_Factory *CoreFactory() const
			{  return(setting->CoreFactory());  }
		
		// Get associated ani object instance: 
		AniObjectInstance *GetAniObjectInstance() const
			{  return(ani_object_inst);  }
		
		// Check if the passed variable is provided (checks against flags): 
		// Return value: 
		//  1 -> yes
		//  0 -> no
		int ProvidesVal(OVal ov) const;
		
		// Get current value of passed variable. 
		// Stores value under passed pointer. ObjectMatrix
		// Vector values expect to find an array double[3]
		// while scalar values only need pointer to double. 
		// Return value: 
		//  0 -> OK
		//  1 -> no such value (-> ProvidesVal())
		//  2 -> incompatible type (e.g. integer version 
		//       for ValPos...)
		virtual int GetVal(OVal ov,int *store_here,CCIF_ExecThread *cet);
		virtual int GetVal(OVal ov,double *store_here,CCIF_ExecThread *cet);
		virtual int GetVal(OVal ov,NUM::ObjectMatrix *store_here,
			CCIF_ExecThread *cet);
		
		// This is used "internally" if an ani desc block for the 
		// associated object is evaluated (i.e. "executed"). 
		virtual int EvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et);
};


// Calculation Core InterFace class for VARIABLE:
// Animate a value; available for n-dim vector (n may be 1). 
// Just a base for the actual class provided by some deeper 
// implementetion. 
class CCIF_Value
{
	friend class CCIF_Factory;
	protected:
		// Associated setting interface: 
		CCIF_Setting *setting;
		// Dimension of the var vector. 
		int dimension;
		
		CCIF_Value(CCIF_Setting *_setting,int _dimension);  // Use factory to create. 
	private:
		CCIF_Value &operator=(const CCIF_Value &)
			{  _ccif_failassert(__LINE__);  return(*this);  }
		CCIF_Value(const CCIF_Value &)
			{  _ccif_failassert(__LINE__);  }
	public:  _CPP_OPERATORS
		virtual ~CCIF_Value();
};


// The core factory. This is the place where you choose the core 
// to be used. First, create the factory object of the core you 
// like. Then use the factory to create all the other 
// CCIF_* types. 
// This is just the base class for the different factories. 
// Note: Each calc core has its own factory (derived from CCIF_Factory) 
//       which gets called to create the CCIF_Object and related objects. 
//       There is only one factory per calc core. 
//       One Factory instance for every calc core is created initially 
//       when InitFactories() gets called. 
class CCIF_Factory
{
	private:
		static int n_factories;
		static CCIF_Factory **factories;
	public:
		// Get number of factories: 
		static inline int NFactories()
			 {  return(n_factories);  }
		// See comment for this class: 
		static void InitFactories();
		static void CleanupFactories();
		// Lookup factory with passed name; use NULL for default. 
		// Return pointer to factory or NULL if not found. 
		static CCIF_Factory *GetFactoryByName(const char *name);
		// Create an array of CCIF_Setting instances, one for every 
		// registered calc core. This will call CreateCCIF_Setting()
		// for each factory and store the returned (new) CCIF_Setting 
		// objects in the array. Use DestroyCCIF_SettingArray() to 
		// destroy again. 
		static CCIF_Setting **CreateCCIF_SettingArray();
		static CCIF_Setting **DestroyCCIF_SettingArray(CCIF_Setting **ccif);
		// This is similar for the CCIF_Object pointers. 
		// However, the CCIF_Object instances get created in a lazy way, 
		// i.e. only when needed. But, we need to properly clean them up 
		// and hence also properly set up NULL pointers: 
		static CCIF_Object **SetupCCIF_ObjectArray();
		static CCIF_Object **DestroyCCIF_ObjectArray(CCIF_Object **ccif);
		
	protected:
		// Must create a derived class. 
		CCIF_Factory(const char *name,int _factory_index);
	private:
		const char *name;   // non-alloc
		// Index of this factory/core (0...CCIF_Factory::n_factories-1) for 
		// index operations on arrays storing per-core information 
		// (such as Animation_Context::ccif): 
		int core_index;   
		
		CCIF_Factory &operator=(const CCIF_Factory &)
			{  _ccif_failassert(__LINE__);  return(*this);  }
		CCIF_Factory(const CCIF_Factory &)
			{  _ccif_failassert(__LINE__);  }
	public:  _CPP_OPERATORS
		virtual ~CCIF_Factory();
		
		// Get the name and index of the core: 
		const char *Name() const
			{  return(name);  }
		int CoreIndex() const
			{  return(core_index);  }
		
		// Create a setting interface: (Use delete to destroy again.)
		virtual CCIF_Setting *CreateCCIF_Setting();
		
		// Create an object interface: For the vflags, see 
		// the enum VF_* above. 
		// You need to pass the associated setting interface as 
		// first arg. 
		// Note that VF_Location and VF_Active require VF_Time. 
		// (Use delete to destroy again.)
		virtual CCIF_Object *CreateCCIF_Object(CCIF_Setting *setting,
			int vflags,AniObjectInstance *obj_inst);
		
		// Create a value interface: (Use delete to destroy again.)
		virtual CCIF_Value *CreateCCIF_Value(CCIF_Setting *setting,
			int dimension);
		
		// This gets called to perform the registration step of 
		// the passed ADTN (ani desc tree node). This should 
		// normally pass the DoRegisrtation() calls on but can also 
		// do some additional work such as looking up scope names. 
		// Default implementation will simply pass it on. 
		// NOTE: Will only get called for tri->who=TRL_AllIdentifiers 
		//       and TRL_AutoVars. 
		virtual void DoADTNRegsistration(ADTNAniDescBlock *tn,
			ANI::TRegistrationInfo *tri);
		virtual void DoADTNRegsistration(ADTNTypedScope *tn,
			ANI::TRegistrationInfo *tri);
		virtual void DoADTNRegsistration(ADTNScopeEntry *tn,
			ANI::TRegistrationInfo *tri);
		
		// This is for the ExprTF step to give the calc core control over 
		// what is happening. Must act like usual. Has default implementation. 
		virtual int DoADTNExprTF(ANI::TFInfo *ti,ADTNAniDescBlock *tn);
		virtual int DoADTNExprTF(ANI::TFInfo *ti,ADTNTypedScope *tn);
		virtual int DoADTNExprTF(ANI::TFInfo *ti,ADTNScopeEntry *tn);
};

}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_CCIF_H_ */
