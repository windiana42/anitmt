/*
 * pov-core/povtree.h
 * 
 * POV command comment tree representation. 
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

#ifndef _ANIVISION_POV_TREE_H_
#define _ANIVISION_POV_TREE_H_ 1

#include <ani-parser/tree.h>

namespace POV
{

// POVTreeNode type: 
enum PTNType
{
	PTN_None=0,
	PTN_Expression,
	PTN_Assignment,
	PTN_ObjectEntry,
	PTN_ObjectSpec,
	PTN_GeneralCmd,
	
};

struct POVTreeNode : ANI::TreeNode
{
	public:
		static const ANI::TNType tn_type=ANI::TN_POV;
	private:
		PTNType pntype;
	public:
		POVTreeNode(PTNType _pntype) : ANI::TreeNode(ANI::TN_POV),
			pntype(_pntype)
			{  }
		// Special version for PTN_None: 
		POVTreeNode(ANI::TreeNode *child) : ANI::TreeNode(ANI::TN_POV),
			pntype(PTN_None)
			{  AddChild(child);  }
		virtual ~POVTreeNode()
			{ }
		
		// Get POVTreeNode type: 
		PTNType PNType() const
			{  return(pntype);  }
		
		// Should be overridden by derived class: 
		virtual void DumpTree(StringTreeDump *d);
		virtual TreeNode *CloneTree();
		void DumpTree(FILE *out)
			{  ANI::TreeNode::DumpTree(out);  }
		// Furthermore, if needed: 
		// void DoRegistration(ANI::TRegistrationInfo *tri);
		// int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		// int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
		
		// Should be overridden by derived class: 
		// Get start and end location of complete subtree. [overriding a virtual]
		//virtual TNLocation GetLocationRange() const;  // <-- default implementation
};


//------------------------------------------------------------------------------

class PTNExpression : public POVTreeNode
{
	// ONE CHILD: ANI::TNExpression. 
	private:
	public:
		PTNExpression(ANI::TNExpression *_expr) : POVTreeNode(PTN_Expression)
			{  if(_expr)  AddChild(_expr);  }
		~PTNExpression()
			{  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		//ANI::TNLocation GetLocationRange() const; <-- default implementation
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
};


class PTNAssignment : public POVTreeNode
{
	// CHILDREN: exactly 2. 
	//   first:  TNIdentifier
	//   second: PTNExpression
	private:
	public:
		PTNAssignment(ANI::TNIdentifier *idf,PTNExpression *expr) : 
			POVTreeNode(PTN_Assignment)
			{  if(idf) AddChild(idf);  if(expr) AddChild(expr);  }
		~PTNAssignment()
			{  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		//ANI::TNLocation GetLocationRange() const; <-- default implementation
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
};


class PTNObjectEntry : public POVTreeNode
{
	// CHILDREN: Depending on type. 
	//   AS_None -> no children (may not happen)
	//   AS_Macro,AS_Declare -> 1 child: TNIdentifier
	//   AS_Params -> any number of PTNExpression
	//   AS_Defs -> any number of PTNAssignment
	//   AS_Include -> any number of PTNExpression (should be strings)
	//   AS_Type -> 1 child: PTNExpression
	//   AS_AppendRaw -> 1 child: PTNExpression
	public:
		enum ASymbol  // "a" -> "assignment"
		{
			AS_None=0,   // may not happen...
			AS_Macro,
			AS_Declare,
			AS_Params,
			AS_Defs,
			AS_Include,
			AS_Type,
			AS_AppendRaw,
			_AS_LAST  // MUST BE LAST
		};
		// Return string representation: 
		static const char *ASymbolStr(ASymbol as);
	private:
		ANI::TNLocation loc0;  // of the left part of the "assignment"
	public:
		// Symbol on the left side: 
		ASymbol asymbol;
	public:
		PTNObjectEntry(ASymbol _as,const ANI::TNLocation &_loc0,
			ANI::TreeNode *child=NULL) : 
			POVTreeNode(PTN_ObjectEntry),loc0(_loc0),asymbol(_as)
			{  if(child)  AddChild(child);  }
		~PTNObjectEntry()
			{  }
		
		ASymbol GetSymbol() const
			{  return(asymbol);  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		ANI::TNLocation GetLocationRange() const;
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
};


class PTNObjectSpec : public POVTreeNode
{
	// CHILDREN: 
	//   any number of PTNObjectEntry nodes. 
	private:
		ANI::TNLocation loc0;  // of the "@"
	public:
		PTNObjectSpec(const ANI::TNLocation &_loc0) : 
			POVTreeNode(PTN_ObjectSpec),loc0(_loc0)
			{  }
		~PTNObjectSpec()
			{  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		ANI::TNLocation GetLocationRange() const;
		void DoRegistration(ANI::TRegistrationInfo *tri);
		int DoExprTF(ANI::TFInfo *ti,ANI::TreeNode **update_tn=NULL);
		int CheckIfEvaluable(ANI::CheckIfEvalInfo *ci);
};


class PTNGeneralCmd : public POVTreeNode
{
	// CHILDREN: Depends on type, see below
	public:
		enum PCommand
		{
			PC_None=0,   // may not happen
			PC_FileID,   // child: ONE TNIdentifier
		};
		// Return string representation: 
		static const char *PCommandStr(PCommand pc);
	private:
		ANI::TNLocation loc0;  // of the command string
	public:
		PCommand pcmd;
	public:
		PTNGeneralCmd(PCommand _pc,const ANI::TNLocation &_loc0,
			ANI::TreeNode *child=NULL) : 
			POVTreeNode(PTN_GeneralCmd),loc0(_loc0),pcmd(_pc)
			{  if(child)  AddChild(child);  }
		~PTNGeneralCmd()
			{  }
		
		// [overriding virtuals:]
		void DumpTree(StringTreeDump *d);
		TreeNode *CloneTree();
		ANI::TNLocation GetLocationRange() const;
};


}  // end of namespace POV

#endif  /* _ANIVISION_POV_TREE_H_ */
