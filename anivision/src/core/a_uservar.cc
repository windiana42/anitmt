/*
 * core/a_uservar.cc
 * 
 * ANI tree variable, user-defined. 
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
#include "aniinstance.h"
#include "execthread.h"

using namespace ANI;


// Static var, either automatic or member without member select: 
ExprValue AniUserVariable::EAF_Static::Eval(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	assert(((AniScopeBase*)idf->ani_scope)->ASType()==AST_UserVar);
	// Note: this works for members as well as automatic vars. 
	// Check if var is really static done by GetStaticVal(). 
	return(((AniUserVariable*)idf->ani_scope)->GetStaticVal(idf));
}

// Static member var var with member select: 
ExprValue AniUserVariable::EAF_Static::Eval(TNIdentifier *idf,
	const ExprValue & /*base*/,ANI::ExecThreadInfo *)
{
	assert(((AniScopeBase*)idf->ani_scope)->ASType()==AST_UserVar);
	// Note: this works for members as well as automatic vars. 
	// Check if var is really static done by GetStaticVal(). 
	return(((AniUserVariable*)idf->ani_scope)->GetStaticVal(idf));
}

// Version without member select returning a true C++ reference: 
ExprValue &AniUserVariable::EAF_Static::GetRef(TNIdentifier *idf,
	ExecThreadInfo * /*info*/)
{
	assert(((AniScopeBase*)idf->ani_scope)->ASType()==AST_UserVar);
	// Note: this works for members as well as automatic vars. 
	// Check if var is really static done by GetStaticVal(). 
	return(((AniUserVariable*)idf->ani_scope)->GetStaticVal(idf));
}

int AniUserVariable::EAF_Static::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_static;
	return(0);
}

//------------------------------------------------------------------------------

// Automatic var: 
ExprValue AniUserVariable::EAF_Auto::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	//return( ((ExecThread*)info)->stack->GetValue(addr_idx,scopes_up) );
	return( info->stack->GetValue(addr_idx,scopes_up) );
}

// Version without member select returning a true C++ reference: 
ExprValue &AniUserVariable::EAF_Auto::GetRef(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	//return( ((ExecThread*)info)->stack->GetValue(addr_idx,scopes_up) );
	return( info->stack->GetValue(addr_idx,scopes_up) );
}

int AniUserVariable::EAF_Auto::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_local_stack;
	return(0);
}

//------------------------------------------------------------------------------

// Non-static member var, version for implicit member select from "this": 
ExprValue AniUserVariable::EAF_Member::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	const AniGlue_ScopeInstanceBase *asic=info->stack->GetThisPtr().GetASIB();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec which is 
	// a runtime error here. 
	assert(asic);   // FIXME: error message (see also: a_ivarobj.cc!!!!!)
	return(((const AniScopeInstanceBase*)asic)->GetMemberVar(addr_idx));
}

// Non-static member var, version for explicit member select: 
ExprValue AniUserVariable::EAF_Member::Eval(TNIdentifier *idf,
	const ExprValue &base,ANI::ExecThreadInfo *)
{
	ScopeInstance inst;
	base.GetAniScope(&inst);
	AniGlue_ScopeInstanceBase *asic=inst.GetASIB();   // get internal instance base
	if(!asic)  _NullScopeExplicitMemberSelect(idf);
	return(((AniScopeInstanceBase*)asic)->GetMemberVar(addr_idx));
}

// Version without member select returning a true C++ reference: 
ExprValue &AniUserVariable::EAF_Member::GetRef(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	const AniGlue_ScopeInstanceBase *asic=info->stack->GetThisPtr().GetASIB();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec which is 
	// a runtime error here. 
	assert(asic);
	return(((const AniScopeInstanceBase*)asic)->GetMemberVar(addr_idx));
}

int AniUserVariable::EAF_Member::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_this_ptr;
	return(0);
}

//------------------------------------------------------------------------------

ExprValue AniUserVariable::EAF_This::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	const ScopeInstance &thisptr=info->stack->GetThisPtr();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec. 
	//assert(!!thisptr);
	// Removed this assert because this=NULL is valid as long as we do 
	// no member select. 
	return(ExprValue(ExprValue::Copy,thisptr));
}

int AniUserVariable::EAF_This::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_this_ptr;
	return(0);
}

//------------------------------------------------------------------------------

ExprValue AniUserVariable::EAF_Repeater::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo *info)
{
	const ScopeInstance &thisptr=info->stack->GetThisPtr();
	// If this assert fails, we accepted "this" in a location where 
	// no "this" pointer is defined which is an error in the TF step. 
	// OR, the "this" pointer is NULL due to user spec. 
	//assert(!!thisptr);
	// Removed this assert because this=NULL is valid as long as we do 
	// no member select. 
	return(ExprValue(ExprValue::Copy,thisptr));
}

