/*
 * core/a_ifuncpov.cc
 * 
 * ANI POVRay-specific function ($pov())
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
#include "aniinstance.h"


using namespace ANI;

//------------------------------------------------------------------------------
// $pov(scan={"file","file2","file3"})
//   Return value: number of errors while scanning
// object.$pov(attach="POV-object",[scan="file_to_scan_if_obj_not_found"])
//   Return value: 
//     0 -> OK
//     1 -> object not found
//     2 -> failed to attach
//     3 -> error scanning file
// object.$pov(detach="POV-object")
//     0 -> OK
//     1 -> not attached to passed object
//------------------------------------------------------------------------------

void AniInternalFunc_POV::_RegisterInternalFunctions(AniAniSetObj *parent)
{
	RefString tmp;
	tmp.set("$pov");
	
	AniInternalFunc_POV *fp=NULL;
	switch(parent->ASType())
	{
		case AST_Object:
			fp=new AniInternalFunc_POV(tmp,parent,POVFT_Member,
				/*treenode_pov_hook=*/RegisterPOVHook((AniObject*)parent));
			// Also set explicitly for faster access: 
			if(((AniObject*)parent)->pov_attach_detach) assert(0);
			((AniObject*)parent)->pov_attach_detach=fp;
			// FALL THROUGH
		case AST_Setting:
		case AST_Animation:
			fp=new AniInternalFunc_POV(tmp,parent,POVFT_Static,NULL);
			break;
		default:  break;  // do nothing
	}
}


TNFunctionDef *AniInternalFunc_POV::RegisterPOVHook(AniObject *obj)
{
	// HOW IT WORKS: 
	// We want to process the expressions in the POV (object) command 
	// comment and hence, these expressions need to be in the main 
	// expression tree. As it contains variables from the object, it 
	// must be inside the object. But we cannot put it at the place 
	// where the $pov(attach=...) function is called because it may 
	// not access automatic vars. AND, we may not put it simply inside 
	// the TNObject because the var lookup code expects it to be inside 
	// some function which has an anonymous scope (compound stmt). 
	// So, here we go: 
	// We create an own "internal" function called "$$povhook". This 
	// function is a TNFunctionDef (no return type) and contains a 
	// CompoundStmt. This compound stmt now contains all the 
	// PTNObjectSpec nodes (etc) currently attached. 
	// Object spec node_s_??
	// YES! We parse the object spec tree when reading the POV command 
	// comment and keep it as a raw parsed tree (no eval funcs, no 
	// registration). When the object is attached, we _CLONE_ the tree, 
	// put it into the mentioned TNCompoundStmt, register it and 
	// TF (ExprTF) it so that we can actually use it. 
	// Upon detach, we have to remove and delete it again. 
	// NOTE: The povhook function is the TreeNode associated with 
	//       the member "$pov" function. And vice versa: the function 
	//       pointer of the povhook TNFunctionDef points to the 
	//       associated "$pov" (member) func. 
	// NOTE: The POV objects are actually attached to the animation 
	//       context and NOT to the object instance. This is to avoid 
	//       problems with object instances shared among different 
	//       settings. 
	
	// So, here we create the POV hook (TNFunctionDef + TNCompoundStmt). 
	assert(obj->ASType()==AST_Object);
	
	RefString _pov_hook_str;
	_pov_hook_str.set("$$povhook");
	
	TNCompoundStmt *compound=new TNCompoundStmt(TNLocation(),TNLocation());
	TNFunctionDef *povhook=new TNFunctionDef(
		/*name=*/new TNIdentifier( new TNIdentifierSimple(
			TNLocation(), _pov_hook_str) ),
		/*ret_type=*/NULL,  /*allowed: "destructor"*/
		/*func_body=*/compound);
	
	TNObject *tn_obj=(TNObject*)obj->GetTreeNode();
	assert(tn_obj && tn_obj->NType()==TN_Object);
	tn_obj->AddChild(povhook);
	
	return(povhook);
}


//------------------------------------------------------------------------------

int AniInternalFunc_POV::EvalFunc_POVStatic::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// Currently simple. Only one arg. 
	// We have one arg, so context switches aren't a problem. 
	ExprValue tmp;
	if(tn->down.first()->next->Eval(tmp,info))
	{  return(1);  }
	// It is important that we DO assign here. So that 
	// I can use ExprValueArray and dito::IndexGet() in the 
	// handling functions (_DO_ScanPOVFiles()...). 
	assert(scan_assfunc);
	ExprValue file_list;
	scan_assfunc->Assign(/*lvalue=*/file_list,/*rvalue=*/tmp);
	
	int rv=function->_DO_ScanPOVFiles(file_list);
	retval.assign(rv);
	
	return(0);
}

AniInternalFunc_POV::EvalFunc_POVStatic::~EvalFunc_POVStatic()
{
	DELETE(scan_assfunc);
}

