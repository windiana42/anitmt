/*
 * core/a_anisetobj.cc
 * 
 * ANI common part for animation, setting and object scope. 
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


using namespace ANI;


void AniAniSetObj::CG_LookupConstructor(LookupFunctionInfo *lfi)
{
	lfi->ani_function=NULL;
	
	// See if some named args are specified more than once: 
	int nerrors=_CheckFuncArgRepetition(lfi);
	if(nerrors)  return;
	
	// Do the "lookup": 
	ASB_List lul;
	for(AniFunction *af=functionlist.first(); af; af=af->next)
	{
		if(!(af->attrib & AniFunction::IS_Constructor))  continue;
		lul.append(af);
	}
	
	RefString name_for_error;
	if(lfi->name_idf)
	{
		assert(lfi->name_idf->NType()==TN_Identifier);
		name_for_error=lfi->name_idf->CompleteStr();
	}
	else
	{
		name_for_error.sprintf(0,"%s::%s",name.str(),name.str());
	}
	
	TNLocation loc;
	if(lfi->call_loc)  // for new array only
	{  loc=*lfi->call_loc;  }
	else if(lfi->name_idf)
	{  loc=lfi->name_idf->GetLocationRange();  }
	else assert(0);
	
	AniFunction *afunc=_LookupFunction(&lul,lfi,name_for_error,loc);
	
	if(afunc)
	{
		// Store the info for the caller: 
		lfi->ani_function=afunc;
		// ret_evt is [void] here but everyone knows better...
		// (Because the constructor func must return "this".) 
		lfi->ret_evt=ExprValueType::CVoidType; //afunc->GetReturnType(); <-- which is [void]
		lfi->allocfunc=new AniUserVariable::EAF_OpNew(this);
		assert(lfi->evalfunc);
		
		if(lfi->name_idf)
		{
			// Do the TreeNode update and self removal:
			assert(((AniScopeBase*)lfi->name_idf->ani_scope)==this);
			// ***ALREADY DONE***
			/*
			AniIncomplete *ai=(AniIncomplete*)lfi->name_idf->ani_scope;
			assert(ai->ASType()==AST_Incomplete);
			ai->_TNUpdate_SelfRemoval(afunc);*/
		}
	}
}


AniScopeBase *AniAniSetObj::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo *tri)
{
	// Function works recursively. Tell all our children: 
	anisetobjlist.RegisterTN_DoFinalChecks(tri);
	functionlist.RegisterTN_DoFinalChecks(tri);
	variablelist.RegisterTN_DoFinalChecks(tri);
	//incompletetlist.RegisterTN_DoFinalChecks(tri);
	
	// After having checked the functions (above), 
	// count constructors and destructors: 
	int n_constr=0;
	AniFunction *destructor=NULL;
	AniUserFunction *settinginit=NULL;
	for(AniFunction *func=functionlist.first(); func; func=func->next)
	{
		if(func->attrib & AniFunction::IS_Constructor)  ++n_constr;
		if(func->attrib & AniFunction::IS_Destructor)
		{
			if(!destructor)
			{  destructor=func;  }
			else
			{
				TNLocation loc=func->NameLocation();
				Error(loc,
					"destructor for %s \"%s\" already defined @%s\n",
					AniScopeTypeStr(),CompleteName().str(),
					destructor->NameLocation().PosRangeStringRelative(loc).str());
				++tri->n_errors;
			}
		}
		if(func->attrib & AniFunction::IS_SettingInit)
		{
			if(!settinginit)
			{
				// func may not be an internal function here 
				// because of GetSettingInitEvalFunc(). 
				assert(func->ASType()==AST_UserFunc);
				settinginit=(AniUserFunction*)func;
			}
			else
			{
				TNLocation loc=func->NameLocation();
				Error(loc,
					"setting init for %s \"%s\" already defined @%s\n",
					AniScopeTypeStr(),CompleteName().str(),
					settinginit->NameLocation().PosRangeStringRelative(loc).str());
				++tri->n_errors;
			}
		}
	}
	
	if(!n_constr)
	{
		if(ASType()==AST_Object)
		{  Warning(NameLocation(),
			"%s \"%s\" does not have a constructor\n",
			AniScopeTypeStr(),CompleteName().str());  }
	}
	
	if(destructor)
	{
		if(destructor->ASType()==AST_UserFunc && 
		   !((AniUserFunction*)destructor)->arglist.is_empty())
		{
			// CURRENTLY, this cannot happen because the grammar 
			// already rejects this. 
			Error(((TNFunctionDef*)destructor->GetTreeNode())->first_arg->
				GetLocationRange(),
				"destructor may not take arguments\n");
			++tri->n_errors;
		}
		// Return value already checked. 
		
		Warning(NameLocation(),
			"%s \"%s\": destructors are not supported\n",
			AniScopeTypeStr(),CompleteName().str());
	}
	else
	{
		/*if(ASType()==AST_Object)
		{  Warning(NameLocation(),
			"%s \"%s\" does not have a destructor\n",
			AniScopeTypeStr(),CompleteName().str());  }*/
	}
	
	if(settinginit)
	{
		// The setting init function currently may not 
		// take any arguments. This may be changed in the future. 
		if(settinginit->ASType()==AST_UserFunc && 
			!((AniUserFunction*)settinginit)->arglist.is_empty())
		{
			Error(((TNFunctionDef*)settinginit->GetTreeNode())->first_arg->
				GetLocationRange(),
				"setting init function may not take arguments\n");
			++tri->n_errors;
		}
		else
		{
			// Create eval handler for setting init: 
			TNOperatorFunction::EvalFunc *evalfunc=
				settinginit->GetSettingInitEvalFunc();
			assert(evalfunc);
			((AniSetting*)this)->settinginit_eval_func=evalfunc;
		}
	}
	else
	{
		if(ASType()==AST_Setting)
		{
			Error(NameLocation(),
				"%s \"%s\" does not have an init function\n",
				AniScopeTypeStr(),CompleteName().str());
			++tri->n_errors;
		}
	}
	
	return(NULL);  // Always NULL; that is okay. 
}


