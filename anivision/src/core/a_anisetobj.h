/*
 * core/a_anisetobj.h
 * 
 * ANI tree animation, setting and object scopes. 
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

#ifndef _ANIVISION_CORE_ANIANISETOBJ_H_
#define _ANIVISION_CORE_ANIANISETOBJ_H_ 1

#include <hlib/cplusplus.h>

#include <core/aniscope.h>

// For POV command parser: 
#include <pov-core/povparser.h>
// For the POV frame writer: 
#include <pov-core/povwriter.h>

// How it works... -> complete comment in anicontext.h. 
#include <core/anicontext.h>


// Description layer for Animation, Setting and Object. 
class AniAniSetObj : 
	public AniScopeBase,  // <-- MUST BE FIRST. 
	public LinkedListBase<AniAniSetObj>  // list: parent
{
	protected:
		// Contains all sub-AniSetObj nodes: 
		_AniComponentList<AniAniSetObj> anisetobjlist;
		// Contains all functions in this ani/setting/obj: 
		_AniComponentList<AniFunction> functionlist;
		// Contains all member variables. (non-auto)
		_AniComponentList<AniVariable> variablelist;
		// Contains incomplete (not yet resolved) identifiers: 
		_AniComponentList<AniIncomplete> incompletelist;
		// See aniscope.h/struct TypeFixEntry for more info: 
		LinkedList<TypeFixEntry> typefix_list;
		
		// See InstanceInfo in aniscope.h: 
		InstanceInfo *iinfo;
		
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
		// [overriding a virtual from AniGlue_ScopeBase]
		const char *CG_AniScopeTypeStr() const;
	protected:
		// MUST USE DERIVED CLASS. 
		// DO NOT CREATE INSTANCES OF THIS CLASS DIRECTLY. 
		// Must pass AST_Animation, AST_Setting or AST_Object as astype. 
		// _parent is NULL for AST_Animation. 
		AniAniSetObj(AniScopeType astype,const RefString &name,
			ANI::TreeNode *_treenode,AniAniSetObj *_parent);
	public:  _CPP_OPERATORS
		~AniAniSetObj();
		
		// Return variable/animation-setting-object child list. 
		// Be careful; only use in read-only manner. 
		const _AniComponentList<AniVariable> *GetVariableList() const
			{  return(&variablelist);  }
		const _AniComponentList<AniAniSetObj> *GetAniSetObjList() const
			{  return(&anisetobjlist);  }
		
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
		// [overriding a virtual from AniGlue_ScopeBase:]
		void CG_LookupConstructor(LookupFunctionInfo *lfi);
};


// As defined below. 
class AniAnimation;
class AniSetting;
class AniObject;

class AniUserFunction;


// DERIVED TYPES: 
class AniAnimation : 
	public AniAniSetObj,
	public ANI::ValueNotifyHandler
{
	public:
		// This is the built-in variable index in the ivar_ev array below. 
		enum BuiltinVarIndex
		{
			BVI_Time=0,  // <-- First MUST be 0. 
			
			_BVI_LAST
		};
	
	private:
		// The animation holds the POV archive. 
		// This is the command parser which holds a list of all 
		// commands, POV objects, ...
		// It can be asked at any time to read in some file, etc. 
		POV::CommandParser pov_cmd_parser;
		
		// The animation also holds the POV frame writer. 
		POV::FrameWriter pov_frame_writer;
		
		// The animation holds a list of animation contexts. 
		// See the above comment for more info. 
		LinkedList<Animation_Context> context_list;
		// This is a pointer to the active animation context 
		// or NULL in case we're not in ani context. 
		// FIXME: No longer needed. 
		//Animation_Context *active_context;
		
		// These are the internal variables of the animation. 
		// These variables represent the current state and are also lvalues. 
		// Change notifiers are used to notify the CCIF about changes in 
		// the internal variables. 
		ANI::ExprValue ivar_ev[_BVI_LAST];
		
		// This ist the primary animation time. There is a "double" version 
		// for fast access and a ExprValue version (ivar_ev[BVI_Time]) 
		// to provide an lvalue. 
		// We stay informed about changes to the ExprValue version via 
		// ValueNotifyHandler's vn_change(). 
		double ani_time;
		
		// Used by _GetUniqueThreadID(): 
		int thread_id_counter;
		
		// Get (new) unique thread_id for the passed context. 
		int _GetUniqueThreadID(Animation_Context *ctx);
		
		// Internal helpers of RunContext(): 
		int _DoRunThread(Animation_Context *ctx,ExecThread *run_thread);
		void _DoCheckAllWaitConditions(Animation_Context *ctx);
		ExecThread *_DoContextSwitch(Animation_Context *ctx,
			ExecThread *run_thread,int eval_rv);
		void _DoCheckThreadDelays(Animation_Context *ctx,
			ExecThread **update_run_thread);
		
		// Internally used POV helpers: 
		int _DoPOVAttach(Animation_Context::ObjectInstanceNode *oin,
			POV::ObjectSpec *ospec);
		int _DoPOVDetach(Animation_Context::ObjectInstanceNode *oin,
			Animation_Context::POVAttachmentNode *an);
		
		// Delete all contexts (and hence all threads): 
		void _CleanupAllContexts();
		
		// [overriding virtuals from ValueNotifyHandler:]
		void vn_change(void *hook);
		void vn_delete(void *hook);
	public:  _CPP_OPERATORS
		AniAnimation(const RefString &name,ANI::TreeNode *_treenode);
		~AniAnimation();
		
		// Get internal variable (this will return the current state 
		// and will not trigger a computation by the calc core): 
		// Returns a true reference of the internal variable with 
		// specified index. Use copy constructor once if you do not 
		// need a true lvalue. 
		ANI::ExprValue &GetInternalVarRef(/*BuiltinVarIndex*/int bvi);
		
		// Scan a POV file. 
		// Returns error count. 
		int ScanPOVFile(const RefString &filename);
		
		// Find POV object; return NULL if not found. 
		POV::ObjectSpec *FindPOVObject(const RefString &objname);
		
		// Attach POV object to object instance for passed animation context 
		// or detach again. Called by the internal pov function. 
		// Attach adds passed object to the end, detach removes last 
		// occurance in list. 
		// Return value: success status (as $pov()). 
		int POVAttach(Animation_Context *ctx,AniObjectInstance *obj_inst,
			const RefString &objname);
		int POVDetach(Animation_Context *ctx,AniObjectInstance *obj_inst,
			const RefString &objname);
		
		// Find the animation block with the passed name. 
		// If no name is specified (NULL ref), return the 
		// first one. Return NULL if not found. 
		AniUserFunction *FindAnimationFunction(const RefString &name);
		
		// Get currently active animation context or NULL: 
		//Animation_Context *GetActiveContext()  [currently unused]
		//	{  return(active_context);  }
		
		// Get context for passed setting with passed name. 
		// If such a context does not exist, create a new one. 
		Animation_Context *GetContextByName(const String &name,
			AniSetting *setting);
		
		// Run (start/cont) the animation using the passed context. 
		void RunContext(Animation_Context *ctx);
		
		// Execute animation with specified name. 
		// Use NULL ref for first one. 
		// Return value: 
		//   0 -> OK
		//  -2 -> animation lookup failure / no animation
		int ExecAnimation(const RefString &ani_name);
};