//------------------------------------------------------------------------------

int AniInternalFunc_POV::EvalFunc_POVMember::Eval(ExprValue &retval,
	TNOperatorFunction *tn,ExecThreadInfo *info)
{
	// May only be called with thread info. 
	assert(info);
	EvalStateStack *ess=info->ess;
	
	// First, there is a pretty standard arg eval step (like for simple 
	// internal functions), just without arg permutation: 
	ExprValue val[nargs];
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
			return(1);
		}
		assert(arg_assfunc[idx]);
		arg_assfunc[idx]->Assign(
			/*lvalue=*/val[idx],
			/*rvalue=*/tmpval);
	}
	//assert(idx==nargs);  // can be left away
	
	// Now, check which args were specified and supply defaults for 
	// the other ones. 
	ExprValue *objname=&val[objname_idx];
	ExprValue *scanfile=scanfile_idx<0 ? NULL : &val[scanfile_idx];
	
	// Get context: 
	Animation_Context *ctx=((ExecThread*)info)->Context();
	// NOTE: ctx may be NULL here in case we're not in 
	//       ani context. This is valid. 
	
	int rv=function->_DO_POVAttachDetach(ctx,objname,scanfile,action,
		info->stack->GetThisPtr());
	retval.assign(rv);
	
	return(0);
}

AniInternalFunc_POV::EvalFunc_POVMember::~EvalFunc_POVMember()
{
	if(arg_assfunc)
	{
		for(int i=0; i<nargs; i++)
		{  DELETE(arg_assfunc[i]);  }
		arg_assfunc=(ANI::AssignmentFuncBase**)LFree(arg_assfunc);
	}
}

//******************************************************************************


AniAnimation *AniInternalFunc_POV::_GetAnimation()
{
	#warning "Could be replaced by something O(1), e.g. by setting ptr in constructor."
	for(AniScopeBase *asb=Parent(); asb; asb=asb->Parent())
	{
		if(asb->ASType()==AST_Animation)
		{  return((AniAnimation*)asb);  }
	}
	assert(0);  // Animation must be in hierarchy. 
	return(NULL);
}


int AniInternalFunc_POV::_DO_ScanPOVFiles(const ExprValue &file_list)
{
	// Counts number of errors while scanning: 
	int scan_errors=0;
	
	// file_list is a string array. 
	// Need to get the elements...
	ExprValueArray filearr;
	file_list.GetArray(&filearr);
	ssize_t nfiles=filearr.GetSize();
	// If nfiles<0 -> ExprValueArray::None, i.e. file_list has wrong type. 
	assert(nfiles>=0);
	for(ssize_t filei=0; filei<nfiles; filei++)
	{
		ExprValue tmp;
		int rv=filearr.IndexGet(filei,&tmp);
		assert(rv==0);
		Value tmpv;
		tmp.GetPOD(&tmpv);
		String file(*tmpv.GetPtr<String>());
		// Oh dear... Quite a lot code just to extract an element. 
		
		// Silently ignore empty strings. 
		if(file.len()<=0)  continue;
		
		RefString filename;
		filename.set(file.str());
		
		scan_errors+=_ScanPOVFile(filename);
	}
	
	return(scan_errors);
}


int AniInternalFunc_POV::_ScanPOVFile(const RefString &filename)
{
	// Simply forward the call. 
	return(_GetAnimation()->ScanPOVFile(filename));
}


int AniInternalFunc_POV::_DO_POVAttachDetach(Animation_Context *ctx,
	const ExprValue *_objname,const ExprValue *_scanfile,int action,
	const ScopeInstance &obj_instance)
{
	RefString objname,scanfile;
	
	Value tmpv;
	_objname->GetPOD(&tmpv);
	objname.set(tmpv.GetPtr<String>()->str());
	
	if(_scanfile)
	{
		_scanfile->GetPOD(&tmpv);
		scanfile.set(tmpv.GetPtr<String>()->str());
	}
	
	// Must have "this" pointer set (_member_ function). 
	assert(!!obj_instance);
	
	// Okay, values extracted. Do it: 
	return(_POVAttachDetach(ctx,objname,scanfile,action,obj_instance));
}


int AniInternalFunc_POV::_POVAttachDetach(Animation_Context *ctx,
	const RefString &objname,const RefString &scanfile,int action,
	const ScopeInstance &obj_instance)
{
	AniAnimation *ani=_GetAnimation();
	const AniScopeInstanceBase *asib=
		(AniScopeInstanceBase*)obj_instance.GetASIB();
	assert(asib->ASType()==AST_Object);
	AniObjectInstance *obj_inst=(AniObjectInstance*)asib;
	
	if(action>0)
	{
		int rv=ani->POVAttach(ctx,obj_inst,objname);
		if(rv==1 // Object not found
			&& !!scanfile && scanfile.len()>0)
		{
			rv=_ScanPOVFile(scanfile);
			if(rv)  return(3);
			rv=ani->POVAttach(ctx,obj_inst,objname);  // (try again)
		}
		return(rv);
	}
	else if(action<0)
	{
		return(ani->POVDetach(ctx,obj_inst,objname));
	}
	
	assert(0);  // May not happen (action==0). 
	return(0);
}