ExprValue AniUserVariable::EAF_Repeater::Eval(TNIdentifier * /*idf*/,
	const ExprValue &base,ANI::ExecThreadInfo *)
{
	return(base);
}

int AniUserVariable::EAF_Repeater::CheckIfEvaluable(ANI::CheckIfEvalInfo *ci)
{
	++ci->nrefs_to_this_ptr;
	return(0);
}

//------------------------------------------------------------------------------

ExprValue AniUserVariable::EAF_Null::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	return(ExprValue::None);
}

ExprValue AniUserVariable::EAF_Null::Eval(TNIdentifier * /*idf*/,
	const ExprValue & /*base*/,ANI::ExecThreadInfo *)
{
	// Needed because a static function CAN be called with an 
	// object using "a.static_foo()". In that case, the "a." is 
	// just the same as "A::" (if a is of type A). 
	return(ExprValue::None);
}

int AniUserVariable::EAF_Null::CheckIfEvaluable(ANI::CheckIfEvalInfo *)
{
	return(0);
}


//------------------------------------------------------------------------------

ExprValue AniUserVariable::EAF_OpNew::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	AniScopeInstanceBase *_instance=AniScopeInstanceBase::CreateAniInstance(
		asb_type);
	return(ExprValue(ExprValue::Copy,
		ScopeInstance(ScopeInstance::InternalCreate,_instance)));
}

int AniUserVariable::EAF_OpNew::CheckIfEvaluable(ANI::CheckIfEvalInfo *)
{
	return(0);
}

//------------------------------------------------------------------------------

ExprValue AniUserVariable::EAF_NULL::Eval(TNIdentifier * /*idf*/,
	ExecThreadInfo * /*info*/)
{
	// This is for (literal) "NULL" in code. 
	return(ExprValue::None);
}

int AniUserVariable::EAF_NULL::CheckIfEvaluable(ANI::CheckIfEvalInfo *)
{
	return(0);
}

/*----------------------------------------------------------------------------*/

AniUserVariable::EAF_Static::~EAF_Static()
{}
AniUserVariable::EAF_Auto::~EAF_Auto()
{}
AniUserVariable::EAF_Member::~EAF_Member()
{}
AniUserVariable::EAF_This::~EAF_This()
{}
AniUserVariable::EAF_Repeater::~EAF_Repeater()
{}
AniUserVariable::EAF_Null::~EAF_Null()
{}
AniUserVariable::EAF_OpNew::~EAF_OpNew()
{}
AniUserVariable::EAF_NULL::~EAF_NULL()
{}

/*============================================================================*/

void *AniUserVariable::CG_GetDeclaratorEvalFunc(ANI::TNDeclarator *decl,
	ANI::TNIdentifier *name)
{
	// NOTE: If this assert fails, this may be called by a 
	//       child declarator (i.e. DT_Array or DT_Name 
	//       although there is a DT_Initialize). This is 
	//       illegal. 
	assert(decl==GetTreeNode());
	
	if(attrib & HAS_Initializer)
	{
		return( (attrib & IS_Static) ? 
			(void*)new TNDeclarator::EvalFunc_Static(name) : 
			(void*)new TNDeclarator::EvalFunc_Normal(name) );
	}
	else
	{
		// Does not have initializer. 
		// Special version setting prototype value: 
		return( (attrib & IS_Static) ? 
			(void*)new TNDeclarator::EvalFunc_ProtoStatic(name,vtype) : 
			(void*)new TNDeclarator::EvalFunc_ProtoNormal(name,vtype) );
	}
}


AniGlue_ScopeBase::EvalAddressationFunc *AniUserVariable::CG_GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug)
{
	return(GetVarEvalAdrFuncFunc(idf_for_debug,/*ainc=*/NULL));
}


