/*
 * tree.cpp
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

#include "tree.hpp"
#include "fcall.hpp"

#include <stdio.h>

#ifndef TESTING
#define TESTING 1 
#endif 

#if TESTING
#warning TESTING switched on
#endif

#warning try out if the two fast ways are really faster (with everyday examples!) 
#warning (rm me; better verbosity control) 
int verbose=2;


namespace exparse
{

std::ostream &ExpressionTree::_WriteOperator(
	std::ostream &os,
	Operator *op)
{
	const FunctionDesc *fdesc=op->function();
	if(fdesc)
	{  os << fdesc->name;  }
	else if(op->type()==OTValue)
	{  os << *op->value();  }
	else
	{  os << op->desc()->name;  }
	return(os);
}

std::ostream &ExpressionTree::_WriteExpression(
	std::ostream &os,
	LinkedList<OperatorNode> *exp)
{
	for(OperatorNode *on=exp->first(); on; on=on->next)
	{  _WriteOperator(os,on->op);  }
	return(os);
}


// Little internal helper for _GenFuncRecursive(). 
int ExpressionTree::_FuncAddChild(
	OperatorNode *func,
	Operator *op)
{
	int rv=func->op->AddChild(op);
	if(rv)
	{
		if(rv==1)
		{  fprintf(stderr,"Too many (>%d) arguments for function %s\n",
			func->op->NChld(),func->op->function()->name);  }
		else
		{  fprintf(stderr,"Oops: Failed to add function child node: "
			"rv=%d\n",rv);  }
		return(1);
	}
	return(0);
}


// This function (together with _GenTreeRecursive) is the main 
// (recursive) part of expression tree generation. 
// This function gets a sub-expression consisting of arguments 
// to the function passed on func (not queued at all) separated 
// by OTSep (","). 
// For each subexpression, _GenTreeRecursive / _GenFuncRecursive 
// is called (depending on whether a function has to be called 
// or not). 
// The resulting expression roots for each function argument 
// are assigned as child operators of the function and the function 
// is appended to the garbage list thus returning the subexpression's 
// root node in garbage->last(). 
// Returns 0 on success; >0 on error. 
// (See Create() for a description on correct_parser_ambiguities.) 
int ExpressionTree::_GenFuncRecursive(
	OperatorNode *func,
	LinkedList<OperatorNode> *exp,
	LinkedList<OperatorNode> *garbage,
	bool correct_parser_ambiguities)
{
	if(verbose>1)
	{
		cout << "FuncRec ";
		_WriteOperator(cout,func->op) << "[ ";
		_WriteExpression(cout,exp);
		cout << " ]";
	}
	
	int errors=0;
	
	if(exp->first() && 
	   exp->first()==exp->last() && 
	   exp->first()->op->type()==OTValue)
	{
		// This is the fast path (little optimization): 
		// If the subexpression is just a value, we can 
		// return immediately. 
		if(verbose>1)
		{  cout << " (FAST)" << std::endl;  }
		OperatorNode *on=exp->popfirst();
		errors+=_FuncAddChild(func,on->op);
		garbage->append(on);
	}
	else
	{
		// This is the slow path...
		if(verbose>1)
		{  cout << std::endl;  }
		
		if(correct_parser_ambiguities)
		{  _CorrectGVAmbiguity(exp,&errors,/*respect_brackets=*/true);  }
		
		LinkedList<OperatorNode> subexp;
		
		// If exp->is_empty(), then there are no args (e.g. `foo()'). 
		int depth=0;
		OperatorNode *on=NULL;
		if(!errors && !exp->is_empty())  for(;;)   // only if the args are given
		{
			on=exp->popfirst();
			if(!on)
			{
				if(!depth)
				{  goto subexp_end;  }
				//++errors;
				break;
			}
			if(on->op->type()==OTBrOp)
			{  ++depth;  }
			else if(on->op->type()==OTBrCl)
			{
				--depth;
				//if(depth<0)  ++errors;   // error written somewhere else (I hope)
			}
			if(depth || on->op->type()!=OTSep)
			{  subexp.append(on);  }
			else   // "," and depth==0
			{
				// Must parse subexpression and add as arg to function. 
				garbage->append(on);  // throw away the "," 
				subexp_end:;
				errors+=_GenTreeRecursive(&subexp,garbage,
					correct_parser_ambiguities);
				assert(subexp.is_empty());
				if(errors)  break;
				errors+=_FuncAddChild(func,garbage->last()->op);
				if(errors)  break;
				if(!on)  break;
			}
		}
		
		// Make sure no nodes are left in *exp and subexp: 
		if(!exp->is_empty() || !subexp.is_empty())
		{
			assert(errors);
			while((on=exp->popfirst()))
			{  garbage->append(on);  }
			while((on=subexp.popfirst()))
			{  garbage->append(on);  }
		}
	}
	
	if(!errors)
	{
		// See if func has the correct number of args: 
		int rv=(*func->op->function()->check_nargs)(func->op->NChld());
		assert(rv!=2);   // must have been caught earlier 
		if(rv)
		{
			fprintf(stderr,"Wrong number of args (%d) to function %s.\n",
				func->op->NChld(),func->op->function()->name);
			++errors;
		}
	}
	
	// Make function operator available as subexpression root: 
	func->done=1;
	garbage->append(func);
	return(errors);
}


