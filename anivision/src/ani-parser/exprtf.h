/*
 * ani-parser/exprtf.h
 * 
 * Type fixup, const folding, subexpression evaluation - specific header. 
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

// TF = Type Fixup

#ifndef _ANIVISION_ANI_EXPRTF_H_
#define _ANIVISION_ANI_EXPRTF_H_ 1

#include <ani-parser/exprtype.h>

namespace ANI
{

// As passed to virtual TreeNode::DoExprTF(): 
struct TFInfo
{
	// Number of nodes visited: 
	int n_nodes_visited;
	// If we're inside an initialisation statement or not. 
	// NOTE: Actually unused; just for bug traps. 
	int in_init;   // : 1
	
	// The "ADB root", i.e. pointer to the AniDescStmt we're currently in: 
	TNAniDescStmt *anidescstmt;   // ...or NULL
	
	// This is set by DoExprTF() if the return value is 0. 
	struct Type
	{
		// Subexpression value type: 
		// May not be EVT_Unknown unless error occured (retval>0). 
		// (Even functions are passed through here using the ani scope.)
		// THIS MAY GENERALLY NOT BE AniVariable UNLESS WE ALLOW POINTERS. 
		// It may be AniIncomplete for the unresolved function call. 
		ExprValueType sevt;
		// Is the result an lvalue?
		// NOTE: THIS IS INDEPENDENT ON ExprValueType. My original plans 
		// that only EVT_AniScope can be an lvalue are superseded. 
		// NOTE that is_lvalue is set for variables even if they are 
		// const (i.e. is_const set). This allows for better error 
		// messages. 
		int is_lvalue : 1;
		// Is the result a constant (i.e. can be subexpression-evaluated)?
		// NOTE: Actually unused except to see if expr can be assigned 
		//       or inc/decremented. Need not be valid otherwise. 
		int is_const : 1;
		// Does the expression have an effect? 
		// Gets checked for statements. 
		int has_effect : 1;
		// Try most trivial subexpression evaluation, i.e. only 
		// immediate constant folding; nothing with identifiers in it. 
		// If this is 0, it makes no sense to try. 
		int try_ieval : 1;
		
		_CPP_OPERATORS
		Type(const Type &t) : sevt(t.sevt),is_lvalue(t.is_lvalue),
			is_const(t.is_const),has_effect(t.has_effect),
			try_ieval(t.try_ieval) { }
		Type() : sevt()  { is_lvalue=0; is_const=0; has_effect=0;
			try_ieval=1; }
		~Type() {}
		Type &operator=(const Type &t)
			{  sevt=t.sevt; is_lvalue=t.is_lvalue; is_const=t.is_const;
				has_effect=t.has_effect; try_ieval=t.try_ieval;
				return(*this);  }
	}__attribute__((__packed__)) tp;
	
	_CPP_OPERATORS
	TFInfo();
	~TFInfo();
};

}  // End of namespace ANI. 

#endif  /* _ANIVISION_ANI_EXPRTF_H_ */
