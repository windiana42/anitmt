/*
 * core/a_setting.cc
 * 
 * ANI tree setting scope. 
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
#include <ani-parser/treereg.h>

#include "execthread.h"
#include <calccore/ccif.h>


using namespace ANI;


int AniSetting::EvalFunc_FCallSetting::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	EvalStateStack *ess=info ? info->ess : NULL;
	
	ExprValue val[nargs];
	switch(ess->popS())
	{
		// First, there is a pretty standard arg eval step (like for simple 
		// internal functions), just without arg permutation: 
		case 0:  // arg evaluation
		{
			int idx=0;
			TreeNode *i=tn->down.first()->next;
			TreeNode *stn=ess->popTN(i);
			for(; i!=stn; i=i->next,idx++)
			{  ess->popEV(&val[idx]);  }
			ExprValue tmpval;
			for(; i; i=i->next,idx++)
			{
				if(i->Eval(tmpval,info))
				{
					ess->pushTN(i);  // <-- Do not move!
					for(--idx,i=i->prev; idx>=0; i=i->prev,idx--)
					{  ess->pushEV(val[idx]);  }
					ess->pushS(0);
					return(1);
				}
				assert(arg_assfunc[idx]);
				arg_assfunc[idx]->Assign(
					/*lvalue=*/val[idx],
					/*rvalue=*/tmpval);
			}
			assert(idx==nargs);  // can be left away
		} goto skippop;
		case 1:
			for(int idx=0; idx<nargs; idx++)
			{  ess->popEV(&val[idx]);  }
		skippop:;
		{
			// Now, check which args were specified and supply defaults for 
			// the other ones. 
			EvalSettingArgs esa;
			esa.t=        arg_idx_t<0        ? NULL : &val[arg_idx_t];
			esa.toff=     arg_idx_toff<0     ? NULL : &val[arg_idx_toff];
			esa.run_ctx=  arg_idx_run_ctx<0  ? NULL : &val[arg_idx_run_ctx];
			esa.save_ctx= arg_idx_save_ctx<0 ? NULL : &val[arg_idx_save_ctx];
			esa.skip=     arg_idx_skip<0     ? NULL : &val[arg_idx_skip];
			
			// We could allow a return value here (but then must 
			// also change the function code to set ret_type to 
			// integer/scalar for the TF step. 
			if(setting->_DO_EvalSetting(&esa,info))
			{
				for(int idx=nargs-1; idx>=0; idx--)
				{  ess->pushEV(val[idx]);  }
				ess->pushS(1);
				return(1);
			}
			retval=ExprValue::CVoidType;
		}	break;
		default: assert(0);
	}
	
	return(0);	
}

AniSetting::EvalFunc_FCallSetting::EvalFunc_FCallSetting(
	AniSetting *_setting,int _nargs) : 
	ANI::TNOperatorFunction::EvalFunc()
{
	setting=_setting;
	nargs=_nargs;
	
	arg_idx_t=-1;
	arg_idx_toff=-1;
	arg_idx_run_ctx=-1;
	arg_idx_save_ctx=-1;
	arg_idx_skip=-1;
	
	arg_assfunc=(ANI::AssignmentFuncBase**)LMalloc(
		nargs*sizeof(ANI::AssignmentFuncBase*));
	for(int i=0; i<nargs; i++)
		arg_assfunc[i]=NULL;
}

AniSetting::EvalFunc_FCallSetting::~EvalFunc_FCallSetting()
{
	setting=NULL;
	
	for(int i=0; i<nargs; i++)
		DELETE(arg_assfunc[i]);
	LFree(arg_assfunc);
}

//------------------------------------------------------------------------------