// This function (together with _GenFuncRecursive) is the main 
// (recursive) part of expression tree generation. 
// This function takes the expression *exp and calls itself 
// for each sub-expression in exp which is enclosed by brackets. 
// In case a function call node is immediately before the bracket, 
// it calls _GenFuncRecursive instead of itself. 
// The resulting bracket-less expression list is passed to 
// _GenTreeLinear() for final tree building. 
// The OperatorNodes are dequeued from *exp and queued under 
// *garbage. The subtree root is stored as garbage->last(). 
// Returns 0 on success; >0 on error. 
// (See Create() for a description on correct_parser_ambiguities.) 
int ExpressionTree::_GenTreeRecursive(
	LinkedList<OperatorNode> *exp,
	LinkedList<OperatorNode> *garbage,
	bool correct_parser_ambiguities)
{
	if(verbose>1)
	{
		cout << "TreeRec[ ";
		_WriteExpression(cout,exp);
		cout << " ]";
	}
	
	if(exp->first() && 
	   exp->first()==exp->last() && 
	   exp->first()->op->type()==OTValue)
	{
		// This is the fast path (little optimization): 
		// If the subexpression is just a value, we can 
		// return immediately. 
		if(verbose>1)
		{  cout << " (FAST)" << std::endl;  }
		garbage->append(exp->popfirst());  // store subtree root 
		return(0);                         // done. 
	}
	else if(verbose>1)
	{  cout << std::endl;  }
	
	int errors=0;
	if(correct_parser_ambiguities)
	{  _CorrectGVAmbiguity(exp,&errors,/*respect_brackets=*/true);  }
	
	LinkedList<OperatorNode> _subexp,_topexp;
	LinkedList<OperatorNode> *subexp=&_subexp,*topexp=&_topexp;
	OperatorNode *func=NULL;
	
	OperatorNode *on=NULL;
	int depth=0;
	if(!errors)  while((on=exp->popfirst()))
	{
		if(!depth && on->op->type()==OTFunction)
		{
			assert(!func);
			func=on;
			on=exp->popfirst();
			if(!on)
			{
				fprintf(stderr,"incomplete function call\n");
				++errors;
				break;
			}
			if(on->op->type()!=OTBrOp)   // "("
			{
				fprintf(stderr,"expecting `(' after function name\n");
				++errors;  garbage->append(on);
				break;
			}
		}
		if(on->op->type()==OTBrOp)   // "("
		{  ++depth;  }
		
		(depth ? subexp : topexp)->append(on);
		
		if(on->op->type()==OTBrCl)   // ")"
		{
			if(!depth)
			{
				fprintf(stderr,"too many `)'s\n");
				++errors;  // DONT append on to garbage! (is on topexp)
				break;
			}
			--depth;
			if(!depth)
			{
				// Reached end of subexpression. 
				// First, get rid of the brackets at the beginning and 
				// at the end: 
				assert(subexp->first()->op->type()==OTBrOp);
				assert(subexp->last()->op->type()==OTBrCl);
				garbage->append(subexp->popfirst());
				garbage->append(subexp->poplast());
				// Secondly, recursively parse the subexp: 
				if(func)  // note: func not queued here. 
				{  errors+=_GenFuncRecursive(func,subexp,garbage,
					correct_parser_ambiguities);  }
				else
				{  errors+=_GenTreeRecursive(subexp,garbage,
					correct_parser_ambiguities);  }
				func=NULL;
				assert(subexp->is_empty());
				if(errors)  break;
				// The subexpression root must be made available for 
				// the linear build function in *topexp: 
				topexp->append(garbage->poplast());
			}
		}
	}
	if(depth)
	{
		fprintf(stderr,"too many `('s\n");
		++errors;
	}
	
	if(errors)
	{
		// Make sure no nodes get left in *exp: 
		garbage->append(func);  // if non-NULL
		while((on=exp->popfirst()))
		{  garbage->append(on);  }
		// Then, re-queue the nodes in topexp and subexp: 
		while((on=subexp->popfirst()))
		{  garbage->append(on);  }
		while((on=topexp->popfirst()))
		{  garbage->append(on);  }
		return(errors);
	}
	assert(!func);
	assert(subexp->is_empty());
	assert(exp->is_empty());
	
	// Everything concerning functions and brackets is done now. 
	// Time to do the linear tree building step: 
	// (errors=0 here)
	errors+=_GenTreeLinear(topexp,garbage,correct_parser_ambiguities);
	assert(topexp->is_empty());
	// GenTreeLinear() puts the subexpression root into 
	// garbage->last() as expected. 
	return(errors);
}