int AniAniSetObj::InitStaticVariables(ExecThreadInfo *info,int flag)
{
	int nerrors=0;
	
	if(flag>0)
	{
		// Okay, so order is important here. 
		// First, init our variables: 
		nerrors+=variablelist.InitStaticVariables(info,flag);
		// Then traverse the level below. 
		nerrors+=anisetobjlist.InitStaticVariables(info,flag);
		// We need not visit the functions because automatic 
		// static vars are initialized when used for the first 
		// time, as usual. RIGHT... BUT: 
		// WE NEED to set prototype values for those or 
		// the ExprValues will have ExprValue::iev=NULL and 
		// thus cannot be used as lvalue in the decl. 
		nerrors+=functionlist.InitStaticVariables(info,flag);
	}
	else if(flag<0)
	{
		// Cleanup goes the other way round than init. 
		// Note that also automatic static vals need to be cleaned up. 
		nerrors+=variablelist.InitStaticVariables(info,flag);
		nerrors+=anisetobjlist.InitStaticVariables(info,flag);
		nerrors+=functionlist.InitStaticVariables(info,flag);
	}
	else assert(0);
	
	return(nerrors);
}


int AniAniSetObj::FixIncompleteTypes()
{
	int nerrors=0;
	
	nerrors+=anisetobjlist.FixIncompleteTypes();
	nerrors+=functionlist.FixIncompleteTypes();
	nerrors+=variablelist.FixIncompleteTypes();
	//nerrors+=incompletelist.FixIncompleteTypes();
	
	nerrors+=_FixEntriesInTypeFixList(&typefix_list);
	
	return(nerrors);
}


int AniAniSetObj::PerformCleanup(int cstep)
{
	int nerr=0;
	nerr+=anisetobjlist.PerformCleanup(cstep);
	nerr+=functionlist.PerformCleanup(cstep);
	nerr+=variablelist.PerformCleanup(cstep);
	nerr+=incompletelist.PerformCleanup(cstep);
	
	switch(cstep)
	{
		case 1:
			while(!incompletelist.is_empty())
			{  delete incompletelist.first();  }  // NOT popfirst(). 
			assert(typefix_list.is_empty());
			break;
		case 2:
			iinfo=new InstanceInfo();
			FillInInstanceInfo(iinfo,&variablelist,/*vtype=*/0);
			break;
		default:  assert(0);
	}
	
	return(nerr);
}