int AniSetting::CG_LookupFunction(AniGlue_ScopeBase::LookupFunctionInfo *lfi)
{
	// Inside the animation block, one can call settings 
	// like functions. 
	// We need opfunc pointer for the checks. 
	assert(lfi->opfunc);
	// See if this is inside an animation block: 
	// Find function def. 
	TreeNode *tn=lfi->opfunc->parent();
	for(; tn && tn->NType()!=TN_FunctionDef; tn=tn->parent());
	if(!tn || !((TNFunctionDef*)tn)->function || !(
		((AniFunction*)(((TNFunctionDef*)tn)->function))->GetAttrib() & 
			AniFunction::IS_Animation) )
	{
		Error(lfi->opfunc->down.first()->GetLocationRange(),
			"cannot call setting %s like a function outside animation block\n",
			CompleteName().str());
		return(0);
	}
	
	// So, we're inside an animation block and the user calls 
	// a setting to be processed. 
	// That looks like this: 
	// "setting(t=<range>,toff=delta, skip=0/1, run_ctx=str_name, 
	//          save_ctx=str_name);"
	
	// This especially means that the setting is given directly 
	// as an identifier. Silly jokes such as 
	//   (val ? TheSetting : TheSetting)(t=1..2); 
	// will not work (there is no point in doing the above). 
	// Hence, we check here if there is an identifier as the first 
	// child if the function call. If so, assign an eval func to 
	// it. If not, simply go on without doing so, we'll catch an 
	// "inalid use of ..." error lateron in checkifeval.cc. 
	do {
		TreeNode *nnode=lfi->opfunc->down.first();
		if(nnode->NType()!=TN_Expression) break;
		if(((TNExpression*)nnode)->ExprType()!=TNE_Expression) break;
		if(nnode->down.first()->NType()!=TN_Identifier) break;
		TNIdentifier *idf=(TNIdentifier*)nnode->down.first();
		assert(!idf->evaladrfunc);
		// This eval func may do just anything. It will never be 
		// called. 
		idf->evaladrfunc=new AniUserVariable::EAF_NULL();
	} while(0);
	
	// Perform argument checking. 
	
	// Create eval function handler: 
	EvalFunc_FCallSetting *evalfunc=new EvalFunc_FCallSetting(this,lfi->n_args);
	// Argument indices stored in evalfunc. 
	
	bool match_ok=0;
	do {
		char which_arg='\0'; 
		for(int i=0; i<lfi->n_args; i++)
		{
			if(lfi->arg_name[i])
			{
				// Look up argument: 
				RefString name=lfi->arg_name[i]->CompleteStr();
				     if(name=="t")         which_arg='t';
				else if(name=="toff")      which_arg='o';
				else if(name=="run_ctx")   which_arg='R';
				else if(name=="save_ctx")  which_arg='S';
				else if(name=="skip")      which_arg='s';
				else goto breakmatch;
			}
			else switch(which_arg)  // <-- switch forward one arg
			{
				case'\0':  which_arg='t';  break;
				case 't':  which_arg='o';  break;
				case 'o':  which_arg='R';  break;
				case 'R':  which_arg='S';  break;
				case 'S':  which_arg='s';  break;
				case 's':  goto breakmatch;
				default:  assert(0);
			}
			
			// Check arg type and repetition:
			int rv=-10;
			switch(which_arg)
			{
				case '\0':  goto breakmatch;
				case 't':
					if(evalfunc->arg_idx_t>=0)  goto breakmatch;
					evalfunc->arg_idx_t=i;
					rv=ExprValueType::RRangeType().CanAssignType(
						lfi->arg_type[i],&evalfunc->arg_assfunc[i]);
					break;
				case 'o':
					if(evalfunc->arg_idx_toff>=0)  goto breakmatch;
					evalfunc->arg_idx_toff=i;
					rv=ExprValueType::RScalarType().CanAssignType(
						lfi->arg_type[i],&evalfunc->arg_assfunc[i]);
					break;
				case 'R':
					if(evalfunc->arg_idx_run_ctx>=0)  goto breakmatch;
					evalfunc->arg_idx_run_ctx=i;
					rv=ExprValueType::RStringType().CanAssignType(
						lfi->arg_type[i],&evalfunc->arg_assfunc[i]);
					break;
				case 'S':
					if(evalfunc->arg_idx_save_ctx>=0)  goto breakmatch;
					evalfunc->arg_idx_save_ctx=i;
					rv=ExprValueType::RStringType().CanAssignType(
						lfi->arg_type[i],&evalfunc->arg_assfunc[i]);
					break;
				case 's':
					if(evalfunc->arg_idx_skip>=0)  goto breakmatch;
					evalfunc->arg_idx_skip=i;
					rv=ExprValueType::RIntegerType().CanAssignType(
						lfi->arg_type[i],&evalfunc->arg_assfunc[i]);
					break;
				default:  assert(0);
			}
			if(rv==0)  // No, cannot. Type mismatch. 
			{  goto breakmatch;  }
		}
		
		// Currently, time arg MUST be present. 
		if(evalfunc->arg_idx_t<0) break;
		
		// Looks good. 
		match_ok=1;
	} while(0); breakmatch:;
	if(!match_ok)
	{
		Error(lfi->opfunc->GetLocationRange(),
			"no matching call to setting %s\n",
			FuncCallWithArgsString(CompleteName(),lfi).str());
		DELETE(evalfunc);
		return(0);
	}
	
	// Okay, store eval info: 
	lfi->ani_function=this;
	lfi->ret_evt=ExprValueType::CVoidType;
	lfi->evalfunc=evalfunc;
	lfi->may_try_ieval=0;
	
	return(0);
}


