/*
 * operator.hpp
 * 
 * Abstraction of operator tree node in expression tree 
 * for input lib / exparse. 
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

#ifndef _NS_ExParse_Operator_HPP_
#define _NS_ExParse_Operator_HPP_ 1

#include "function.hpp"

namespace exparse
{

class Operator
{
	private:
		union {
			//! 1 -> value attached; 0 -> no value attached
			int free_val;  //! only for values (OTValue)
			//! number of set operator children in down[] array 
			int nchld;     //! for all other operators
		}; union {
			Value *val;                  //! only for values (OTValue)
			const FunctionDesc *fdesc;   //! for all other operators
		};
		
		// NOTE: DO NOT USE odesc->fdesc AS THIS IS NULL FOR OTFunction. 
		//       ALWAYS USE fdesc TO GET THE FUNCTION FOR EVERY OPERATOR. 
		const OperatorDesc *odesc;
		OperatorType otype;
		Operator **down;  //! array of odesc->noperands elements initialized to NULL
		
		inline void _zap_val();
		inline void _SetType(OperatorType type);
	public:
		//! Up pointer (points into the direction of the tree root). 
		Operator *up;
		
		//! Set up an empty node: 
		Operator();
		//! Set up operator of passed type: 
		//! nd is set to a correct value (unless otype==OTFunction) 
		//! and down[] is allocated. (see also AddChild())
		Operator(OperatorType otype);
		//! Set up an Operator of type OTValue with specified value
		//! attach_val: true -> attach the value (like attach(val))
		//!            false -> assign the value (like assign(val))
		Operator(Value *val,bool attach_val=true);
		//! Set up operator of type OTFunction with the specified 
		//! function description. 
		//! initializes: down=NULL and nd=-1. 
		//! Call AddChild() to assign the parameters. 
		Operator(const FunctionDesc *fdesc);  
		~Operator();  //! does not delete the corresponding value
		
		//! Get down pointer with index idx (pointer to child[idx]): 
		Operator *Down(int idx)
			{  return((idx>=nchld || idx<0) ? NULL : (down[idx]));  }
		
		//! Get the number of children (max idx for Down() pointer). 
		int NChld()  {  return((otype==OTValue) ? 0 : nchld);  }
		
		//! Cound the children, sub-children, sub-sub-children, etc of 
		//! this operator. This operator itself is not counted. 
		int CountChildren();
		
		//! Get the operator type (_OTLast if not assigned). 
		OperatorType type() const  {  return(otype);  }
		
		//! Returns the currently assigned or attached value or NULL 
		//! if none is assigned/attached or type!=OTValue. 
		Value *value()
			{  return((otype==OTValue) ? val : NULL);  }
		
		//! Returns the function associated to the operator or NULL. 
		//! NULL is returned 
		//!  - for OTValue operators, 
		//!  - for uninitailized operators (_OTLast) and 
		//!  - for OTFunction operators if no function was assigned 
		//!    (see SetFunction())
		const FunctionDesc *function()
			{  return((otype==OTValue) ? NULL : fdesc);  }  // condition necessary beacuse of union 
		
		//! Returns the operator description for the operator or NULL 
		//! in case the operator is uninitialized (_OTLast). 
		//! DO NOT USE OperatorDesc::fdesc; 
		//! USE Operator::function() INSTEAD. 
		const OperatorDesc *desc()  {  return(odesc);  }
		
		//! Add a child operator (build up tree structure). 
		//! The child is put into the next free down[] pointer. 
		//! In case this is an OTFunction, you MUST have called 
		//! SetFunction() before or have set the FunctionDescription 
		//! in the constructor. 
		//! Return value: 
		//!  0 -> OK; child added 
		//! -1 -> This is an OTValue. Cannot add children. 
		//! -2 -> This operator does not have a type. 
		//!       Call SetType() first. 
		//! -3 -> Function not set. Call SetFunction() first. 
		//!  1 -> Cannot add more children. Operator does 
		//!       not take so many. 
		//! (Also assigns child->up=this.) 
		int AddChild(Operator *child);
		
		//! Set the type of this operator if not done by the 
		//! constructor. Fails if a different type is already set. 
		//! Return value: 0 -> success; 1 -> failed 
		int SetType(OperatorType type);
		
		//! Change the operator type, even if a different type is 
		//! already set. Fails if children/a value were/was already 
		//! added/assigned/attached. 
		int ForceSetType(OperatorType type);
		
		//! Set the function assigned to this Operator. Only allowed 
		//! if otype==OTFunction and no other function was previously 
		//! set. Returns 0 on success; 1 on error. 
		int SetFunction(const FunctionDesc *fdesc);
		
		//! Very special function used by the expression tree 
		//! optimizer. It will overwrite the current content of this 
		//! Operator whatever it is and set it to OTValue. Then, the 
		//! passed Value will be attach()ed. 
		//! Make sure to free (recursively delete) all the 
		//! sub-operators (down pointers; see Down()) before calling 
		//! this function as it will NOT free the subtrees (only 
		//! the down[] array making the down pointers inaccessible 
		//! after the call). 
		void AttachSubtreeResult(Value *v);
		
		//! Attach a value to an operator. This is only valid if the 
		//! otype==OTValue. An attached value gets deleted when the 
		//! Operator is destroyed. 
		//! Pass val=NULL to delete the current value and set none. 
		//! Returns 0 on success and 1 on failure (not OTValue). 
		int attach(Value *val);
		
		//! Detach the value: the value is returned but not deleted. 
		//! The internal value pointer is set to NULL; the operator 
		//! no longer refers to the value. 
		//! Returns NULL if otype!=OTValue or if no value is attached. 
		Value *detach();
		
		//! Assign a value to the Operator. This is like attaching a value 
		//! except that it does not get deleted by the Operator. 
		//! Pass NULL to assign no value. A currently attached value gets 
		//! deleted. 
		//! Returns 0 on success and 1 on failure (not OTValue). 
		int assign(Value *val);
};

}  // namespace end 

#endif  /* _NS_ExParse_Operator_HPP_ */
