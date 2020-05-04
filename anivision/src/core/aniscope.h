/*
 * core/aniscope.h
 * 
 * ANI scope tree base class. 
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

#ifndef _ANIVISION_CORE_SCOPEBASE_H_
#define _ANIVISION_CORE_SCOPEBASE_H_ 1

#include <ani-parser/tree.h>
#include <ani-parser/coreglue.h>


// NOTE: AniScopeBase must be derived from AniGlue_ScopeBase 
// becuase that is expected by the ani-parser. 
// AniGlue_ScopeBase must be the primary ancestor!

template<class Ani>struct _AniComponentList;


// NOTE: You may NOT simply delete an AniScopeBase. 
//       It is likely to be referenced from several points! 
//   If you are certain that you want to delete an ani scope: 
//   * To delete the complete ani scope tree, delete its root; 
//     all other nodes will be deleted recursively. 
//   * To delete an unneeded subtree (normally a single node -- 
//     an AniIncomplete), simply delete that node. It will 
//     remove itself from the tree (telling the parent that it 
//     is gone) and recursively delete the sub-nodes. 
class AniScopeBase : public ANI::AniGlue_ScopeBase
{
	public:
		// See <ani-parser/coreglue.h> for this one: 
		//enum AniScopeType
		//{
		//	// This are also the types of the corresponding tree nodes. 
		//	AST_Animation=ANI::TN_Animation,
		//	AST_Setting=  ANI::TN_Setting,
		//	AST_Object=   ANI::TN_Object,
		//	AST_UserFunc= ANI::TN_FunctionDef,
		//	AST_InternalFunc= ANI::TN_None-1,
		//	AST_AnonScope=ANI::TN_Statement,
		//	AST_UserVar= ANI::TN_Declarator,
		//	AST_InternalVar= ANI::TN_None-2,
		//  AST_Incomplete=ANI::TN_None,
		//};
		typedef AniGlue_ScopeBase::AniScopeType AniScopeType;
		
		enum LookupExpectedType
		{
			LET_None=        0x00,
			LET_TypeName=    0x01,   // animation, setting, object
			LET_FunctionName=0x02,
			LET_VariableName=0x04,
			LET_Any         =0x07,   // anything above
		};
		
		// Return string representation: 
		//static const char *_AniScopeTypeStr(AniScopeType ast); -> coreglue.h
		//const char *AniScopeTypeStr() const; -> coreglue.h
		static const char *LookupExpectedTypeStr(LookupExpectedType let);
		
		// A list of AniScopeBases. 
		// No external queuing info needed; does internal queuing by 
		// allocating nodes. 
		class ASB_List
		{
			public:
				struct Node : LinkedListBase<Node>
				{
					AniScopeBase *asb;
					
					operator AniScopeBase *()
						{  return(asb);  }
					
					_CPP_OPERATORS
					Node(AniScopeBase *_asb) : LinkedListBase<Node>()
						{  asb=_asb;  }
					~Node()  {  asb=NULL;  }
				};
			private:
				LinkedList<Node> list;
				
				// Do not use copy constructor or assignment: 
				void _forbidden();
				ASB_List(const ASB_List &)  { _forbidden(); }
				ASB_List &operator=(const ASB_List &)
					{  _forbidden();  return(*this);  }
			public:  _CPP_OPERATORS
				ASB_List() : list() {}
				~ASB_List()
					{  while(!list.is_empty()) delete list.popfirst();  }
				
				// Add an AniScopeBase at the end: 
				void append(AniScopeBase *asb)
					{  list.append(new Node(asb));  }
				
				// Transfer entries from the passed list to this one. 
				void transfer_from(ASB_List *l)
					{ while(!l->is_empty()) list.append(l->list.popfirst()); }
				// Transfer individual node n from list l to *this: 
				void transfer_from(ASB_List *l,Node *n)
					{  l->list.dequeue(n);  list.append(n);  }
				
				// Get first/last node; use next/prev for loops; don't 
				// modify otherwise. 
				Node *first()   {  return(list.first());  }
				Node *last()    {  return(list.last());  }
				
				// Remove node from list; make sure it is queued 
				// (e.g. because it is first() or first()->next...)
				// This also deletes the node. 
				void dequeue(Node *n)
					{  if(n) { list.dequeue(n); delete n; }  }
				
				// Clear list: 
				void clear()
					{  while(!list.is_empty()) delete list.popfirst();  }
				
				// Find AniScopeBase in the list. 
				const Node *find(AniScopeBase *asb)
					{  for(Node *i=list.first(); i; i=i->next)
						if(i->asb==asb) return(i);  return(NULL);  }
				
				// Count number of elements: 
				int count() const  {  return(list.count());  }
				// List empty?
				int is_empty() const  {  return(list.is_empty());  }
				// Has exactly 1 entry?
				int has_1_entry() const
					{  return(list.first() && list.first()==list.last());  }
		};
		
		// This is the special type fix entry node as used 
		// for TNTypeSpecifiers which are not fixed by other 
		// means because there is no corresponding AniScope, 
		// i.e. inside POD-type cast and new array operators. 
		// NOTE: The evt pointer is problematic because it is 
		//       suspicious for creating dangling pointers (normally 
		//       points to TNOperatorFunction::new_type). 
		//       HOWEVER, it only exists between registration and 
		//       type fixup. 
		struct TypeFixEntry : LinkedListBase<TypeFixEntry>
		{
			ANI::TNTypeSpecifier *tspec;
			ANI::ExprValueType *evt;   // Points to location of the EVT to fix. 
			
			_CPP_OPERATORS
			TypeFixEntry(ANI::TNTypeSpecifier *ts,ANI::ExprValueType *_evt) : 
				LinkedListBase<TypeFixEntry>()
				{  tspec=ts;  evt=_evt;  }
			~TypeFixEntry() {}
		};
		
		// This is used when creating an instance of an AniScope (i.e. 
		// by AniScopeInstanceBase's and derived types' constructor. 
		// Created with PerformCleanup(cstep=2). 
		// Can be queried via virtual GetInstanceInfo(). 
		// Also used by (ani) functions. 
		struct InstanceInfo
		{
			// Number vars on the stack (i.e. non-static). 
			// Animation,Setting,Object -> real non-static member vars
			//   (hence, 0 for animation & setting) 
			// Function -> arguments (automatic vars), this pointer, 
			//   return value; see <core/execthread.h> for details. 
			// AnonScope -> automatic vars
			int n_stack_vars;
			// Array of n_stack_vars variable types: 
			// NOTE: The order is the same as expected by the 
			// addressation indices. 
			ANI::ExprValueType *var_types;
			
			// This is a counter which is modified by the instances and 
			// counts the number of currently existing instances: 
			// For functions, counts the number of function calls, i.e. 
			// only _calls_ and no _returns_. 
			int instance_counter;
			
			_CPP_OPERATORS
			InstanceInfo();
			~InstanceInfo();
		};
		
		// Fill in basic elements: (used_at_top: number of elems used 
		// at beginning of stack frame)
		// vtype: 0 -> member (animation, setting, object)
		//        1 -> anon scope: auto vars
		//        2 -> function: auto vars, retval, this pointer
		void FillInInstanceInfo(InstanceInfo *iinfo,
			_AniComponentList<AniVariable> *varlist,int vtype,
			int used_at_beginning=0);
		
		// This returns a string representation of the function 
		// call passed in *lfi. Pass name of the function as first arg. 
		static String FuncCallWithArgsString(String func_name,
			LookupFunctionInfo *lfi);
		
	private:
		// Scope type: 
		AniScopeType astype;
		// Corresponding TreeNode: 
		ANI::TreeNode *treenode;  // TNAnimation, TNSetting, ...
		
		// This is ASType() for <ani-parser/coreglue.h>. 
		// [overriding virtual from AniGlue_ScopeBase.]
		AniScopeType CG_ASType() const   // inline for clarity
			{  return(astype);  }
		
		// [overriding virtuals from AniGlue_ScopeBase.]
		AniGlue_ScopeBase *CG_ReplaceAniIncomplete(
			AniGlue_ScopeBase *old_location,
			AniGlue_ScopeBase *ms_from,int ms_quality);
		void CG_DeleteNotify(ANI::TreeNode *tn);
	public:
		// Used by constructor to set up tree structure 
		// (parent = up pointer; already set; a _AniComponentList 
		// holds the children = down pointrs.)
		// do_queue: +1 -> queue; -1 -> dequeue. 
		// DO NOT USE IN OTHER PLACES THAN: 
		//   Constructor & destructor of derived classes. 
		virtual void QueueChild(AniScopeBase *asb,int do_queue=+1);
		
		// Do checks when all the registration is done. 
		// MUST BE OVERRIDDEN. 
		virtual AniScopeBase *RegisterTN_DoFinalChecks(
			ANI::TRegistrationInfo *tri);
		
	protected:
		// Name of this scope; without "::". 
		RefString name;
		// Parent pointer (or NULL): 
		AniScopeBase *parent;
		
		// Special functions used by RegisterTN(): 
		// Do the first check for registration and write error and 
		// return NULL if it fails. 
		ANI::TNIdentifierSimple *_RegisterCheckTN(ANI::TNIdentifier *id,
			AniScopeType wanted_scope,ANI::TRegistrationInfo *tri);
		// Check if this name is known here and by the immediate parent. 
		// Return value: 0 -> OK; 1 -> name already known; error written. 
		int _RegisterCheckNameKnown(ANI::TNIdentifierSimple *fid,
			ANI::TRegistrationInfo *tri);
		// Actually register object or setting: 
		AniScopeBase *_DoRegisterTN_ScopeOrSetting(ANI::TreeNode *tn,
			ANI::TRegistrationInfo *tri);
		// Actually register function: 
		AniScopeBase *_DoRegisterTN_Function(ANI::TNFunctionDef *tn,
			ANI::TRegistrationInfo *tri);
		// Register jump statement: 
		AniScopeBase *_DoRegisterTN_JumpStmt(ANI::TNJumpStmt *tn,
			ANI::TRegistrationInfo *tri);
		// Register anonymous scope from compound stmt: 
		AniScopeBase *_DoRegisterTN_CompoundStmt(ANI::TNCompoundStmt *tn,
			ANI::TRegistrationInfo *tri);
		AniScopeBase *_DoRegisterTN_DeclarationStmt(ANI::TNDeclarationStmt *tn,
			ANI::TRegistrationInfo *tri);
		AniScopeBase *_DoRegisterTN_AniDescStmt(ANI::TNAniDescStmt *ads,
			ANI::TRegistrationInfo *tri);
		// For few special operator functions, only: 
		AniScopeBase *_DoRegisterTN_OpFunc(ANI::TNOperatorFunction *tn,
			ANI::TRegistrationInfo *tri);
		// "Final" step: register all identifiers which were not yet caught.  
		AniScopeBase *_DoRegisterTN_Identifier(ANI::TNIdentifier *tn,
			ANI::TRegistrationInfo *tri);
		// Check for "reserved" words: 
		int _MatchReservedNames(const RefString &var_name,
			ANI::TNIdentifier *name_idf);
		// Check if idf begins with '$': 
		int CheckDollarSimpleIDF(ANI::TNIdentifierSimple *idf);
		
		// Internally used by _DoFixIncompleteType(): 
		// Get constant immediate value of expression (must be 
		// TNE_Value for that) and store it in ret_val. 
		// non_negative -> also error if the value turns out to be <0. 
		// short_val -> must fit into <short int>. 
		// Return value: 
		//  0 -> OK
		//  1 -> not an integer or not an immediate value
		int _ConstExprEval(ANI::TNExpression *expr,int *ret_val,
			int non_negative,int short_val);
		
		// Internally used by LookupIdentifierName(): 
		int _TopDownLookupIdentifierName(ANI::TNIdentifierSimple *start_ids,
			AniScopeBase *start_asb,AniScopeBase::ASB_List *ret_list,
			AniScopeBase::LookupExpectedType let,int write_errors,
			AniScopeBase **first_guess=NULL);
		
		// Check the passed structure and see if any of the named args 
		// was specified more than once. 
		// Return value: number of errors; error written. 
		int _CheckFuncArgRepetition(LookupFunctionInfo *lfi);
		
		// Lookup a function and write an error on failure. 
		// Return value NULL on failure. 
		AniFunction *_LookupFunction(ASB_List *lul,
			LookupFunctionInfo *lfi,RefString name_for_error,
			ANI::TNLocation name_loc);
		
		// Used by FixIncompleteTypes() to fix the entries in the 
		// typefix_list (see struct TypeFixEntry above) and 
		// thereby clearing the list. 
		// Returns number of errors. 
		int _FixEntriesInTypeFixList(LinkedList<TypeFixEntry> *list);
		
		// Forbidden: 
		AniScopeBase(const AniScopeBase &);
		AniScopeBase &operator=(const AniScopeBase &);
	public:   // CPP_OPERATORS in AniGlue_ScopeBase. 
		AniScopeBase(AniScopeType _t,ANI::TreeNode *_treenode,
			const RefString &_name,AniScopeBase *_parent);
		// Virtual destructor recursively deletes all children in the lists. 
		virtual ~AniScopeBase();
		
		// Just a wrapper for derived classes...
		inline AniScopeBase *asb()  {  return(this);  }
		
		// Return parent: 
		inline AniScopeBase *Parent()  {  return(parent);  }
		
		// Query the name of this scope (no "::"). 
		const RefString &GetName() const
			{  return(name);  }
		// Query type: 
		inline AniScopeType ASType() const
			{  return(astype);  }
		// Query associated tree node: 
		inline const ANI::TreeNode *GetTreeNode() const
			{  return(treenode);  }
		
		// This is used by LookupName() to see if the passed name matches 
		// the name of *this during lookup. 
		// Most classes use the default implementation. 
		virtual bool LookupMatchName(const RefString &name) const
			{  return(GetName()==name);  }
		
		// Get complete string name (with "::" separation): 
		String CompleteName() const;
		// Get location of the name. 
		virtual ANI::TNLocation NameLocation() const;
		
		// Recursively count scopes: 
		virtual int CountScopes(int &n_incomplete) const;
		
		// Helper: See if the passed ASB matches the passed lookup 
		// type; return 1 if yes and 0 if not. 
		int Match_LookupExpectedType(AniScopeBase *,LookupExpectedType letype);
		
		// Look up complete identifier name from the current 
		// AniScopeBase with passed (possibly scoped) name in 
		// TNIdentifier. Returns list of matching AniScopeBases 
		// found (the scopes are simply added to the ret_list). 
		// If write_errors=0, writing of error messages is suppressed. 
		// accumulate_results: 0 -> return first match; 1 -> accumulate all 
		//     matches while traversing the list up to the ani scope root. 
		// LookupExpectedType let: select which types of things to look for. 
		// Return value: 
		//   0 -> OK (ret_list guaranteed to be not empty)
		//   1 -> error; lookup failed
		// NOTE: For FUNCTIONS, correct overloading has to be applied...
		//       For VARIABLES, the var_seq_num check has to be applied...
		// ...by caller. 
		int LookupIdentifierName(ANI::TNIdentifier *idf,ASB_List *ret_list,
			AniScopeBase::LookupExpectedType let,
			int write_errors,int accumulate_results);
		
		// Search for the passed name which must not contain "::". 
		// name is interpreted relative to this scope. 
		// query_parent is the number of parent scopes to query 
		// if the name is not found immediately. Use 0 to NOT query 
		// any parents; use -1 to query them all. 
		// Returns NULL if nothing was found. 
		virtual void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		
		// Check for already used names. This is for variable names 
		// and not for other identifiers. 
		// Return value: 0 -> OK; 1 -> error and message written. 
		int CheckVarNameAlreadyUsed(const RefString &var_name,
			ANI::TNIdentifier *name_identifier);
		
		// Dump this tree, i.e. dump names & attributes recursively. 
		virtual void AniTreeDump(StringTreeDump *d);
		
		// Register TreeNode during TreeNode::DoRegistration() step. 
		// [overriding virtual from AniGlue_ScopeBase.]
		AniGlue_ScopeBase *RegisterTN(ANI::TreeNode *tn,
			ANI::TRegistrationInfo *tri);
		
		// This is essentially CompleteName(): 
		// [overriding virtual from AniGlue_ScopeBase.]
		String CG_GetName() const;
		
		// Get ExprValueType from declarator and type specifier: 
		// If ret_name is non-NULL, store name there. 
		// The associated identifier node is stored in *ret_idf 
		// if non-NULL. 
		// On error, a NULL ref is returned. 
		// Does not allow the name to be scoped (i.e. contain "::") 
		// and writes error in that case. 
		// is_automatic_var 1 -> auto var (as opposed to 
		// static/member vars) which get allocated by the thread "on 
		// the stack". 
		ANI::ExprValueType GetDeclarationType(ANI::TNTypeSpecifier *tspec,
			ANI::TNDeclarator *decl,int is_automatic_var,
			RefString *ret_name,ANI::TNIdentifier **ret_idf);
		
		// Helper: Get type as stored in TNTypeSpecifier. 
		// Return value: 
		//  0 -> OK
		// -2 -> AniScope lookup failure. 
		int GetBasicDeclType(ANI::TNTypeSpecifier *tspec,
			ANI::ExprValueType *evt);
		
		// Internally used by the following function: 
		// Returns number of errors encountered (1 or 0).
		// NOTE: decl may be NULL (e.g. for function return types)
		int _DoFixIncompleteType(ANI::ExprValueType *evt,AniScopeBase *scope,
			ANI::TNTypeSpecifier *tspec,ANI::TNDeclarator *decl);
		// Recursively do the incomplete type fixup...
		// Returns number of errors encountered.
		virtual int FixIncompleteTypes();
		// This is similar; has to retutn pointer to the typefix_list 
		// if the derived class has one. 
		virtual LinkedList<TypeFixEntry> *GetTypeFixList();
		
		// Recursively perform some cleanup depending on cstep. 
		// Return value: error counter. 
		// Currently: 
		//   cstep=1: 
		//     Clean up unused AniIncomplete; make sure the typefix_list 
		//     is empty  (done after TF step). 
		//   cstep=2:
		//     Create InstanceInfo entry for those classes which have 
		//     one. 
		virtual int PerformCleanup(int cstep);
		
		// Query InstanceInfo entry (if any): 
		// Must be overridden by those classes which have one; 
		// default implementation returns NULL. 
		virtual InstanceInfo *GetInstanceInfo();
		
		// Call this to initialize the non-auto static variables. 
		// This is done in level order; vars on the same level are 
		// constructed in the order they appear in the source code. 
		// Returns number of errors. 
		// Must be overridden by AniVariable and AniAniSetObj. 
		virtual int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};


// Derived classes: 
class AniAniSetObj;
class AniFunction;
class AniAnonScope;
class AniVariable;
class AniIncomplete;


// Internally used for lists of AniScopeBase-derived types. 
template<class Ani>struct _AniComponentList : public LinkedList<Ani>
{
	_AniComponentList() : LinkedList<Ani>()
		{  }
	~_AniComponentList()
		{  assert(LinkedList<Ani>::is_empty());  }
	
	// Find entry with specified name. 
	#warning "Need speedup for this lookup."
	void LookupName(const RefString &name,AniScopeBase::ASB_List *ret_list)
	{
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  if(i->LookupMatchName(name))  ret_list->append(i);  }
	}
	
	// Free all elements in list: 
	void ClearList()
	{
		while(!LinkedList<Ani>::is_empty())
		{  delete LinkedList<Ani>::first();  }   // NOT popfirst()!!! (destructor does removal)
	}
	
	// Write node dump: 
	void AniTreeDump(StringTreeDump *d)
	{
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  i->AniTreeDump(d);  }
	}
	
	// Promote the final checks call: 
	void RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri)
	{
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  i->RegisterTN_DoFinalChecks(tri);  }
	}
	
	// Promote CountScopes() call: 
	int CountScopes(int &n_incomplete) const
	{
		int n=0;  // 0 is correct. Only count elements, not *this. 
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  n+=i->CountScopes(n_incomplete);  }
		return(n);
	}
	
	// Promote yet another call...
	int FixIncompleteTypes()
	{
		int errors=0;
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  errors+=i->FixIncompleteTypes();  }
		return(errors);
	}
	//... and another...
	int PerformCleanup(int cstep)
	{
		int errors=0;
		for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
		{  errors+=i->PerformCleanup(cstep);  }
		return(errors);
	}
	// ...eh...
	int InitStaticVariables(ANI::ExecThreadInfo *info,int flag)
	{
		int errors=0;
		// Note that cleanup is done the other way round... 
		if(flag>0) for(Ani *i=LinkedList<Ani>::first(); i; i=i->next)
			{  errors+=i->InitStaticVariables(info,flag);  }
		else if(flag<0) for(Ani *i=LinkedList<Ani>::last(); i; i=i->prev)
			{  errors+=i->InitStaticVariables(info,flag);  }
		else ++errors;
		return(errors);
	}
};

#endif  /* _ANIVISION_CORE_SCOPEBASE_H_ */
