/*
 * core/a_incomplete.h
 * 
 * ANI tree: incomplete place holder. 
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

#ifndef _ANIVISION_CORE_ANIINCOMPLETE_H_
#define _ANIVISION_CORE_ANIINCOMPLETE_H_ 1

#include <hlib/cplusplus.h>

#include <core/aniscope.h>


// Used for incomplete identifiers. E.g. in registration step, I cannot 
// look up the function because the type of the args is not yet known. 
// Hence, use an AniIncomplete which occupies the correct position in 
// the ani scope tree and resolve the function in the TF step. 
class AniIncomplete : public AniScopeBase,  // <-- MUST BE FIRST. 
	public LinkedListBase<AniIncomplete>  // list: parent
{
	friend class AniScopeBase;
	private:
		// Read comment in AniVariable. 
		int var_seq_num;
		
		// This is lookup info for variables and functions. 
		// The lookup is done from *this location but we may 
		// be inside a member_select/mapping from a different 
		// location: 
		AniScopeBase *ms_from;
		// ms_quality specifies how the member select is: 
		//  0 -> none (ms_from=NULL only)
		//  1 -> implicit, i.e. inside object without explicit spec
		//  2 -> explicit, i.e. OBJ.name or this.name. 
		// If ms_from is not an AniAniSetObj, an error is written. 
		int ms_quality : 5;
		
		// The incomplete type; see <coreglue.h>. 
		IncompleteType incomplete_type : 5;
		
		// What to do with idf_to_update: 
		int itu_for_fcall : 1;  // for function call
		
		// Update the eval func of this identifier when the 
		// action corresponding to the value above is resolved 
		// -- or NULL. 
		ANI::TNIdentifier *idf_to_update;
		
		// Internally used by CG_LookupIdentifier(): 
		// Returns NULL on failure. 
		AniScopeBase *_LookupIdentifier_Normal(LookupIdentifierInfo *lii);
		// Return value: 
		//  1 -> special identifier "this", object stored in *ascope. 
		//  2 -> special idf "NULL" found, *ascope=NULL. 
		//  0 -> no special identifier, use _LookupIdentifier_Normal(). 
		// -1 -> error
		int _LookupIdentifier_Special(LookupIdentifierInfo *lii,
			AniScopeBase **ascope);
		
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
		// [overriding virtuals from AniGlue_ScopeBase]
		int CG_LookupFunction(LookupFunctionInfo *lfi);
		void CG_LookupIdentifier(LookupIdentifierInfo *lii);
		IncompleteType CG_IncompleteType() const;
		const char *CG_AniScopeTypeStr() const;
	public:
		// ms_from and ms_quality may be set to NULL,0. 
		AniIncomplete(ANI::TreeNode *_treenode,AniScopeBase *_parent,
			int _var_seq_num,AniScopeBase *_ms_from,int _ms_quality);
		~AniIncomplete();
		
		// Internally used for self-removal: Update the attached 
		// TreeNode with the new scope and then delete *this 
		// (automatically dequeuing from the tree just as all 
		// AniScopeBases do). 
		// DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING. 
		void _TNUpdate_SelfRemoval(AniScopeBase *update_base);
		
		// [overriding virtuals]
		void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		void AniTreeDump(StringTreeDump *d);
		ANI::TNLocation NameLocation() const;
		AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		int CountScopes(int &n_incomplete) const;
		int FixIncompleteTypes();
		int PerformCleanup(int cstep);
};

#endif  /* _ANIVISION_CORE_ANIINCOMPLETE_H_ */