class AniSetting : 
	public AniAniSetObj,
	public ANI::ValueNotifyHandler
{
	friend class AniAniSetObj;
	public:
		// This is the built-in variable index in the ivar_ev array below. 
		enum BuiltinVarIndex
		{
			BVI_Time=0,  // <-- First MUST be 0. 
			
			_BVI_LAST
		};
	
	public:
		struct EvalFunc_FCallSetting : ANI::TNOperatorFunction::EvalFunc
		{
			// Setting to be "called". 
			AniSetting *setting;
			
			// Number of args: 
			int nargs;
			// Assignment functions for the args: [nargs]
			ANI::AssignmentFuncBase **arg_assfunc;
			
			// Argument index for each possible arg: 
			int arg_idx_t;
			int arg_idx_toff;
			int arg_idx_run_ctx;
			int arg_idx_save_ctx;
			int arg_idx_skip;
			
			EvalFunc_FCallSetting(AniSetting *_setting,int _nargs);
			// [overriding virtuals]
			~EvalFunc_FCallSetting();
			int Eval(ANI::ExprValue &retval,ANI::TNOperatorFunction *tn,
				ANI::ExecThreadInfo *info);
		};
		friend class EvalFunc_FCallSetting;
		
		// This is only to ease the parameter transfer in function calls 
		// for setting eval function(s): 
		struct EvalSettingArgs
		{
			ANI::ExprValue *t;
			ANI::ExprValue *toff;
			ANI::ExprValue *run_ctx;
			ANI::ExprValue *save_ctx;
			ANI::ExprValue *skip;
		};
		
		// Called by EvalFunc_FCallSetting::Eval(). 
		// Return value: like for all eval functions: 
		//  0 -> OK; 1 -> context switch
		int _DO_EvalSetting(EvalSettingArgs *args,ANI::ExecThreadInfo *info);
	private:
		// These are the internal variables of the setting. 
		// These variables represent the current state and are also lvalues. 
		// Change notifiers are used to notify the CCIF about changes in 
		// the internal variables. 
		ANI::ExprValue ivar_ev[_BVI_LAST];
		
		// This ist the setting time. There is a "double" version 
		// for fast access and a ExprValue version (ivar_ev[BVI_Time]) 
		// to provide an lvalue. 
		// We stay informed about changes to the ExprValue version via 
		// ValueNotifyHandler's vn_change(). 
		double setting_time;
		
		// This is the eval function for the setting init function. 
		// Created by RegisterTN_DoFinalChecks(). 
		ANI::TNOperatorFunction::EvalFunc *settinginit_eval_func;
		
		// [overriding virtual from AniGlue_ScopeBase]
		int CG_LookupFunction(LookupFunctionInfo *lfi);
		
		// [overriding virtuals from ValueNotifyHandler:]
		void vn_change(void *hook);
		void vn_delete(void *hook);
	public:  _CPP_OPERATORS
		AniSetting(const RefString &name,ANI::TreeNode *_treenode,
			AniAniSetObj *_parent);
		~AniSetting();
		
		// Return setting init eval func: 
		ANI::TNOperatorFunction::EvalFunc *GetSettingInitEvalFunc()
			{  return(settinginit_eval_func);  }
		
		// Get internal variable (this will return the current state 
		// and will not trigger a computation by the calc core): 
		// Returns a true reference of the internal variable with 
		// specified index. Use copy constructor once if you do not 
		// need a true lvalue. 
		ANI::ExprValue &GetInternalVarRef(/*BuiltinVarIndex*/int bvi);
};


// See <a_function.h> or similar: 
class AniInternalFunc_POV;

class AniObject : public AniAniSetObj
{
	public:
		// This is the $pov(attach/detach) function for this object. 
		// It is also queued in the functionlist but here for 
		// direct access. 
		// Set by AniInternalFunc_POV::_RegisterInternalFunctions(). 
		AniInternalFunc_POV *pov_attach_detach;
		
	public:
		AniObject(const RefString &name,ANI::TreeNode *_treenode,
			AniAniSetObj *_parent);
		~AniObject();
};

#endif  /* _ANIVISION_CORE_ANIANISETOBJ_H_ */
