/*
 * core/a_ivarobj.cc
 * 
 * ANI internal variables which are member of animation objects. 
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

#include "animation.h"
#include "execthread.h"
#include "aniinstance.h"

using namespace ANI;


static void _DoRegisterVar(AniAniSetObj *parent,
	AniObjectInstance::BuiltinVarIndex bvi,bool is_computed,const char *name)
{
	RefString tmp;
	tmp.set(name);
	new AniInternalVariable_Object(tmp,bvi,is_computed,NULL,parent);
}


void AniInternalVariable_Object::_RegisterInternalVariables(
	AniAniSetObj *parent)
{
	#define INST AniObjectInstance
	if(parent->ASType()==AST_Object)
	{
		_DoRegisterVar(parent,INST::BVI_Time,0,"$time");
		_DoRegisterVar(parent,INST::BVI_Pos,0,"$pos");
		_DoRegisterVar(parent,INST::BVI_Front,0,"$front");
		_DoRegisterVar(parent,INST::BVI_Up,0,"$up");
		_DoRegisterVar(parent,INST::BVI_Speed,0,"$speed");
		_DoRegisterVar(parent,INST::BVI_Accel,0,"$accel");
		
		_DoRegisterVar(parent,INST::BVI_Time,1,"$Time");
		_DoRegisterVar(parent,INST::BVI_Pos,1,"$Pos");
		_DoRegisterVar(parent,INST::BVI_Front,1,"$Front");
		_DoRegisterVar(parent,INST::BVI_Up,1,"$Up");
		_DoRegisterVar(parent,INST::BVI_Speed,1,"$Speed");
		_DoRegisterVar(parent,INST::BVI_Accel,1,"$Accel");
	}
	#undef INST
}

//------------------------------------------------------------------------------

ExprValue AniInternalVariable_Object::EAF_Member::Eval(TNIdentifier *idf,
	ExecThreadInfo *info)
{
	const AniGlue_ScopeInstanceBase *asic=info->stack->GetThisPtr().GetASIB();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec which is 
	// a runtime error here. 
	assert(asic);
	// This will use the copy constructor: 
	return( ((AniScopeInstanceBase*)asic)->GetInternalVarRef(bvi) );
}

ExprValue AniInternalVariable_Object::EAF_Member::Eval(TNIdentifier *idf,
	const ExprValue &base,ANI::ExecThreadInfo *)
{
	ScopeInstance inst;
	base.GetAniScope(&inst);
	AniGlue_ScopeInstanceBase *asic=inst.GetASIB();   // get internal instance base
	if(!asic)  _NullScopeExplicitMemberSelect(idf);
	// This will use the copy constructor: 
	return( ((AniScopeInstanceBase*)asic)->GetInternalVarRef(bvi) );
}

ExprValue &AniInternalVariable_Object::EAF_Member::GetRef(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	// See Eval(idf,info). 
	const AniGlue_ScopeInstanceBase *asic=info->stack->GetThisPtr().GetASIB();
	assert(asic);
	return(((AniScopeInstanceBase*)asic)->GetInternalVarRef(bvi));
}

int AniInternalVariable_Object::EAF_Member::CheckIfEvaluable(
	ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_this_ptr;
	return(0);
}

AniInternalVariable_Object::EAF_Member::~EAF_Member()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------

ExprValue AniInternalVariable_Object::EAF_Compute::Eval(TNIdentifier *idf,
	ExecThreadInfo *info)
{
	const AniGlue_ScopeInstanceBase *asic=info->stack->GetThisPtr().GetASIB();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec which is 
	// a runtime error here. 
	assert(asic);
	ExprValue tmp;
	int rv=((AniScopeInstanceBase*)asic)->ComputeInternalVar(bvi,tmp,
		(ExecThread*)info);
	// If this assert fails, we had an error in ComputeInternalVar(). 
	// Should be handeled by prompting the user and aborting, probably. (see also below)
	if(rv)
	{
		if(rv==-2)
		{
			Error(idf->GetLocationRange(),
				"no active calc core context for object %s "
				"(use at least empty ADB \"%%{};\")\n",
				((AniScopeInstanceBase*)asic)->GetASB()->CompleteName().str());
		}
		else
		{
			Error(idf->GetLocationRange(),
				"failed to query internal var (bvi=%d): rv=%d\n",
				bvi,rv);
		}
		abort();
	}
	return(tmp);
}

ExprValue AniInternalVariable_Object::EAF_Compute::Eval(TNIdentifier *idf,
	const ExprValue &base,ANI::ExecThreadInfo *info)
{
	ScopeInstance inst;
	base.GetAniScope(&inst);
	AniGlue_ScopeInstanceBase *asic=inst.GetASIB();   // get internal instance base
	if(!asic)  _NullScopeExplicitMemberSelect(idf);
	ExprValue tmp;
	int rv=((AniScopeInstanceBase*)asic)->ComputeInternalVar(bvi,tmp,
		(ExecThread*)info);
	// If this assert fails, we had an error in ComputeInternalVar(). 
	// Should be handeled by prompting the user and aborting, probably. (see also above)
	assert(rv==0);
	return(tmp);
}

ExprValue &AniInternalVariable_Object::EAF_Compute::GetRef(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	// These values are computed by the core on the fly and hence are no 
	// lvalues. 
	assert(0);
}

int AniInternalVariable_Object::EAF_Compute::CheckIfEvaluable(
	ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_this_ptr;
	return(0);
}

AniInternalVariable_Object::EAF_Compute::~EAF_Compute()
{
	// Not inline because virtual. 
}


//------------------------------------------------------------------------------


ExprValueType AniInternalVariable_Object::GetType() const
{
	switch(bvi)  // Fall through switch. 
	{
		case AniObjectInstance::BVI_Time:
			return(ExprValueType::RScalarType());
		case AniObjectInstance::BVI_Pos:
		case AniObjectInstance::BVI_Front:
		case AniObjectInstance::BVI_Up:
		case AniObjectInstance::BVI_Speed:
		case AniObjectInstance::BVI_Accel:
			return(ExprValueType::RVectorType(3));
		default: assert(0);
	}
	return ExprValueType();  // not reached
}


int AniInternalVariable_Object::CG_GetVariableType(VariableTypeInfo *vti) const
{
	vti->store_here->operator=(GetType());
	vti->is_lvalue=(computed_var ? 0 : 1);
	vti->is_const=0;
	vti->may_ieval=0;
	return(0);
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_Object::CG_GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug)
{
	return(GetVarEvalAdrFuncFunc(idf_for_debug,/*ainc=*/NULL));
}


AniGlue_ScopeBase::EvalAddressationFunc *AniInternalVariable_Object::GetVarEvalAdrFuncFunc(
	TNIdentifier * /*idf_for_debug*/,AniIncomplete * /*ainc*/)
{
	// We're (non-static) member of an object. Hence: 
	assert(Parent()->ASType()==AST_Object);
	if(computed_var)
	{  return(new AniInternalVariable_Object::EAF_Compute(bvi));  }
	else
	{  return(new AniInternalVariable_Object::EAF_Member(bvi));  }
}


void AniInternalVariable_Object::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(GetType().TypeString().str());
	d->Append(" ");
	d->Append(name.str());
	d->Append(":");
	d->Append(computed_var ? " (computed member)" : " (member)");
	d->Append("\n");
}


AniInternalVariable_Object::AniInternalVariable_Object(
	const RefString &_name,/*AniObjectInstance::BuiltinVarIndex*/int _bvi,
	bool _computed_var,ANI::TreeNode *_treenode,AniScopeBase *_parent) : 
	AniInternalVariable(_name,_treenode,_parent),bvi(_bvi)
{
	computed_var=_computed_var;
}

AniInternalVariable_Object::~AniInternalVariable_Object()
{
}
