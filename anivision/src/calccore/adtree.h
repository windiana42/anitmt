/*
 * calccore/adtree.h
 * 
 * Animation description tree representation. 
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

#ifndef _ANIVISION_CALCCORE_TREE_H_
#define _ANIVISION_CALCCORE_TREE_H_ 1

#include <ani-parser/tree.h>

namespace CC
{

// ADTreeNode type: 
enum ADTNType
{
	ADTN_None=0,
	ADTN_AniDescBlock,
	ADTN_TypedScope,
	ADTN_ScopeEntry,
};

struct ADTreeNode : ANI::TreeNode
{
	public:
		static const ANI::TNType tn_type=ANI::TN_AniDesc;
	private:
		ADTNType adntype;
	public:
		ADTreeNode(ADTNType _adntype) : ANI::TreeNode(ANI::TN_AniDesc),
			adntype(_adntype)
			{  }
		// Special version for ADTN_None: 
		ADTreeNode(ANI::TreeNode *child) : ANI::TreeNode(ANI::TN_AniDesc),
			adntype(ADTN_None)
			{  AddChild(child);  }
		virtual ~ADTreeNode()
			{ }
		
		// Get ADTreeNode type: 
		ADTNType ADNType() const
			{  return(adntype);  }
		
		// Should be overridden by derived class: 
		virtual void DumpTree(StringTreeDump *d);
		virtual TreeNode *CloneTree();
		void DumpTree(FILE *out)
			{  ANI::TreeNode::DumpTree(out);  }
		// Furthermore, if needed: 
		// void DoRegistration(ANI::TRegistrationInfo *tri);
		// int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		// int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		// int Eval(ANI::ExprValue &retval,ANI::ExecThreadInfo *info);
		
		// Should be overridden by derived class: 
		// Get start and end location of complete subtree. [overriding a virtual]
		//virtual ANI::TNLocation GetLocationRange() const;  // <-- default implementation
};


//------------------------------------------------------------------------------

// This is the "{...}" block in "%name{...};". 
class ADTNAniDescBlock : public ADTreeNode
{
	// CHILDREN: Any number of ADTNScopeEntry. 
	private:
		ANI::TNLocation loc0;  // ... of the "{".
		ANI::TNLocation loc1;  // ... of the "}".
	public:
		ADTNAniDescBlock(const ANI::TNLocation &_loc0,
			const ANI::TNLocation &_loc1) : 
			ADTreeNode(ADTN_AniDescBlock),loc0(_loc0),loc1(_loc1)
			{  }
		~ADTNAniDescBlock()
			{  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		ANI::TNLocation GetLocationRange() const;
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		int Eval(ANI::ExprValue &nvalue,ANI::ExecThreadInfo *info);
};


// This is a typed scope, i.e. something like "combine { ... }" 
// or "cspline MySpline { ... }". 
class ADTNTypedScope : public ADTreeNode
{
	// CHILDREN: Any number in this order: 
	//   scope_type (TNIdentifierSimple)
	//   scope_name (TNIdentifier)  [may be missing]
	//   entires... (ADTNScopeEntry )  [any number, possibly zero]
	// NEVER use the children directly; use the members instead. 
	public:
		ANI::TNIdentifierSimple *scope_type;
		ANI::TNIdentifier *scope_name;       // Or NULL. 
		ADTNScopeEntry *first_entry;         // Or NULL if there are no entries. 
		// This is set during registration step: 
		// The calc core to be used is known at "compile time", hence 
		// lookups etc. can be performed. 
		// FIXME: the following should be moved into an aproprite calc core hook. 
		int scope_type_tok;  // Looked up scope_name token or -1. 
	private:
		ANI::TNLocation loc0;  // ... of the "{".
		ANI::TNLocation loc1;  // ... of the "}".
	public:
		ADTNTypedScope(ANI::TNIdentifierSimple *_type,
			const ANI::TNLocation &_loc0,const ANI::TNLocation &_loc1,
			ANI::TNIdentifier *_name=NULL) : 
			ADTreeNode(ADTN_TypedScope),loc0(_loc0),loc1(_loc1)
			{
				scope_type=_type;  if(scope_type) AddChild(scope_type);
				scope_name=_name;  if(scope_name) AddChild(scope_name);
				first_entry=NULL;
				scope_type_tok=-1;
			}
		~ADTNTypedScope()
			{  }
		
		// Used during construction: 
		void TransferScopeEntries(ANI::TreeNode *ents)
			{  first_entry=(ADTNScopeEntry*)ents->down.first();
				TransferChildrenFrom(ents);  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		ANI::TNLocation GetLocationRange() const;
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		//int Eval(ANI::ExprValue &nvalue,ANI::ExecThreadInfo *info);
};


// This is a scope entry, i.e. something like "name = expr ;" or 
// "name = ADTNTypedScope" or simply "ADTNTypedScope" or simply "expr;". 
class ADTNScopeEntry : public ADTreeNode
{
	// CHILDREN: One or two: 
	//   FIRST: The name part (if any; TNIdentifier)
	//   LAST:  The value part (i.e. TNExpression or ADTNTypedScope)
	public:
		// This is a core hook for the core to hang arbitrary data: 
		// May be allocated via new in the core and will be deleted by 
		// the destructor. This is set during the registration step. 
		struct CoreHook
		{
			_CPP_OPERATORS
			CoreHook(){}
			virtual ~CoreHook(){}
		} *ent_core_hook;
		
	private:
		ANI::TNLocation loc0;  // ... of the "=" (if any).
	public:
		ADTNScopeEntry(ADTNTypedScope *sc,ANI::TNIdentifier *_name=NULL,
			ANI::TNLocation *_loc0=NULL) : ADTreeNode(ADTN_ScopeEntry),
			ent_core_hook(NULL),loc0(_loc0 ? *_loc0 : ANI::TNLocation())
			{  if(_name) AddChild(_name);  if(sc) AddChild(sc);  }
		ADTNScopeEntry(ANI::TNExpression *expr,ANI::TNIdentifier *_name=NULL,
			ANI::TNLocation *_loc0=NULL) : ADTreeNode(ADTN_ScopeEntry),
			ent_core_hook(NULL),loc0(_loc0 ? *_loc0 : ANI::TNLocation())
			{  if(_name) AddChild(_name);  if(expr) AddChild(expr);  }
		~ADTNScopeEntry()
			{  DELETE(ent_core_hook);  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		//ANI::TNLocation GetLocationRange() const;  <-- use default
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		//int Eval(ANI::ExprValue &nvalue,ANI::ExecThreadInfo *info);
};

}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_TREE_H_ */