void AniAniSetObj::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	anisetobjlist.LookupName(name,ret_list);
	functionlist.LookupName(name,ret_list);
	variablelist.LookupName(name,ret_list);
	// NEVER try to look up in AniIncomplete -- that are no identifiers 
	// just placeholders from where to look up existing ones!
	if(query_parent && parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniAniSetObj::QueueChild(AniScopeBase *asb,int do_queue)
{
	bool is_ok=0;
	switch(ASType())
	{
		case AST_Animation:
			is_ok=1;  break;
		case AST_Setting:
			is_ok=(asb->ASType()!=AST_Animation);  break;
		case AST_Object:
			is_ok=(asb->ASType()!=AST_Animation && 
			       asb->ASType()!=AST_Setting);  break;
		default: assert(0);
	}
	assert(is_ok);
	
	if(do_queue>0) switch(asb->ASType())
	{
		case AST_Animation:  // fall 
		case AST_Setting:    //      throgh
		case AST_Object:     anisetobjlist.append((AniAniSetObj*)asb);    break;
		case AST_UserFunc:   //  fall through
		case AST_InternalFunc:  functionlist.append((AniFunction*)asb);   break;
		case AST_UserVar:    //  fall through
		case AST_InternalVar:   variablelist.append((AniVariable*)asb);   break;
		case AST_Incomplete: incompletelist.append((AniIncomplete*)asb);  break;
		default: assert(0);
	}
	else if(do_queue<0) switch(asb->ASType())
	{
		case AST_Animation:  // fall 
		case AST_Setting:    //      throgh
		case AST_Object:     anisetobjlist.dequeue((AniAniSetObj*)asb);   break;
		case AST_UserFunc:   //  fall through
		case AST_InternalFunc:  functionlist.dequeue((AniFunction*)asb);  break;
		case AST_UserVar:    //  fall through
		case AST_InternalVar:   variablelist.dequeue((AniVariable*)asb);  break;
		case AST_Incomplete: //assert(incompletelist.find((AniIncomplete*)asb));
		                     incompletelist.dequeue((AniIncomplete*)asb); break;
		default: assert(0);
	}
}


int AniAniSetObj::CountScopes(int &n_incomplete) const
{
	int n=1;
	n+=anisetobjlist.CountScopes(n_incomplete);
	n+=functionlist.CountScopes(n_incomplete);
	n+=variablelist.CountScopes(n_incomplete);
	n+=incompletelist.CountScopes(n_incomplete);
	return(n);
}


ANI::TNLocation AniAniSetObj::NameLocation() const
{
	switch(ASType())
	{
		case AST_Animation:
		{
			fprintf(stderr,"OOPS: AniAniSetObj::NameLocation() for "
				"AST_Animation called.\n");
			assert(0);
			return TNLocation();
		}
		case AST_Setting:
		{
			const TNIdentifier *idf=(const TNIdentifier*)
				((TNSetting*)GetTreeNode())->down.first();
			return(idf->GetLocationRange());
		}
		case AST_Object:
		{
			const TNIdentifier *idf=(const TNIdentifier*)
				((TNObject*)GetTreeNode())->down.first();
			return(idf->GetLocationRange());
		}
		default: assert(0);
	}
	return TNLocation();
}


LinkedList<AniScopeBase::TypeFixEntry> *AniAniSetObj::GetTypeFixList()
{
	// Not inline as virtual. 
	return(&typefix_list);
}


AniScopeBase::InstanceInfo *AniAniSetObj::GetInstanceInfo()
{
	// Not inline as virtual. 
	return(iinfo);
}


const char *AniAniSetObj::CG_AniScopeTypeStr() const
{
	switch(ASType())
	{
		case AST_Animation:    return("animation");
		case AST_Setting:      return("setting");
		case AST_Object:       return("object");
	}
	return("-??""?-");  // argh... "??-" is a "trigraph"
}


void AniAniSetObj::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(name.str());
	d->Append("\n");
	
	d->AddIndent();
	anisetobjlist.AniTreeDump(d);
	functionlist.AniTreeDump(d);
	variablelist.AniTreeDump(d);
	incompletelist.AniTreeDump(d);
	d->SubIndent();
}


AniAniSetObj::AniAniSetObj(AniScopeType _astype,const RefString &_name,
	TreeNode *_treenode,AniAniSetObj *_parent) : 
	AniScopeBase(_astype,_treenode,_name,_parent)
{
	if(_astype!=AST_Animation && 
	   _astype!=AST_Setting && 
	   _astype!=AST_Object )
	{  assert(0);  }
	
	iinfo=NULL;
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniAniSetObj::~AniAniSetObj()
{
	anisetobjlist.ClearList();
	functionlist.ClearList();
	variablelist.ClearList();
	incompletelist.ClearList();
	
	while(!typefix_list.is_empty())
	{  delete typefix_list.popfirst();  }
	
	DELETE(iinfo);
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/-1);  }
}
