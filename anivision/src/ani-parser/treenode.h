/*
 * ani-parser/treenode.h
 * 
 * ANI tree node base class. 
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

#ifndef _ANIVISION_ANI_TREENODE_H_
#define _ANIVISION_ANI_TREENODE_H_ 1

#include <hlib/prototypes.h>
#include <hlib/linkedlist.h>

#include <ani-parser/strtreedump.h>

//----------------------------------------------------------
// These classes are provided by the core. 
// For more info, see <coreglue.h>. 
namespace ANI { class AniGlue_ScopeBase; }
// These must be derived from AniGlue_ScopeBase: 
class AniAniSetObj;
class AniFunction;
class AniAnonScope;
class AniVariable;
//----------------------------------------------------------


namespace ANI
{

// TreeNode type: 
enum TNType
{
	TN_None=0,             // may never happen in tree
	TN_IdentifierSimple,   // Simply a node with a string. 
	TN_Identifier,         // Several TN_IdentifierSimple, i.e. A::B::C
	TN_Expression,         // (Sub)Expression (tree). 
	TN_ExpressionList,     // Expression list expr,expr,expr
	TN_TypeSpecifier,      // Type specifier ("int", "vector<4>", "MyObject")
	TN_Statement,          // Statement (iteration, selection, ...)
	TN_Declarator,         // Declarator (e.g. "x[][3]={..}" in "int x[][3]={..}")
	TN_NewArraySpecifier,  // For OF_NewArray: array declarators
	TN_FuncDefArgDecl,     // argument decl inside func def (func args)
	TN_FunctionDef,        // function with body
	TN_Object,             // object node
	TN_Setting,            // setting node
	TN_Animation,          // complete animation; root node type
	TN_POV,                // externally used by POV code
	TN_AniDesc,            // animation description -> calc core
	                       // for entries inside %..{ <here> }" 
};

// TNExpression (derived from TreeNode) type:
enum TNEType
{
	TNE_None=0,             // may not happen...
	TNE_Value,              // This node is a value node. 
	TNE_Array,              // This is an array node (e.g. "{ 1,3,4 }" )
	TNE_OperatorFunction,   // Implicit operator function: +,-, <,,>,  ...
	TNE_Expression,         // (Sub)expression
	TNE_FCallArgument,      // argument for fcall (either assignment or not)
	TNE_Assignment,         // assignment (normally only in statement)
};

// TNStatement (derived from TreeNode) type: 
enum TNSType
{
	TNS_None=0,            // may not happen
	TNS_ExpressionStmt,    // expression stmt, assignment stmt or empty stmt
	TNS_IterationStmt,     // loop statement (for, while)
	TNS_SelectionStmt,     // selection statement (if, if..else)
	TNS_JumpStmt,          // jump statement (return, break)
	TNS_CompoundStmt,      // compound statement "{ .;.;.; }"
	TNS_DeclarationStmt,   // declarations
	TNS_AniDescStmt,       // the "%..{}" directive; the "{}" block is TN_AniDesc 
};


// For TreeNode::DoExprTF(TFInfo *ti): 
struct TFInfo;
struct TRegistrationInfo;

// As passed to TNExpression::Eval() and TNStatement::Exec(): 
struct ExecThreadInfo;

// --> location.h
class TNLocation;

// --> exprvalue.h
class ExprValue;


// As passed to CheckIfEvaluable(): 
struct CheckIfEvalInfo
{
	// How often the this pointer is referenced (by 
	// variables / function calls...): 
	int nrefs_to_this_ptr;
	// How often the local stack is referenced (without 
	// the this pointer on the stack): 
	int nrefs_to_local_stack;
	// Number of static vars accesses: 
	int nrefs_to_static;
	// Number of direct values (TNValue) in tree: 
	int number_of_values;
	
	CheckIfEvalInfo();
	~CheckIfEvalInfo() {}
};
		

// NEVER ALLOCATE A TreeNode ON THE STACK. ALWAYS USE operator new. 
struct TreeNode : LinkedListBase<TreeNode>
{
	public:
		// There is a list of all TreeNode heads for clean de-alloc. 
		// (Older nodes at the beginning.) 
		// TreeNode::operator new puts nodes into this list. 
		// BE SURE TO REMOVE FROM THIS LIST BEFORE PUTTING INTO 
		// SOME HIERARCHY. 
		// !!THUS!! ALWAYS USE SetParent() instead of 
		//          assigning tn->up directly. 
		static LinkedList<TreeNode> head_nodes;
		
		// Free all nodes in head_nodes except the one passed (which 
		// may be NULL for "none"). 
		// The passed node is just dequeued from the head nodes but 
		// NOT deleted. You have now the responsibility over the head 
		// node. Use TreeNode::DeleteTree() to delete the tree. 
		static void FreeAllHeadNodes(TreeNode *except);
		
		// Read comment above. 
		// You may delete any node using operator delete in the 
		// following way: "delete node->up->RemoveChild(node);" 
		// For a head node, which was dequeued from the head list 
		// using FreeAllHeadNodes(except=<head_node>), you must 
		// use this function. It is save, however, to use it for 
		// any node, even for NULL nodes :)
		void DeleteTree();
		
		// This is like FreeAllHeadNodes() except that it does not 
		// free the head nodes but only removes the passed node from 
		// the head node list so that you now have the responsibility 
		// for the passed head node. Useful after cloning a tree 
		// (i.e. CloneTree()). 
		static void RemoveFromHeadNodes(TreeNode *tn);
		// Very special function... Add to head nodes list again. 
		// May be used if previously removed from head nodes using 
		// RemoveFromHeadNodes(). This also means that the head nodes 
		// list now has the responsibility for the tree again. 
		static void AppendToHeadNodes(TreeNode *tn);
		
	private:
		TNType ntype;
		TreeNode *up;
		
		// Check if TreeNode was allocated via operator new or on 
		// the stack (the latter is not allowed). 
		void _check_stack_error();
		void _check_stack()  // Only call from constructor. 
		//	{  if(head_nodes.last()!=this && !this->next) _check_stack_error();  }
		// Cannot use the above code because we use LinkedListBase<TreeNode>(LLB_NoInit) 
		// as the head_nodes queuing is set up by operator new. 
		// NOTE: find_rev() will usually terminate after one or two loops: 
			{  if(!head_nodes.find_rev(this))  _check_stack_error();  }
		
	protected:
		// Used by CloneTree(): 
		// Call CloneTree() for all children and add the cloned 
		// children to the passed new parent. Returns passed parent. 
		TreeNode *_CloneChildren(TreeNode *new_parent);
		
	public:
		// This is public for convenience: 
		LinkedList<TreeNode> down;
		
		// Node allocation and deallocation: 
		void *operator new(size_t s);
		void operator delete(void *ptr,size_t s);
		// Number of allocated TreeNodes: 
		static int n_alloc_tree_nodes;
		// Number of bytes allocated for TreeNodes: 
		static size_t tree_nodes_tot_size;
		
		// Get node type: 
		TNType NType() const
			{  return(ntype);  }
		
		// Returns true if the node is of type TN_None and has no children: 
		bool IsNull() const
			{  return(ntype==TN_None && down.is_empty());  }
		
		// Get up vector (pointer to parent TreeNode): 
		TreeNode *parent() const
			{  return(up);  }
		
		// USE THIS INSTEAD OF ASSIGNING this->up DIRECTLY. 
		// Read head_nodes for details. 
		void SetParent(TreeNode *p);
		// Add a child node; this also sets us as parent of the child node: 
		// If you set parent NULL, the node gets added to head_nodes again. 
		void AddChild(TreeNode *c)
			{  c->SetParent(this);  down.append(c);  }
		// The same as AddChild but insert child at beginning of down list: 
		void AddChildAtBeginning(TreeNode *c)  // seldom used
			{  c->SetParent(this);  down.insert(c);  }
		// And the same at some other position: 
		void InsertChildBefore(TreeNode *tn,TreeNode *where)
			{  tn->SetParent(this);  down.queuebefore(tn,where);  }
		
		// Remove child node: Make sure the passed TreeNode IS 
		// actually a child; returns the passed node. 
		TreeNode *RemoveChild(TreeNode *c)
			{  down.dequeue(c);  c->SetParent(NULL);  return(c);  }
		
		// Transfer children from other TreeNode to this one. 
		void TransferChildrenFrom(TreeNode *src);
		
		// Replace *this with the passed node deleting *this. 
		void ReplaceBy(TreeNode *new_node);
		
		// Recursively count nodes. 
		int CountNodes() const;
		
		// Check if there are any nodes without associated ani scope 
		// but which should have one. Return their number and write 
		// an ugly error (stderr) for each. 
		// THIS IS ONLY A "HACK" TO CHECK FOR INTERNAL ERRORS. 
		// Function in treereg.cc. 
		int CheckAniScopePresence() const;
		
		// Try most primitive immediate constant evaluation/"folding": 
		// BE CAREFUL! MAY DELETE *this. 
		// Returns CUnknownType if not successful. 
		ExprValue DoTryIEval(TreeNode **update_tn_here=NULL);
		
		// Note destructor automatically deletes all nodes below. 
		TreeNode(/*const TNLocation &_loc,*/TNType t) : 
			LinkedListBase<TreeNode>(LLB_NoInit),/*loc(_loc),*/down()
			{  ntype=t;  _check_stack();  up=NULL;  }
		virtual ~TreeNode()
			{  while(!down.is_empty())  delete down.popfirst();  /*up=NULL; <-- !!NO!! */  }
		
		// Generate a tree node of specified type with passed 
		// TreeNodes as children: 
		TreeNode(/*const TNLocation &_loc,*/TNType nt,TreeNode *a0) : 
			LinkedListBase<TreeNode>(LLB_NoInit),/*loc(_loc),*/down()
			{  ntype=nt;  _check_stack();  up=NULL;  AddChild(a0);  }
		TreeNode(/*const TNLocation &_loc,*/TNType nt,TreeNode *a0,TreeNode *a1) : 
			LinkedListBase<TreeNode>(LLB_NoInit),/*loc(_loc),*/down()
			{  ntype=nt;  _check_stack();  up=NULL;  AddChild(a0);  
				AddChild(a1);  }
		TreeNode(/*const TNLocation &_loc,*/TNType nt,TreeNode *a0,TreeNode *a1,TreeNode *a2) : 
			LinkedListBase<TreeNode>(LLB_NoInit),/*loc(_loc),*/down()
			{  ntype=nt;  _check_stack();  up=NULL;  AddChild(a0);  
				AddChild(a1);  AddChild(a2);  }
	public:
		// VIRTUAL FUNCTIONS: 
		
		// Recursively dump a string representation of the complete 
		// subtree: 
		virtual void DumpTree(StringTreeDump *d);
		// This is a shortcut for debugging purposes: 
		void DumpTree(FILE *out);
		
		// Recursively clone a tree. 
		// Only works for non-TF'ed, non-registered trees 
		// (i.e. attached AniScopes and eval handlers will NOT be cloned). 
		// Also read comment at TreeNode::RemoveFromHeadNodes(). 
		virtual TreeNode *CloneTree();
		
		// Get start and end location of complete subtree. 
		virtual TNLocation GetLocationRange() const;
		
		// Recursively perform the registration step: 
		virtual void DoRegistration(TRegistrationInfo *tri);
		
		// Recursively perform the expression type fixup stage: 
		// Sometimes, DoExprTF() may change *this; in that case, 
		// update_tn is updated to the new value of *this. 
		// Return value: 
		//   0 -> Ok, type fixup was successful for subtree
		//  >0 -> errors when doing type fixup 
		virtual int DoExprTF(TFInfo *ti,TreeNode **update_tn=NULL);
		
		// Must be provided by all TNExpression-derived types 
		// and by TNStatement: 
		// Evaluate the expression: 
		virtual int Eval(ExprValue &retval,ExecThreadInfo *info);
		// Check if we CAN Eval() or Exec(). This is done 
		// once after type fixup and before static data 
		// initialisation. (But may be done lateron for subtrees, 
		// as well). Returns error count. 
		virtual int CheckIfEvaluable(CheckIfEvalInfo *ci);
};


