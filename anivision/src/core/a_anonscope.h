/*
 * core/a_anonscope.h
 * 
 * ANI anonymous scope. 
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

#ifndef _ANIVISION_CORE_ANIANONSCOPE_H_
#define _ANIVISION_CORE_ANIANONSCOPE_H_ 1

#include <hlib/cplusplus.h>

#include <core/aniscope.h>


// Anonymous scope. 
// Each function contains at least one anonymous scope; each 
// compound stmt is one. 
// Can NOT be inside AniAniSetObj. 
// Can only be inside AniFunction, currently. 
class AniAnonScope : 
	public AniScopeBase,  // <-- MUST BE FIRST. 
	public LinkedListBase<AniAnonScope>  // list: parent
{
	friend class AniScopeBase;
	public:
		enum AnonScopType
		{
			AS_AnonScope,
			AS_FunctionScope,
		};
	private:
		// Contains all vars local to this scope ([normally] automatic). 
		_AniComponentList<AniVariable> variablelist;
		// Contains all anonymous sub-scopes: 
		_AniComponentList<AniAnonScope> anonscopelist;
		// Contains incomplete (not yet resolved) identifiers: 
		_AniComponentList<AniIncomplete> incompletelist;
		// See aniscope.h/struct TypeFixEntry for more info: 
		LinkedList<TypeFixEntry> typefix_list;
		
		// Type of the anonymous scope: 
		AnonScopType scopetype;
		
		// See InstanceInfo in aniscope.h: 
		InstanceInfo *iinfo;
		
		// [overriding a virtual]
		void QueueChild(AniScopeBase *asb,int);  // Only used by AniScopeBase constructor.
		
		// [overriding virtuals from AniGlue_ScopeBase]: 
		void CG_ExecEnterCompound(ANI::ExecThreadInfo *info);
		void CG_ExecLeaveCompound(ANI::ExecThreadInfo *info);
		const char *CG_AniScopeTypeStr() const;
	public:
		AniAnonScope(ANI::TreeNode *_treenode,AniScopeBase *_parent);
		~AniAnonScope();
		
		// [overriding virtuals]
		void LookupName(const RefString &name,ASB_List *ret_list,
			int query_parent);
		void AniTreeDump(StringTreeDump *d);
		ANI::TNLocation NameLocation() const;
		AniScopeBase *RegisterTN_DoFinalChecks(ANI::TRegistrationInfo *tri);
		int CountScopes(int &n_incomplete) const;
		int FixIncompleteTypes();
		LinkedList<TypeFixEntry> *GetTypeFixList();
		int PerformCleanup(int cstep);
		InstanceInfo *GetInstanceInfo();
		int InitStaticVariables(ANI::ExecThreadInfo *info,int flag);
};

#endif  /* _ANIVISION_CORE_ANIANONSCOPE_H_ */
