/*
 * pov-core/objectspec.cc
 * 
 * POV command comment @object() command. 
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

#include "objectspec.h"

namespace POV
{

const char *ObjectSpec::ObjectTypeStr(ObjectType ot)
{
	switch(ot)
	{
		case OT_None:     return("[none]");
		case OT_Macro:    return("macro");
		case OT_Declare:  return("declare");
	}
	return("???");
}


PTNObjectEntry *ObjectSpec::GetEntryByASymbol(PTNObjectEntry::ASymbol asymbol)
{
	for(ANI::TreeNode *tn=ospec->down.first(); tn; tn=tn->next)
	{
		PTNObjectEntry *oent=(PTNObjectEntry*)tn;
		assert(oent->PNType()==PTN_ObjectEntry);
		if(oent->GetSymbol()==asymbol)
		{  return(oent);  }
	}
	return(NULL);
}


PTNObjectSpec *ObjectSpec::CloneObjSpec()
{
	PTNObjectSpec *cl=(PTNObjectSpec*)ospec->CloneTree();
	ANI::TreeNode::RemoveFromHeadNodes(cl);
	return(cl);
}


ObjectSpec::ObjectSpec(PTNObjectSpec *_ospec) : 
	LinkedListBase<ObjectSpec>(),otype(OT_None),oname()
{
	ospec=_ospec;
}

ObjectSpec::~ObjectSpec()
{
	// empty
}

}  // End of namespace POV. 