// Derived classes: (-> tree.h)
class TNIdentifierSimple;
class TNIdentifier;
class TNExpression;
class TNExpressionList;
class TNValue;
class TNArray;
class TNTypeSpecifier;
class TNOperatorFunction;
class TNFCallArgument;
class TNAssignment;
class TNStatement;
class TNExpressionStmt;
class TNIterationStmt;
class TNSelectionStmt;
class TNJumpStmt;
class TNCompoundStmt;
class TNDeclarator;
class TNNewArraySpecifier;
class TNDeclarationStmt;
class TNFuncDefArgDecl;
class TNFunctionDef;
class TNObject;
class TNSetting;
class TNAnimation;


// Operator functions and arguments: 
// Arguments in down list: A,B,C,...
// CHANGES HERE --> Update _OperatorFunc_IDDesc[] in tree.cc. 
//                  Update OpFuncID_PODDesc[_OF_LAST] in opfunc.cc. 
enum OperatorFuncID
{
	OF_None=0,
	
	OF_Subscript,   // A[B]
	OF_FCall,       // A(B,C,D,...)          B,C,D,...=TNFCallArgument
	OF_PODCast,     // A(B)                  A=TNTypeSpecifier
	OF_MembSel,     // A.B  (member select)  B=TNIdentifier
	OF_Mapping,     // A->B                  B=TNIdentifier
	