// This function is the non-recursive last part of tree creation. 
// The list *exp is an expression without brackets and functions. 
// The function can deal only with normal operators 
//(!,+,-,*,/,?:,&&,||,...), NOT with brackets and functions. 
// All the OperatorNodes are dequeued from *exp and queued in 
// *garbage with the top node (subtree root) being garbage->last(). 
// Returns 0 on success; >0 on error. 
int ExpressionTree::_GenTreeLinear(
	LinkedList<OperatorNode> *exp,
	LinkedList<OperatorNode> *garbage,
	bool correct_parser_ambiguities)
{
	if(verbose>1)
	{
		cout << "TreeLin[ ";
		_WriteExpression(cout,exp);
		cout << " ]" << std::endl;
	}
	
	if(exp->is_empty())
	{
		fprintf(stderr,"empty subexpression\n");
		return(1);
	}
	
	// Correct unary vs. binary +/- if necessary: 
	if(correct_parser_ambiguities)
	{
		for(OperatorNode *on=exp->first(); on; on=on->next)
		{
			Operator *op=on->op;
			if(!on->done && 
			   (op->type()==OTAdd || op->type()==OTSub) )
			{
				// Must check for unary vs. binary +/-: 
				int tp=0;   // 1 -> unary; 2 -> binary 
				if(!on->prev)  tp=1;
				/*else
				 *{
				 *	OperatorType pot=on->prev->op->type();
				 *	tp = (pot==OTValue || pot==OTBrCl) ? 2 : 1;
				 *}
				 */
				else
				{  tp = on->prev->done ? 2 : 1;  }
				assert(tp);
				if(tp==1)
				{
					int rv=op->ForceSetType((op->type()==OTAdd) ? OTPos : OTNeg);
					assert(!rv);
				}
			}
		}
	}
	
	int maxprec=max_precedence_val;
	assert(maxprec>5);   // be sure...
	int caught_prec[maxprec];
	for(int i=0; i<maxprec; i++)
	{  caught_prec[i]=0;  }
	
	int errors=0;
	
	// First, we check which precedence values are present at all: 
	// This is done merely to make things faster. 
	int maxp=0;
	for(OperatorNode *on=exp->first(); on; on=on->next)
	{
		register int p=on->prec();
		if(p<0)  continue;
		++caught_prec[p];
		if(maxp<p)
		{  maxp=p;  }
		// If this assertion fails, this may be due to some non-allowed 
		// operator here ( `(`, `)', `,' ). 
		if(on->op->type()==OTSep)
		{
			fprintf(stderr,"encountered stray `,'\n");
			++errors;
			break;
		}
		assert(on->op->type()==OTValue || 
		       on->op->type()==OTCondC || 
		       on->op->function());
	}
	
	int cond_prec=cond_precedence_val;
	
	if(errors)
	{  goto errs;  }
	
	// Now, actually do it: 
	for(int p=maxp; p>=0; p--)  // Iterate the precedences down 
	{
		if(!caught_prec[p])  continue;
		if(p==cond_prec)  // ooh... conditional expressions
		{
			// This is a special case and involves a special algorithm: 
			for(OperatorNode *on=exp->last(); on; on=on->prev)
			{
				if(on->prec()<p)  continue;   // will be put into tree lateron
				if(on->done)  continue;   // is already in expression tree
				assert(on->prec()==p);
				Operator *op=on->op;
				if(op->type()==OTCondC)  continue;
				assert(op->type()==OTCondQ);
				assert(op->desc()->noperands==3);
				if(!on->prev || !on->prev->done)
				{
					fprintf(stderr,"%s value in condition: <??>?b:c\n",
						(on->prev) ? "not a" : "missing");
					++errors;
					goto errs;
				}
				errors+=_TreeAddBranch(on,exp,garbage,on->prev);  // child[0]
				if(on->next && on->next->op->type()==OTCondC)
				{
					fprintf(stderr,"value omitted: a?<missing>:c\n");
					++errors;
					goto errs;
				}
				if(!on->next || !on->next->done)
				{
					fprintf(stderr,"%s value in condition: a?<??>:c\n",
						(on->next) ? "not a" : "missing");
					++errors;
					goto errs;
				}
				errors+=_TreeAddBranch(on,exp,garbage,on->next);  // child[1]
				if(!on->next || on->next->op->type()!=OTCondC)
				{
					// Oh! `?' and `:' mixed incorrectly/unbalanced. 
					fprintf(stderr,"unbalanced/incorrect ?: operator\n");
					++errors;
					goto errs;
				}
				
				// We don't need these OTCondC; we throw them away: 
				//on->next->done=1;
				garbage->append(exp->dequeue(on->next));
				
				// Okay, now, on->next should be some sort of value. 
				if(on->next && on->next->op->type()==OTCondC)
				{
					fprintf(stderr,"value omitted: a?b:<missing>\n");
					++errors;
					goto errs;
				}
				if(!on->next || !on->next->done)
				{
					fprintf(stderr,"%s value in condition: a?b:<??>\n",
						(on->next) ? "not a" : "missing");
					++errors;
					goto errs;
				}
				
				errors+=_TreeAddBranch(on,exp,garbage,on->next);  // child[2]
				on->done=1;
				
				if(errors)
				{  goto errs;  }
			}
			
			// Check for stray `:' (and do other internal checks) 
			for(OperatorNode *on=exp->last(); on; on=on->prev)
			{
				if(on->prec()<cond_prec)  continue;
				if(on->done)  continue;
				assert(on->prec()==cond_prec);
				if(on->op->type()==OTCondC)
				{
					fprintf(stderr,"encountered stray `:'\n");
					++errors;
					continue;
				}
				assert(on->op->type()==OTCondQ);
			}
			if(errors)
			{  goto errs;  }
			
			continue;
		}
		int bdir=binddir_of_precedence[p];
		for(OperatorNode *on = ((bdir<0) ? exp->last() : exp->first()); on; 
			on=((bdir<0) ? on->prev : on->next))
		{
			if(on->prec()<p)  continue;   // will be put into tree lateron
			if(on->done)  continue;   // is already in expression tree
			assert(on->prec()==p);
			Operator *op=on->op;
			
			// Not sure if any input will ever trap this: 
			// (** If so, replace the assert() with the block below. **) 
			assert(op->function());
			/*if(!op->function())
			{
				fprintf(stderr,"Encountered stray `%s'.\n",op->desc()->name);
				++errors;  goto errs;
			}*/
			
			switch(op->desc()->noperands)
			{
				case 1:  // 1 operand
					// The one operand is always AFTER the operator (!a, -a, +a). 
					// In case we get ones which are before the operator 
					// (like a++), we must discriminate here. 
					errors+=_TreeAddBranch(on,exp,garbage,on->next);  // child[0]
					break;
				case 2:  // 2 operands
					// This assert needs explaining: 
					// The on->prev, on->next args in the calls to the 
					// _TreeAddBranch() below may be wrong for operators 
					// binding right-to-left, so if there are any, check 
					// that and remove the assert. 
					assert(bdir==+1);
					
					errors+=_TreeAddBranch(on,exp,garbage,on->prev);  // child[0]
					errors+=_TreeAddBranch(on,exp,garbage,on->next);  // child[1]
					break;
				default:
					fprintf(stderr,"illegal number (%d) of operands\n",
						op->desc()->noperands);
					abort();
					break;
			}
			on->done=1;
			
			if(errors)
			{  goto errs;  }
		}
	}
	errs:;
	
	// Now, exp should contain exactly one node which is the root of the 
	// tree. Let's see...
	if(!exp->is_empty() && exp->first()!=exp->last())
	{
		if(!errors)
		{
			fprintf(stderr,"Oops: failed to create expression tree\n");
			++errors;
		}
		// Must put the elements of *exp into garbage: 
		OperatorNode *on;
		while((on=exp->popfirst()))
		{  garbage->append(on);  }
	}
	
	// Make sure the last node in garbage is the tree root: 
	OperatorNode *on=exp->popfirst();
	assert(on || errors);
	if(on)
	{
		garbage->append(on);
		assert(on->done || errors);
	}
	
	return(errors);
}


