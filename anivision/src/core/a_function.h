/*
 * core/a_function.h
 * 
 * ANI tree (user and internal) function description. 
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

#ifndef _ANIVISION_CORE_ANIFUNCTION_H_
#define _ANIVISION_CORE_ANIFUNCTION_H_ 1

#include <hlib/cplusplus.h>

#include <core/aniscope.h>


class AniFunction : 
	public AniScopeBase,  // <-- MUST BE FIRST. 
	public LinkedListBase<AniFunction>  // list: parent
{
	friend class AniScopeBase;
	friend class AniAniSetObj;
	public:
		// For the attributes: 
		enum
		{
			IS_Const=      0x0001,
			IS_Static=     0x0002,
			IS_Constructor=0x0010,
			IS_Destructor= 0x0020,
			IS_Animation=  0x0040,  // "animation NAME { ... }"
			IS_SettingInit=0x0080,  // setting init function "setting constructor"
		};
		
	protected:
		// Flags and attribuites: 
		int attrib;  // OR of the IS_* identifiers from above. 
	public:
		// Internal functions do not have attached tree nodes 
		// and thus set it to NULL. 
		AniFunction(AniScopeType _func_t,const RefString &name,
			ANI::TreeNode *_treenode,AniAniSetObj *_parent,int _attrib);
		// Destructor recursively deletes all children in the lists. 
		virtual ~AniFunction();
		
		// Get attribute flags: 
		int GetAttrib() const
			{  return(attrib);  }
		
		// Check if the function *this can be called with the passed 
		// arguments in lfi. 
		// Only if store_evalhandler is set, do the following: 
		//   * Alloocate and store eval handler in lfi->evalfunc. 
		//   * Set lfi->may_try_ieval if we may try to const-fold 
		//     subexpr eval this function (such as sin(),cos()...). 
		//   * Set lfi->ret_evt to the return type. 
		// Alloocate and store eval handler in lfi only if 
		// store_evalhandler is set. 
		// Return value: 
		//   0 -> No, cannot be called. 
		//   1 -> Can be called with (impicit) type conversion applied. 
		//   2 -> Can be called without type conversion. 
		// Must be overridden by derived class. 
		virtual int Match_FunctionArguments(LookupFunctionInfo *lfi,
			int store_evalhandler);
		
		// Return pretty name string with scope, name and args with name. 
		// Must be overridden by derived class. 
		virtual String PrettyCompleteNameString() const;
};


// There are two types of functions: 
// Internal functions and user-defined functions. 
class AniUserFunction : public AniFunction
{
	friend class AniScopeBase;
	friend class AniAniSetObj;
	public:
		struct FuncInstanceInfo : InstanceInfo
		{
			// Addressation index of retval or -1 (void). 
			// or -2 -> constructor ("officially" void but 
			// internally returns "this"). 
			int aidx_retval;
			// Number of used stack positions at stack beginning: 
			// (e.g. 1 for retval)
			int used_at_beginning;
			
			FuncInstanceInfo() : InstanceInfo()
				{ /*aidx_this=-1;*/ aidx_retval=-1; used_at_beginning=0; }
			~FuncInstanceInfo()  { }
		};
		
		// And here comes the king of the eval functions...
		// The function call handler: 
		struct EvalFunc_FCall : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniUserFunction *function;
			// Argument permutation arrays: 
			int *arg_perm;  // LMalloc()/LFree(); array [args_spec]
			// Number of specified args: 
			int args_spec;
			// For the apecified args (i.e. NOT the ones filled with 
			// default args), these are the assignment functions: 
			ANI::AssignmentFuncBase **arg_assfunc;  // array [args_spec]
			// Number of args filled with default values: 
			int n_def_args;
			// Finally the default args; these are the 
			// declarators to be evaluated: 
			ANI::TNDeclarator **default_decl;  // array [n_def_args]
			
			EvalFunc_FCall(AniUserFunction *_f,int _args_spec) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  arg_perm=NULL;  args_spec=_args_spec;  
					arg_assfunc=NULL;  n_def_args=0;  default_decl=NULL;  }
			// [overriding virtuals]
			~EvalFunc_FCall();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_FCall;
		
		// Eval func for the return statement: 
		struct EvalFunc_FuncReturn : ANI::TNJumpStmt::EvalFunc
		{
			// For return statement: Assignment function. 
			// Set in TF step and NULL if retval is void. 
			ANI::AssignmentFuncBase *assfunc;
			// Number of stack frames to go up to reach the return value: 
			int up_frames;
			// Return value addressation index or -1 for [void]. 
			int aidx_retval;
			
			EvalFunc_FuncReturn(ANI::AssignmentFuncBase *_assfunc,
				int _up_frames,int _aidx_retval) : ANI::TNJumpStmt::EvalFunc()
				{  assfunc=_assfunc;  up_frames=_up_frames;
					aidx_retval=_aidx_retval;  }
			// [overriding virtuals]
			~EvalFunc_FuncReturn();
			int Eval(ANI::TNJumpStmt *tn,ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_FuncReturn;
		
		// Eval func for animation block: 
		struct EvalFunc_Animation : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniUserFunction *function;
			
			EvalFunc_Animation(AniUserFunction *_f) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  }
			// [overriding virtuals]
			~EvalFunc_Animation();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Eval func for setting init. 
		struct EvalFunc_SettingInit : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniUserFunction *function;
			
			EvalFunc_SettingInit(AniUserFunction *_f) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  }
			// [overriding virtuals]
			~EvalFunc_SettingInit();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		
	private:
		// Contains all ARGUMENTS to the function: (automatic)
		_AniComponentList<AniVariable> arglist;
		// Contains all scopes (e.g. compound stmts) inside the function: 
		_AniComponentList<AniAnonScope> anonscopelist;
		// Contains incomplete (not yet resolved) identifiers: 
		_AniComponentList<AniIncomplete> incompletelist;
		// See aniscope.h/struct TypeFixEntry for more info: 
		LinkedList<TypeFixEntry> typefix_list;
		
		// Return type: 
		ANI::ExprValueType return_type;
		
		// See InstanceInfo in aniscope.h: 
		FuncInstanceInfo *fiinfo;
		
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
		// [overriding virtual from AniGlue_ScopeBase]
		// NOTE: Return type is (TNJumpStmt::EvalFunc*): 
		void *CG_GetJumpStmtEvalFunc(ANI::TNJumpStmt *tn,
			ANI::AssignmentFuncBase *assfunc,
			ANI::AniGlue_ScopeBase *from_scope);
		
		// [overriding virtuals from AniGlue_ScopeBase]
		int CG_GetReturnType(ANI::ExprValueType *store_here) const;
		const char *CG_AniScopeTypeStr() const;
	public:
		AniUserFunction(const RefString &name,ANI::TreeNode *_treenode,
			AniAniSetObj *_parent,int _attrib);
		// Destructor recursively deletes all children in the lists. 
		~AniUserFunction();
		
		// Get return type: 
		ANI::ExprValueType GetReturnType() const
			{  return(return_type);  }
		
		// Get the addressation index of the return value on the 
		// stack frame. 
		// Check for invalid retval -1/-2. 
		int GetAddressationIndex_RetVal() const
			{  return(fiinfo ? fiinfo->aidx_retval : -2);  }
		
		// Used to register function args: 
		AniVariable *RegisterTN_FuncDefArgDecl(ANI::TNFuncDefArgDecl *tn,
			ANI::TRegistrationInfo *tri);
		
		// [ovrriding virtuals from AniFunction]
		int Match_FunctionArguments(LookupFunctionInfo *lfi,
			int store_evalhandler);
		String PrettyCompleteNameString() const;
		
		// Get the eval function for this function if it is 
		// an animation block. Otherwise, return NULL. 
		// Returns handler allocated using new; must be freed 
		// by caller. 
		EvalFunc_Animation *GetAniEvalFunc();
		
		// Get the eval function for this function if it is 
		// a setting init function. Otherwise, return NULL. 
		// Returns handler allocated using new; must be freed 
		// by caller. 
		EvalFunc_SettingInit *GetSettingInitEvalFunc();
		
		// [overriding virtuals]
		void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		void AniTreeDump(StringTreeDump *d);
		ANI::TNLocation NameLocation() const;
		AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		int CountScopes(int &n_incomplete) const;
		int FixIncompleteTypes();
		LinkedList<TypeFixEntry> *GetTypeFixList();
		int PerformCleanup(int cstep);
		InstanceInfo *GetInstanceInfo();
		int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};


