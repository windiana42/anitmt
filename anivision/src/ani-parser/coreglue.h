/*
 * ani-parser/coreglue.h
 * 
 * Main interface definition for interface to animation core. 
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

#ifndef _ANIVISION_ANI_COREGLUE_H_
#define _ANIVISION_ANI_COREGLUE_H_ 1

#include <hlib/cplusplus.h>
#include <ani-parser/treenode.h>  /* for TN_* */
#include <ani-parser/exprtype.h>
// MAY NOT INCLUDE <execstack.h> OR <evalstack.h> HERE. 

class String;


namespace ANI
{

// These are provided here to avoid including all the files. 
class TreeNode;
class TRegistrationInfo;
class ExprValueType;
struct CheckIfEvalInfo;

// Provided further down: 
class AniGlue_ScopeInstanceBase;
class ExecThreadInfo;


// Used inside TRegistrationInfo::SpecialIdf as core hook: 
struct SpecialIdf_CoreHook
{
	// Incremented whenever an AniInternalVariable_ADB was created 
	// which is using this CoreHook. Decremented when the 
	// AniInternalVariable_ADB gets deleted. 
	int core_hook_used;
	
	// Use these rather than core_hook_used directly. 
	inline void aqref()  {  ++core_hook_used;  }
	inline void deref()  {  if(--core_hook_used<=0) delete this;  }
	
	_CPP_OPERATORS
	SpecialIdf_CoreHook() : core_hook_used(0) {}
	virtual ~SpecialIdf_CoreHook();
	
	// Must be provided if used: 
	virtual ExprValue Eval(TNIdentifier *idf,ExecThreadInfo *info);
};  


// This is an abstract base class which must be provided by the core. 
// All the Ani* types must be derived from it with this class as the 
// topmost base. 
struct AniGlue_ScopeBase
{
	// For the CG_ASType() call: Type of derived class. 
	enum AniScopeType
	{
		// This are also the types of the corresponding tree nodes 
		// (except for AST_InternalFunc and AST_Incomplete). 
		// Note: AST_Incomplete is for not-yet-resolved identifiers, 
		//       notably functions which cannot be looked up in 
		//       registration step becuase the type of their args is 
		//       not known until the TF step. TF will eliminate all 
		//       AST_Incomplete (which are needed to keep the correct 
		//       place of the identifier in the ani scope tree). 
		AST_Animation=ANI::TN_Animation,
		AST_Setting=  ANI::TN_Setting,
		AST_Object=   ANI::TN_Object,
		AST_UserFunc= ANI::TN_FunctionDef,
		AST_InternalFunc=ANI::TN_None-1,  // POV member function attached to TNFunctionDef "$$povhook". 
		AST_AnonScope=ANI::TN_Statement,
		AST_UserVar=  ANI::TN_Declarator,
		AST_InternalVar=ANI::TN_None-2,
		AST_Incomplete=ANI::TN_None,  // <-- for not yet resolved identifiers
	};
	// Return string representation:  
	const char *AniScopeTypeStr() const
		{  return(CG_AniScopeTypeStr());  }
	// Must be overridden by all classes. 
	virtual const char *CG_AniScopeTypeStr() const;
	// This is a different version but use the member version 
	// above whenever you can. 
	const char *_AniScopeTypeStr(AniScopeType ast);
	
	enum IncompleteType
	{
		IT_Unknown=0,
		IT_Function,  // some function
		IT_NULL,      // "NULL"
	};
	
	// Needed for CG_LookupFunction. 
	struct LookupFunctionInfo
	{
		//--- THIS HAS TO BE FILLED IN BY CALLER ---
		// Number of args which got specified for the function call. 
		int n_args;
		// Array of TNIdentifier tree nodes for the "name=value"-style 
		// argument spec. Each arg which does not have a "name=" part 
		// has set that to NULL. Array of size [n_args]. 
		TNIdentifier **arg_name;
		// Array of argument types. Must be valid for each argument. 
		// Array of size [n_args]. 
		ExprValueType *arg_type;
		// Location where it is called. Only needed when 
		// CG_LookupConstructor() is called for !arrays!!
		TNLocation *call_loc;
		// Name identifier; for "new object" only. 
		// For name extraction only because the TNIdentifier is 
		// no longer attached to an AniIncomplete at this stage. 
		TNIdentifier *name_idf;
		// The function call operator function, if any. May be 
		// needed for special functions such as calling a setting 
		// as a function. 
		TNOperatorFunction *opfunc;
		
		//--- THIS GETS FILLED IN BY CG_LookupFunction() ---
		// The ani_function which matched. NULL for failure. 
		// (Can also be a AniSetting for special cases, e.g. when 
		// "calling" a setting if core provides that.)
		AniGlue_ScopeBase *ani_function;
		// The return type of the function: 
		ExprValueType ret_evt;
		// The eval handler "function" for this function call: 
		// (Allocated via new and to be free'd by ani-parser.) 
		void *evalfunc;  // OF TYPE TNOperatorFunction::EvalFunc. 
		// ONLY valid for CG_LookupConstructor(): 
		// AniGlue_ScopeBase::EvalAddressationFunc to attach to the 
		// identifier holding the type: this evalfunc will actually 
		// do the alloc & prototype vals. 
		void *allocfunc;  // OF TYPE AniGlue_ScopeBase::EvalAddressationFunc
		// This is set to 1 if the function is trivial const-folding 
		// (subexpression) evaluable, such as sin(), cos(), ...
		int may_try_ieval;
		
