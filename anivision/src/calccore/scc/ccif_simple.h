/*
 * calccore/scc/ccif_simple.h
 * 
 * Simple calc core (internal) setting/object/value description. 
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

#ifndef _ANIVISION_CALCCORE_SCC_CCIF_H_
#define _ANIVISION_CALCCORE_SCC_CCIF_H_ 1

#include <hlib/linkedlist.h>
#include <calccore/ccif.h>

#include <core/execthread.h>

#include <calccore/adtree.h>
//#include <numerics/la_basics.h>
#include <numerics/spline/sl_base.h>

#include <objdesc/move.h>

#include "scc_token.h"


namespace CC
{

class CCIF_Factory_Simple;

namespace S
{

// As defined in this file: 
class CCSetting;
class CCObject;
class CCValue;


// Note: Base class has manual reference counter. 
// This is for function { xxx = f($t); }. 
struct CCIF_SpecialIdf_CoreHook_T : ANI::SpecialIdf_CoreHook
{
	// This is the value for $t which which is set before we 
	// call the evaulation function. It has to be saved when 
	// a context switch happens. Note that this core hook is 
	// attached directly to the AniInternalVariable_ADB and 
	// hence exists only once. It is necessary to pay attention 
	// to re-entrant calls. An ADB is not re-entrant but we may 
	// have something like function { pos = this.$Pos; }
	// We just need to make sure to load the correct value 
	// before entering the evaluation function. 
	ANI::ExprValue t_val;
	// This is the recursion counter to prevent infinite recursion: 
	int recursion_depth;
	
	CCIF_SpecialIdf_CoreHook_T() : ANI::SpecialIdf_CoreHook(),t_val()
		{  recursion_depth=0;  }
	~CCIF_SpecialIdf_CoreHook_T() {}
	
	// [Overriding a virtual:] 
	ANI::ExprValue Eval(ANI::TNIdentifier *idf,
		ANI::ExecThreadInfo *info);
};


// As attached to ADTNScopeEntry: 
struct ADTNScopeEntry_CoreHook : ADTNScopeEntry::CoreHook
{
	// Looked up scope_name token or -1 (unknown) or 0 if none specified: 
	int entry_name_tok;
	// Type ID (from the grammar SCC_ScopeTreeParser; initially -1): 
	int scc_expr_type_id;
	
	// In case we're attached to an entry which belongs to a function 
	// with "special" variables (such as $t), then this is non-NULL 
	// and stores the (ref counting) special identifier core hook which 
	// is also attached to AniInternalVariable_ADB. 
	CCIF_SpecialIdf_CoreHook_T *func_val_t;
	
	ADTNScopeEntry_CoreHook() : ADTNScopeEntry::CoreHook()
		{  entry_name_tok=-1;  scc_expr_type_id=-1;  func_val_t=NULL;  }
	~ADTNScopeEntry_CoreHook();  // <-- will dereference func_val_t. 
};


// As passed to the eval functions: evaluation context. 
class EvalContext_SCC : public NUM::EvalContext
{
	public:
		ExecThread *et;
		CCObject *ccobj;  // Use NULL if there is no this pointer 
		                  // (i.e. no object ADB). 
	public:
		EvalContext_SCC(ExecThread *_et,CCObject *_ccobj)
			{  et=_et;  ccobj=_ccobj;  }
		~EvalContext_SCC()  {  et=NULL;  }
};


struct CCBaseFunctionality
{
	public:
		// Internally used by NextScopeEntry(): 
		struct InterpreteScopeEntries_IData
		{
			ANI::TreeNode *curr_tn;  // where we are in the loop
			
			_CPP_OPERATORS
			InterpreteScopeEntries_IData(ANI::TreeNode *first) : 
				curr_tn(first) {}
			~InterpreteScopeEntries_IData() {}
		};
		
		struct InterpreteScopeEntriesInfo
		{
			AniDescBlockTokID tok_id;   // found entry (token)
			ANI::TreeNode *value;       // value part
			void *dptr;                 // as passed to InterpreteScopeEntries()
			// In case value is an ADTNTypedScope, the following is also set: 
			// (Otherwise tsc is NULL) 
			ADTNTypedScope *tsc;   // =value, casted
			AniDescBlockTokID tsc_tok_id;  // ID of tsc->scope_type->name
		};
		
		enum InterpreteScopeSlot_State
		{
			// DO NOT CHANGE the sign of these constants: 
			ISS_OOPSError=-1,   // uncaught error; may not happen...
			ISS_Cont=0,
			ISS_CSwitch=1,
			// Error codes with special error message being written: 
			ISS_AlreadySet=-2,  // report "property was already set" error
			ISS_TimeSpecConflict=-3,  // report "conflicting time specs" error
			ISS_IllegalValue=-4,  // reort "illegal value for ..." error
		};
	protected:
		// thread_id of the thread currently executing an ADB for 
		// this object. -1 for "none". 
		int lock_thread;
		// Pointer to the ADB currently being executed. 
		ADTNAniDescBlock *lock_adb;
		// Error counter for current ADB execution (see lock above). 
		int n_errors;
		
		// We use the EvalStateStack to temporarily store curve 
		// objects across context switches. Cloning makes no 
		// trouble as it is forbidden inside ADBs anyways and 
		// the pushPtr()/popPtr() does not allow cloning at all. 
		// These are helper functions: 
		inline void _PushCPtr(ExecThread *et,void *ptr)
			{  et->ess->pushPtr(ptr);  }
		template<class T>inline T *_PopCPtr(ExecThread *et)
			{  T *ptr=(T*)et->ess->popPtr(NULL); return(ptr ? ptr : new T()); }
		
		// Lock/unlock ADB. Lock function returns 1 if an error 
		// happened and the lock could not be obtained. 
		// AquireADBLock() will also clear n_errors counter. 
		int AquireADBLock(ADTNAniDescBlock *adb,ExecThread *et);
		void LoseADBLock();  // <-- Does NOT clear n_errors. 
		
		// Helper for scope entry parsing. Get next entry. 
		// scope_name: The name of the scope we're currently in ("curve",...)
		//             Only needed for error messages. 
		// For each entry, the virtual function InterpreteScopeSlot() 
		// is called with slot_id as first argument and dptr in the 
		// info parameter (see there). 
		// Return value: 
		//    0 -> OK, done
		//    1 -> context switch
		int InterpreteScopeEntries(
			ADTNTypedScope *tsc,ExecThread *et,
			const char *scope_name,void *dptr,int slot_id);
		// Used by InterpreteScopeEntries(): 
		// Return value is actually an enum InterpreteScopeSlot_State. 
		//    0 -> OK
		//    1 -> context switch
		//  ...more...
		virtual int InterpreteScopeSlot(int slot_id,ExecThread *et,
			InterpreteScopeEntriesInfo *info);
	public:  _CPP_OPERATORS
		CCBaseFunctionality();
		virtual ~CCBaseFunctionality();
		
		// Eval scalar/vector array/string/... expression. 
		// For vector array, vec_dim is the desired vector space dimension 
		// or -1 for "any". 
		// Return value: 
		//   0 -> OK, value read and stored in ret_val
		//   1 -> context switch
		//  -1 -> error: message written, n_errors incremented and ret_val 
		//        not modified
		int _GetScalarValue(double &ret_val,ANI::TreeNode *tn,ExecThread *et);
		int _GetStringValue(String &ret_val,ANI::TreeNode *tn,ExecThread *et);
		int _GetRangeValue(Range &ret_val,ANI::TreeNode *tn,ExecThread *et);
		int _GetVector3Value(double *vector,ANI::TreeNode *tn,ExecThread *et);
		int _GetVectorArray(NUM::VectorArray<double> &array,
			ANI::TreeNode *tn,ExecThread *et,int vec_dim=-1);
		
		// Write unknown property error and increment error 
		// counter n_errors. name may be NULL. 
		void _UnknownProperty(ANI::TNIdentifier *name,
			ANI::TreeNode *value,const char *scope_name);
};


class CCObject : 
	public CCIF_Object,
	public LinkedListBase<CCObject>,
	public CCBaseFunctionality
{
	private:
		// List of movements: 
		LinkedList<NUM::OMovement> movements;
		
		// Movement cache: the movement looked up previously: 
		// Internally used by _FindMovementByTime(). 
		NUM::OMovement *_prev_looked_up_movement;
		
		// Look up a movement node by passed setting time. 
		// Returns NULL if none matches. 
		NUM::OMovement *_FindMovementByTime(double setting_time);
		
		template<typename T,typename B> inline int _InterpreteScope_Template(
			const char *scope_name,int slot_id,
			ADTNTypedScope *tsc,ExecThread *et,B **_dptr);
		
		int _InterpreteCurvePosFixed(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		int _InterpreteCurvePosFixed_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		int _InterpreteCurvePosLspline(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		int _InterpreteCurvePosCspline(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		int _InterpreteCurvePosLCspline_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info,int spline_type);
		int _InterpreteCurvePosFunction(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		int _InterpreteCurvePosFunction_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		int _InterpreteCurvePosCurve(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		int _InterpreteCurvePosJoin(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveFunctionBase **_curvefunc);
		
		int _InterpreteCurveTMapNone(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveTMapFunction **_tmapfunc);
		int _InterpreteCurveTMapNone_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		int _InterpreteCurveTMapConst(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveTMapFunction **_tmapfunc);
		int _InterpreteCurveTMapConst_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		int _InterpreteCurveTMapCspline(ADTNTypedScope *tsc,ExecThread *et,NUM::CurveTMapFunction **_tmapfunc);
		
		int _InterpreteCurvePos(InterpreteScopeEntriesInfo *info,ExecThread *et,NUM::CurveFunctionBase **_posfunc);
		int _InterpreteCurveTMap(InterpreteScopeEntriesInfo *info,ExecThread *et,NUM::CurveTMapFunction **_tmapfunc,bool is_speed);
		
		int _InterpreteCurve(ADTNTypedScope *tsc,ExecThread *et,NUM::OCurve **_ocurve);
		int _InterpreteCurve_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		
		int _InterpreteFrontSpeed(ADTNTypedScope *tsc,ExecThread *et,NUM::OFrontSpec **_ofront);
		int _InterpreteFrontSpeed_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		
		int _InterpreteUpGravity(ADTNTypedScope *tsc,ExecThread *et,NUM::OUpSpec **_oup);
		int _InterpreteUpGravity_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		
		int _InterpreteMovePos(InterpreteScopeEntriesInfo *info,ExecThread *et,NUM::OCurve **_ocurve);
		int _InterpreteMoveFront(InterpreteScopeEntriesInfo *info,ExecThread *et,NUM::OFrontSpec **_ocurve);
		int _InterpreteMoveUp(InterpreteScopeEntriesInfo *info,ExecThread *et,NUM::OUpSpec **_oup);
		
		int _InterpreteMoveScope(ADTNTypedScope *tsc,ExecThread *et,NUM::OMovement **_movement);
		int _InterpreteMoveScope_Slot(ExecThread *et,InterpreteScopeEntriesInfo *info);
		
		// [overriding virtual]
		virtual int InterpreteScopeSlot(int slot_id,ExecThread *et,
			InterpreteScopeEntriesInfo *info);
		
		// Helper for EvalAniDescBlock(); internal use ONLY: 
		int _DoEvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et);
	public:   _CPP_OPERATORS
		CCObject(CCSetting *_setting,int _vflags,AniObjectInstance *_inst);
		~CCObject();
		
		// Get associated setting: 
		inline CCSetting *Setting()
			{  return((CCSetting*)setting);  }
		
		// [overriding virtuals from CCIF_Object; see there.]
		int GetVal(OVal ov,int *store_here,CCIF_ExecThread *cet);
		int GetVal(OVal ov,double *store_here,CCIF_ExecThread *cet);
		int GetVal(OVal ov,NUM::ObjectMatrix *store_here,CCIF_ExecThread *cet);
		
		// [overriding virtuals from CCIF_Object; see there.]
		int EvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et);
};


class CCValue : 
	public CCIF_Value,
	public LinkedListBase<CCValue>,
	public CCBaseFunctionality
{
	private:
		
	public:  _CPP_OPERATORS
		CCValue(CCSetting *_setting,int _dimension);
		~CCValue();
		
		// Get associated setting: 
		inline CCSetting *Setting()
			{  return((CCSetting*)setting);  }
		
		// [overriding virtuals from CCIF_Value; see there.]
};


class CCSetting : 
	public CCIF_Setting
{
	private:
		// Child objects: 
		LinkedList<CCObject> objlist;
		// Child vals: 
		LinkedList<CCValue> vallist;
		
		// Current time range: 
		Range timerange;
		
		// Counts time steps (->TimeStep()) so far: 
		int n_time_steps;
		
	public:  _CPP_OPERATORS
		CCSetting(CCIF_Factory_Simple *_factory);
		~CCSetting();
		
		// [overriding virtuals from CCIF_Setting; see there.]
		int SetTimeRange(const Range &tr);
		int ComputeAll();
		int TimeStep();
		
		// For setting/value (un)registration (done by their 
		// con/destructor): 
		void RegisterValue(CCValue *val);
		void UnregisterValue(CCValue *val);
		void RegisterObject(CCObject *obj);
		void UnregisterObject(CCObject *obj);
};

}  // end of namespace S
}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_SCC_CCIF_H_ */
