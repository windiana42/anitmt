/*
 * pov-core/objectspec.h
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

#ifndef _ANIVISION_POV_OBJECTSPEC_H_
#define _ANIVISION_POV_OBJECTSPEC_H_

#include <hlib/refstring.h>
#include <hlib/linkedlist.h>
#include <pov-core/povtree.h>

namespace POV
{

// Object spec representation (POV namespace). 
// This is generated from a CCommand by the registration 
// function _Register_ObjectSpec() and represents an object entry 
// in some POV file (i.e. "prototype"). 
struct ObjectSpec : LinkedListBase<ObjectSpec>
{
	public:
		enum ObjectType
		{
			OT_None=0,     // may not happen
			OT_Macro,      // object as #macro
			OT_Declare,    // object as #declare
		};
		static const char *ObjectTypeStr(ObjectType ot);

	public:
		// Associated object entry: 
		PTNObjectSpec *ospec;
		// Type of object and name in POV space: 
		ObjectType otype;
		RefString oname;

	public:  _CPP_OPERATORS
		ObjectSpec(PTNObjectSpec *_ospec);
		~ObjectSpec();
		
		// Get PTNObjectEntry (tree node) with specified 
		// PTNObjectEntry::ASymbol or NULL if nonexistant 
		// for this object spec: 
		PTNObjectEntry *GetEntryByASymbol(PTNObjectEntry::ASymbol asymbol);
		
		// Clone the object spec tree: 
		// (Also calls TreeNode::RemoveFromHeadNodes(), so you have the 
		// responsibility for the tree and need to delete it using 
		// TreeNode::DeleteTree().) 
		PTNObjectSpec *CloneObjSpec();
		
		// Get location of the POV command: 
		ANI::TNLocation GetLocationRange() const
			{  return(ospec->GetLocationRange());  }
};

}  // end of namespace POV

#endif  /* _ANIVISION_POV_OBJECTSPEC_H_ */
