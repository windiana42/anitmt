/*
 * core/a_ivaradb.cc
 * 
 * ANI internal variables for ADB blocks. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "animation.h"
#include "execthread.h"
#include "aniinstance.h"

using namespace ANI;


ExprValue AniInternalVariable_ADB::EAF_ADBVar::Eval(TNIdentifier *idf,
	ExecThreadInfo *info)
{
	AniInternalVariable_ADB *ivaradb=(AniInternalVariable_ADB*)idf->ani_scope;
	ANI::SpecialIdf_CoreHook *core_hook=ivaradb->adb_idf_core_hook;
	assert(core_hook);
	return(core_hook->Eval(idf,info));
}

ExprValue AniInternalVariable_ADB::EAF_ADBVar::Eval(TNIdentifier *idf,
	const ExprValue &base,ANI::ExecThreadInfo *)
{
	// Explicit member select may not happen. 
	assert(0);
	return ExprValue();
}

ExprValue &AniInternalVariable_ADB::EAF_ADBVar::GetRef(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	// These internal vars are no lvalues. 
	assert(0);
}

int AniInternalVariable_ADB::EAF_ADBVar::CheckIfEvaluable(
	ANI::CheckIfEvalInfo *)
{
	// Nothing to do here. 
	return(0);
}

AniInternalVariable_ADB::EAF_ADBVar::~EAF_ADBVar()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------


ExprValueType AniInternalVariable_ADB::GetType() const
{
	return(v_type);
}


int AniInternalVariable_ADB::CG_GetVariableType(VariableTypeInfo *vti) const
{
	vti->store_here->operator=(GetType());
	vti->is_lvalue=0;
	vti->is_const=0;
	vti->may_ieval=0;
	return(0);
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_ADB::CG_GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug)
{
	return(GetVarEvalAdrFuncFunc(idf_for_debug,/*ainc=*/NULL));
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_ADB::GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug,AniIncomplete * /*ainc*/)
{
	// We're a special (non-static and nowhere declared) variable 
	// which is neither a member nor on the stack. 
	
	// This assert makes sure that we can access ourselves from within 
	// the eval addressation function by following idf->ani_scope. 
	assert(idf_for_debug->ani_scope==this);
	
	return(new AniInternalVariable_ADB::EAF_ADBVar());
}


void AniInternalVariable_ADB::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(GetType().TypeString().str());
	d->Append(" ");
	d->Append(name.str());
	d->Append(":");
	d->Append(" (ADB special)");
	d->Append("\n");
}


AniInternalVariable_ADB::AniInternalVariable_ADB(
	const RefString &_name,
	ANI::TreeNode *_treenode,AniScopeBase *_parent,
	const ANI::ExprValueType &_v_type,
	ANI::SpecialIdf_CoreHook *_adb_idf_core_hook) : 
	AniInternalVariable(_name,_treenode,_parent),
	v_type(_v_type)
{
	adb_idf_core_hook=_adb_idf_core_hook;
	if(adb_idf_core_hook)
	{  adb_idf_core_hook->aqref();  }
}

AniInternalVariable_ADB::~AniInternalVariable_ADB()
{
	if(adb_idf_core_hook)
	{
		adb_idf_core_hook->deref();
		adb_idf_core_hook=NULL;
	}
}