// Helper function for _GenTreeLinear(). 
int ExpressionTree::_TreeAddBranch(OperatorNode *top,
	LinkedList<OperatorNode> *exp,
	LinkedList<OperatorNode> *garbage,
	OperatorNode *on)
{
	if(!on)
	{
		fprintf(stderr,"Missing %dth arg/operand to <%s,%d>\n",
			top->op->NChld()+1,top->op->desc()->name,top->op->type());
		return(1);
	}
	assert(!top->done);
	if(!on->done)
	{
		fprintf(stderr,"`%s' is not an rvalue (as arg/operand %d for `%s')\n",
			on->op->function() ? on->op->function()->name : on->op->desc()->name,
			top->op->NChld()+1,
			top->op->function() ? top->op->function()->name : top->op->desc()->name);
		return(1);
	}
	if(verbose>4)  // VERY verbose; must re-write this using _WriteOperator(). 
		printf("Adding <%s,%d> (%p) as child %d of <%s,%d> (%p): ",
			on->op->desc()->name,on->op->type(),on->op,
			top->op->NChld(),
			top->op->desc()->name,top->op->type(),top->op);
	int rv=top->op->AddChild(on->op);
	garbage->append(exp->dequeue(on));
	if(verbose>4)
		printf("%d\n",rv);
	if(rv)
	{
		fprintf(stderr,"AddChild failed: rv=%d\n",rv);
	}
	return(rv ? 1 : 0);
}


