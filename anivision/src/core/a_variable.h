/*
 * core/a_variable.h
 * 
 * ANI tree scope for (user and internal) variables. 
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

#ifndef _ANIVISION_CORE_ANIVARIABLE_H_
#define _ANIVISION_CORE_ANIVARIABLE_H_ 1

#include <hlib/cplusplus.h>

#include <core/aniscope.h>
#include <ani-parser/treereg.h>


class AniVariable : 
	public AniScopeBase,  // <-- MUST BE FIRST. 
	public LinkedListBase<AniVariable>  // list: parent
{
	public:
		// For the attributes: 
		// Note: AniUserVariable can have any attribute listed here. 
		//       AniInternalVariable can only have IS_Static. 
		enum
		{
			IS_Const=    0x0001,
			IS_Static=   0x0002,
			IS_Automatic=0x0004,  // [automatic] -- as opposed to [member]
			HAS_Initializer=0x0010,  // has "=value" initializer?
		};
		
		// For AniVariable, parent MUST be non-null and determines which 
		// sort of varaible it is: 
		//  AST_Animation -> "global" var          [member]
		//  AST_Setting   -> "setting-global" var  [member]
		//  AST_Object    -> object member var     [member]
		//  AST_UserFunc  -> function _argument_ var - NO OTHER. [automatic]
		//  AST_AnonScope -> well... var inside the scope        [automatic]
		//                   (e.g. inside compound stmt)
		// [automatic] vars are the ones which get allocated automatically 
		//            for each thread (like "on the stack"). 
		// [member] vars are allocated for each parent object/setting/animation 
		//          instance. 
		
	private:
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
	protected:
		// Internal error for member select from NULL scope. 
		static void _NullScopeExplicitMemberSelect(ANI::TNIdentifier *idf);
		
		// Constructor is protected -- use one of the derived classes, 
		// never construct plain AniVariable. 
		AniVariable(AniScopeType atype,const RefString &name,
			ANI::TreeNode *_treenode,AniScopeBase *_parent);
	public:
		virtual ~AniVariable();
		
		// Get var type: 
		virtual ANI::ExprValueType GetType() const;
		
		// Get attributes (see enum IS_*, HAS_* above): 
		virtual int GetAttrib() const;
		
		// Get the apropriate eval adr func; idf_for_debug is used 
		// for debugging messages; ainc is the placeholder to count 
		// the number of stack frames to go up for automatic vars; 
		// you may set this to NULL if we're in the current stack 
		// frame (i.e. scopes_up=0). 
		virtual EvalAddressationFunc *GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc);
		
		// [overriding virtuals]
		void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		int CountScopes(int &n_incomplete) const;
};


// Normal user-declared variable. 
class AniUserVariable : public AniVariable
{
	public:
		// Eval addressation "lookup" functions: 
		struct EAF_Static : EvalAddressationFunc
		{
			EAF_Static() : EvalAddressationFunc()  { }
			// [overriding virtuals:]
			~EAF_Static();
			// Static var, either automatic or member without member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Static member var var with member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		struct EAF_Auto : EvalAddressationFunc
		{
			int scopes_up;
			int addr_idx;
			EAF_Auto(int _scopes_up,int _addr_idx) : EvalAddressationFunc()
				{  scopes_up=_scopes_up;  addr_idx=_addr_idx;  }
			// [overriding virtuals:]
			~EAF_Auto();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		struct EAF_Member : EvalAddressationFunc
		{
			int addr_idx;
			EAF_Member(int _addr_idx) : EvalAddressationFunc()
				{  addr_idx=_addr_idx;  }
			// [overriding virtuals:]
			~EAF_Member();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// Return "this" pointer: 
		struct EAF_This : EvalAddressationFunc
		{
			EAF_This() : EvalAddressationFunc()  { }
			// [overriding virtuals:]
			~EAF_This();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// Repeater: For member select, return the base again; 
		// for non-member select return current "this" pointer.  
		// Only used for first child of function call (function 
		// call base pointer). 
		struct EAF_Repeater : EvalAddressationFunc
		{
			EAF_Repeater() : EvalAddressationFunc()  { }
			// [overriding virtuals:]
			~EAF_Repeater();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// Simply return NULL ref: ExprValue::None. 
		struct EAF_Null : EvalAddressationFunc
		{
			EAF_Null() : EvalAddressationFunc()  { }
			// [overriding virtuals:]
			~EAF_Null();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// Operator new: only alloc & prototype vals, no constructor. 
		struct EAF_OpNew : EvalAddressationFunc
		{
			AniScopeBase *asb_type;
			
			EAF_OpNew(AniScopeBase *_asb_type) : EvalAddressationFunc()
				{  asb_type=_asb_type;  }
			// [overriding virtuals:]
			~EAF_OpNew();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// For (literal) "NULL"; we MUST supply SOMETHING, so use 
		// this one. 
		// It may be called but the result may not be used. 
		struct EAF_NULL : EvalAddressationFunc
		{
			EAF_NULL() : EvalAddressationFunc()  { }
			// [overriding virtuals:]
			~EAF_NULL();
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
	
	private:
		// Attributes: 
		int attrib;  // OR of the IS_*, HAS_* identifiers. 
		
		// Type of the variable: 
		ANI::ExprValueType vtype;
		
		// Sequence number (= "statement counter number" but actually not 
		// counting statements but variabled) when the var is declared. 
		// To avoid that vars are getting used before they were declared. 
		// Vars with higher number are declared later in the 
		// function/setting, etc. 
		// Counter gets reset when entering new fucntion, NOT when entering 
		// scopes. Seq also used to prevent illgeal initialisation in 
		// settings/animation (re-set there, too). 
		// MORE INFO: Variable lookup code. 
		int var_seq_num;
		
		union
		{
			// If this is a static variable, this stores the static value: 
			ANI::ExprValue *static_val;
			// This is the addressation index of this variable if non-static. 
			// Set when InstanceInfo of parent is created. 
			int addressation_idx;
		}/*__attribute__((__packed__))*/;  // <-- gcc-3.4 no longer accepts this here. 
		
		// Used internally; called as failed assertion: 
		void _sai_gsv_assertfail(int idx) const;
		void _accessing_unset_static(ANI::TNIdentifier *idf_for_error) const;
		
		// [overriding virtuals from AniGlue_ScopeBase]
		int CG_GetVariableType(VariableTypeInfo *vti) const;
		EvalAddressationFunc *CG_GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug);
		// NOTE: Return type is (ANI::TNDeclarator::EvalFunc*)
		void *CG_GetDeclaratorEvalFunc(ANI::TNDeclarator *decl,
			ANI::TNIdentifier *name);
		const char *CG_AniScopeTypeStr() const;
	public:
		AniUserVariable(const RefString &name,ANI::TreeNode *_treenode,
			AniScopeBase *_parent,const ANI::ExprValueType &_type,int _attrib,
			int _var_seq_num);
		~AniUserVariable();
		
		// Get attribute flags: (also overrding a virtual)
		int GetAttrib() const
			{  return(attrib);  }
		
		// [overriding virtual from AniVariable]
		ANI::ExprValueType GetType() const;
		
		// Get var seq num: 
		int GetVarSeqNum() const
			{  return(var_seq_num);  }
		
		// Set the addressation index (Use only if non-static): 
		void SetAddressationIndex(int aidx)
			{  if(!(attrib & IS_Static)) addressation_idx=aidx;
			   else _sai_gsv_assertfail(aidx);  }
		// Get addressation index value (non-static only): 
		int GetAddressationIndex() const
			{  if((attrib & IS_Static)) _sai_gsv_assertfail(-1);
			   return(addressation_idx); }
		// Get static value (only call if var is static and the 
		// static val was set up). 
		// IMPORTANT: Returns true C++ reference. 
		// The passed TNIdentifier is used for the error message in 
		// case a not-yet-set-up static var is accessed. 
		ANI::ExprValue &GetStaticVal(ANI::TNIdentifier *idf_for_error)
		{
			if(!(attrib & IS_Static))  _sai_gsv_assertfail(-2);
			if(!static_val) _accessing_unset_static(idf_for_error);
			return(*static_val);
		}
		
		// [overriding virtual from AniVariable]
		EvalAddressationFunc *GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc);
		
		// [overriding virtuals]
		void AniTreeDump(StringTreeDump *d);
		ANI::TNLocation NameLocation() const;
		AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		int FixIncompleteTypes();
		int PerformCleanup(int cstep);
		int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};