int AniSetting::_DO_EvalSetting(EvalSettingArgs *args,ExecThreadInfo *info)
{
	Value tmp;
	assert(args->t);  // t _must_ be specified (currently)
	args->t->GetPOD(&tmp);
	const Range *timerange=tmp.GetPtr<Range>();
	
	// Now we "eval" the setting. That means it "was called like 
	// a function". And that actually means: we run the animation 
	// in this setting. 
	
	// Context name. Currently, this is always the default name "". 
	String ctx_name("");
	
	// A verbose message: 
	fprintf(stderr,"Running setting %s for time %s using context \"%s\".\n",
		CompleteName().str(),
		timerange->ToString().str(),
		ctx_name.str());
	
	// TODO: Check name; create new context if needed; make the 
	// context the active one; create new exec thread if needed. 
	// Let's do it: 
	AniAnimation *ani=(AniAnimation*)Parent();
	assert(ani->ASType()==AST_Animation);
	// This will return the context with the passed name or 
	// allocate a new one if it did not yet exist. 
	Animation_Context *ctx=ani->GetContextByName(ctx_name,this);
	
	// Set the time range: 
	// FIXME: This needs to be fixed, too. We need ONE CCIF_Setting 
	//    with all the time info etc (any maybe one per calc core 
	//    for the calc core internals). 
	for(int i=0; i<CC::CCIF_Factory::NFactories(); i++)
	{
		ctx->ccif[i]->SetTimeRange(*timerange);
	}
	
	// Actually switch contexts and run them: 
	ani->RunContext(ctx);
	
	return(0);
}


ExprValue &AniSetting::GetInternalVarRef(/*BuiltinVarIndex*/int bvi)
{
	assert(bvi>=0 && bvi<_BVI_LAST);
	return(ivar_ev[bvi]);
}


void AniSetting::vn_change(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	switch(bvi)
	{
		case BVI_Time:
		{
			double new_val=ivar_ev[bvi].GetPODDouble();
			if(new_val==setting_time)  return;
			
			// setting_time_ev changed. 
			assert(!"!implemented");
			
		}	break;
		default: assert(0);
	}
}

void AniSetting::vn_delete(void *hook)
{
	int bvi=((ExprValue*)hook)-ivar_ev;
	assert(bvi>=0 && bvi<_BVI_LAST);
	
	// This may not happen. 
	assert(0);
}


AniSetting::AniSetting(const RefString &name,ANI::TreeNode *_treenode,
	AniAniSetObj *_parent) : 
	AniAniSetObj(AST_Setting,name,_treenode,_parent),
	ValueNotifyHandler()
{
	settinginit_eval_func=NULL;
	
	// Set up initial values for the internal vars: 
	setting_time=0.0;
	
	int _count=0;
	ivar_ev[BVI_Time].assign(setting_time);  ++_count;
	assert(_count==_BVI_LAST);
	
	// Register value change notifier: 
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNInstall(ivar_ev[i],&ivar_ev[i]);
		assert(!rv);
	}
	
	// Register internal functions (atan(),sin(),$pov(),...) 
	// and variables ($time,...): 
	AniInternalFunction::RegisterInternalFunctions(this);
	AniInternalVariable::RegisterInternalVariables(this);
}

AniSetting::~AniSetting()
{
	for(int i=0; i<_BVI_LAST; i++)
	{
		int rv=ValueNotifyHandler::VNUninstall(ivar_ev[i]);
		assert(rv==0);
	}
	
	DELETE(settinginit_eval_func);
}