// Corrects the </> lesser/greater vs. opening closing vector 
// parser ambiguity. Returns the number of inserted `gv(' 
// calls. *errors is incremented for each error. 
// respect_brackets: 1 -> only correct ambiguity in this bracket 
// level; 0 -> correct all ambiguities (without having a look at 
// the brackets which makes things bad). 
int ExpressionTree::_CorrectGVAmbiguity(
	LinkedList<OperatorNode> *exp,
	int *_errors,
	bool respect_brackets)
{
	int in_vec_func=0;  // vector `<', `>' nesting depth 
	int changed=0;
	int errors=0;
	int depth=0;
	for(OperatorNode *on=exp->first(); on; on=on->next)
	{
		OperatorType opt=on->op->type();
		if(respect_brackets)
		{
			if(opt==OTBrOp)
			{  ++depth;  }
			else if(opt==OTBrCl)
			{  --depth;  }
			if(depth)  continue;
		}
		if(opt==OTL)  // `<'
		{
			// Must check if this is `opening vector' or 
			// `lesser' (`<'). 
			int tp=0;  // 1 -> vector; 2 -> lesser 
			if(!on->prev)  tp=1;
			// ??? INSERT THAT: else if(on->prev->done)  tp=2;
			else
			{
				OperatorType pot=on->prev->op->type();
				tp = (pot==OTValue || pot==OTBrCl) ? 2 : 1;
			}
			assert(tp);
			if(tp==1)   // opening vector
			{
				++in_vec_func;
				// Construct a `gv(' for the `<': 
				// I replace the `<' by `(' and insert `gv' before it. 
				int rv=on->op->ForceSetType(OTBrOp);
				assert(!rv);
				Operator *gvop = new Operator(gen_vec_fdesc);
				OperatorNode *gvopn = new OperatorNode(gvop);
				exp->queuebefore(/*p=*/gvopn,/*where=*/on);
				++changed;
			}
		}
		else if(opt==OTG)  // `>'
		{
			// Must check if this is `closing vector' or 
			// `greater' (`>'). 
			OperatorNode *next_on=on->next;
			int tp=0;   // 1 -> vector; 2 -> greater
			if(!next_on)  tp=1;
			// ??? INSERT THAT:  else if(next_on->done)  tp=2;
			else
			{
				OperatorType ntyp=next_on->op->type();
				if(ntyp==OTValue || ntyp==OTFunction || ntyp==OTBrOp)
				{  tp=2;  }
				else
				{  tp=1;  }
			}
			if(tp==1)
			{
				if(!in_vec_func)
				{
					fprintf(stderr,"warning `>' interpreted as `greater' "
						"due to missing opening vector `<'.\n");
				}
				else
				{
					--in_vec_func;
					int rv=on->op->ForceSetType(OTBrCl);
					assert(!rv);
				}
			}
		}
	}
	if(in_vec_func)
	{
		// Too many `<'s... we will get an error about too many `('s 
		// so I should tell the user here. 
		fprintf(stderr,"Too many opening vectors `<'.\n");
		++errors;
	}
	
	if(changed && !errors && verbose>1)
	{
		cout << "AMBIG->TreeRec[ ";
		_WriteExpression(cout,exp);
		cout << " ]" << std::endl;
	}
	
	(*_errors)+=errors;
	return(changed);
}


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
int ExpressionTree::Create(
	LinkedList<OperatorNode> *exp,
	bool correct_parser_ambiguities,
	bool free_operatornodes)
{
	if(verbose>1)
	{
		cout << "CREATE[ ";
		_WriteExpression(cout,exp);
		cout << " ]" << std::endl;
	}
	
	Clear();
	
	LinkedList<OperatorNode> garbage;
	this->root=NULL;
	#if TESTING
	int nnodes0=exp->count();
	#endif
	
	// Must reset done values: 
	for(OperatorNode *on=exp->first(); on; on=on->next)
	{  on->done=(on->op->type()==OTValue);  }
	
	/* PREVIOUS LOCATION OF AMBIGUITY CORRECTION FOR <> */
	
	#warning rm me?: 
	#if 0  /* REMOVE TO #else ONLY!! */
	// If correct_parser_ambiguities was set, we now have to check 
	// for vector declarations (<...>). 
	int ambig_gv_added=0;
	int errors=0;
	if(correct_parser_ambiguities)
	{  ambig_gv_added=_CorrectGVAmbiguity(exp,&errors,false);  }
	
	if(!errors)
	{
		// Really do it: 
		errors+=_GenTreeRecursive(exp,&garbage,correct_parser_ambiguities);
		assert(exp->is_empty());
	}
	else
	{
		OperatorNode *on;
		while((on=exp->popfirst()))
		{  garbage.append(on);  }
	}
	#else
	// Really do it: 
	int errors=_GenTreeRecursive(exp,&garbage,correct_parser_ambiguities);
	assert(exp->is_empty());
	#endif
	
	#if TESTING
	int nnodes1=garbage.count();
	if(correct_parser_ambiguities)
	{
		/* METHOD IF WE DONT HAVE ambig_gv_added: */
		// Okay... Each inserted `gv(' instead of `<' (vector creation) 
		// adds a further node: 
		// (Note that this test cannot protect us from stupid double 
		// bugs where one `gv'-node gets lost and another gets added 
		// instead of `<'. Oh, well...)
		for(OperatorNode *on=garbage.first(); on; on=on->next)
		{
			if(on->op->function()==gen_vec_fdesc)
			{  ++nnodes0;  }
		}
		
		/* METHOD IF WE DO HAVE ambig_gv_added: */
		/*nnodes0+=ambig_gv_added;*/
	}
	if(nnodes0!=nnodes1)
	{
		fprintf(stderr,"Oops: OperatorNodes got lost during expression "
			"tree creation: %d -> %d nodes (errors=%d)\n",
			nnodes0,nnodes1,errors);
		assert(0);
	}
	#endif
	
	if(!errors)
	{  this->root=garbage.last()->op;  }
	
	if(free_operatornodes)
	{
		// if errors: We simply free everything. Basta. 
		// if not: We free all OperatorNodes and unused Operators. 
		OperatorNode *on;
		int delops=0;
		while((on=garbage.popfirst()))
		{
			if(on->op && (errors || 
				(!on->op->up && !on->op->NChld() && on->op!=this->root) ))
			{  delete on->op;  ++delops;  }
			on->op=NULL;
			delete on;
		}
		#if TESTING
		if(!errors)   // If there are errors, then we delete them 
		{             // all and this->root is NULL. 
			int treeops=this->root->CountChildren()+1;
			if(treeops+delops!=nnodes0 && nnodes0==nnodes1)
			{
				fprintf(stderr,"Oops: Operators got lost during tree creation. "
					"total=%d; tree=%d; deleted=%d; lost=%d\n",
					nnodes0,treeops,delops,nnodes0-(treeops+delops));
				assert(0);
			}
		}
		#endif
	}
	
	exp->assign_list(&garbage);
	return(errors);
}


