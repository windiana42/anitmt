/*
 * calccore/anitmt/ccif_anitmt.h
 * 
 * AniTMT calc core (internal) setting/object/value description. 
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

#ifndef _ANIVISION_CALCCORE_ANITMT_CCIF_H_
#define _ANIVISION_CALCCORE_ANITMT_CCIF_H_ 1

#include <hlib/linkedlist.h>
#include <calccore/ccif.h>

#include <core/execthread.h>

#include <calccore/adtree.h>
//#include <numerics/la_basics.h>
#include <numerics/spline/sl_base.h>

#include <objdesc/move.h>

#include <functionality/object_base.hpp>
#include <stack>


namespace CC
{

class CCIF_Factory_AniTMT;

namespace AniTMT
{

// As defined in this file: 
class CCSetting;
class CCObject;
class CCValue;


class CCObject : 
	public CCIF_Object,
	public LinkedListBase<CCObject>
{
	private:
		// AniTMT 
		functionality::node_object *pt_root;
		proptree::Child_Manager *child_manager;
		
		// Used within Pass1
		std::stack<proptree::Prop_Tree_Node*> ptn_stack;
		proptree::Prop_Tree_Node* get_current_tree_node()
			{  return ptn_stack.top();  }
		void set_new_tree_node( proptree::Prop_Tree_Node* node )
			{  ptn_stack.push(node);  }
		void tree_node_done()
			{  ptn_stack.pop();  }
		
		// Helper for EvalAniDescBlock(): 
		int _PerformPass1(ADTNAniDescBlock *adtn,ExecThread *et,int subpass);
		int _PerformPass1(ADTNTypedScope *adtn,ExecThread *et,int subpass);
		int _PerformPass1(ADTNScopeEntry *adtn,ExecThread *et,int subpass);
		
		int _Pass1PropertyDeclaration(ADTNScopeEntry *scent,
			const RefString &name,const ANI::ExprValue &val);
		int _Pass1OpenBlockDecl(ADTNTypedScope *tsc,
			const RefString &str_type,const RefString &str_name,
			int subpass);
		int _Pass1FinishBlockDecl(int subpass);
		
		// Error counter: 
		int n_errors;
		
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
	public LinkedListBase<CCValue>
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
		CCSetting(CCIF_Factory_AniTMT *_factory);
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

}  // end of namespace AniTMT
}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_ANITMT_CCIF_H_ */