AniGlue_ScopeBase::EvalAddressationFunc *AniUserVariable::GetVarEvalAdrFuncFunc(
	TNIdentifier *idf_for_debug,AniIncomplete *ainc)
{
	#define PRINT_EVFUNC_SEL 0
	
	if((attrib & AniUserVariable::IS_Static))
	{
		// Static. May be member or automatic -- does not matter. 
		#if PRINT_EVFUNC_SEL
		fprintf(stderr,"EVALFUNC: %s: %s --static-> %s\n",
			idf_for_debug->GetLocationRange().PosRangeString().str(),
			idf_for_debug->CompleteStr().str(),
			CompleteName().str());
		#endif
		return(new AniUserVariable::EAF_Static());
	}
	else
	{
		// Non-static. 
		int scope_up_cnt=0;
		AniScopeBase *asb=Parent();
		// ainc=0 -> scope_up_cnt=0 and asb=<our parent>. 
		if(ainc)
		{
			// Find us in hierarchy of *ainc; must be further up: 
			for(asb=ainc->Parent(); asb && asb!=Parent();
				 asb=asb->Parent(),++scope_up_cnt);
			// If this fails, we could not find us. Would be strange: 
			if(!asb)
			{
				fprintf(stderr,"OOPS: %s not in hierarchy of %s.\n",
					CompleteName().str(),ainc->CompleteName().str());
				assert(0);
			}
		}
		// *asb now parent of us (= "looked up variable" when called 
		// from AniIncomplete's CG_LookupIdentifier()). 
		// Well, we could have that O(1) using Parent() but the 
		// loop is necessary to get the scope_up_cnt. 
		
		// Fill in eval lookup info for the variable: 
		if((attrib & AniUserVariable::IS_Automatic))
		{
			// Automatic, i.e. non-member var. 
			// Automatic vars must live in AnonScope or Funcion. 
			if(asb->ASType()==AST_AnonScope || 
			   asb->ASType()==AST_UserFunc || 
			   asb->ASType()==AST_InternalFunc )  // <-- will not happen (?)
			{
				int adr_idx=GetAddressationIndex();
				#if PRINT_EVFUNC_SEL
				fprintf(stderr,"EVALFUNC: %s: %s "
					"--auto(f_up=%d,a_idx=%d)-> %s\n",
					idf_for_debug->GetLocationRange().PosRangeString().str(),
					idf_for_debug->CompleteStr().str(),
					scope_up_cnt,adr_idx,
					CompleteName().str());
				#endif
				return(new AniUserVariable::EAF_Auto(scope_up_cnt,adr_idx));
			}
			else assert(0);
		}
		else
		{
			// Member variable, nonstatic. 
			// Both implicit or explicit member select. 
			if(asb->ASType()==AST_Animation || 
			   asb->ASType()==AST_Setting || 
			   asb->ASType()==AST_Object )
			{
				#if PRINT_EVFUNC_SEL
				fprintf(stderr,"EVALFUNC: %s: %s "
					"--member(a_idx=%d)-> %s\n",
					idf_for_debug->GetLocationRange().PosRangeString().str(),
					idf_for_debug->CompleteStr().str(),
					GetAddressationIndex(),
					CompleteName().str());
				#endif
				return(new AniUserVariable::EAF_Member(GetAddressationIndex()));
			}
			else assert(0);
		}
	}
	
	assert(0);
	return(NULL);
}


int AniUserVariable::CG_GetVariableType(VariableTypeInfo *vti) const
{
	// May only return complete types here. 
	assert(!vtype.IsIncomplete());
	vti->store_here->operator=(vtype);
	vti->is_lvalue=1;  // Even const ones...
	vti->is_const = (attrib & IS_Const) ? 1 : 0;
	vti->may_ieval=0;
	return(0);
}


ExprValueType AniUserVariable::GetType() const
{
	return(vtype);
}


int AniUserVariable::InitStaticVariables(ExecThreadInfo *info,int flag)
{
	int nerrors=0;
	
	if(flag>0)
	{
		// So, see if we're a static variable... 
		// Only non-automatic vars (because the automatic ones 
		// get initialized when first used). The automatic ones 
		// even do not need prototype values because the initializer 
		// code uses real C++ references and hence can still be used 
		// when ExprValue::iev=NULL. 
		// We must, however, allocate the static_val for the automatic 
		// vars, too. 
		if(attrib & IS_Static)
		{
			// First, allocate the ExprValue: 
			assert(!static_val);
			static_val=new ExprValue();
			
			if(!(attrib & IS_Automatic))
			{
				//fprintf(stderr,"  Initializing %s...\n",CompleteName().str());
				
				// This is "simple". 
				// Just execute (i.e. "eval") the declarator. 
				TNDeclarator *decl=(TNDeclarator*)GetTreeNode();
				assert(decl->NType()==TN_Declarator);
				ExprValue tmp;
				
				int n_switches=0;
				bool fail=0;
				for(;;)
				{
					if(!decl->Eval(tmp,info))  break;
					if((++n_switches)>16)  {  fail=1;  break;  }
				}
				if(n_switches || fail)
				{
					Error(decl->GetLocationRange(),
						"OOPS: %s%d context switch attempts during "
						"initialisation of static var %s\n",
						fail ? "more than " : "",
						fail ? (n_switches-1) : n_switches,
						CompleteName().str());
					if(fail) abort();
				}
				
				//fprintf(stdout,"  Initialized %s = %s\n",
				//	CompleteName().str(),static_val->ToString().str());
				
				// Make sure ExprValue::iev != NULL. 
				assert(static_val->operator!()==0);
			}
		}
	}
	else if(flag<0)
	{
		// Cleanup static variable. 
		// Also automatic static vars need to be cleaned up if they 
		// were set. 
		if(attrib & IS_Static)
		{  DELETE(static_val);  }
	}
	else assert(0);
	
	return(nerrors);
}


