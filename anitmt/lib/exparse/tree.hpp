/*
 * iltree.hpp
 * 
 * Expression tree for input lib / exparse. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _NS_ExParse_Tree_HPP_
#define _NS_ExParse_Tree_HPP_ 1

#include "operator.hpp"
#include "linkedlist.h"

namespace exparse
{

//! This is needed internally for the process of making an expression 
//! tree out of an expression list (which notes down the expression as it 
//! is parsed in. 
struct OperatorNode : LinkedListBase<OperatorNode>
{
	Operator *op;  //! pointer to corresponding Operator
	int done;      //! already put into expression tree?
	
	//! Returns the precedence value: 
	int prec()  {  return(op->desc()->precedence);  }
	
	OperatorNode(Operator *_op)  //! Not copied; just referenced. 
		{  op=_op;  done=0;  }
	~OperatorNode()
		{  op=NULL;  }
};

class ExpressionTree
{
	private:
		Operator *root;
		
		std::ostream &_WriteOperator(
			std::ostream &os,
			Operator *op);
		std::ostream &_WriteExpression(
			std::ostream &os,
			LinkedList<OperatorNode> *exp);
		
		int _TreeAddBranch(OperatorNode *top,
			LinkedList<OperatorNode> *exp,
			LinkedList<OperatorNode> *garbage,
			OperatorNode *on);
		int _FuncAddChild(
			OperatorNode *func,
			Operator *op);
		int _CorrectGVAmbiguity(
			LinkedList<OperatorNode> *exp,
			int *_errors,
			bool respect_brackets);
		
		// See detailed description in tree.cpp for these: 
		int _GenFuncRecursive(
			OperatorNode *func,
			LinkedList<OperatorNode> *exp,
			LinkedList<OperatorNode> *garbage,
			bool correct_parser_ambiguities);
		int _GenTreeRecursive(
			LinkedList<OperatorNode> *exp,
			LinkedList<OperatorNode> *garbage,
			bool correct_parser_ambiguities);
		int _GenTreeLinear(
			LinkedList<OperatorNode> *exp,
			LinkedList<OperatorNode> *garbage,
			bool correct_parser_ambiguities);
		
		int ComputeTree(Value *ret,Operator *top);
		int OptimizeTree(Operator *top);
		void _RecursiveClear(Operator *top);
	public:
		ExpressionTree();
		~ExpressionTree();  // calls Clear()
		
		//! Counts all the operators in the complete expression 
		//! tree. Returns 0, if no tree is set (after Clear()). 
		int CountOperators()
			{  return(root ? (root->CountChildren()+1) : 0);  }
		
		//! Clear up the currently set tree (all the Operators; the 
		//! OperatorNodes are not stored in here. Either you delete 
		//! them yourself after Create() or tell Create() to do so). 
		void Clear();
		
		//! Generate an expression tree out of a linked list of separate 
		//! Operators which are put into the LinkedList as they are 
		//! parsed in, e.g. [a] [+] [b] [*] [c] [+] [d]. 
		//! The Operators are put into an expression tree respecting 
		//! precedence rules. 
		//! correct_parser_ambiguities: 
		//!   true -> for use with text parsers; changes if necessary: 
		//!        OTAdd -> OTPos; OTSub -> OTNeg; (binary -> unary +/-) 
		//!        `<' -> `gv(' ; `>' -> `)'  (vector generation)
		//!   false -> don't change anything in the expression
		//! free_operatornodes: 
		//!   Decides whether to free the OperatorNodes and the unused 
		//!   Operators using delete (and returning an empty expression 
		//!   list in *exp) or not. 
		//! Note that the OperatorNodes get dequeued as the tree is 
		//! built but they get re-queued so that you can free the nodes. 
		//! The root operator of the tree is stored in the LAST node of 
		//! the list (-> exp->last()). 
		//! Don't assume anything else about the order of the nodes in 
		//! *exp when the function returns. 
		//! Returns 0 on success; >0 on error(s). 
		//! NOTE: All operators of the previously set tree get freed 
		//!       (delerted) using Clear() as also done by the 
		//!       destructor. 
		int Create(
			LinkedList<OperatorNode> *exp,
			bool correct_parser_ambiguities,
			bool free_operatornodes);
		
		//! Tries to optimize the tree by calculating all non-variable 
		//! subtrees and then freeing them (using delete). 
		//! Return value: 
		//!  0 -> OK, tree was optimized
		//! -1 -> root is NULL: Create() not called or returned error. 
		//! >0 -> errors occured (while computing subtree)
		int Optimize();
		
		//! Compute the value of an expression tree. 
		//! Return value: 
		//!  0 -> OK, value computed and returned in val. 
		//! -1 -> root is NULL: Create() not called or returned error. 
		//! >0 -> errors occured. 
		int Compute(Value *val);
};

}  // namespace end 

#endif  /* _NS_ExParse_Tree_HPP_ */