//------------------------------------------------------------------------------

int AniInternalFunc_POV::_Match_FuncArgs_Member(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	// Okay, so the present syntax is: 
	//  Either an "attach=" or a "detach=" object; if only a string is 
	//  specified, treat as attach. 
	//  Additionally, you may specify a second arg called "scan" which 
	//  specifies a file to be scanned if the object was not found (and 
	//  then search again for the objec6). 
	
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	
	ExprValueType str_t=ExprValueType::RStringType();
	
	// Stores argument indices in lfi. 
	int attach_idx=-1;
	int detach_idx=-1;
	int scan_idx=-1;
	char which_arg='\0';  // 'a','d','s': attach,detach,scan
	
	for(int i=0; i<lfi->n_args; i++)
	{
		if(lfi->arg_name[i])
		{
			// Look up argument: 
			RefString name=lfi->arg_name[i]->CompleteStr();
			if(name=="attach")  which_arg='a';
			else if(name=="detach")  which_arg='d';
			else if(name=="scan")  which_arg='s';
			else return(0);
		}
		else switch(which_arg)  // <-- switch forward one arg
		{
			case '\0':  which_arg='a';  break;
			case 'a':  // fall through
			case 'd':  which_arg='s';  break;
			case 's':  return(0);
			default:  assert(0);
		}
		
		switch(which_arg)
		{
			case '\0':  return(0);
			case 's':
				if(scan_idx>=0)  return(0);  // Two or more scan args.
				scan_idx=i;
				break;
			case 'a':  // fall through
			case 'd':
				if(attach_idx>=0 || detach_idx>=0)  // Two or more {at/de}tach args.
				{  return(0);  }
				(which_arg=='a' ? attach_idx : detach_idx)=i;
				break;
			default:  assert(0);
		}
		
		// Check arg type: 
		int rv=str_t.CanAssignType(lfi->arg_type[i],NULL);
		if(rv==0)  // No, cannot. Type mismatch. 
		{  return(0);  }
		else if(rv==1)  ++args_matching_cv;
		else  ++args_matching;
	}
	assert(args_matching+args_matching_cv==lfi->n_args);
	
	// Need at least attach or detach. 
	if(attach_idx<0 && detach_idx<0)  return(0);
	
	// Cannot use "scan" with "detach". 
	if(detach_idx>=0 && scan_idx>=0)  return(0);
	
	if(store_evalhandler)
	{
		EvalFunc_POVMember *evalfunc=new EvalFunc_POVMember(this);
		
		evalfunc->nargs=lfi->n_args;
		assert(evalfunc->nargs<=2);  // <-- Remove once there are more args. 
		evalfunc->arg_assfunc=(ANI::AssignmentFuncBase**)LMalloc(
			evalfunc->nargs*sizeof(ANI::AssignmentFuncBase*));
		for(int i=0; i<evalfunc->nargs; i++)
		{  evalfunc->arg_assfunc[i]=NULL;  }
		
		evalfunc->objname_idx=attach_idx>=0 ? attach_idx : detach_idx;
		if(evalfunc->objname_idx<0 || evalfunc->objname_idx>=lfi->n_args)
		{  assert(0);  }
		evalfunc->action = attach_idx>=0 ? +1 : -1;
		
		int rv=str_t.CanAssignType(lfi->arg_type[evalfunc->objname_idx],
			&evalfunc->arg_assfunc[evalfunc->objname_idx]);
		assert(rv>0);  // NOT 0 or <0. 
		
		// "scan" argument if any: 
		evalfunc->scanfile_idx=scan_idx;
		if(scan_idx>=0)
		{
			rv=str_t.CanAssignType(lfi->arg_type[evalfunc->scanfile_idx],
				&evalfunc->arg_assfunc[evalfunc->scanfile_idx]);
			assert(rv>0);  // NOT 0 or <0. 
		}
		
		lfi->evalfunc=evalfunc;
		lfi->ret_evt=ExprValueType::RIntegerType();
		//lfi->may_try_ieval stays 0 (of course)
	}
	
	// Okay, looks good. 
	return(args_matching_cv ? 1 : 2);
}


