/*
 * ani-parser/coreglue.cc
 * 
 * Default implementation for parser interface to core. 
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

#include "coreglue.h"

//#include <valtype/valtypes.h>
#include "exprvalue.h"

#include <stdio.h>


namespace ANI
{

AniGlue_ScopeBase::LookupFunctionInfo::LookupFunctionInfo() : 
	ret_evt()
{
	n_args=0;
	arg_name=NULL;
	arg_type=NULL;
	call_loc=NULL;
	name_idf=NULL;
	opfunc=NULL;
	ani_function=NULL;
	evalfunc=NULL;
	allocfunc=NULL;
	may_try_ieval=0;
}

//------------------------------------------------------------------------------


const char *AniGlue_ScopeBase::_AniScopeTypeStr(AniScopeType ast)
{
	switch(ast)
	{
		case AST_Animation:    return("animation");
		case AST_Setting:      return("setting");
		case AST_Object:       return("object");
		case AST_UserFunc:     return("function");
		case AST_InternalFunc: return("internal function");
		case AST_AnonScope:    return("[anonscope]");
		case AST_UserVar:      return("variable");
		case AST_InternalVar:  return("internal variable");
		case AST_Incomplete:   return("[incomplete]");
		default:  assert(0);
	}
	return("???");
}


const char *AniGlue_ScopeBase::CG_AniScopeTypeStr() const
{
	// Must be overridden. 
	assert(0);
	return("???");
}


void AniGlue_ScopeBase::CG_DeleteNotify(TreeNode *)
{
	/* Does not have to be overridden. */
}


void AniGlue_ScopeBase::CG_ExecEnterCompound(ExecThreadInfo * /*info*/)
{
	// Should probably have been overridden. 
	fprintf(stderr,"AniGlue_ScopeBase::CG_ExecEnterCompound() for %s "
		"not overridden\n",AniScopeTypeStr());
	assert(0);
}

void AniGlue_ScopeBase::CG_ExecLeaveCompound(ExecThreadInfo * /*info*/)
{
	// Should probably have been overridden. 
	fprintf(stderr,"AniGlue_ScopeBase::CG_ExecLeaveCompound() for %s "
		"not overridden\n",AniScopeTypeStr());
	assert(0);
}


AniGlue_ScopeBase *AniGlue_ScopeBase::CG_ReplaceAniIncomplete(
	AniGlue_ScopeBase * /*old_location*/,
	AniGlue_ScopeBase * /*ms_from*/,int /*ms_quality*/)
{
	/* Must be overridden. */
	fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_ReplaceAniIncomplete() "
		"not overridden.\n");
	return(NULL);
}


int AniGlue_ScopeBase::CG_LookupFunction(
	AniGlue_ScopeBase::LookupFunctionInfo *lfi)
{
	lfi->ani_function=NULL;
	
	/* Must be overridden by AniIncomplete. */
	if(CG_ASType()==AST_Incomplete)
	{
		fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_LookupFunction() not "
			"overridden.\n");
		return(0);
	}
	else
	{  return(-1);  }  // -> error "cannot call as a function"
}


void AniGlue_ScopeBase::CG_LookupConstructor(
	AniGlue_ScopeBase::LookupFunctionInfo *lfi)
{
	/* Must be overridden by AniObject. */
	if(CG_ASType()==AST_Object)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_LookupConstructor() not "
		"overridden.\n");  }
	else
	{
		// This is an internal error, probably somewhere in ani-parser code. 
		fprintf(stderr,"OOPS: CG_LookupConstructor called for %s.\n",
			AniScopeTypeStr());
		assert(0);
	}
	lfi->ani_function=NULL;
}


void AniGlue_ScopeBase::CG_LookupIdentifier(
	AniGlue_ScopeBase::LookupIdentifierInfo *lii)
{
	/* Must be overridden by AniIncomplete. */
	if(CG_ASType()==AST_Incomplete)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_LookupIdentifier() not "
		"overridden.\n");  }
	else
	{
		// This is an internal error, probably somewhere in ani-parser code. 
		fprintf(stderr,"OOPS: CG_LookupIdentifier called for %s.\n",
			AniScopeTypeStr());
		assert(0);
	}
	lii->ani_scope=NULL;
}


AniGlue_ScopeBase::EvalAddressationFunc *AniGlue_ScopeBase::CG_GetVarEvalAdrFuncFunc(
	TNIdentifier * /*idf_for_debug*/)
{
	/* Must be overridden by AniVariable. */
	if(CG_ASType()==AST_UserVar || CG_ASType()==AST_InternalVar)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_GetVarEvalAdrFuncFunc() not "
		"overridden.\n");  }
	else
	{  assert(0);  }
	return(NULL);
}