// Internal (i.e. built-in) function: 
// There are several types of internal functions, so this is merely a 
// "base class". 
class AniInternalFunction : public AniFunction
{
	protected:
		// [overriding a virtual]
		virtual void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
		// [overriding virtual from AniGlue_ScopeBase]
		const char *CG_AniScopeTypeStr() const;
	public:
		AniInternalFunction(const RefString &name,AniAniSetObj *_parent,
			int _attrib,ANI::TreeNode *_treenode=NULL);
		virtual ~AniInternalFunction();
		
		// Register all internal (built-in) functions for the 
		// passed scope: 
		static void RegisterInternalFunctions(AniAniSetObj *parent);
		
		// AniInternalFunction provides default implementations of 
		// several virtual functions. Override them for different 
		// implementations. 
		// [default virtual implementations]
		virtual void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		virtual ANI::TNLocation NameLocation() const;
		virtual AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		virtual int CountScopes(int &n_incomplete) const;
		virtual int FixIncompleteTypes();
		virtual LinkedList<TypeFixEntry> *GetTypeFixList();
		virtual int PerformCleanup(int cstep);
		virtual InstanceInfo *GetInstanceInfo();
		virtual int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};

// Simple intenral STATIC function taking a FIXED NUMBER OF ARGS and 
// NO DEFAULT VALUES for args (i.e. sin(), atan2(), vcross(), ...)
// AND: No context switches. 
class AniInternalFunc_Simple : public AniInternalFunction
{
	public:
		// Never call directly. Use AniInternalFunction::RegisterInternalFunctions() 
		// instead. 
		static void _RegisterInternalFunctions(AniAniSetObj *parent);
	public:
		// Values for flags. 
		enum
		{
			IFF_MayNotTryIEval=0x100,  // may NOT try... (e.g. "puts()")
			IFF_EvalFuncMASK=  0x00f,
			IFF_EvalFunc0A=    0x001,  // use 0 arg eval func
			IFF_EvalFunc1A=    0x002,  // use 1 arg eval func
			IFF_EvalFuncClone= 0x003,  // eval func for clone()
			IFF_EvalFuncYield= 0x004,  //               yield()
			IFF_EvalFuncDelay= 0x005,  //               delay()
			IFF_EvalFuncKillS= 0x006,  //               kill([void])
			IFF_EvalFuncAbort= 0x007,  //               abort()
		};
		