// Internal variables like "$time", "$pos",....
// This is the base class for all internal variable (prototype) classes. 
class AniInternalVariable : public AniVariable
{
	private:
		// [overriding virtual from AniGlue_ScopeBase]
		const char *CG_AniScopeTypeStr() const;
	public:
		AniInternalVariable(const RefString &name,
			ANI::TreeNode *_treenode,AniScopeBase *_parent);
		virtual ~AniInternalVariable();
		
		// Register all internal (built-in) variables for the 
		// passed scope: 
		static void RegisterInternalVariables(AniAniSetObj *parent);
		
		// [Default implementation for virtuals]
		virtual ANI::TNLocation NameLocation() const;
		virtual AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		virtual int FixIncompleteTypes();
		virtual int PerformCleanup(int cstep);
		virtual int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};


// Internal variables of an object. This class can represent all 
// built-in object vars; see AniObjectInstance::BuiltinVarIndex for details. 
class AniInternalVariable_Object : public AniInternalVariable
{
	public:
		// Never call directly; use 
		// AniInternalVariable::RegisterInternalVariables() instead. 
		static void _RegisterInternalVariables(AniAniSetObj *parent);
		
		// Eval addressation "lookup" functions: 
		// For the member state vars: 
		struct EAF_Member : EvalAddressationFunc
		{
			/*AniObjectInstance::BuiltinVarIndex*/int bvi;
			