int AniInternalFunc_POV::_Match_FuncArgs_Static(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	// Static is currently simple. 
	// We accept one arg which is a string array. 
	// This may become more complicated in the future...
	
	int args_matching=0;     // args matching
	int args_matching_cv=0;  // args matching with type conversion
	
	ExprValueType strarr_t(ExprValueType::RStringArrayType());
	
	if(lfi->n_args!=1)
	{  return(0);  }
	
	if(lfi->arg_name[0])
	{
		RefString name=lfi->arg_name[0]->CompleteStr();
		// Name specified. "Look it up." 
		if(name!="scan")  return(0);
	}
	
	int rv=strarr_t.CanAssignType(lfi->arg_type[0],NULL);
	if(rv==0)  // No, cannot. Type mismatch. 
	{  return(0);  }
	else if(rv==1)  ++args_matching_cv;
	else  ++args_matching;
	
	// Okay, valid "$pov(scan=<strarray>)" call. 
	
	if(store_evalhandler)
	{
		EvalFunc_POVStatic *evalfunc=new EvalFunc_POVStatic(this);
		rv=strarr_t.CanAssignType(lfi->arg_type[0],&evalfunc->scan_assfunc);
		assert(rv>0);  // NOT 0 or <0. 
		
		lfi->evalfunc=evalfunc;
		lfi->ret_evt=ExprValueType::RIntegerType();
		//lfi->may_try_ieval stays 0 (of course)
	}
	
	// Okay, looks good. 
	assert(args_matching+args_matching_cv==1/*=nargs*/);
	return(args_matching_cv ? 1 : 2);
}


int AniInternalFunc_POV::Match_FunctionArguments(LookupFunctionInfo *lfi,
	int store_evalhandler)
{
	//                         STOP. 
	// KEEP YOUR FINGERS OFF THIS FUNCTION UNLESS YOU REALLY 
	//               KNOW WHAT YOU ARE DOING. 
	// Huch?! ... Well it's meant for the helpers called below. 
	
	// None of the named args in lfi should occure more than once. 
	// This is checked on higher level (because it would make no 
	// sense to check that for each function -- it has to be done 
	// once per lookup and not once per function). 
	
	switch(povftype)
	{
		case POVFT_Member:
			return(_Match_FuncArgs_Member(lfi,store_evalhandler));
		case POVFT_Static:
			return(_Match_FuncArgs_Static(lfi,store_evalhandler));
		default: assert(0);
	}
	
	return(0);  // "no match"
}


void AniInternalFunc_POV::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	// Forward calls from within the anon scope. 
	if(query_parent && parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniInternalFunc_POV::QueueChild(AniScopeBase *asb,int do_queue)
{
	if(do_queue>0) switch(asb->ASType())
	{
		case AST_AnonScope:  anonscopelist.append((AniAnonScope*)asb); break;
		default: assert(0);
	}
	else if(do_queue<0) switch(asb->ASType())
	{
		case AST_AnonScope:  anonscopelist.dequeue((AniAnonScope*)asb); break;
		default: assert(0);
	}
}

int AniInternalFunc_POV::CountScopes(int &n_incomplete) const
{
	int n=1;
	n+=anonscopelist.CountScopes(n_incomplete);
	return(n);
}


int AniInternalFunc_POV::PerformCleanup(int cstep)
{
	int nerr=0;
	
	nerr+=anonscopelist.PerformCleanup(cstep);
	
	switch(cstep)
	{
		case 1:  break;  // Nothing to do here. 
		case 2:
			// Create instance info... No need for one. 
			break;
		default:  assert(0);
	}
	
	return(nerr);
}


String AniInternalFunc_POV::PrettyCompleteNameString() const
{
	switch(povftype)
	{
		case POVFT_Member:
			return(String("int $pov("
				"string attach|detach,"
				"string scan="","
				") [special member]"));
		case POVFT_Static:
			return(String("int $pov("
				"string[] scan=NULL) [special member]"));
		default: ; // Fall off. 
	}
	assert(0);
	return(String("???"));
}


void AniInternalFunc_POV::AniTreeDump(StringTreeDump *d)
{
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	switch(povftype)
	{
		case POVFT_Member:
			d->Append("$pov(attach|detach,scan=\"\"): int [special member]");
			break;
		case POVFT_Static:
			d->Append("$pov(scan): int [special]");
			break;
		default: assert(0);
	}
	d->Append("\n");
}


AniInternalFunc_POV::AniInternalFunc_POV(const RefString &_name,
	AniAniSetObj *_parent,POVFuncType _povftype,
	TNFunctionDef *_treenode_pov_hook) : 
	AniInternalFunction(_name,_parent,
		/*attrib=*/_povftype==POVFT_Static ? IS_Static : 0,
		/*treenode=*/_treenode_pov_hook),
	povftype(_povftype)
{
	if(_treenode_pov_hook && _treenode_pov_hook->function) assert(0);
	//_treenode_pov_hook->function=this; gets set up in registration step. 
}

AniInternalFunc_POV::~AniInternalFunc_POV()
{
	anonscopelist.ClearList();
}
