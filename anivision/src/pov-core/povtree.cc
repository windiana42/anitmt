/*
 * pov-core/povtree.cc
 * 
 * POV command comment tree implementation. 
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

#include "povtree.h"
#include <ani-parser/exprtf.h>


namespace POV
{

void POVTreeNode::DumpTree(StringTreeDump *d)
{
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{
		tn->DumpTree(d);
		d->Append("\n");
	}
}

ANI::TreeNode *POVTreeNode::CloneTree()
{
	// May only be called if there is no derived class. 
	assert(pntype==PTN_None);
	
	TreeNode *tn=new POVTreeNode(pntype);
	return(_CloneChildren(tn));
}


//------------------------------------------------------------------------------

void PTNExpression::DumpTree(StringTreeDump *d)
{
	assert(down.count()==1);
	down.first()->DumpTree(d);
}

ANI::TreeNode *PTNExpression::CloneTree()
{
	TreeNode *tn=new PTNExpression(NULL);
	return(_CloneChildren(tn));
}

void PTNExpression::DoRegistration(ANI::TRegistrationInfo *tri)
{
	down.first()->DoRegistration(tri);
}

int PTNExpression::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	return(down.first()->DoExprTF(ti));
}

int PTNExpression::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	return(down.first()->CheckIfEvaluable(ci));
}


//------------------------------------------------------------------------------

void PTNAssignment::DumpTree(StringTreeDump *d)
{
	assert(down.count()==2);
	down.first()->DumpTree(d);
	d->Append("=");
	down.last()->DumpTree(d);
}

ANI::TreeNode *PTNAssignment::CloneTree()
{
	TreeNode *tn=new PTNAssignment(NULL,NULL);
	return(_CloneChildren(tn));
}

void PTNAssignment::DoRegistration(ANI::TRegistrationInfo *tri)
{
	// We may NOT register down.first() because this 
	// identifier is for POV namespace only. 
	down.last()->DoRegistration(tri);
}

int PTNAssignment::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	// Don't look at POV-space identifier in down.first(). 
	return(down.last()->DoExprTF(ti));
}

int PTNAssignment::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	// Don't look at POV-space identifier in down.first(). 
	return(down.last()->CheckIfEvaluable(ci));
}


//------------------------------------------------------------------------------

const char *PTNObjectEntry::ASymbolStr(PTNObjectEntry::ASymbol as)
{
	switch(as)
	{
		case AS_None:    return("[none]");
		case AS_Macro:   return("macro");
		case AS_Declare: return("declare");
		case AS_Params:  return("params");
		case AS_Defs:    return("defs");
		case AS_Include: return("include");
		case AS_Type:    return("type");
		case AS_AppendRaw: return("append_raw");
	}
	return("???");
}

void PTNObjectEntry::DumpTree(StringTreeDump *d)
{
	d->Append(ASymbolStr(asymbol));
	d->Append("=");
	switch(asymbol)
	{
		case AS_None:  break;
		case AS_Macro:
		case AS_Declare:
		case AS_Type:
		case AS_AppendRaw:
			assert(down.count()==1);
			down.first()->DumpTree(d);
			break;
		case AS_Params:
		case AS_Defs:
		case AS_Include:
			d->Append("{");
			for(TreeNode *tn=down.first(); tn; tn=tn->next)
			{
				tn->DumpTree(d);
				if(tn->next)  d->Append(",");
			}
			d->Append("}");
			break;
		default: assert(0);
	}
}

ANI::TreeNode *PTNObjectEntry::CloneTree()
{
	TreeNode *tn=new PTNObjectEntry(asymbol,loc0);
	return(_CloneChildren(tn));
}

ANI::TNLocation PTNObjectEntry::GetLocationRange() const
{
	ANI::TNLocation loc;
	
	loc=loc0;
	if(down.last())
	{  loc.pos1=down.last()->GetLocationRange().pos1;  }
	
	return(loc);
}

void PTNObjectEntry::DoRegistration(ANI::TRegistrationInfo *tri)
{
	switch(asymbol)
	{
		case AS_None:
			break;
		case AS_Macro:    // fall through
		case AS_Declare:
			// macro,declare: We may NOT register down.first() because 
			// this identifier is for POV namespace only. 
			break;
		case AS_Params:  // fall through
		case AS_Defs:
		case AS_Include:
		case AS_Type:
		case AS_AppendRaw:
			for(TreeNode *tn=down.first(); tn; tn=tn->next)
			{  tn->DoRegistration(tri);  }
			break;
		default:  assert(0);
	}
}

int PTNObjectEntry::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	switch(asymbol)
	{
		case AS_None:
			break;
		case AS_Macro:    // fall through
		case AS_Declare:
			// macro,declare: Don't look at POV-space identifier. 
			break;
		case AS_Params:  // fall through
		case AS_Defs:
		case AS_Include:
		case AS_Type:
		case AS_AppendRaw:
			for(TreeNode *tn=down.first(); tn; tn=tn->next)
			{  if(tn->DoExprTF(ti)) ++fail;  }
			break;
		default:  assert(0);
	}
	return(fail);
}

int PTNObjectEntry::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	int fail=0;
	switch(asymbol)
	{
		case AS_None:
			break;
		case AS_Macro:    // fall through
		case AS_Declare:
			// macro,declare: Don't look at POV-space identifier. 
			break;
		case AS_Params:  // fall through
		case AS_Defs:
		case AS_Include:
		case AS_Type:
		case AS_AppendRaw:
			for(TreeNode *tn=down.first(); tn; tn=tn->next)
			{  fail+=tn->CheckIfEvaluable(ci);  }
			break;
		default:  assert(0);
	}
	return(fail);
}


//------------------------------------------------------------------------------

void PTNObjectSpec::DumpTree(StringTreeDump *d)
{
	d->Append("@object(\n");
	d->AddIndent();
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{
		tn->DumpTree(d);
		if(tn->next)  d->Append(",\n");
	}
	d->SubIndent();
	d->Append(")\n");
}

ANI::TreeNode *PTNObjectSpec::CloneTree()
{
	TreeNode *tn=new PTNObjectSpec(loc0);
	return(_CloneChildren(tn));
}

ANI::TNLocation PTNObjectSpec::GetLocationRange() const
{
	ANI::TNLocation loc;
	
	loc=loc0;
	if(down.last())
	{  loc.pos1=down.last()->GetLocationRange().pos1;  }
	
	return(loc);
}

void PTNObjectSpec::DoRegistration(ANI::TRegistrationInfo *tri)
{
	// Register the object entries: 
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  tn->DoRegistration(tri);  }
}

int PTNObjectSpec::DoExprTF(ANI::TFInfo *ti,ANI::TreeNode ** /*update_tn*/)
{
	++ti->n_nodes_visited;
	
	int fail=0;
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  if(tn->DoExprTF(ti))  ++fail;  }
	return(fail);
}

int PTNObjectSpec::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	int fail=0;
	for(TreeNode *tn=down.first(); tn; tn=tn->next)
	{  fail+=tn->CheckIfEvaluable(ci);  }
	return(fail);
}


//------------------------------------------------------------------------------

const char *PTNGeneralCmd::PCommandStr(PTNGeneralCmd::PCommand pc)
{
	switch(pc)
	{
		case PC_None:    return("[none]");
		case PC_FileID:  return("@fileID");
	}
	return("???");
}

void PTNGeneralCmd::DumpTree(StringTreeDump *d)
{
	d->Append(PCommandStr(pcmd));
	d->Append("=");
	assert(down.count()==1);
	down.first()->DumpTree(d);
}

ANI::TreeNode *PTNGeneralCmd::CloneTree()
{
	TreeNode *tn=new PTNGeneralCmd(pcmd,loc0);
	return(_CloneChildren(tn));
}

ANI::TNLocation PTNGeneralCmd::GetLocationRange() const
{
	ANI::TNLocation loc;
	
	loc=loc0;
	if(down.last())
	{  loc.pos1=down.last()->GetLocationRange().pos1;  }
	
	return(loc);
}

}  // end of namespace POV
