/*
 * core/a_ivaraniset.cc
 * 
 * ANI internal variables inside setting and animation. 
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


static void _DoRegisterVar(AniAniSetObj *parent,int bvi,const char *name)
{
	RefString tmp;
	tmp.set(name);
	new AniInternalVariable_AniSet(tmp,bvi,NULL,parent);
}


void AniInternalVariable_AniSet::_RegisterInternalVariables(
	AniAniSetObj *parent)
{
	if(parent->ASType()==AST_Setting)
	{
		_DoRegisterVar(parent,AniSetting::BVI_Time,"$time");
	}
	else if(parent->ASType()==AST_Animation)
	{
		_DoRegisterVar(parent,AniAnimation::BVI_Time,"$time");
	}
}

//------------------------------------------------------------------------------

inline ExprValue &AniInternalVariable_AniSet::EAF_IVarSetting::_DoEval(
	TNIdentifier *idf)
{
	assert(((AniScopeBase*)idf->ani_scope)->ASType()==AST_InternalVar);
	AniSetting *set=(AniSetting*)(
		((AniInternalVariable_AniSet*)idf->ani_scope)->Parent());
	
	assert(set->ASType()==AST_Setting);
	
	return( set->GetInternalVarRef(bvi) );
}

ExprValue AniInternalVariable_AniSet::EAF_IVarSetting::Eval(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	// This will use the copy constructor: 
	return( _DoEval(idf) );
}

ExprValue AniInternalVariable_AniSet::EAF_IVarSetting::Eval(TNIdentifier *idf,
	const ExprValue &/*base*/,ANI::ExecThreadInfo *)
{
	// This will use the copy constructor: 
	return( _DoEval(idf) );
}

ExprValue &AniInternalVariable_AniSet::EAF_IVarSetting::GetRef(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	// This will use the copy constructor: 
	return( _DoEval(idf) );
}

int AniInternalVariable_AniSet::EAF_IVarSetting::CheckIfEvaluable(
	ANI::CheckIfEvalInfo *)
{
	return(0);
}

AniInternalVariable_AniSet::EAF_IVarSetting::~EAF_IVarSetting()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------

inline ExprValue &AniInternalVariable_AniSet::EAF_IVarAnimation::_DoEval(
	TNIdentifier *idf)
{
	assert(((AniScopeBase*)idf->ani_scope)->ASType()==AST_InternalVar);
	AniAnimation *ani=(AniAnimation*)(
		((AniInternalVariable_AniSet*)idf->ani_scope)->Parent());
	
	assert(ani->ASType()==AST_Animation);
	
	return( ani->GetInternalVarRef(bvi) );
}

ExprValue AniInternalVariable_AniSet::EAF_IVarAnimation::Eval(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	// This will use the copy constructor: 
	return( _DoEval(idf) );
}

ExprValue AniInternalVariable_AniSet::EAF_IVarAnimation::Eval(TNIdentifier *idf,
	const ExprValue &/*base*/,ANI::ExecThreadInfo *)
{
	// This will use the copy constructor: 
	return( _DoEval(idf) );
}

ExprValue &AniInternalVariable_AniSet::EAF_IVarAnimation::GetRef(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	return( _DoEval(idf) );
}

int AniInternalVariable_AniSet::EAF_IVarAnimation::CheckIfEvaluable(
	ANI::CheckIfEvalInfo *)
{
	return(0);
}

AniInternalVariable_AniSet::EAF_IVarAnimation::~EAF_IVarAnimation()
{
	// Not inline because virtual. 
}


//------------------------------------------------------------------------------


ExprValueType AniInternalVariable_AniSet::GetType() const
{
	switch(parent->ASType())
	{
		case AST_Animation:
			switch(bvi)  // Fall through switch. 
			{
				case AniAnimation::BVI_Time:
					return(ExprValueType::RScalarType());
				default: assert(0);
			}
			break;
		case AST_Setting:
			switch(bvi)  // Fall through switch. 
			{
				case AniSetting::BVI_Time:
					return(ExprValueType::RScalarType());
				default: assert(0);
			}
			break;
		default: assert(0);
	}
	return ExprValueType();  // not reached
}


int AniInternalVariable_AniSet::CG_GetVariableType(VariableTypeInfo *vti) const
{
	vti->store_here->operator=(GetType());
	vti->is_lvalue=1;
	vti->is_const=0;
	vti->may_ieval=0;
	return(0);
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_AniSet::CG_GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug)
{
	return(GetVarEvalAdrFuncFunc(idf_for_debug,/*ainc=*/NULL));
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_AniSet::GetVarEvalAdrFuncFunc(
	TNIdentifier * /*idf_for_debug*/,AniIncomplete * /*ainc*/)
{
	// We're (static) member of setting/animation. Hence: 
	switch(Parent()->ASType())
	{
		case AST_Setting:
			return(new AniInternalVariable_AniSet::EAF_IVarSetting(bvi));
		case AST_Animation:
			return(new AniInternalVariable_AniSet::EAF_IVarAnimation(bvi));
		default:  assert(0);
	}
	return(NULL);  // Not reached. 
}


void AniInternalVariable_AniSet::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(GetType().TypeString().str());
	d->Append(" ");
	d->Append(name.str());
	d->Append(":");
	d->Append(" (static)");
	d->Append("\n");
}


AniInternalVariable_AniSet::AniInternalVariable_AniSet(
	const RefString &_name,int _bvi,
	ANI::TreeNode *_treenode,AniScopeBase *_parent) : 
	AniInternalVariable(_name,_treenode,_parent),bvi(_bvi)
{
}

AniInternalVariable_AniSet::~AniInternalVariable_AniSet()
{
}
