/*
 * ani-parser/treereg.h
 * 
 * Tree registration-specific header. 
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

#ifndef _ANIVISION_ANI_TREEREG_H_
#define _ANIVISION_ANI_TREEREG_H_

#include <ani-parser/coreglue.h>


namespace ANI
{

class TNAniDescStmt;


// As passed to TreeNode's virtual DoRegistration(). 
struct TRegistrationInfo
{
	enum TRegistrationLevel
	{
		// NOTE: TRL_Optimize MUST BE FIRST PASS; we may otherwise remove 
		//       nodes which already got referenced. 
		TRL_Optimize=0,  // Optimization step: cut out empty stmts, etc. 
		TRL_Animation,
		TRL_Settings,
		TRL_Objects,
		TRL_Functions,
		TRL_MemberVars,
		TRL_AutoVars,  // <-- and func args
		TRL_AllIdentifiers,  // All the identifiers (which have no ani_scope yet). 
		_TRL_LAST
	};
	
	// Who should register now: 
	TRegistrationLevel who;
	
	// The hierarchy up; pointers are NULL if not available. 
	AniAniSetObj *animation;
	AniAniSetObj *setting;
	AniAniSetObj *object;
	AniFunction *function;
	AniAnonScope *anonscope;
	// The "ADB root", i.e. pointer to the AniDescStmt we're currently in: 
	TNAniDescStmt *anidescstmt;
	
	// Error counter: 
	int n_errors;
	// Number of tree nodes deleted during TRL_Optimize. 
	int tn_opt_deleted;
	
	// Variable sequence number counter. 
	// Read comment in class AniVariable for more info. 
	int var_seq_num;
	
	// List held by the below LinkedList. 
	struct SpecialIdf : LinkedListBase<SpecialIdf>
	{
		const char *name;     // identifier name (what to look up)
		ExprValueType vtype;  // type of the variable
		
		// Hook for calc core to hang data (gets passed to 
		// AniInternalVariable_ADB). 
		SpecialIdf_CoreHook *adb_idf_core_hook;  
		
		_CPP_OPERATORS
		SpecialIdf();
		~SpecialIdf() {}  // Does not touch adb_idf_core_hook. 
	};
	
	// List of special identifiers to recognize during registration 
	// TRL_AllIdentifiers step. List is searched LAST-TO-FIRST and the 
	// first match is used. 
	LinkedList<SpecialIdf> special_idf_list;
	
	_CPP_OPERATORS
	TRegistrationInfo();
	~TRegistrationInfo();
	
	// Return innermost scope from the Ani* vars. 
	AniGlue_ScopeBase *GetInnermostScope();
	
	private:  // Do not use: 
		TRegistrationInfo &operator=(const TRegistrationInfo &){return(*this);}
		TRegistrationInfo(const TRegistrationInfo &){}
};

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_TREEREG_H_ */