			EAF_Member(/*AniObjectInstance::BuiltinVarIndex*/int _bvi) : 
				EvalAddressationFunc()  {  bvi=_bvi;  }
			
			// [overriding virtuals:]
			~EAF_Member();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			// This is an assert(0) for internal vars which are no lvalues. 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		// For values to be computed by the calc core at request. 
		struct EAF_Compute : EvalAddressationFunc
		{
			/*AniObjectInstance::BuiltinVarIndex*/int bvi;
			
			EAF_Compute(/*AniObjectInstance::BuiltinVarIndex*/int _bvi) : 
				EvalAddressationFunc()  {  bvi=_bvi;  }
			
			// [overriding virtuals:]
			~EAF_Compute();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			// This is an assert(0) for internal vars which are no lvalues. 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		
	private:
		// Which internal var we represent: 
		/*AniObjectInstance::BuiltinVarIndex*/int bvi;
		// If we represent the state version (lower case) or the 
		// computed version. 
		bool computed_var;
		
		// [overriding virtual from AniGlue_ScopeBase]
		int CG_GetVariableType(VariableTypeInfo *vti) const;
		EvalAddressationFunc *CG_GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug);
	public:
		AniInternalVariable_Object(const RefString &name,
			/*AniObjectInstance::BuiltinVarIndex*/int _bvi,bool _computed_var,
			ANI::TreeNode *_treenode,AniScopeBase *_parent);
		~AniInternalVariable_Object();
		
		// [overriding virtual from AniVariable]
		int GetAttrib() const
			{  return(0);  }
		
		// [overriding virtuals from AniVariable]
		ANI::ExprValueType GetType() const;
		EvalAddressationFunc *GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc);
		
		// [overriding virtuals]
		void AniTreeDump(StringTreeDump *d);
};


// Internal variables of animation and setting. This class can represent all 
// built-in ani/setting vars; see >bvi< for details. 
class AniInternalVariable_AniSet : public AniInternalVariable
{
	public:
		// Never call directly; use 
		// AniInternalVariable::RegisterInternalVariables() instead. 
		static void _RegisterInternalVariables(AniAniSetObj *parent);
		