int AniUserVariable::FixIncompleteTypes()
{
	int nerrors=0;
	// No children to pass on...
	
	if(vtype.IsIncomplete())
	{
		// treenode is TNDeclarator. 
		// treenode->parent is TNDeclarationStmt or TNFuncDefArgDecl. 
		TNTypeSpecifier *tspec=NULL;
		TNDeclarator *decl=(TNDeclarator*)GetTreeNode();
		switch(GetTreeNode()->parent()->NType())
		{
			case TN_Statement:
				if(((TNStatement*)GetTreeNode()->parent())->StmtType()!=
					TNS_DeclarationStmt)  assert(0);
				tspec=(TNTypeSpecifier*)GetTreeNode()->parent()->down.first();
				break;
			case TN_FuncDefArgDecl:
				tspec=(TNTypeSpecifier*)GetTreeNode()->parent()->down.first();
				break;
			default: assert(0);
		}
		
		if(_DoFixIncompleteType(&vtype,this,tspec,decl))
		{  ++nerrors;  }
	}
	
	return(nerrors);
}


AniScopeBase *AniUserVariable::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo * /*tri*/)
{
	// Function works recursively. Tell all our children: 
	// --oops, we have none. Okay. 
	
	// Perform checks...
	if((attrib & IS_Static) && !(attrib & HAS_Initializer))
	{
		Warning(NameLocation(),
			"static variable declaration \"%s\" without initializer\n",
			CompleteName().str());
	}
	
	return(NULL);  // Always NULL; that is okay. 
}


int AniUserVariable::PerformCleanup(int cstep)
{
	int nerr=0;
	
	switch(cstep)
	{
		case 1:  break;  // Nothing to do. 
		case 2:  break;  // Nothing to do for InstanceInfo creation step. 
		default:  assert(0);
	}
	
	return(nerr);
}


ANI::TNLocation AniUserVariable::NameLocation() const
{
	const TNIdentifier *idf=(const TNIdentifier*)
		((TNSetting*)GetTreeNode())->down.first();
	return(idf->GetLocationRange());
}


const char *AniUserVariable::CG_AniScopeTypeStr() const
{
	return("variable");
}


void AniUserVariable::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(vtype.TypeString().str());
	d->Append(" ");
	d->Append(name.str());
	d->Append(":");
	if(attrib & IS_Const)
	{  d->Append(" const");  }
	if(attrib & IS_Static)
	{  d->Append(" static");  }
	if(attrib & IS_Automatic)
	{  d->Append(" automatic");  }
	else
	{  d->Append(" (member)");  }
	if(attrib & HAS_Initializer)
	{  d->Append(" (has initializer)");  }
	if(vtype.IsIncomplete())
	{  d->Append(" [incomplete]");  }
	char tmp[32];
	snprintf(tmp,32," (seq=%d",var_seq_num);
	d->Append(tmp);
	if((attrib & IS_Static) && static_val)
	{
		d->Append(",static_val=");
		d->Append(static_val->ToString().str());
	}
	if(!(attrib & IS_Static) && addressation_idx>=0)
	{
		snprintf(tmp,32,",adr_idx=%d",addressation_idx);
		d->Append(tmp);
	}
	d->Append(")");
	d->Append("\n");
}


void AniUserVariable::_sai_gsv_assertfail(int idx) const
{
	fprintf(stderr,"OOPS: AniUserVariable %s: idx=%d (static=%s)\n",
		CompleteName().str(),idx,(attrib & IS_Static) ? "yes" : "no");
	assert(0);
}


void AniUserVariable::_accessing_unset_static(TNIdentifier *idf /*<- for_error*/) const
{
	AniScopeBase *asb=(AniScopeBase*)idf->ani_scope;
	Error(idf->GetLocationRange(),
		"OOPS: accessing not yet set up static %s %s\n",
		asb ? asb->AniScopeTypeStr() : "[variable?]",
		asb ? asb->CompleteName().str() : "???");
	abort();
}


AniUserVariable::AniUserVariable(const RefString &_name,TreeNode *_treenode,
	AniScopeBase *_parent,const ExprValueType &_vtype,int _attrib,
	int _var_seq_num) : 
	AniVariable(AST_UserVar,_name,_treenode,_parent),
	vtype(_vtype)
{
	attrib=_attrib;
	
	var_seq_num=_var_seq_num;
	assert(var_seq_num>=0);
	
	if((_attrib & IS_Static))
	{  static_val=NULL;  }
	else
	{  addressation_idx=-1;  }
}

AniUserVariable::~AniUserVariable()
{
	if((attrib & IS_Static))
	{  DELETE(static_val);  }
}
