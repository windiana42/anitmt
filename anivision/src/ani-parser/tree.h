/*
 * ani-parser/tree.h
 * 
 * ANI tree class definitions. 
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

#ifndef _ANIVISION_ANI_TREE_H_
#define _ANIVISION_ANI_TREE_H_ 1

#include <hlib/refstring.h>
//#include <valtype/valtypes.h>

#include <ani-parser/treenode.h>
#include <ani-parser/location.h>

#include <ani-parser/exprvalue.h>

#include <ani-parser/opfunc.h>


namespace ANI
{

class TNIdentifierSimple : public TreeNode
{
	// NO CHILDREN. 
	public:
		static const TNType tn_type=TN_IdentifierSimple;
	private:
		TNLocation loc;   // Location in source code. 
	public:
		RefString name;   // Name (e.g. "MyObject"; NULL ref for ::...). 
	public:
		TNIdentifierSimple(const TNLocation &_loc,const RefString &n) : 
			TreeNode(TN_IdentifierSimple),loc(_loc),name(n)
			{  }
		~TNIdentifierSimple();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
};


class TNIdentifier : public TreeNode
{
	// CHILDREN: All of type TNIdentifierSimple for A::B::C. 
	//       BUT IN REVERSE ORDER!
	public:
		static const TNType tn_type=TN_Identifier;
	private:
	public:
		// Associated AniAniSetObj, AniVariable, ...
		// For identifiers in expressions, this is an AniIncomplete 
		// at registration step and gets resolved in the TF step. 
		AniGlue_ScopeBase *ani_scope;
		// Set In TF step: Provides function how to Eval() this 
		// identifier. Provided by core, see coreglue.h. 
		// Must be free'd by this class; managed by associated 
		// AniGlue_ScopeBase. 
		AniGlue_ScopeBase::EvalAddressationFunc *evaladrfunc;
	public:
		TNIdentifier(TNIdentifierSimple *n0=NULL) : TreeNode(TN_Identifier)
			{  ani_scope=NULL;  evaladrfunc=NULL;  if(n0) AddChild(n0);  }
		~TNIdentifier();
		
		// Get the complete identifier name ("A::B::C"): 
		RefString CompleteStr();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNExpression : public TreeNode
{
	// CHILDREN: TNExpression, TNIdentifier, 
	// AND: TNValue, TNOperatorFunction, TNFCallArgument 
	//      (i.e. all nodes derived from TNExpression). 
	public:
		static const TNType tn_type=TN_Expression;
	private:
		TNEType expr_type;  // !=TNE_Expression for derived types. 
	public:
		TNExpression(TNEType et) : TreeNode(TN_Expression),expr_type(et)
			{  }
		TNExpression(TNIdentifier *tn) : 
			TreeNode(TN_Expression),expr_type(TNE_Expression)
			{  if(tn) AddChild(tn);  }
		//TNExpression(TNExpression *tn) : 
		//	TreeNode(TN_Expression),expr_type(TNE_Expression)
		//	{  AddChild(tn);  }*/
		virtual ~TNExpression();
		
		// Get expression node type: 
		TNEType ExprType() const
			{  return(expr_type);  }
		
		// [overriding virtuals]
		virtual void DumpTree(StringTreeDump *d);
		virtual TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		virtual TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		virtual void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		virtual int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNExpressionList : public TreeNode
{
	// CHILDREN: TNExpression ONLY. 
	public:
		static const TNType tn_type=TN_ExpressionList;
	private:
	public:
		TNExpressionList(TNExpression *exp0=NULL) : 
			TreeNode(TN_ExpressionList)
			{  if(exp0) AddChild(exp0);  }
		~TNExpressionList();
		
		// This is essentially AddChild() but allows for 
		// compiler checks of the type: 
		void AddExpression(TNExpression *expr)  {  AddChild(expr);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use default. 
		//TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNValue : public TNExpression
{
	// NO CHILDREN. 
	public:
		static const TNEType tne_type=TNE_Value;
	private:
		TNLocation loc;   // Location in source code. 
	public:
		ExprValue val;        // Stored value. 
	public:
		TNValue(const TNLocation &_loc,const Value &v) : 
			TNExpression(TNE_Value),loc(_loc),val(ExprValue::Copy,v)
			{  }
		TNValue(const TNLocation &_loc,const ExprValue &v) : 
			TNExpression(TNE_Value),loc(_loc),val(ExprValue::Copy,v)
			{  }
		~TNValue();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// E.g. the {}-thingy in points={1,2,3}...
class TNArray : public TNExpression
{
	// CHILDREN: Array entries: TNExpression, only. 
	public:
		static const TNEType tne_type=TNE_Array;
	private:
		TNLocation loc0;   // Location of list start
		TNLocation loc1;   // Location of list end
	public:
		// Extra commas at start and end?
		int extra_comma_start : 1;
		int extra_comma_end : 1;
		// Set at TF step for eval: 
		ExprValueType elem_type;
		ssize_t nelem;  // without extra comma at start and end
		// Assignment functions for array elements; NULL for those 
		// elements where direct assignment is possibke. 
		// Set in TF step. 
		AssignmentFuncBase **assfunc;  // array [nelem]
		
		// Called by constructor to delete **assfunc. 
		void _DeleteAssFunc();
	public:
		TNArray(TNExpression *exp0=NULL) : TNExpression(TNE_Array),
			loc0(TNLocation::NullLoc),loc1(TNLocation::NullLoc),
			elem_type(ExprValueType::CUnknownType),nelem(-1)
			{  extra_comma_start=0;  extra_comma_end=0;  assfunc=NULL;
				if(exp0) AddChild(exp0);  }
		~TNArray();
		
		// Set extra commas: 
		void SetExtraComma(bool A,bool B)
			{  extra_comma_start=A;  extra_comma_end=B;  }
		// Set the location of the braces: 
		void SetLocation(const TNLocation &_loc0,const TNLocation &_loc1)
			{  loc0=_loc0;  loc1=_loc1;  }
		
		// This is essentially AddChild() but allows for 
		// compiler checks of the type: 
		void AddExpressionL(TNExpression *expr)  {  AddChild(expr);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNTypeSpecifier : public TreeNode
{
	// CHILDREN: No children; exceptons: 
	//  EVT_POD: 
	//    Value::VTVector: 0 or 1 TNExpression 
	//    Value::VTMatrix: 0 or 2 TNExpression 
	//  EVT_AniScope:
	//    1 TNIdentifier, exact type not yet known. 
	//  EVT_Array:  <-- for array return types of function
	//    1 TNTypeSpecifier
	//    TNTypeSpecifier with EVT_Array is always of unspecified size. 
	public:
		static const TNType tn_type=TN_TypeSpecifier;
		enum _VoidType { VoidType };
	private:
		TNLocation loc;      // Location in source code. 
	public:
		ExprValueTypeID ev_type_id;
		Value::VType vtype;  // for EVT_POD, else VTNone
	public:
		TNTypeSpecifier(const TNLocation &_loc,TNIdentifier *id) : 
			TreeNode(TN_TypeSpecifier),loc(_loc),ev_type_id(EVT_AniScope),
			vtype(Value::VTNone)
			{  if(id) AddChild(id);  }
		TNTypeSpecifier(const TNLocation &_loc,Value::VType vt) : 
			TreeNode(TN_TypeSpecifier),loc(_loc),ev_type_id(EVT_POD),
			vtype(vt)
			{  }
		TNTypeSpecifier(const TNLocation &_loc,_VoidType) : 
			TreeNode(TN_TypeSpecifier),loc(_loc),ev_type_id(EVT_Void),
			vtype(Value::VTNone)
			{  }
		TNTypeSpecifier(const TNLocation &_loc,TNTypeSpecifier *array_type) : 
			TreeNode(TN_TypeSpecifier),loc(_loc),ev_type_id(EVT_Array),
			vtype(Value::VTNone)
			{  if(array_type) AddChild(array_type);  }
		~TNTypeSpecifier();
		
		// This is essentially AddChild() but allows for 
		// compiler checks of the type: 
		void AddExpressionT(TNExpression *expr)  {  AddChild(expr);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
};


class TNOperatorFunction : public TNExpression
{
	friend class TNExpressionStmt;
	
	// CHILDREN: Operator arguments, normally all of type TNExpression. 
	// Exceptions: OF_PODCast: first arg = TNTypeSpecifier
	//             OF_MembSel: second arg = TNIdentifier
	//             OF_New,OF_NewArray,...etc
	public:
		static const TNEType tne_type=TNE_OperatorFunction;
		
		// Evaluation "function": 
		// FIXME: Could hash + ref count these EvalFunctions to save memory. 
		struct EvalFunc
		{
			_CPP_OPERATORS
			EvalFunc() {}
			virtual ~EvalFunc();
			// Normal eval func, always called. 
			virtual int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
			// Delayed eval func. Only called if the normal eval 
			// func queues this one for delayed evaluation. 
			// Only used for post inc/decrement (and maybe destructor 
			// calls). 
			virtual int DelayedEval(ExprValue &val,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// Derived types: 
		// This is for the default operator functions as in opfunc.cc: 
		struct EvalFunc_DefaultOpFunc : EvalFunc
		{
			int nargs;
			OpFunc_Ptr fptr;
			
			EvalFunc_DefaultOpFunc(int _nargs,OpFunc_Ptr _fptr) : EvalFunc()
				{  nargs=_nargs;  fptr=_fptr;  }
			// [overriding virtuals]
			~EvalFunc_DefaultOpFunc();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// POD cast. Much like default. 
		struct EvalFunc_PODCast : EvalFunc
		{
			ExprValue proto_type;  // cast destination type
			OpFunc_Ptr fptr;
			
			EvalFunc_PODCast(const ExprValue &_proto,OpFunc_Ptr _fptr) : 
				EvalFunc(),proto_type(_proto)  {  fptr=_fptr;  }
			// [overriding virtuals]
			~EvalFunc_PODCast();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// These are for subscript operation. 
		struct EvalFunc_SubscriptArray : EvalFunc
		{
			EvalFunc_SubscriptArray() : EvalFunc()
				{  }
			// [overriding virtuals]
			~EvalFunc_SubscriptArray();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_SubscriptPOD : EvalFunc
		{
			// If fixed_idx>=0, use it, otherwise evaluate 
			// tn->down.last(). 
			ssize_t fixed_idx;
			// POD subscription type: 
			enum ExprValue::_IndexPOD index_type;
			
			EvalFunc_SubscriptPOD(ExprValue::_IndexPOD _index_type,
				ssize_t _fixed_idx=-1) : EvalFunc()
				{  fixed_idx=_fixed_idx;  index_type=_index_type;  }
			// [overriding virtuals]
			~EvalFunc_SubscriptPOD();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// Evaulation of the "?:" operators: 
		struct EvalFunc_IfElse : EvalFunc
		{
			EvalFunc_IfElse() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_IfElse();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_IfElse2 : EvalFunc
		{
			EvalFunc_IfElse2() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_IfElse2();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// Evaluation of "&&" and "||": 
		struct EvalFunc_LogicalAnd : EvalFunc
		{
			EvalFunc_LogicalAnd() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_LogicalAnd();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_LogicalOr : EvalFunc
		{
			EvalFunc_LogicalOr() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_LogicalOr();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// For member select: Required info stored in down.last() 
		//  (which is TNIdentifer and has an ealfunc). 
		struct EvalFunc_MemberSelect : EvalFunc
		{
			EvalFunc_MemberSelect() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_MemberSelect();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// And here come some special "member select" functions 
		// (which have ms_is_subscript=1). 
		struct EvalFunc_PODLength : EvalFunc
		{
			Value::VType ptype;
			
			EvalFunc_PODLength(Value::VType _ptype) : EvalFunc()
				{  ptype=_ptype;  }
			// [overriding virtuals]
			~EvalFunc_PODLength();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_PODSize : EvalFunc
		{
			int magic;  // 'd', 'r', 'c'  (vector dim, rows, cols)
			
			EvalFunc_PODSize(int _magic) : EvalFunc()  {  magic=_magic; }
			// [overriding virtuals]
			~EvalFunc_PODSize();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_ArraySize : EvalFunc
		{
			EvalFunc_ArraySize() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_ArraySize();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// For normal (non-array) operator new: 
		// Allocates the object but does not call the constructor. 
		// Normally attached to the TNIdentifier which holds the type name. 
		struct EvalFunc_OpNew : EvalFunc
		{
			AniGlue_ScopeBase *obj_type;
			
			EvalFunc_OpNew(AniGlue_ScopeBase *_obj_type) : EvalFunc()
				{  obj_type=_obj_type;  }
			// [overriding virtuals]
			~EvalFunc_OpNew();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// This is the handler for new array; pretty trivial because 
		// the work is done by the NewArraySpecifier. 
		struct EvalFunc_NewArray : EvalFunc
		{
			EvalFunc_NewArray() : EvalFunc()  { }
			// [overriding virtuals]
			~EvalFunc_NewArray();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// This is actually attached to TNNewArraySpecifier: 
		// Bottom most new array handler; may need to call constructor. 
		struct EvalFunc_NewArrayBottom : EvalFunc
		{
			ExprValueType elem_type;
			
			EvalFunc_NewArrayBottom(const ExprValueType &_elem_type) : 
				EvalFunc(),elem_type(_elem_type)  { }
			// [overriding virtuals]
			~EvalFunc_NewArrayBottom();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// This is actually attached to TNNewArraySpecifier: 
		// New array handler above of EvalFunc_NewArrayBottom; only creates 
		// arrays of types. 
		struct EvalFunc_NewArrayArray : EvalFunc
		{
			ExprValueType elem_type;
			
			EvalFunc_NewArrayArray(const ExprValueType &_elem_type) : 
				EvalFunc(),elem_type(_elem_type)  { }
			// [overriding virtuals]
			~EvalFunc_NewArrayArray();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// Finally, the function for the pre/post increment/decrement. 
		struct EvalFunc_PreIncDec : EvalFunc
		{
			IncDecAssFuncBase *ida_func;
			
			EvalFunc_PreIncDec(IncDecAssFuncBase *_ida_func) : 
				EvalFunc()  {  ida_func=_ida_func;  }
			// [overriding virtuals]
			~EvalFunc_PreIncDec();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		struct EvalFunc_PostIncDec : EvalFunc
		{
			IncDecAssFuncBase *ida_func;
			
			EvalFunc_PostIncDec(IncDecAssFuncBase *_ida_func) : 
				EvalFunc()  {  ida_func=_ida_func;  }
			// [overriding virtuals]
			~EvalFunc_PostIncDec();
			int Eval(ExprValue &retval,TNOperatorFunction *tn,
				ExecThreadInfo *info);
			int DelayedEval(ExprValue &val,TNOperatorFunction *tn,
				ExecThreadInfo *info);
		};
		// Well... and where's the King? (I.e. the function call eval 
		// handler?) -> It is provided by the core, see a_userfunc.cc. 
	private:
		OperatorFuncID func_id : 31;
		// 1 -> If this is a MembSel which is actually a subscript. 
		//      Set in TF step. 
		int ms_is_subscript : 1;
		TNLocation loc0;   // location of operator
		TNLocation loc1;   // location of second operator if present
		                   // (e.g. for <,,>, [..], .?.:.)
	public:
		// This is the TNTypeSpecifier type for OF_NewArray. 
		// Set in registration step. 
		ExprValueType new_type;
		// This is set during TF step: 
		// Apropriate evaulation function: 
		struct EvalFunc *evalfunc;  // Allocated via new. 
	public:
		// Construct operator function with ID and location; add 
		// arguments using AddOperand(). 
		TNOperatorFunction(OperatorFuncID fid,const TNLocation &l) : 
			TNExpression(TNE_OperatorFunction),
			func_id(fid),loc0(l),loc1(TNLocation::NullLoc),new_type(),
			evalfunc(NULL)
			{  ms_is_subscript=0;  }
		TNOperatorFunction(OperatorFuncID fid,const TNLocation &l0,
			const TNLocation &l1) : 
			TNExpression(TNE_OperatorFunction),
			func_id(fid),loc0(l0),loc1(l1),evalfunc(NULL)
			{  ms_is_subscript=0;  }
		// Special constructor for the case " Exp OP Exp ": 
		TNOperatorFunction(OperatorFuncID fid,const TNLocation &l,
			TNExpression *expA,TNExpression *expB) : 
			TNExpression(TNE_OperatorFunction),
			func_id(fid),loc0(l),loc1(TNLocation::NullLoc),new_type(),
			evalfunc(NULL)
			{  ms_is_subscript=0;  AddChild(expA);  AddChild(expB);  }
		// Special constructor for the case " Exp OP " or " Op Exp "
		TNOperatorFunction(OperatorFuncID fid,const TNLocation &l,
			TNExpression *exp) : 
			TNExpression(TNE_OperatorFunction),
			func_id(fid),loc0(l),loc1(TNLocation::NullLoc),new_type(),
			evalfunc(NULL)
			{  ms_is_subscript=0;  AddChild(exp);  }
		// Destructor: Make sure to free the evaluation stuff: 
		~TNOperatorFunction();
		
		// Get the func ID: 
		OperatorFuncID GetFuncID() const
			{  return(func_id);  }
		
		// This is essentially AddChild() but allows for 
		// compiler checks of the type: 
		void AddOperator(TNExpression *op)  {  AddChild(op);  }
		void AddOperator(TNIdentifier *op)  {  AddChild(op);  }
		void AddOperator(TNTypeSpecifier *op)  {  AddChild(op);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNFCallArgument : public TNExpression
{
	// CHILDREN: if 1 child: -> TNExpression (function arg)
	//           if 2 children -> TNIdentifier '=' TNExpression 
	//           (function call: argument name assignment)
	// (Especially note that the TNExpression can be a TNArray.) 
	public:
		static const TNEType tne_type=TNE_FCallArgument;
	private:
		TNLocation loc;   // Location of the '=' if there is one. 
	public:
		TNFCallArgument(TNExpression *val) : 
			TNExpression(TNE_FCallArgument),loc(TNLocation::NullLoc)
			{  if(val) AddChild(val);  }
		TNFCallArgument(const TNLocation &_loc,TNIdentifier *name,
			TNExpression *val) : 
			TNExpression(TNE_FCallArgument),loc(_loc)
			{  AddChild(name);  AddChild(val);  }
		~TNFCallArgument();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use defaults. 
		//TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNAssignment : public TNExpression
{
	// CHILDREN: In this order: 
	//    TNExpression (lvalue), TNExpression (rvalue)
	public:
		static const TNEType tne_type=TNE_Assignment;
	private:
		TNLocation loc;   // Location of the assignment OP. 
		AssignmentOpID ass_op;
		// Assignment function as set up in TF step: 
		AssignmentFuncBase *assfunc;
	public:
		TNAssignment(AssignmentOpID aop,const TNLocation &_loc,
			TNExpression *lval,TNExpression *rval) : 
			TNExpression(TNE_Assignment),loc(_loc),ass_op(aop)
			{ assfunc=NULL; if(lval) AddChild(lval); if(rval) AddChild(rval); }
		~TNAssignment();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use defaults. 
		//TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNStatement : public TreeNode
{
	// CHILDREN: TN???
	// AND: All nodes derived from TNStatement, e.g. 
	//      TNIterationStmt, TNSelectionStatement 
	public:
		static const TNType tn_type=TN_Statement;
		// Status as returned by Exec(). 
		enum ExecRStatus
		{
			ERS_Cont=0,  // MUST be 0, normal return value
			ERS_ContextSwitch=1,  // save state for context switch; should be 1. 
			ERS_Return,   // function return
			ERS_Break,    // break statement
		};
	private:
		TNSType stmt_type;     // !=TNS_Statement for derived types. 
	protected:
		// This is not set for iteration and selection statements and 
		// not for the "for" loop "increment" statement (as it has no ";"). 
		TNLocation Sloc;       // location of the ';'. 
	public:
		// Pass the location of the ';' as _loc. 
		TNStatement(TNSType st,const TNLocation &_loc) : 
			TreeNode(TN_Statement),stmt_type(st),Sloc(_loc)
			{  }
		virtual ~TNStatement();
		
		// Get statement node type: 
		TNSType StmtType() const
			{  return(stmt_type);  }
		
		// [overriding virtuals]
		virtual void DumpTree(StringTreeDump *d);
		virtual TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		virtual TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		virtual void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		virtual int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// Execute statement: 
		// Must be overridden by all derived classes. 
		virtual ExecRStatus Exec(ExecThreadInfo *info);
		virtual int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// NOTE: Expression statement can be: 
//        -> expression, assignment, empty statement
class TNExpressionStmt : public TNStatement
{
	// CHILD: TNExpression, TNAssignment, <no child> = empty statement
	// ONLY ONE CHILD MAX. Note that TNAssignment is TNExpression-derived. 
	public:
		static const TNSType tns_type=TNS_ExpressionStmt;
	private:
	public:
		// Create expression statement or empty statement: 
		// _Sloc: location of the ';'. 
		TNExpressionStmt(const TNLocation &_Sloc,TNExpression *expr=NULL) : 
			TNStatement(TNS_ExpressionStmt,_Sloc)
		{  if(expr) AddChild(expr);  }
		// Create assignment statement: 
		TNExpressionStmt(const TNLocation &_Sloc,TNAssignment *ass) : 
			TNStatement(TNS_ExpressionStmt,_Sloc)
		{  AddChild(ass);  }
		~TNExpressionStmt();
		
		// Special function used when building the tree: 
		// Remove the (first) child from down list and return it: 
		TNExpression *ExportFirstChild()
			{  TreeNode *tn=down.first();  if(tn) RemoveChild(tn);
				return((TNExpression*)tn);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use defaults: 
		//TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNIterationStmt : public TNStatement
{
	// CHILDREN: There are children (for correct tree cleanup), 
	//           but do not access them!
	//  Everything which needs to be known is in the extra 
	//  pointers loop_cond, etc,
	public:
		static const TNSType tns_type=TNS_IterationStmt;
		enum IterationType
		{
			IT_None=0,  // may not happen
			IT_For,
			IT_While,
			IT_DoWhile
		};
	private:
		TNLocation loc0;   // Location of the for,while,do. 
		TNLocation loc1;   // location of the "while" in "do..while"
		IterationType itertype;
		// Children (also queued under LinkedList down.): 
		TNStatement *loop_stmt;     // "loop body" (not NULL)
		TNExpression *loop_cond;    // "loop condition" (NULL if "error" or empty)
		// These are only for "for" loops: 
		TNStatement *for_init_stmt;  // not NULL for "for()"
		TNStatement *for_inc_stmt;   // not NULL for "for()"
		
		inline void _construct(TNStatement *_l_stmt,TNExpression *_l_cond)
		{
			loop_stmt=_l_stmt;  if(loop_stmt) AddChild(loop_stmt);
			loop_cond=_l_cond;  if(loop_cond) AddChild(loop_cond);
			for_init_stmt=NULL;  for_inc_stmt=NULL;
		}
	public:
		TNIterationStmt(IterationType it,const TNLocation &_loc0,
			TNStatement *_l_stmt=NULL,TNExpression *_l_cond=NULL) : 
			TNStatement(TNS_IterationStmt,TNLocation()),loc0(_loc0),
			loc1(TNLocation::NullLoc),itertype(it)
			{  _construct(_l_stmt,_l_cond);  }
		TNIterationStmt(IterationType it,const TNLocation &_loc0,
			const TNLocation &_loc1,TNStatement *_l_stmt=NULL,
			TNExpression *_l_cond=NULL) : 
			TNStatement(TNS_IterationStmt,TNLocation()),loc0(_loc0),
			loc1(_loc1),itertype(it)
			{  _construct(_l_stmt,_l_cond);  }
		~TNIterationStmt();
		
		// Set the other statements in for loops: 
		// ONLY CALL ONCE AFTER CONSTRUCTION. 
		void SetForLoopStmts(TNStatement *_init,TNStatement *_inc)
			{  for_init_stmt=_init;  if(_init) AddChild(_init);
				for_inc_stmt=_inc;   if(_inc) AddChild(_inc);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNSelectionStmt : public TNStatement
{
	// CHILDREN: 1, 2 or 3: but do not access them!
	//  Everything which needs to be known is in the extra 
	//  pointers cond_expr, if_stmt, else_stmt. 
	public:
		static const TNSType tns_type=TNS_SelectionStmt;
	private:
		TNLocation loc0;   // of "if"
		TNLocation loc1;   // of "else" if any
		TNExpression *cond_expr;  // NULL or "if(error)"
		TNStatement *if_stmt;
		TNStatement *else_stmt;   // NULL if no "else" branch
		
		inline void _construct(TNExpression *e,TNStatement *s0,
			TNStatement *s1)
		{	cond_expr=e;   if(cond_expr) AddChild(cond_expr);
			if_stmt=s0;    if(if_stmt) AddChild(if_stmt);
			else_stmt=s1;  if(else_stmt) AddChild(else_stmt);  }
	public:
		// Create if(..).. selection statement: 
		TNSelectionStmt(const TNLocation &_loc0,TNExpression *e,TNStatement *s) : 
			TNStatement(TNS_SelectionStmt,TNLocation()),loc0(_loc0),
			loc1(TNLocation::NullLoc)
			{  _construct(e,s,NULL);  }
		// Create if(..)..else.. selection statement: 
		TNSelectionStmt(const TNLocation &_loc0,TNExpression *e,
			TNStatement *s0,const TNLocation &_loc1,TNStatement *s1) : 
			TNStatement(TNS_SelectionStmt,TNLocation()),loc0(_loc0),loc1(_loc1)
			{  _construct(e,s0,s1);  }
		~TNSelectionStmt();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNJumpStmt : public TNStatement
{
	// CHILDREN: depending on JumpType. 
	//    Break: no child
	//    Return: one TNExpression or NO child (for "return;")
	public:
		static const TNSType tns_type=TNS_JumpStmt;
		enum JumpType
		{
			JT_None=0,  // may not happen
			JT_Break,
			JT_Return,
			_JT_LAST
		};
		// Evaluation "function"...
		struct EvalFunc
		{
			_CPP_OPERATORS
			EvalFunc() {}
			virtual ~EvalFunc();
			virtual int Eval(TNJumpStmt *tn,ExecThreadInfo *info);
		};
		// Core provides eval funcs. 
	private:
		TNLocation loc;   // of the statement
	public:
		JumpType jumptype;
		// Associated ani scope: 
		// JT_Return -> associated AniAnonScope which contains 
		//              the return stmt; the AniFunction must be 
		//              somewhere up in the hierarchy. 
		// JT_Break -> NULL (may be changed, BUT THEN CHECK TNJumpStmt::DoRegistration())
		AniGlue_ScopeBase *in_scope;
		// Evaluation function, NULL for "break;". 
		EvalFunc *evalfunc;
	public:
		// _Sloc: location of the ';'. 
		TNJumpStmt(const TNLocation &_loc,JumpType jt,const TNLocation &_Sloc,
			TNExpression *exp=NULL) : TNStatement(TNS_JumpStmt,_Sloc),
			loc(_loc),jumptype(jt)
			{  evalfunc=NULL;  if(exp)  AddChild(exp);  in_scope=NULL;  }
		~TNJumpStmt();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNCompoundStmt : public TNStatement
{
	// CHILDREN: any number of TNStatement's 
	//        But also: POV nodes; at least temporatily. 
	public:
		static const TNSType tns_type=TNS_CompoundStmt;
	private:
		TNLocation loc0;   // beginning of compount
		TNLocation loc1;   // end of compount
	public:
		// Associated anonymous scope; set up by DoRegistration(). 
		AniAnonScope *anonscope;
	public:
		TNCompoundStmt(const TNLocation &_loc0,const TNLocation &_loc1) : 
			TNStatement(TNS_CompoundStmt,_loc1),loc0(_loc0),loc1(_loc1)
			{  anonscope=NULL;  }
		~TNCompoundStmt();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// In int x[2][]={...}; the declarator is "x[2][]={...}". 
// The tree layout for this example is: 
//   |  DT_Initialize
//   |  DT_Array
//   |  DT_Array
//   V  DT_Name
class TNDeclarator : public TreeNode
{
	// CHILDREN: One or two. 
	//   DT_Name   -> child: TNIdentifier
	//   DT_Array: -> children (in this order): TNDeclarator, TNExpression
	//                where TNExpression is the array size and may 
	//                be left away for error OR cases like "int x[]={...};"
	//   DT_Initialize -> children (in this order): TNDeclarator, TNExpression
	//                where TNExpression is the initializer (the "{...}"). 
	public:
		static const TNType tn_type=TN_TypeSpecifier;
		
		// Evaluation "function": 
		struct EvalFunc
		{
			_CPP_OPERATORS
			EvalFunc() {}
			virtual ~EvalFunc();
			virtual int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		// Derived types: Most provided by core. 
		// NoOp eval func: does nothing. 
		struct EvalFunc_NoOp : EvalFunc
		{
			// We do external reference counting on this one: 
			int refcnt;
			
			EvalFunc_NoOp();
			// [overriding virtuals]
			~EvalFunc_NoOp();
			int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		// Declaration functions: 
		// This is for static variables WITH INITIALIZER which are 
		// not yet initialized. Once Eval() is called, the evalfunc 
		// is replaced with the no-op eval function. 
		struct EvalFunc_Static : TNDeclarator::EvalFunc
		{
			// Pointer to the name node: 
			TNIdentifier *name;
			
			EvalFunc_Static(TNIdentifier *_name) : EvalFunc()
				{  name=_name;  }
			// [overriding virtuals]
			~EvalFunc_Static();
			int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		// For non-static automatic variables WITH INITIALIZER ONLY: 
		struct EvalFunc_Normal : TNDeclarator::EvalFunc
		{
			// Pointer to the name node: 
			TNIdentifier *name;
			
			EvalFunc_Normal(TNIdentifier *_name) : EvalFunc()
				{  name=_name;  }
			// [overriding virtuals]
			~EvalFunc_Normal();
			int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		// Special version setting prototype value. 
		struct EvalFunc_ProtoStatic : TNDeclarator::EvalFunc
		{
			// Pointer to the name node: 
			TNIdentifier *name;
			ExprValueType type;
			
			EvalFunc_ProtoStatic(TNIdentifier *_name,
				const ExprValueType &_type) : EvalFunc(),type(_type)
				{  name=_name;  }
			// [overriding virtuals]
			~EvalFunc_ProtoStatic();
			int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		struct EvalFunc_ProtoNormal : TNDeclarator::EvalFunc
		{
			// Pointer to the name node: 
			TNIdentifier *name;
			ExprValueType type;
			
			EvalFunc_ProtoNormal(TNIdentifier *_name,
				const ExprValueType &_type) : EvalFunc(),type(_type)
				{  name=_name;  }
			// [overriding virtuals]
			~EvalFunc_ProtoNormal();
			int Eval(TNDeclarator *tn,ExecThreadInfo *info);
		};
		
		// Static no-op evaluator: 
		static EvalFunc_NoOp *evalfunc_noop;
		// Use this if you need to allocate a NoOp eval func: 
		static EvalFunc *GetNoOpEvalFunc();
		
		enum DeclType
		{
			DT_None=0,   // may not happen
			DT_Name,     // name declarator (lowest decl level in tree)
			DT_Array,    // array declarator
			DT_Initialize,  // initialize (=assign value)
		};
	private:
		DeclType decltype;
	public:
		// Evaluation function; set during type fixup. 
		EvalFunc *evalfunc;
		// Assignment function (used by evalfunc) as set up in TF step. 
		// This stays NULL if declarator has no initializer (because 
		// prototype value can be set directly). 
		AssignmentFuncBase *assfunc;
		
		// Called by destructor and before you replace the eval func: 
		void _RemoveEvalFunc();
	public:
		// Construct DT_Name declarator: 
		TNDeclarator(TNIdentifier *id) : TreeNode(TN_Declarator),
			decltype(DT_Name)
			{  assfunc=NULL;  evalfunc=NULL;  if(id) AddChild(id);  }
		// Construct DT_Array or DT_Initialize declarator: 
		TNDeclarator(TNDeclarator *dec,DeclType dt,TNExpression *expr=NULL) : 
			TreeNode(TN_Declarator),decltype(dt)
			{  assfunc=NULL;  evalfunc=NULL;  AddChild(dec);
				if(expr) AddChild(expr);  }
		~TNDeclarator();
		
		DeclType GetDeclType() const
			{  return(decltype);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// Used for OF_NewArray, only (currently). 
class TNNewArraySpecifier : public TreeNode
{
	// CHILDREN: Zero or one. 
	//   0 children -> "[]" part of array spec
	//   1 child -> TNExpression; "[expr]" part of array spec. 
	private:
		TNLocation loc;  // location range from "[" to "]". 
	public:
		// One of EvalFunc_NewArrayBottom, EvalFunc_NewArrayArray; 
		// set in TF step. Will be NULL if no child. 
		TNOperatorFunction::EvalFunc *evalfunc;
	public:
		TNNewArraySpecifier(const TNLocation &_loc,TNExpression *expr=NULL) : 
			TreeNode(TN_NewArraySpecifier),loc(_loc)
			{  evalfunc=NULL;  if(expr)  AddChild(expr);  }
		~TNNewArraySpecifier();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


class TNDeclarationStmt : public TNStatement
{
	// CHILDREN: 
	//   FIRST: TNTypeSpecifier: type specifier. 
	//   All other: TNDeclarator; a list of declarations to be made. 
	public:
		static const TNSType tns_type=TNS_DeclarationStmt;
	public:
		TNLocation aloc;  // location of the attributes
		enum
		{
			IS_Const=    0x0001,  // set if "const" was specified
			IS_Static=   0x0002,  // set if "static" was specified
		};
		int attrib;  // Bitwise OR of the IS_* flags above. 
	public:
		// _Sloc: location of the ';'. 
		TNDeclarationStmt(TNTypeSpecifier *type_spec,int _attrib,
			const TNLocation &_Sloc,const TNLocation &_aloc) : 
			TNStatement(TNS_DeclarationStmt,_Sloc),aloc(_aloc)
			{  attrib=_attrib;  if(type_spec) AddChild(type_spec);  }
		~TNDeclarationStmt();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// This is the complete "%name{...};" ani statement; the "{...}" block 
// is an TN_AniDesc provided by calccore. 
// Can also be "%{...};" or "%core:{...};" or "%core:name{...};". 
class TNAniDescStmt : public TNStatement
{
	// CHILDREN: 1, 2 or 3: but do not access them directly!
	//  Everything which needs to be known about the first children 
	//  is in the extra pointers core_name, name_expr, adb. 
	public:
		static const TNSType tns_type=TNS_AniDescStmt;
	public:
		TNLocation loc0;  // ... of the "%".
		//TNLocation Sloc; (in TNStatement)  // ... of the ";" at the end.
		TNIdentifier *core_name;   // calc core name
		TNExpression *name_expr;   // object/value name
		TreeNode *adb_child;       // ani desc block
		// This is filled in in the registration step: 
		void *cc_factory;  // pointer to the calc core factory to 
		                   // use (not allocated; type CC::CCIF_Factory)
	public:
		// _Sloc: location of the ';'. 
		TNAniDescStmt(const TNLocation &_loc0,const TNLocation &_Sloc,
			TreeNode *ani_desc_block,TNExpression *_name_expr=NULL,
			TNIdentifier *_core_name=NULL) : 
			TNStatement(TNS_AniDescStmt,_Sloc),loc0(_loc0)
			{
				if(core_name=_core_name)  AddChild(core_name);
				if(name_expr=_name_expr)  AddChild(name_expr);
				if(adb_child=ani_desc_block)  AddChild(adb_child);
				cc_factory=NULL;
			}
		~TNAniDescStmt();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual from TNStatement]
		ExecRStatus Exec(ExecThreadInfo *info);
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// This is a function/method definition argument specifier. 
// In "int foo(vector<3> pts[]={a,b,c})" this is 
//   "vector<3> pts[]={a,b,c}". 
class TNFuncDefArgDecl : public TreeNode
{
	// CHILDREN:  2: 
	//   FIRST: TNTypeSpecifier: type specifier. 
	//   SECOND: TNDeclarator name; may also contain default argument
	public:
		static const TNType tn_type=TN_FuncDefArgDecl;
	private:
	public:
		TNFuncDefArgDecl(TNTypeSpecifier *type_spec,TNDeclarator *_name) : 
			TreeNode(TN_FuncDefArgDecl)
			{ if(type_spec) AddChild(type_spec);  if(_name) AddChild(_name); }
		~TNFuncDefArgDecl();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use default. 
		//TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding virtuals]
		int Eval(ExprValue &retval,ExecThreadInfo *info);
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// This is a function/method def. 
class TNFunctionDef : public TreeNode
{
	// CHILDREN: 
	//  FIRST: TNIdentifier (name)
	//  SECOND: TNTypeSpecifier return type, or void
	//  THIRD: TNCompoundStmt (function body)
	//  ALL OTHER: TNFuncDefArgDecl (function arguments)
	// Use the pointers instead accessing down.xxx directly. 
	// NOTE: Destructor has ret_type=NULL, void functions use 
	//       TNTypeSpefifier with EVT_Void. 
	//       The animation spec ("animation NAME {}") uses 
	//       ret_type=NULL and has attrib IS_Animation set. 
	//       The destructor has IS_Destructor bit in attrib. 
	public:
		static const TNType tn_type=TN_FunctionDef;
	public:
		TNIdentifier *func_name;
		TNTypeSpecifier *ret_type;
		TNCompoundStmt *func_body;
		TNFuncDefArgDecl *first_arg;  // first arg; next ones with ->next
		// Associated function; set up by DoRegistration(). 
		// Can be internal function in some cases (POV). 
		AniFunction *function;
		
		TNLocation aloc0;  // location of the attributes at beginning
		TNLocation aloc1;  // location of the attributes at end
		// A static function: Needs no explanation
		// A constant function ("int foo() const { blah }"): 
		//   Can be evaluated in subexpression evaluation. 
		//   I.e. it is static and has no side effects. 
		enum
		{
			IS_Const=     0x0001,  // set if "const" was specified
			IS_Static=    0x0002,  // set if "static" was specified
			IS_Destructor=0x0004,  // destructor? ("~" prefix)
			IS_Animation= 0x0008,  // "animation NAME { ... }"
		};
		int attrib;  // Bitwise OR of the IS_* flags above. 
	public:
		TNFunctionDef(TNIdentifier *_name,TNTypeSpecifier *_ret_type,
			TNStatement *_func_body) : 
			TreeNode(TN_FunctionDef),aloc0(),aloc1()
			{
				AddChild(func_name=_name);
				ret_type=_ret_type;  if(ret_type) AddChild(ret_type);
				AddChild(func_body=(TNCompoundStmt*)_func_body);
				first_arg=NULL;  function=NULL;  attrib=0;
			}
		~TNFunctionDef();
		
		// Used when building tree to set the args: 
		void SetFuncArguments(TreeNode *list)
			{  first_arg=(TNFuncDefArgDecl*)list->down.first();
				TransferChildrenFrom(list);  }
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// Modelling the ANI object. 
class TNObject : public TreeNode
{
	// CHILDREN: 
	//  FIRST: TNIdentifier (object name)
	//  THEN: body entries, i.e. 
	//        TNFunctionDef, TNDeclarationStmt
	public:
		static const TNType tn_type=TN_Object;
	private:
		TNLocation loc0;   // beginning of object scope (brace)
		TNLocation loc1;   // end of object scope (brace)
	public:
		// Associated object; set up by DoRegistration(). 
		AniAniSetObj *object;
	public:
		TNObject(TNIdentifier *_name,const TNLocation &_loc0,
			const TNLocation &_loc1) : 
			TreeNode(TN_Object),loc0(_loc0),loc1(_loc1)
			{  if(_name) AddChild(_name);  object=NULL;  }
		~TNObject();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// Modelling the ANI setting. 
class TNSetting : public TreeNode
{
	// CHILDREN: 
	//  FIRST: TNIdentifier (setting name)
	//  THEN: body entries, i.e. 
	//        TNFunctionDef, TNDeclarationStmt, TNObject
	public:
		static const TNType tn_type=TN_Setting;
	private:
		TNLocation loc0;   // beginning of setting scope (brace)
		TNLocation loc1;   // end of setting scope (brace)
	public:
		// Associated setting; set up by DoRegistration(). 
		AniAniSetObj *setting;
	public:
		TNSetting(TNIdentifier *_name,const TNLocation &_loc0,
			const TNLocation &_loc1) : 
			TreeNode(TN_Setting),loc0(_loc0),loc1(_loc1)
			{  if(_name) AddChild(_name);  setting=NULL;  }
		~TNSetting();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// This is the type of the root node: 
class TNAnimation : public TreeNode
{
	// CHILDREN: 
	//  List of entries like TNSetting, TNFunctionDef, TNDeclarationStmt,
	//     TNObject
	public:
		static const TNType tn_type=TN_Animation;
	public:
		// Associated animation; set up by DoRegistration(). 
		AniAniSetObj *animation;
	public:
		TNAnimation(TreeNode *first_child=NULL) : TreeNode(TN_Animation)
			{  if(first_child)  AddChild(first_child);  animation=NULL;  }
		~TNAnimation();
		
		// [overriding virtuals]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. [overriding a virtual]
		// Use default. 
		// TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		// [overriding a virtual]
		void DoRegistration(TRegistrationInfo *tri);
		
		// Type fix [overriding a virtual]. 
		int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// [overriding a virtual]
		int CheckIfEvaluable(CheckIfEvalInfo *ci);
};

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_TREE_H_ */