		struct AIFSFuncArgDesc
		{
			const char *aname;          // argument name (normally "x")
			ANI::ExprValueType atype;   // argument type
			
			_CPP_OPERATORS_ARRAY
			AIFSFuncArgDesc() : aname(NULL),atype()  { }
			AIFSFuncArgDesc(const AIFSFuncArgDesc &s) : 
				aname(s.aname),atype(s.atype)  { }
			AIFSFuncArgDesc(const char *_aname,
				const ANI::ExprValueType &_atype) : 
				aname(_aname),atype(_atype)  { }
			~AIFSFuncArgDesc()  { }
			
			AIFSFuncArgDesc &operator=(const AIFSFuncArgDesc &s)
				{  aname=s.aname;  atype=s.atype;  return(*this);  }
		};
		
		// Eval function for the simple internal functions with 
		// more than one arg. 
		struct EvalFunc_FCallSimpleInternal : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniInternalFunc_Simple *function;
			// Argument permutation arrays: 
			int *arg_perm;  // LMalloc()/LFree(); array [args_spec]
			// For the apecified args, these are the assignment functions: 
			ANI::AssignmentFuncBase **arg_assfunc;  // array [args_spec]
			// Pointer to function for the actual eval func: 
			void (*ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *);
			
			EvalFunc_FCallSimpleInternal(AniInternalFunc_Simple *_f,
				void (*_ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *)) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  arg_perm=NULL;  arg_assfunc=NULL;
					ifptrN=_ifptrN;  }
			// [overriding virtuals]
			~EvalFunc_FCallSimpleInternal();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_FCallSimpleInternal;
		// ...and for simple internal functions with exactly one arg: 
		struct EvalFunc_FCallSimpleInternal1A : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniInternalFunc_Simple *function;
			// Assignment function or NULL: 
			ANI::AssignmentFuncBase *arg_assfunc;
			// Pointer to function for the actual eval func: 
			void (*ifptr1)(ANI::ExprValue &,const ANI::ExprValue &);
			
			EvalFunc_FCallSimpleInternal1A(AniInternalFunc_Simple *_f,
				void (*_ifptr1)(ANI::ExprValue &,const ANI::ExprValue &)) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  ifptr1=_ifptr1;  arg_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_FCallSimpleInternal1A();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// ...and for simple internal functions with no args: 
		struct EvalFunc_FCallSimpleInternal0A : ANI::TNOperatorFunction::EvalFunc
		{
			// Function to be called. 
			AniInternalFunc_Simple *function;
			// Pointer to function for the actual eval func: 
			void (*ifptr0)(ANI::ExprValue &);
			
			EvalFunc_FCallSimpleInternal0A(AniInternalFunc_Simple *_f,
				void (*_ifptr0)(ANI::ExprValue &)) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  ifptr0=_ifptr0;  }
			// [overriding virtuals]
			~EvalFunc_FCallSimpleInternal0A();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Special eval func for clone() call: 
		struct EvalFunc_FCallInternalClone : ANI::TNOperatorFunction::EvalFunc
		{
			EvalFunc_FCallInternalClone() : ANI::TNOperatorFunction::EvalFunc()
				{ }
			// [overriding virtuals]
			~EvalFunc_FCallInternalClone();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Special eval func for yield() call: 
		struct EvalFunc_FCallInternalYield : ANI::TNOperatorFunction::EvalFunc
		{
			EvalFunc_FCallInternalYield() : ANI::TNOperatorFunction::EvalFunc()
				{ }
			// [overriding virtuals]
			~EvalFunc_FCallInternalYield();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Special eval func for delay() call: 
		struct EvalFunc_FCallInternalDelay : ANI::TNOperatorFunction::EvalFunc
		{
			// Assignment function or NULL: 
			ANI::AssignmentFuncBase *arg_assfunc;
			
			EvalFunc_FCallInternalDelay() : ANI::TNOperatorFunction::EvalFunc()
				{  arg_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_FCallInternalDelay();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Special eval func for kill([void]): 
		struct EvalFunc_FCallInternalKillS : ANI::TNOperatorFunction::EvalFunc
		{
			EvalFunc_FCallInternalKillS() : ANI::TNOperatorFunction::EvalFunc()
				{ }
			// [overriding virtuals]
			~EvalFunc_FCallInternalKillS();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Special eval func for abort([void]): 
		struct EvalFunc_FCallInternalAbort : ANI::TNOperatorFunction::EvalFunc
		{
			ANI::TNOperatorFunction *call_tn;
			
			EvalFunc_FCallInternalAbort(ANI::TNOperatorFunction *_call_tn) : 
				ANI::TNOperatorFunction::EvalFunc()  {  call_tn=_call_tn;  }
			// [overriding virtuals]
			~EvalFunc_FCallInternalAbort();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
	private:
		// Number of args: 
		int nargs;
		// Type and name of the arguments: 
		AIFSFuncArgDesc *argdesc;  // array size [nargs]
		
		// Pointer to the actual "function". 
		// If IFF_EvalFunc1A flag is set, the ifptr1 is used, 
		// if IFF_EvalFunc0A if set, use ifptr0, 
		// otherwise, ifptrN is used. 
		union {
			void (*ifptr0)(ANI::ExprValue &);
			void (*ifptr1)(ANI::ExprValue &,const ANI::ExprValue &);
			void (*ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *);
		};
		
		// Flags; see enum IFF_* 
		int flags;
		
		// Return type of the funtion. There is a handler function which 
		// returns the return type...
		// Args are: array of function arg types (in the order as accepted 
		// by the function and NOT as specified, of course) and the size of 
		// that array (must be nargs). 
		// Special case: int-arg=-1 -> to get return type to be displayed 
		// for messages/dumps. 
		// NOTE: ALSO USED TO VERIFY IF THE PASSED ARG COMBINATION IS 
		//       ALLOWED. 
		ANI::ExprValueType (*return_type_hdl)(int,const ANI::ExprValueType *);
		
	public:
		// NOTE: _args is array of size [_nargs] and is copied by 
		//       the constructor (internally allocating this->argdesc 
		//       using operator new[])
		// After calling the constructor, do not forget to call 
		// SetIFPtr() to set the actual computation function. 
		AniInternalFunc_Simple(const RefString &_name,AniAniSetObj *_parent,
			ANI::ExprValueType (*_return_type_hdl)(int,const ANI::ExprValueType *),
			int _nargs,const AIFSFuncArgDesc *_args);
		~AniInternalFunc_Simple();
		
		// This MUST be used to set the the actual computation 
		// function: First one sets the IFF_EvalFunc0A flag, second 
		// one the IFF_EvalFunc1A flag. 
		void SetIFPtr(void (*ifptr0)(ANI::ExprValue &),int flags);
		void SetIFPtr(void (*ifptr1)(ANI::ExprValue &,const ANI::ExprValue &),
			int flags);
		void SetIFPtr(void (*ifptrN)(ANI::ExprValue &,int,const ANI::ExprValue *),
			int flags);
		
		// Special function; do not use: 
		void _InternalSetFlags(int _flags)
			{  flags|=_flags;  }
		
		// [ovrriding virtuals from AniFunction]
		int Match_FunctionArguments(LookupFunctionInfo *lfi,
			int store_evalhandler);
		String PrettyCompleteNameString() const;
		
		// [overriding non-default virtuals -> iInternalFunction]
		void AniTreeDump(StringTreeDump *d);
};

// All the $pov() functions. Attaching/detaching objects, scanning files,...
class AniInternalFunc_POV : public AniInternalFunction
{
	public:
		// Never call directly. Use AniInternalFunction::RegisterInternalFunctions() 
		// instead. 
		static void _RegisterInternalFunctions(AniAniSetObj *parent);
		
	private:
		// Special: Create and register pov hook. This is a "function" 
		// (TNFunctionDef with TNCompoundStmt) in the associated 
		// TNObject tree node and is used to hang the attached 
		// POV objects, etc. Called by _RegisterInternalFunctions(). 
		// Returns function def tree node. 
		// READ COMMENT IN a_ifuncpov.cc. 
		static ANI::TNFunctionDef *RegisterPOVHook(AniObject *obj);
		
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int do_queue);
	public:
		enum POVFuncType
		{
			POVFT_Member=1,  // member version (objects)
			POVFT_Static     // static version (objects,animation,setting)
		};
		
		// Eval func for static type: 
		struct EvalFunc_POVStatic : ANI::TNOperatorFunction::EvalFunc
		{
			// Associated POV function... well, simply the "placeholder" 
			// in the ANI tree. 
			AniInternalFunc_POV *function;
			// Assignment function or NULL for "scan=": 
			ANI::AssignmentFuncBase *scan_assfunc;
			
			EvalFunc_POVStatic(AniInternalFunc_POV *_f) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  scan_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_POVStatic();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_POVStatic;
		
		// Eval func for member type: 
		struct EvalFunc_POVMember : ANI::TNOperatorFunction::EvalFunc
		{
			// Associated POV function... well, simply the "placeholder" 
			// in the ANI tree. 
			AniInternalFunc_POV *function;
			// Number of args: 
			int nargs;
			// Action: +1 -> attach, -1 -> detach; 
			int action;
			// Object name arg index: 
			int objname_idx;
			// Scan file name arg index: 
			int scanfile_idx;
			// Assignment functions for the args: [nargs]
			ANI::AssignmentFuncBase **arg_assfunc;
			
			EvalFunc_POVMember(AniInternalFunc_POV *_f) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_f;  nargs=0;  action=0;  objname_idx=-1;
					scanfile_idx=-1;  arg_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_POVMember();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_POVMember;
		
	private:
		POVFuncType povftype;
		
		// Associated with the compound in the attached $$povhook 
		// function: (It's only one but oh well... [needed for tree struct])
		_AniComponentList<AniAnonScope> anonscopelist;
		
		// Get pointer to animation: 
		AniAnimation *_GetAnimation();
		
		//** All the _DO_* functions are called by eval functions 
		//** during execution. 
		// Scan POV files. ("$pov(scan={...})")
		int _DO_ScanPOVFiles(const ANI::ExprValue &file_list);
		// Do attach/detach: 
		// Be careful: _scanfile may be NULL; pointers refer to 
		//   values on the stack of the caller. 
		// Returns status code (see a_ifuncpov.cc). 
		int _DO_POVAttachDetach(Animation_Context *ctx,
			const ANI::ExprValue *_objname,const ANI::ExprValue *_scanfile,
			int action,const ANI::ScopeInstance &obj_instance);
		
		// Actually scan POV file... or better: call some function 
		// which does it. Returns error count. 
		int _ScanPOVFile(const RefString &filename);
		// Handle the attach/detach (scan file if not found): 
		// Returns status code (see a_ifuncpov.cc). 
		int _POVAttachDetach(Animation_Context *ctx,
			const RefString &objname,const RefString &scanfile,int action,
			const ANI::ScopeInstance &obj_instance);
		
		// Helper for Match_FunctionArguments(): 
		int _Match_FuncArgs_Member(LookupFunctionInfo *lfi,
			int store_evalhandler);
		int _Match_FuncArgs_Static(LookupFunctionInfo *lfi,
			int store_evalhandler);
	public:
		// Must pass "$pov" as name. 
		AniInternalFunc_POV(const RefString &_name,AniAniSetObj *_parent,
			POVFuncType _povftype,ANI::TNFunctionDef *_treenode_pov_hook);
		~AniInternalFunc_POV();
		
		// [ovrriding virtuals from AniFunction]
		int Match_FunctionArguments(LookupFunctionInfo *lfi,
			int store_evalhandler);
		String PrettyCompleteNameString() const;
		
		// [overriding non-default virtuals -> iInternalFunction]
		void LookupName(const RefString &name,
			AniScopeBase::ASB_List *ret_list,int query_parent);
		int CountScopes(int &n_incomplete) const;
		void AniTreeDump(StringTreeDump *d);
		int PerformCleanup(int cstep);
};

// The wait() and trigger() functions. 
class AniInternalFunc_Wait : public AniInternalFunction
{
	public:
		// Never call directly. Use AniInternalFunction::RegisterInternalFunctions() 
		// instead. 
		static void _RegisterInternalFunctions(AniAniSetObj *parent);
		
	public:
		enum WaitFuncType
		{
			WFT_PWait=1,
			WFT_Wait,
			WFT_Trigger
		};
		
		// Eval func for the pwait() call. 
		struct EvalFunc_PWait : ANI::TNOperatorFunction::EvalFunc
		{
			AniInternalFunc_Wait *function;
			
			// Number of args: 
			int nargs;
			// Arg index for "until": 
			int until_idx;
			// Assignment functions for the args: [nargs]
			ANI::AssignmentFuncBase **arg_assfunc;
			
			EvalFunc_PWait(AniInternalFunc_Wait *_function) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_function;  nargs=0;  until_idx=-1;
					arg_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_PWait();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		// Eval func for the wait() call. 
		struct EvalFunc_Wait : ANI::TNOperatorFunction::EvalFunc
		{
			AniInternalFunc_Wait *function;
			
			// Number of args: 
			int nargs;
			// Arg index for "until": 
			int until_idx;
			// Arg index for "poll": 
			int poll_idx;
			// Assignment functions for the args: [nargs]
			ANI::AssignmentFuncBase **arg_assfunc;
			
			EvalFunc_Wait(AniInternalFunc_Wait *_function) : 
				ANI::TNOperatorFunction::EvalFunc()
				{  function=_function;  nargs=0;  until_idx=-1;
					poll_idx=-1;  arg_assfunc=NULL;  }
			// [overriding virtuals]
			~EvalFunc_Wait();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_Wait;
		// Eval func for the trigger() call. 
		struct EvalFunc_Trigger : ANI::TNOperatorFunction::EvalFunc
		{
			EvalFunc_Trigger() : ANI::TNOperatorFunction::EvalFunc()
				{  }
			// [overriding virtuals]
			~EvalFunc_Trigger();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		
	private:
		// Which function we are: 
		WaitFuncType func_type;
		
		// Match_FunctionArguments() helper: 
		int _Match_FuncArgs_PWait(LookupFunctionInfo *lfi,
			int store_evalhandler);
		int _Match_FuncArgs_Wait(LookupFunctionInfo *lfi,
			int store_evalhandler);
		int _Match_FuncArgs_Trigger(LookupFunctionInfo *lfi,
			int store_evalhandler);
	public:
		AniInternalFunc_Wait(const RefString &_name,AniAniSetObj *_parent,
			WaitFuncType _func_type);
		~AniInternalFunc_Wait();
		
		// [ovrriding virtuals from AniFunction]
		int Match_FunctionArguments(LookupFunctionInfo *lfi,
			int store_evalhandler);
		String PrettyCompleteNameString() const;
		
		// [overriding non-default virtuals -> iInternalFunction]
		void AniTreeDump(StringTreeDump *d);
};

#endif  /* _ANIVISION_CORE_ANIFUNCTION_H_ */