// Recursively computes the result of the (sub)expression *top. 
// Returns 0 on success; >0 on failure. 
int ExpressionTree::ComputeTree(Value *ret,Operator *top)
{
	if(top->type()==OTValue)
	{
		// We simply have to return the value. That's all. 
		assert(top->value());
		ret->assign(top->value());
		return(0);
	}
	
	// Okay, this is not a value. We have to recurse. 
	assert(top->function());   // must be set for operators and functions 
	FunctionCaller fcall(top->function(),ret);
	int errors=0;
	int nargs=top->NChld();
	for(int i=0; i<nargs; i++)
	{
		Value *tmpres=new Value();
		Operator *down=top->Down(i);
		errors+=ComputeTree(tmpres,down);   // recursive call 
		FunctionState fs=fcall.AddArg(tmpres);
		if(fs)
		{
			fprintf(stderr,"Oops: ComputeTree: failed to add arg for "
				"function: fs=%d (i=%d; nargs=%d; <%s,%d>)\n",
				fs,i,nargs,top->desc()->name,top->type());
			++errors;
			delete down;
		}
		if(errors)
		{  goto failed;  }
	}
	// errors=0 here. 
	{
		// Really call the function: 
		FunctionState fs=fcall.Compute();
		if(fs)
		{
			fprintf(stderr,"ComputeTree: failed to call function "
				"`%s': fs=%d\n",top->function()->name,fs);
			++errors;
		}
	}
	failed:
	// Must free the allocated values: 
	fcall.FreeArgs();
	return(errors);
}