		// Eval addressation "lookup" functions: 
		struct EAF_IVarSetting : EvalAddressationFunc
		{
			int bvi;  // See AniInternalVariable_AniSet member. 
			EAF_IVarSetting(int _bvi) : EvalAddressationFunc()
				{  bvi=_bvi;  }
			
			inline ANI::ExprValue &_DoEval(ANI::TNIdentifier *idf);
			
			// [overriding virtuals:]
			~EAF_IVarSetting();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			// This is an assert(0) for internal vars which are no lvalues. 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		struct EAF_IVarAnimation : EvalAddressationFunc
		{
			int bvi;  // See AniInternalVariable_AniSet member. 
			EAF_IVarAnimation(int _bvi) : EvalAddressationFunc()
				{  bvi=_bvi;  }
			
			inline ANI::ExprValue &_DoEval(ANI::TNIdentifier *idf);
			
			// [overriding virtuals:]
			~EAF_IVarAnimation();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			// This is an assert(0) for internal vars which are no lvalues. 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		
	private:
		// Which internal var we represent: 
		// This is the internal ivar_ev index of the animation or setting. 
		// See AniSetting::BuiltinVarIndex and AniAnimation::BuiltinVarIndex. 
		int bvi;
		
		// [overriding virtual from AniGlue_ScopeBase]
		int CG_GetVariableType(VariableTypeInfo *vti) const;
		EvalAddressationFunc *CG_GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug);
	public:
		AniInternalVariable_AniSet(const RefString &name,int _bvi,
			ANI::TreeNode *_treenode,AniScopeBase *_parent);
		~AniInternalVariable_AniSet();
		
		// [overriding virtual from AniVariable]
		int GetAttrib() const
			{  return(IS_Static);  }
		
		// [overriding virtuals from AniVariable]
		ANI::ExprValueType GetType() const;
		EvalAddressationFunc *GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc);
		
		// [overriding virtuals]
		void AniTreeDump(StringTreeDump *d);
};


// Internal variables like "$t" for variables in ADBs like in 
// "function {}". 
class AniInternalVariable_ADB : public AniInternalVariable
{
	public:
		// Eval addressation "lookup" functions: 
		struct EAF_ADBVar : EvalAddressationFunc
		{
			EAF_ADBVar() : EvalAddressationFunc()
				{  }
			// [overriding virtuals:]
			~EAF_ADBVar();
			// For implicit member select based [implicitly] on "this" pointer: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// For explicit member select: 
			ANI::ExprValue Eval(ANI::TNIdentifier *idf,
				const ANI::ExprValue &base,ANI::ExecThreadInfo *info);
			// Version without member select returning a true C++ reference: 
			// This is an assert(0) for internal vars which are no lvalues. 
			ANI::ExprValue &GetRef(ANI::TNIdentifier *idf,
				ANI::ExecThreadInfo *info);
			// Perform evaluable check: 
			int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		};
		friend class EAF_ADBVar;
		
	private:
		// Value type...
		ANI::ExprValueType v_type;
		// Hook for calc core to hang information: 
		// This is set via constructor. 
		ANI::SpecialIdf_CoreHook *adb_idf_core_hook;
		
		// [overriding virtual from AniGlue_ScopeBase]
		int CG_GetVariableType(VariableTypeInfo *vti) const;
		EvalAddressationFunc *CG_GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug);
	public:
		AniInternalVariable_ADB(const RefString &name,
			ANI::TreeNode *_treenode,AniScopeBase *_parent,
			const ANI::ExprValueType &_v_type,
			ANI::SpecialIdf_CoreHook *_adb_idf_core_hook);
		virtual ~AniInternalVariable_ADB();
		
		// [overriding virtuals from AniVariable]
		ANI::ExprValueType GetType() const;
		EvalAddressationFunc *GetVarEvalAdrFuncFunc(
			ANI::TNIdentifier *idf_for_debug,AniIncomplete *ainc);
		
		// [overriding virtuals]
		void AniTreeDump(StringTreeDump *d);
		bool LookupMatchName(const RefString &name) const
			{  return(0); /* We're invisible for lookups. */ }
};

#endif   /* _ANIVISION_CORE_ANIVARIABLE_H_ */