		_CPP_OPERATORS
		LookupFunctionInfo();
		~LookupFunctionInfo() {}
	};
	
	// Passed as argument to CG_GetVariableType(). 
	struct VariableTypeInfo
	{
		ExprValueType *store_here;  // <-- Must be set by caller. 
		// These are returned by the function: 
		int is_lvalue : 1;
		int is_const : 1;
		int may_ieval : 1;   // may const-fold without exec context (exec thread info)?
		
		_CPP_OPERATORS
		VariableTypeInfo()
			{  store_here=NULL;  is_lvalue=0; is_const=0; may_ieval=0;  }
		~VariableTypeInfo() {}
	};
	
	// This is the addressation handler used to evaluate 
	// TNIdentifiers. Must be provided by core. 
	struct EvalAddressationFunc
	{
		_CPP_OPERATORS
		EvalAddressationFunc() {}
		virtual ~EvalAddressationFunc();
		// Version without (explicit) member select: 
		virtual ExprValue Eval(TNIdentifier *tn,ExecThreadInfo *info);
		// Version for explicit member select: 
		virtual ExprValue Eval(TNIdentifier *tn,const ExprValue &base,
			ANI::ExecThreadInfo *info);
		// Version without member select returning a true C++ reference: 
		// This may be core-internal only. 
		virtual ExprValue &GetRef(ANI::TNIdentifier *idf,ExecThreadInfo *info);
		// This does the evaulable check, see treenode.h. 
		// Must be overridden. 
		virtual int CheckIfEvaluable(CheckIfEvalInfo *ci);
	}__attribute__((__packed__));
	
	// Needed by CG_LookupIdentifier(): 
	struct LookupIdentifierInfo
	{
		//--- THIS GETS FILLED IN BY CG_LookupIdentifier() ---
		// The ani scope of this identifier or AniIncomplete for 
		// a function. NULL on failure; error written. 
		AniGlue_ScopeBase *ani_scope;
		// EvalAddressationFunc to be used to evaluate that identifier. 
		// May be NULL if no evaluation will be made or the returned 
		// scope is still incomplete. 
		EvalAddressationFunc *evalfunc;
	};
	
	//----------------------------------------------------------------
	
	_CPP_OPERATORS
	
	AniGlue_ScopeBase() {}
	virtual ~AniGlue_ScopeBase();
	
	//----------------------------------------------------------------
	
	// This must be overridden. 
	// Called during the registration step 
	// (i.e. TreeNode's virtual DoRegistration()). 
	virtual AniGlue_ScopeBase *RegisterTN(ANI::TreeNode *tn,
		ANI::TRegistrationInfo *tri);
	
	// This is needed to get the name: 
	// "CG" = "CoreGlue". 
	virtual String CG_GetName() const;
	
	// This is needed to get the type of the derived class. 
	virtual AniScopeType CG_ASType() const;
	
	// Overridden by AniIncomplete to return the incomplete type: 
	virtual IncompleteType CG_IncompleteType() const;
	
	// This has to be overridden for AniVariable. 
	// Get the type of the variable. 
	// Store if the variable is a const in is_const. 
	// Return value: 0 (always)
	virtual int CG_GetVariableType(VariableTypeInfo *vti) const;
	
	// This has to be overridden for AniFunction (user functions, i.e. 
	// AST_UserFunc). 
	// Get the return type of the function. 
	// Return value: 0 (always)
	virtual int CG_GetReturnType(ExprValueType *store_here) const;
	
	// This has to be overridden for AniIncomplete. It gets 
	// called when the AniIncomplete is tried to be used as a 
	// function. Can be overridden by other classes, too, for 
	// special purpose. Default implementation returns -1. 
	// The function will also assign the function ani scope (AniFunction) 
	// to the TNIdentifier which had the AniIncomplete attached. 
	// All communication is done via LookupFunctionInfo. 
	// NOTE: If successful, the function DELETES THIS AniIncomplete. 
	// Return value: 
	//    0 -> see *lfi
	//   -1 -> error: "cannot call ... as a function" (must be written 
	//         by caller); do not use *lfi. 
	virtual int CG_LookupFunction(LookupFunctionInfo *lfi);
	
	// This has to be overridden for object types. 
	// Must serch for the constructor. 
	virtual void CG_LookupConstructor(LookupFunctionInfo *lfi);
	
	// Special identifier lookup function: 
	// If the passed identifier is a type or variable name, look it 
	// up. If it refers to a function, return the AniIncomplete (*this) 
	// because it has to be resolved using CG_LookupFunction() [later 
	// when the type of the args is known]. 
	// MUST BE OVERRIDDEN BY AniIncomplete. 
	// NOTE: If successful, it does the same assign-new-ani-scope-to-
	//       identifier-and-delete-self - "trick" as CG_LookupFunction(). 
	virtual void CG_LookupIdentifier(LookupIdentifierInfo *lii);
	