	OF_PostIncrement,  // A++
	OF_PostDecrement,  // A--
	OF_PreIncrement,   // ++A
	OF_PreDecrement,   // --A
	
	OF_UnPlus,      // +A
	OF_UnMinus,     // -A
	OF_UnNot,       // !A
	
	OF_New,         // new A(B,C,D,...)   A=TNIdentifier, B,C,D,...=TNFCallArgument
	OF_NewArray,    // new A[B][C]        A=TNTypeSpecifier, B,C,..=TNNewArraySpecifier
	OF_Delete,      // delete A           A=TNExpression
	
	OF_Pow,         // A^B
	OF_Mult,        // A*B
	OF_Div,         // A/B
	OF_Modulo,      // A%B
	OF_Add,         // A+B
	OF_Subtract,    // A-B
	OF_Lesser,      // A<B
	OF_Greater,     // A>B
	OF_LesserEq,    // A<=B
	OF_GreaterEq,   // A>=B
	OF_Equal,       // A==B
	OF_NotEqual,    // A!=B
	OF_LogicalAnd,  // A&&B
	OF_LogicalOr,   // A||B
	
	OF_IfElse,      // A?B:C
	OF_IfElse2,     // A?:B
	
	OF_Range,       // A..B
	OF_RangeNoA,    // ..A
	OF_RangeNoB,    // A..
	OF_Vector,      // <A,B,C,...>
	
	_OF_LAST
};

// Return string representation: 
extern const char *OperatorFuncIDString(OperatorFuncID ofid);


enum AssignmentOpID
{
	AO_None=0,    // may not happen
	AO_Set,       // "="
	AO_Add,       // "+="
	AO_Subtract,  // "-="
	AO_Multiply,  // "*="
	AO_Divide,    // "/="
	_AO_LAST
};
// Return string representation: 
extern const char *AssignmentOpIDString(AssignmentOpID ass_op);

}  // end of namespace ANI

// Derived classes: (-> pov-core/povtree.h)
namespace POV
{
	class POVTreeNode;
	class PTNExpression;
	class PTNAssignment;
	class PTNObjectEntry;
	class PTNObjectSpec;
}

// Derived classes: (-> calccore/adtree.h)
namespace CC
{
	class ADTreeNode;
	class ADTNAniDescBlock;
	class ADTNTypedScope;
	class ADTNScopeEntry;
}

#endif  /* _ANIVISION_ANI_TREENODE_H_ */