void *AniGlue_ScopeBase::CG_GetDeclaratorEvalFunc(TNDeclarator * /*decl*/,
	TNIdentifier * /*name*/)
{
	/* Must be overridden by AniVariable. */
	if(CG_ASType()==AST_UserVar)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_GetDeclaratorEvalFunc() not "
		"overridden.\n");  }
	else
	{  assert(0);  }
	return(NULL);
}


void *AniGlue_ScopeBase::CG_GetJumpStmtEvalFunc(TNJumpStmt * /*tn*/,
	AssignmentFuncBase * /*assfunc*/,AniGlue_ScopeBase * /*from_scope*/)
{
	/* Must be overridden by AniFunction. */
	if(CG_ASType()==AST_UserFunc)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_GetJumpStmtEvalFunc() not "
		"overridden.\n");  }
	else
	{  assert(0);  }
	return(NULL);
}


int AniGlue_ScopeBase::CG_GetVariableType(VariableTypeInfo *vti) const
{
	/* Must be overridden by AniVariable. */
	if(CG_ASType()==AST_UserVar || CG_ASType()==AST_InternalVar)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_GetVariableType() not "
		"overridden.\n");  }
	else
	{
		// This is an internal error, probably somewhere in ani-parser code. 
		fprintf(stderr,"OOPS: CG_GetVariableType called for %s.\n",
			AniScopeTypeStr());
		assert(0);
	}
	vti->store_here->SetUnknown();
	return(1);
}


int AniGlue_ScopeBase::CG_GetReturnType(ExprValueType *store_here) const
{
	/* Must be overridden by AniFunction. */
	if(CG_ASType()==AST_UserFunc)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_GetReturnType() not "
		"overridden.\n");  }
	else
	{
		// This is an internal error, probably somewhere in ani-parser code. 
		fprintf(stderr,"OOPS: CG_GetReturnType called for %s.\n",
			AniScopeTypeStr());
		assert(0);
	}
	store_here->SetUnknown();
	return(1);
}


AniGlue_ScopeBase::IncompleteType AniGlue_ScopeBase::CG_IncompleteType() const
{
	/* Must be overridden by AniIncomplete. */
	if(CG_ASType()==AST_Incomplete)
	{  fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_IncompleteType() not "
		"overridden.\n");  }
	else assert(0);
	return((IncompleteType)(100));  // <-- something invalid...
}


AniGlue_ScopeBase::AniScopeType AniGlue_ScopeBase::CG_ASType() const
{
	/* Must be overridden. */
	fprintf(stderr,"OOPS: AniGlue_ScopeBase::CG_ASType() not overridden.\n");
	return((AniScopeType)(-1));
}

String AniGlue_ScopeBase::CG_GetName() const
{
	/* Must be overridden. */
	fprintf(stderr,"OOPS: AniGlue_ScopeBase::GetName() not overridden.\n");
	String name("[HACKME]");
	return(name);
}

AniGlue_ScopeBase *AniGlue_ScopeBase::RegisterTN(ANI::TreeNode * /*tn*/,
		ANI::TRegistrationInfo * /*tri*/)
{
	/* Must be overridden. */
	fprintf(stderr,"OOPS: AniGlue_ScopeBase::RegisterTN() not overridden.\n");
	return(NULL);
}

AniGlue_ScopeBase::~AniGlue_ScopeBase()
{
	// not inline because virtual. 
}


/******************************************************************************/

void AniGlue_ScopeInstanceBase::GetExprValueType(
	ExprValueType *store_here) const
{
	// Must be overridden. 
	assert(0);
	store_here->SetNull();
}

String AniGlue_ScopeInstanceBase::ToString() const
{
	// Must be overridden. 
	assert(0);
	return String();
}

AniGlue_ScopeInstanceBase &AniGlue_ScopeInstanceBase::operator=(
	const AniGlue_ScopeInstanceBase &)
{
	// Forbidden. 
	assert(0);
	return(*this);
}


AniGlue_ScopeInstanceBase::AniGlue_ScopeInstanceBase(
	const AniGlue_ScopeInstanceBase &)
{
	// Forbidden. 
	assert(0);
}

AniGlue_ScopeInstanceBase::~AniGlue_ScopeInstanceBase()
{
	// not inline because virtual. 
	assert(refcnt==0);
}


/******************************************************************************/

ExprValue SpecialIdf_CoreHook::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	// Must be overridden if used. 
	assert(0);
	return ExprValue();
}

SpecialIdf_CoreHook::~SpecialIdf_CoreHook()
{
	// Make sure this class is no longer used. 
	assert(!core_hook_used);
}

}  // End of namespace ANI. 