	// Must be overridden by AniVariable attached to TNIdentifier 
	// in cases where the variable does not have to be looked up 
	// (using CG_LookupIdentifier) but where we already know the 
	// attached var (i.e. declarations). 
	// Must return eval addressation function or NULL in case 
	// or error (with error message already written). 
	virtual EvalAddressationFunc *CG_GetVarEvalAdrFuncFunc(
		TNIdentifier *idf_for_debug);
	
	// Called by TNDeclarator in TF step via the ani scope attached 
	// to the associated identifier (var name). 
	// Must be overridden by AniVariable. 
	// NOTE: Return type is (TNDeclarator::EvalFunc*). 
	virtual void *CG_GetDeclaratorEvalFunc(TNDeclarator *decl,
		TNIdentifier *name);
	
	// Get eval function for return statement (either with or 
	// without expression). 
	// from_scope: from where it is called (AniAnonScope for "return") 
	// Must be overridden by AniFunction. 
	// NOTE: Return type is (TNJumpStmt::EvalFunc*). 
	virtual void *CG_GetJumpStmtEvalFunc(TNJumpStmt *tn,
		AssignmentFuncBase *assfunc,AniGlue_ScopeBase *from_scope);
	
	// Request an AniIncomplete place holder at the level below *this, 
	// i.e. the returned AniIncomplete must have *this as parent. 
	// Use this AniIncomplete to replace the one currently attached 
	// to the corresponding TNIentifier node in *old_location. 
	// (Therefore, old_location must still be an AniIncomplete.) 
	// MUST BE OVERRIDDEN BY ALL ANI SCOPES (normally AniScopeBase). 
	// ms_quality: member select quality. 
	//   0 -> no member select
	//   1 -> implicit member select from ms_from
	//        ("foo()" from within object)
	//   2 -> explicit member select from ms_from
	virtual AniGlue_ScopeBase *CG_ReplaceAniIncomplete(
		AniGlue_ScopeBase *old_location,
		AniGlue_ScopeBase *ms_from,int ms_quality);
	
	// This must be overridden by AniAnonScope and gets called 
	// when a compound stmt is entered or leaft (during execution). 
	virtual void CG_ExecEnterCompound(ExecThreadInfo *info);
	virtual void CG_ExecLeaveCompound(ExecThreadInfo *info);
	
	// Called when assocuiated TreeNode gets deleted. 
	// Only needs to be overridden if needed. 
	virtual void CG_DeleteNotify(TreeNode *tn);
};


class ScopeInstance;

// This is an abstract base class which must be provided by the core. 
// All the instances of the Ani* types must be derived from it. 
// It is internally used by ScopeInstance and does reference counting; 
// NOT C++ safe; use ScopeInstance. 
class AniGlue_ScopeInstanceBase
{
	friend class ScopeInstance;
	private:
		// Reference counter. 
		int refcnt;
		
		// Reference counting: 
		inline void _aqref()
			{  ++refcnt; /*fprintf(stderr,"####<%p>AQ[[%d]]####\n",this,refcnt);*/ }
		inline void _deref()
			{  /*fprintf(stderr,"####<%p>DE[[%d]]####\n",this,refcnt);*/
				if(--refcnt<=0) delete this;  }
		
		// NEVER USE THESE. 
		AniGlue_ScopeInstanceBase &operator=(const AniGlue_ScopeInstanceBase &);
		AniGlue_ScopeInstanceBase(const AniGlue_ScopeInstanceBase &);
	protected:
		// For derived classes: 
		AniGlue_ScopeInstanceBase()
			{  refcnt=0;  }
	public:  _CPP_OPERATORS
		// NOTE: No constructor. Must use a factory provided by the 
		//       core to create objects of this type. Use the factory 
		//       function CreateInstance of the associated 
		//       AniGlue_ScopeBase for that purpose. 
		virtual ~AniGlue_ScopeInstanceBase();
		
		// Get string representation of "value"(s): 
		// Must be overridden. 
		virtual String ToString() const;
		
		// Get the type of this scope: 
		virtual void GetExprValueType(ExprValueType *store_here) const;
		
		// Get pointer to associated AniGlue_ScopeBase: 
		//AniGlue_ScopeBase *GetScopeBase()
		//	{  return(scopebase);  }
};


// Provided in <execstack.h>: 
class ExecStack;
// Provided in <evalstack.h>:
class EvalStateStack;
// MAY NOT INCLUDE <execstack.h> HERE. 

// This is provided in <threadinfo.h>: 
struct ExecThreadInfo;

}  // End of namespace ANI. 


// These must be derived from AniGlue_ScopeBase as the topmost 
// base class. 
class AniAniSetObj;
class AniFunction;
class AniAnonScope;
class AniVariable;


#endif  /* _ANIVISION_ANI_COREGLUE_H_ */
