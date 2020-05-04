/*
 * ani-parser/treeloc.cc
 * 
 * Compute location ranges for tree nodes. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "tree.h"


namespace ANI
{

TNLocation TreeNode::GetLocationRange() const
{
	TNLocation l(TNLocation::NullLoc);
	
	if(down.first())
	{  l.pos0=down.first()->GetLocationRange().pos0;  }
	
	if(down.last())
	{  l.pos1=down.last()->GetLocationRange().pos1;  }
	
	return(l);
}


TNLocation TNIdentifierSimple::GetLocationRange() const
{
	return(loc);
}


TNLocation TNIdentifier::GetLocationRange() const
{
	TNLocation l;
	// NOTE: for TNIdentifier, it is the other way round (LAST::FIRST). 
	l.pos0=down.last()->GetLocationRange().pos0;
	if(!l.pos0 && down.last()->prev)
	{  l.pos0=down.last()->prev->GetLocationRange().pos0;  }
	l.pos1=down.first()->GetLocationRange().pos1;
	return(l);
}


TNLocation TNExpression::GetLocationRange() const
{
	// Hmmm... maybe a pitfall here. Use default meanwhile. 
	return(TreeNode::GetLocationRange());
}


TNLocation TNValue::GetLocationRange() const
{
	return(loc);
}


TNLocation TNArray::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc0.pos0;
	l.pos1=loc1.pos1;
	return(l);
}


TNLocation TNTypeSpecifier::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc.pos0;
	// This is not completely correct because I 
	// do not respect the closing ">" in e.g. "vector<3>". 
	if(down.last())
	{  l.pos1=down.last()->GetLocationRange().pos1;  }
	if(!l.pos1)
	{  l.pos1=loc.pos1;  }
	return(l);
}


TNLocation TNOperatorFunction::GetLocationRange() const
{
	TNLocation l;
	if(down.first())
	{  l.pos0=down.first()->GetLocationRange().pos0;  }
	if(!l.pos0)  // should not happen, IMO
	{  l.pos0=loc0.pos0;  }
	if(down.last())
	{  l.pos1=down.last()->GetLocationRange().pos1;  }
	if(!l.pos1)
	{  l.pos1=(!loc1.pos1 ? loc0.pos1 : loc1.pos1);  }
	return(l);
}


TNLocation TNStatement::GetLocationRange() const
{
	TNLocation l;
	if(down.first())
	{  l.pos0=down.first()->GetLocationRange().pos0;  }
	if(!l.pos0)
	{  l.pos0=Sloc.pos0;  }
	l.pos1=Sloc.pos1;
	return(l);
}


TNLocation TNIterationStmt::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc0.pos0;
	l.pos1=loop_stmt->GetLocationRange().pos1;
	return(l);
}


TNLocation TNSelectionStmt::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc0.pos0;
	l.pos1=(else_stmt ? else_stmt : if_stmt)->GetLocationRange().pos1;
	return(l);
}


TNLocation TNJumpStmt::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc.pos0;
	if(down.last())
	{  l.pos1=down.last()->GetLocationRange().pos1;  }
	if(!l.pos1)
	{  l.pos1=loc.pos1;  }
	return(l);
}


TNLocation TNCompoundStmt::GetLocationRange() const
{
	TNLocation l;
	l.pos0=loc0.pos0;
	l.pos1=loc1.pos1;
	return(l);
}


TNLocation TNDeclarator::GetLocationRange() const
{
	switch(decl_type)
	{
		case DT_None:  assert(0);  break;
		case DT_Name:
			return(down.first()->GetLocationRange());
		case DT_Array:
			return(down.first()->GetLocationRange());
		case DT_Initialize:
		{
			TNLocation l;
			l.pos0=down.first()->GetLocationRange().pos0;
			l.pos1=down.last()->GetLocationRange().pos1;
			return(l);
		}
		default:  assert(0);
	}
	return TNLocation();
}


TNLocation TNNewArraySpecifier::GetLocationRange() const
{
	return(loc);
}


TNLocation TNDeclarationStmt::GetLocationRange() const
{
	TNLocation l=TNStatement::GetLocationRange();
	if(!!aloc.pos0)
	{  l.pos0=aloc.pos0;  }
	return(l);
}


TNLocation TNAniDescStmt::GetLocationRange() const
{
	return(TNLocation(loc0.pos0,Sloc.pos1));
}


TNLocation TNFunctionDef::GetLocationRange() const
{
	TNLocation l;
	// For "~fname()" this is not true becuase we did not 
	// save the location of the "~" token. 
	// (In the mentioned case, ret_type is NULL.)
	if(ret_type)
	{  l.pos0=ret_type->GetLocationRange().pos0;  }
	if(!l.pos0)
	{  l.pos0=func_name->GetLocationRange().pos0;  }
	l.pos1=func_body->GetLocationRange().pos1;
	return(l);
}


TNLocation TNObject::GetLocationRange() const
{
	TNLocation l;
	l.pos0=down.first()->GetLocationRange().pos0;
	l.pos1=loc1.pos1;
	return(l);
}


TNLocation TNSetting::GetLocationRange() const
{
	TNLocation l;
	l.pos0=down.first()->GetLocationRange().pos0;
	l.pos1=loc1.pos1;
	return(l);
}

}  // End of namespace ANI. 