int ExpressionTree::Compute(Value *val)
{
	if(!root)
	{  return(-1);  }
	return(ComputeTree(val,root));
}


// All operators/functions are variables. 
// Problem: what to do with rand(). 
#define IsVariable(op)  (op->type()==OTValue ? 0 : 1)

int ExpressionTree::OptimizeTree(Operator *top)
{
	int nvar=0;
	int errors=0;
	for(int i=0,nargs=top->NChld(); i<nargs; i++)
	{
		Operator *dn=top->Down(i);
		if(!dn)  continue;
		if(dn->type()!=OTValue)
		{  errors+=OptimizeTree(dn);  }
		if(IsVariable(dn))  ++nvar;
	}
	if(!nvar)
	{
		// No variables as args. We should be able to compute 
		// this subtree. 
		Value *tmp=new Value();
		int rv=ComputeTree(tmp,top);
		errors+=rv;
		if(rv)
		{  delete tmp;  }
		else
		{
			// tmp is the value of the *top operator. 
			// First: recursively delete the subtree: 
			for(int i=0,nargs=top->NChld(); i<nargs; i++)
			{
				Operator *dn=top->Down(i);
				if(!dn)  continue;
				_RecursiveClear(dn);
			}
			// Then, assign the value to the top operator: 
			top->AttachSubtreeResult(tmp);
		}
	}
	return(errors);
}

int ExpressionTree::Optimize()
{
	if(!root)
	{  return(-1);  }
	return(OptimizeTree(root));
}


void ExpressionTree::_RecursiveClear(Operator *top)
{
	for(int i=0; i<top->NChld(); i++)
	{
		Operator *down=top->Down(i);
		if(down)
		{  _RecursiveClear(down);  }
	}
	delete top;
}

void ExpressionTree::Clear()
{
	if(root)
	{  _RecursiveClear(root);  }
	root=NULL;
}


ExpressionTree::ExpressionTree()
{
	root=NULL;
}

ExpressionTree::~ExpressionTree()
{
	Clear();
}

}  // namespace end 
