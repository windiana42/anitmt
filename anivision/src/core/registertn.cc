/*
 * core/registertn.cc
 * 
 * Ani parser tree node registration at core. 
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

#include <calccore/ccif.h>

using namespace ANI;


int AniScopeBase::_MatchReservedNames(const RefString &var_name,
	TNIdentifier *name_idf)
{
	// Need only check for those reserved names which are not own 
	// tokens recognized by the lexer. 
	// Well, the reserved words ARE now ALL identified by the lexer 
	// but this cannot hurt anyways...
	static const char *reserved[]=
	{
		"int", "scalar", "range", "vector", "matrix", "string",
		"new", "delete", 
		"if", "else", "for", "do", "while", "break", "return", 
		"const", "static", "object", "method", "setting", "animation", 
		"this", "NULL", NULL
	};
	
	for(int i=0; reserved[i]; i++)
	{
		if(var_name!=reserved[i])  continue;
		Error(name_idf->GetLocationRange(),
			"\"%s\" cannot be used as an identifier name\n",
			var_name.str());
		return(1);
	}
	
	return(0);
}


int AniScopeBase::CheckDollarSimpleIDF(TNIdentifierSimple *idf)
{
	if(idf->name.str()[0]=='$')
	{
		Error(idf->GetLocationRange(),
			"'$' prefix in \"%s\" is reserved for built-in animation "
			"identifiers\n",
			idf->name.str());
		return(1);
	}
	return(0);
}


ANI::TNIdentifierSimple *AniScopeBase::_RegisterCheckTN(ANI::TNIdentifier *id,
	AniScopeType wanted_scope,ANI::TRegistrationInfo *tri)
{
	if(id->NType()!=TN_Identifier)
	{  fprintf(stderr,"OOPS: %d,%d\n",id->NType(),TN_Identifier);
		assert(0);  }
	
	// See if this is a simple name i.e. without "::". 
	// I only allow such names here (could be patched, however). 
	if(id->down.first()!=id->down.last())
	{
		Error(id->GetLocationRange(),
			"cannot use scoped type \"%s\" as %s name\n",
			id->CompleteStr().str(),_AniScopeTypeStr(wanted_scope));
		++tri->n_errors;
		return(NULL);
	}
	
	// Builtin names start with "$", so this is not allowed. 
	if(CheckDollarSimpleIDF((TNIdentifierSimple*)id->down.first()))
	{
		// Just increase error counter but do not return NULL 
		// so that we can process the scope in question. 
		++tri->n_errors;
	}
	
	return((ANI::TNIdentifierSimple*)id->down.first());
}


int AniScopeBase::_RegisterCheckNameKnown(ANI::TNIdentifierSimple *fid,
	ANI::TRegistrationInfo *tri)
{
	// Only simple names, none with scope here. 
	// (Could be patched to allow for that as well; must re-write
	// check in _DoRegisterTN() first. 
	assert(!fid->next && !fid->prev);
	
	const RefString &name=fid->name;
	
	// See if this name already exists here and in scope of 
	// immediate parent: 
	ASB_List lul;
	LookupName(name,&lul,/*query_parent=*/1/*=only immediate parent*/);
	if(!lul.is_empty())
	{
		// No, that's bad: We already know that identifier. 
		
		for(ASB_List::Node *nn=lul.first(); nn; nn=nn->next)
		{
			AniScopeBase *asb=nn->asb;
			
			TNLocation loc=fid->GetLocationRange();
			Error(loc,
				"name clash with %s \"%s\" @%s\n",
				asb->AniScopeTypeStr(),asb->GetName().str(),
				asb->GetTreeNode() ? 
					asb->GetTreeNode()->down.first()->GetLocationRange().
						PosRangeStringRelative(loc).str() : 
					(asb->ASType()==AST_InternalFunc ? "[built-in]" : "???"));
			
			// The first one is enough as error message. 
			break;
		}
		// This may fail some time. In that case, see if it is really 
		// not a bug somewhere in the registration code...
		// NO: We may collide with any number of internal functions 
		// which have already registered. 
		//assert(lul.has_1_entry());
		
		++tri->n_errors;
		return(1);
	}
	
	return(0);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_ScopeOrSetting(ANI::TreeNode *tn,
	ANI::TRegistrationInfo *tri)
{
	AniScopeType wanted_scope;
	switch(tn->NType())
	{
		case TN_Object:   wanted_scope=AST_Object;   break;
		case TN_Setting:  wanted_scope=AST_Setting;  break;
		default: assert(0);
	}
	
	TNIdentifier *identifier=(TNIdentifier*)tn->down.first();
	TNIdentifierSimple *fid=_RegisterCheckTN(identifier,wanted_scope,tri);
	if(!fid)  return(NULL);
	
	if(_RegisterCheckNameKnown(fid,tri))  return(NULL);
	
	// Register it: 
	AniScopeBase *scope=NULL;
	switch(wanted_scope)
	{
		case AST_Animation:
			scope=new AniAnimation(fid->name,tn);
			assert(0);  // Currently, we do not register AniAnimation here. 
			break;
		case AST_Setting:
			assert(!tri->setting);
			scope=new AniSetting(fid->name,tn,tri->animation);
			break;
		case AST_Object:
			scope=new AniObject(fid->name,tn,
				/*parent=*/tri->setting ? tri->setting : tri->animation);
			break;
		default: assert(0);
	}
	// Assign scope: 
	identifier->ani_scope=scope;
	return(scope);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_Function(ANI::TNFunctionDef *tn,
	ANI::TRegistrationInfo *tri)
{
	AniScopeType wanted_scope=AST_UserFunc;
	assert(tn->NType()==TN_FunctionDef);
	
	// Check special case: $$povhook function. 
	if(tri->object && ((AniObject*)tri->object)->pov_attach_detach && 
		((AniObject*)tri->object)->pov_attach_detach->GetTreeNode()==tn)
	{
		// Okay, it's the "$$povhook" function. 
		// Just register it by setting the tn->function pointer 
		// correctly (done by caller). 
		AniFunction *scope=((AniObject*)tri->object)->pov_attach_detach;
		tn->func_name->ani_scope=scope;  // For the "$$povhook" identifier...
		return(scope);
	}
	
	TNIdentifierSimple *fid=_RegisterCheckTN(tn->func_name,wanted_scope,tri);
	if(!fid)  return(NULL);
	
	// Check for constructor & destructor if inside object: 
	int cd_special=0;  // +1 -> constructor; -1 -> destructor 
	int s_special=0;   // 1 -> setting init
	if(tri->object)
	{
		assert(!(tn->attrib & TNFunctionDef::IS_Animation));
		if(tri->object->GetName()==fid->name)
		{
			// Names for constructor match. 
			if(tn->attrib & TNFunctionDef::IS_Destructor)
			{  assert(!tn->ret_type);  cd_special=-1;  }
			else if(tn->ret_type->ev_type_id==ANI::EVT_Void)
			{  cd_special=+1;  }
			else
			{
				Error(tn->ret_type->GetLocationRange(),
					"constructor must not have return type\n");
				++tri->n_errors;
				return(NULL);
			}
		}
		else if(tn->attrib & TNFunctionDef::IS_Destructor)
		{
			assert(!tn->ret_type);
			Error(tn->func_name->GetLocationRange(),
				"destructor must match object name \"%s\"\n",
				tri->object->GetName().str());
			++tri->n_errors;
			return(NULL);
		}
	}
	else if(tn->attrib & TNFunctionDef::IS_Destructor)
	{
		assert(!tn->ret_type);
		Error(tn->func_name->GetLocationRange(),
			"destructor outside object\n");
		++tri->n_errors;
		return(NULL);
	}
	// Check for setting init function if inside setting. 
	else if(tri->setting && !tri->object)
	{
		assert(!(tn->attrib & TNFunctionDef::IS_Animation));
		assert(!(tn->attrib & TNFunctionDef::IS_Destructor)); // May not happen; see above if() and grammar. 
		if(tri->setting->GetName()==fid->name)
		{
			// Names for setting init match. 
			if(tn->ret_type->ev_type_id==ANI::EVT_Void)
			{  s_special=+2;  }
			else
			{
				Error(tn->ret_type->GetLocationRange(),
					"setting init function must not have return type\n");
				++tri->n_errors;
				return(NULL);
			}
		}
	}
	else if(tn->attrib & TNFunctionDef::IS_Animation)
	{
		// Grammar makes sure that the following does not happen: 
		assert(!tri->setting && !tri->object && !tri->function);
	}
	
	// First, check if function name collides with something else. 
	if(_MatchReservedNames(fid->name,tn->func_name))
	{  ++tri->n_errors;  return(NULL);  }
	
	// Have a look at the return type of the function: 
	ExprValueType return_evt;
	if(tn->ret_type)
	{
		if(GetBasicDeclType(tn->ret_type,&return_evt))
		{  ++tri->n_errors;  return(NULL);  }
	}
	else if((tn->attrib & // <-- bitwise!
		(TNFunctionDef::IS_Destructor|TNFunctionDef::IS_Animation)))
	{  return_evt.SetVoid();  }  // <-- Used for destructor. 
	else assert(0);
	
	// We must check if the function name collides with anything 
	// or if the function collides with any other function. 
	// This, however, has to be done lateron, because the function 
	// args are not yet registered. 
	// But we will check for collisions with type names here. 
	ASB_List lul;
	LookupName(fid->name,&lul,/*query_parent=*/-1/*all*/);
	// If the other code is correct and we can relay on it, then 
	// we can probably skip the loop if there are 2 or more entries 
	// in the list... But better do the checks. 
	for(ASB_List::Node *nn=lul.first(); nn; nn=nn->next)
	{
		// We will NOT find collision with vars here because there are 
		// no vars yet. The vars will check against collision with 
		// a function. 
		AniScopeBase *n_asb=nn->asb;
		
		// Do not error about con/de-structor name colliding with 
		// object name. 
		// Also, no error for setting name colliding with setting init. 
		// And do not report collisions with other functions. 
		if(!(cd_special && n_asb->astype==AST_Object && n_asb==this) && 
		   !(s_special && n_asb->astype==AST_Setting && n_asb==this) && 
		   !(n_asb->astype==AST_UserFunc || n_asb->astype==AST_InternalFunc))
		{
			TNLocation loc=tn->func_name->GetLocationRange();
			Error(loc,
				"%s \"%s\" collides with %s @%s\n",
				(tn->attrib & TNFunctionDef::IS_Animation) ? 
					"animation" : "function",
				fid->name.str(),
				n_asb->AniScopeTypeStr(),
				n_asb->NameLocation().PosRangeStringRelative(loc).str());
			++tri->n_errors;
			return(NULL);
		}
	}
	
	// Check for static functions: 
	int attrib=0;
	bool force_static=(astype==AST_Animation || astype==AST_Setting);
	if((tn->attrib & TNFunctionDef::IS_Animation))
	{  assert(force_static);  attrib|=AniFunction::IS_Animation;  }
	if((tn->attrib & TNFunctionDef::IS_Static) || force_static)
	{
		attrib|=AniFunction::IS_Static;
		if((tn->attrib & TNFunctionDef::IS_Static) && force_static)
		{
			Warning(tn->aloc0,
				"member functions of animation and setting are "
				"implicitly static\n");
		}
	}
	if((tn->attrib & TNFunctionDef::IS_Const))
	{
		attrib|=AniFunction::IS_Const;
		Error(tn->aloc1,
			"const attribute for functions not yet supported\n");
		++tri->n_errors;
		return(NULL);
	}
	     if(cd_special>0)  attrib|=AniFunction::IS_Constructor;
	else if(cd_special<0)  attrib|=AniFunction::IS_Destructor;
	if(s_special)  attrib|=AniFunction::IS_SettingInit;
	assert(!(cd_special && s_special));
	
	if((attrib & (AniFunction::IS_Constructor | AniFunction::IS_Destructor)) && 
		(attrib & AniFunction::IS_Static) )
	{
		Error(tn->aloc0,
			"%sstructor may not be static\n",
			(attrib & AniFunction::IS_Constructor) ? "con" : "de");
		++tri->n_errors;
		return(NULL);
	}
	
	// Register it: 
	AniUserFunction *scope=NULL;
	scope=new AniUserFunction(fid->name,tn,
		/*parent=*/tri->object ? tri->object : (
			tri->setting ? tri->setting : tri->animation),
		attrib);
	scope->return_type=return_evt;
	// Assign associated scope: 
	tn->func_name->ani_scope=scope;
	return(scope);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_JumpStmt(
	TNJumpStmt *dstmt,ANI::TRegistrationInfo *tri)
{
	AniScopeBase *asb=this;
	switch(dstmt->jumptype)
	{
		case TNJumpStmt::JT_None:  assert(0);  break;
		case TNJumpStmt::JT_Break:
		{
			// break breaks out of iteration statements. 
			TreeNode *loop_stmt=NULL;
			for(TreeNode *tn=dstmt; tn; tn=tn->parent())
			{
				if(tn->NType()!=TN_Statement)  continue;
				TNStatement *stmt=(TNStatement*)tn;
				if(stmt->StmtType()!=TNS_IterationStmt)  continue;
				loop_stmt=tn;  break;
			}
			if(!loop_stmt)
			{
				Error(dstmt->GetLocationRange(),
					"break statement outside iteration\n");
				++tri->n_errors;
				return(NULL);
			}
			// NULL is currently always used for break. 
			return(NULL);
		}  break;
		case TNJumpStmt::JT_Return:
		{
			// The associated function. 
			AniFunction *func=NULL;
			for(; asb->parent; asb=asb->parent)
			{
				if(asb->parent->astype==AST_UserFunc)
				{
					if(asb->astype!=AST_AnonScope || 
						((AniAnonScope*)asb)->scopetype!=
							AniAnonScope::AS_FunctionScope)
					{  assert(0);  }
					func=(AniFunction*)asb->parent;
					break;
				}
			}
			if(!func)
			{
				Error(dstmt->GetLocationRange(),
					"return starement outside function\n");
				++tri->n_errors;
				return(NULL);
			}
			
			// The AnonScope we're in... this is what we return. 
			AniScopeBase *ascope=this;
			for(; ascope && ascope->ASType()!=AST_AnonScope; 
				ascope=ascope->Parent());
			assert(ascope);  // MUST be inside an anon scope. 
			
			return(ascope);
		}  break;
		default:  assert(0);  break;
	}
}


AniScopeBase *AniScopeBase::_DoRegisterTN_CompoundStmt(
	TNCompoundStmt *dstmt,ANI::TRegistrationInfo * /*tri*/)
{
	AniAnonScope *scope=new AniAnonScope(dstmt,this);
	// Yeah... that's all here. Easy registration this time. 
	return(scope);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_DeclarationStmt(
	TNDeclarationStmt *dstmt,ANI::TRegistrationInfo *tri)
{
	// First child must be type specifier: 
	TNTypeSpecifier *tspec=(TNTypeSpecifier*)dstmt->down.first();
	assert(tspec && tspec->NType()==TN_TypeSpecifier);
	
	RefString var_name;
	TNIdentifier *name_idf=NULL;
	bool is_automatic_var=(astype==AST_UserFunc || astype==AST_AnonScope);
	bool force_static=(astype==AST_Animation || astype==AST_Setting);
	int cnt=0;
	for(TreeNode *tn=dstmt->down.first()->next; tn; tn=tn->next,cnt++)
	{
		TNDeclarator *decl=(TNDeclarator*)tn;
		ExprValueType evt=GetDeclarationType(tspec,decl,is_automatic_var,
			&var_name,&name_idf);
		if(!evt)
		{  ++tri->n_errors;  break;  }
		
		// Check is reserved or already used: 
		if(CheckVarNameAlreadyUsed(var_name,name_idf))
		{  ++tri->n_errors;  continue;  }
		
		int attrib=0;
		if((dstmt->attrib & TNDeclarationStmt::IS_Const))
		{  attrib|=AniUserVariable::IS_Const;  }
		if((dstmt->attrib & TNDeclarationStmt::IS_Static) || force_static)
		{
			attrib|=AniUserVariable::IS_Static;
			
			if((dstmt->attrib & TNDeclarationStmt::IS_Static) && force_static)
			{
				Warning(dstmt->aloc,
					"member variables of animation and setting are "
					"implicitly static\n");
			}
		}
		if(is_automatic_var)
		{  attrib|=AniUserVariable::IS_Automatic;  }
		if(decl->GetDeclType()==TNDeclarator::DT_Initialize)
		{
			attrib|=AniUserVariable::HAS_Initializer;
			bool okay=(attrib & AniUserVariable::IS_Automatic) || 
			          (attrib & AniUserVariable::IS_Static);
			if(!okay)
			{
				Error(decl->GetLocationRange(),
					"non-static %s member var may not have initializer\n",
					AniScopeTypeStr());
				++tri->n_errors;
			}
		}
		AniUserVariable *var=new AniUserVariable(var_name,tn,this,evt,attrib,
			tri->var_seq_num+cnt);
		// Assign associated scope: 
		name_idf->ani_scope=var;
	}
	
	// Will always return NULL here. That is okay. 
	return(NULL);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_AniDescStmt(
	TNAniDescStmt *ads,ANI::TRegistrationInfo *tri)
{
	while(!ads->name_expr)
	{
		// Must be inside object. 
		if(!tri->object)
		{
			Error(ads->GetLocationRange(),
				"ani desc stmt for \"this\" not inside object\n");
			++tri->n_errors;  break;
		}
		// Must not be in static function: 
		assert(tri->function);
		if(((AniFunction*)tri->function)->GetAttrib() & AniFunction::IS_Static)
		{
			Error(ads->GetLocationRange(),
				"ani desc stmt for \"this\" in static context\n");
			++tri->n_errors;  break;
		}
		
		break;  // <-- important
	}
	
	// Look up calc core if specified; use default one if not. 
	// The calc core must be known at "compile time". 
	assert(!ads->cc_factory);
	if(ads->core_name)
	{
		RefString core_name;
		core_name=ads->core_name->CompleteStr();
		ads->cc_factory=CC::CCIF_Factory::GetFactoryByName(core_name.str());
		if(!ads->cc_factory)
		{
			Error(ads->core_name->GetLocationRange(),
				"unknown calc core name \"%s\"\n",core_name.str());
			++tri->n_errors;
		}
	}
	else
	{
		ads->cc_factory=CC::CCIF_Factory::GetFactoryByName(/*name=*/NULL);
		// Default one MUST be present. 
		assert(ads->cc_factory);
	}
	
	// Will always return NULL here. That is okay. 
	return(NULL);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_OpFunc(ANI::TNOperatorFunction *tn,
	ANI::TRegistrationInfo *tri)
{
	// Only special cases may call this. 
	switch(tn->GetFuncID())
	{
		// Fall through switch. 
		case OF_PODCast:
		case OF_NewArray:
		{
			// Check the type specifier. 
			TNTypeSpecifier *tspec=(TNTypeSpecifier*)tn->down.first();
			int rv=GetBasicDeclType(tspec,&tn->new_type);
			if(rv)
			{
				// Error already written. 
				++tri->n_errors;
				tn->new_type.SetUnknown();
			}
			
			// Okay... we still have to make sure that the types we 
			// return here can be const-folded if incomplete. 
			// E.g. a POD type cast to vector<5> will be returned 
			// as vector<?> [incomplete] here. 
			// Therefore, we use the typefix_list: 
			LinkedList<TypeFixEntry> *typefix_list=GetTypeFixList();
			// If this assert fails, we're in an ani scope which does not 
			// have one (and that is valid; Ani*Var and AniIncomplete 
			// do NOT have one!) I wonder how this could happen. 
			// If this happens VALIDLY, probably go up to parent. 
			assert(typefix_list);
			// NOTE: Currently, it would be sufficient if we only 
			//       queue incomplete types here. 
			typefix_list->append(new TypeFixEntry(tspec,&tn->new_type));
			
		}	break;
		default: assert(0);
	}
	
	// Will always return NULL here. That is okay. 
	return(NULL);
}


AniScopeBase *AniScopeBase::_DoRegisterTN_Identifier(ANI::TNIdentifier *tn,
	ANI::TRegistrationInfo *tri)
{
	assert(tn->NType()==TN_Identifier);
	
	// "this", "NULL" are handeled lateron 
	// (see AniIncomplete::CG_LookupIdentifier()). 
	// We reserve an ordinary AniIncomplete for now. 
	
	// This may need additional work. 
	// NOTE that we cannot look up functions here due to overloading. 
	//      In case a function is found by the lookup, we must return 
	//      an incomplete node: The AniIncomplete. It is part of 
	//      the ani scope tree and thus allows for correct name 
	//      resolution. 
	// NOTE: Also for other identifiers, we cannot do correct lookup 
	//       here because they may be inside a member select which 
	//       changes scopes. So we ALWAYS use the AniIncomplete 
	//       placeholder here. 
	
	//fprintf(stderr,"## RegisterIdentifier(name=%s,this=%s)\n",
	//	tn->CompleteStr().str(),CompleteName().str());
	
	// Member select info: 
	// If inside ani/setting/object -> implicit member select; 
	// else none. 
	int ms_quality=0;
	AniScopeBase *ms_from=NULL;
	// See if we have implicit member select: 
	// NOTE: We're NOT here for function arg defs and for function 
	// name defs, i.e. for "int foo(int x=y)", we're here for "y" but 
	// NOT for "foo" and not for "x". 
	if(!Parent())
	{
		// Expression direcly inside animation (root) scope. 
		// Happens for var decls with initializers. 
		// No member select. 
		goto okay;
	}
	else for(AniScopeBase *asb=Parent(); asb; asb=asb->Parent())
	{
		switch(asb->ASType())
		{
			case AST_AnonScope:   break;  // check parent
			case AST_UserVar:     assert(0);  break;
			case AST_InternalVar: assert(0);  break;
			case AST_Incomplete:  assert(0);  break;
			case AST_UserFunc:    // fall through
			case AST_InternalFunc:  // <-- happens for $$povhook
			{
				if(((AniFunction*)asb)->GetAttrib() & AniFunction::IS_Static)
				{  break;  }
				ms_from=asb->Parent();
				ms_quality=1;
				goto okay;
			}
			case AST_Animation:  // fall
			case AST_Setting:    //     through
			case AST_Object:     //            here
				// This can only happen for expressions which are 
				// NOT inside a function, i.e. static var 
				// initialisations. 
				// Hence, no member select. 
				goto okay;
			default:  assert(0);
		}
	}
	// This may happen in case of "name collisions" (e.g. var decl and 
	// internal or user function, e.g. "int tan;"). This is the reason why 
	// registration step TRegistrationInfo::TRL_AllIdentifiers should only 
	// be done if there are no errors. Note that checking tri->n_errors 
	// here will NOT help becuase the errors in question happened some 
	// steps earlier. 
	fprintf(stderr,"OOPS: %s: parent=%p, name=%s\n",CompleteName().str(),
		Parent(),tn->CompleteStr().str());
	assert(0);  // Should not happen (parent=NULL). 
	okay:;
	
	// Check if this is a special identifier: 
	while(!tri->special_idf_list.is_empty())  // if()-like while() :)
	{
		RefString cname(tn->CompleteStr());
		// Currently, all special identifiers need to begin with a "$". 
		// So, in case this one does not, we need not walk the list. 
		if(cname.str()[0]!='$')  break;
		
		// IT IS IMPORTANT THAT WE SEARCH THIS LIST LAST-TO-FIRST AND 
		// USE THE FIRST MATCH. 
		for(TRegistrationInfo::SpecialIdf *si=tri->special_idf_list.last(); 
			si; si=si->prev)
		{
			if(cname!=si->name)  continue;
			
			// We allocate an own AniInternalVariable_ADB for each occurance 
			// of the internal variable in the expression. 
			AniInternalVariable *scope=new AniInternalVariable_ADB(
				tn->CompleteStr(),tn,
				(AniScopeBase*)tri->GetInnermostScope(),si->vtype,
				si->adb_idf_core_hook);
			return(scope);
		}
		
		break;
	}
	
	// Resolve that lateron. 
	// Need a new incomplete node: 
	AniIncomplete *scope=new AniIncomplete(tn,
		(AniScopeBase*)tri->GetInnermostScope(),tri->var_seq_num,
		/*member select info:*/ ms_from,ms_quality);
	
	// Return NULL on error, only. 
	return(scope);
}


AniScopeBase *AniScopeBase::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo * /*tri*/)
{
	// NOTE: 
	//  (1) This function works recursively. 
	//  (2) It will always return NULL. 
	//  (3) It must be overridden by the derived tyes. 
	assert(0);
	// Will always return NULL here. That is okay. 
	return(NULL);
}


// NOTE: VARIABLE NAMES ONLY. 
int AniScopeBase::CheckVarNameAlreadyUsed(const RefString &var_name,
	TNIdentifier *name_idf)
{
	if(_MatchReservedNames(var_name,name_idf))
	{  return(1);  }
	
	if(var_name.str()[0]=='$')
	{
		Error(name_idf->GetLocationRange(),
			"'$' prefix in \"%s\" is reserved for built-in animation "
			"identifiers\n",
			var_name.str());
		return(1);
	}
	
	ASB_List lul;
	LookupName(var_name,&lul,/*query_parent=*/-1/*all*/);
	
	// Make sure that the identifier does not collide with 
	// any identifier in the same scope: 
	for(ASB_List::Node *nn=lul.first(); nn; nn=nn->next)
	{
		AniScopeBase *n_asb=nn->asb;
		
		if(n_asb->Parent()==this)
		{
			TNLocation loc=name_idf->GetLocationRange();
			Error(loc,
				"\"%s\" collides with %s name @%s\n",
				var_name.str(),n_asb->AniScopeTypeStr(),
				n_asb->NameLocation().PosRangeStringRelative(loc).str());
			return(1);
		}
		
		#warning "more checks: var name must be != type names, etc. (done?)"
		#warning "FIX THIS CODE! >> NOTE THE >&& 0< BELOW. <<"
		// Variable names may not collide with any indentifer in the 
		// same scope (as checked above). 
		// Var names may not collide with type names. 
		// They MAY legally "collide" with function names and var 
		// names of higher scopes. 
		// Maybe one could detect collisions of function args with 
		// the first anonymous scope inside a function. 
		
		// Note: AST_InternalFunc should not happen here (?)
		bool is_func_arg=(astype==AST_UserFunc || astype==AST_InternalFunc);
		AniScopeBase::LookupExpectedType lext;
		do {
			lext=LET_TypeName;
			if(Match_LookupExpectedType(n_asb,lext))  break;
			lext=LET_FunctionName;
			if(Match_LookupExpectedType(n_asb,lext))  break;
			lext=LET_None;
		} while(0);
		if(lext!=LET_None)
		{
			// Do not warn about collision with function name of 
			// constructor/destructor; take type instead. 
			if(lext==LET_FunctionName && n_asb->astype==AST_UserFunc && 
				( ((AniFunction*)n_asb)->attrib & 
				  (AniFunction::IS_Constructor | AniFunction::IS_Destructor) ) )
			{
				n_asb=n_asb->parent;
				lext=LET_TypeName;
			}

			if(lext==LET_TypeName || 
			   (lext==LET_FunctionName && 0))
			{
				TNLocation loc=name_idf->GetLocationRange();
				Error(loc,
					"%s %s \"%s\" collides with %s @%s\n",
					is_func_arg ? "function" : n_asb->AniScopeTypeStr(),
					is_func_arg ? "argument" : "name",
					var_name.str(),
					LookupExpectedTypeStr(lext),
					n_asb->NameLocation().PosRangeStringRelative(loc).str());
				return(1);
			}
		}
		if(Match_LookupExpectedType(n_asb,LET_VariableName))
		{
			TNLocation loc=name_idf->GetLocationRange();
			Warning(loc,
				"%s %s \"%s\" shadows %s @%s\n",
				is_func_arg ? "function" : n_asb->AniScopeTypeStr(),
				is_func_arg ? "argument" : "name",
				var_name.str(),
				n_asb->parent->astype!=AST_UserFunc ? 
					"variable" : "function argument",
				n_asb->NameLocation().PosRangeStringRelative(loc).str());
		}
	}
	
	return(0);
}


// Internally used by LookupIdentifierName(). 
int AniScopeBase::_TopDownLookupIdentifierName(TNIdentifierSimple *start_ids,
	AniScopeBase *start_asb,AniScopeBase::ASB_List *ret_list,
	AniScopeBase::LookupExpectedType let,int write_errors,
	AniScopeBase **first_guess)
{
	AniScopeBase *asb=start_asb;
	assert(start_ids);  // Grammar should protect us from failure here. 
	
	// Come top-down: (Looks funny, eh?)
	for(TNIdentifierSimple *ids=start_ids; ids;
		ids=(TNIdentifierSimple*)ids->prev )
	{
		// "::"-resolvable scopes are animation - setting - object. 
		// You can look up inside these scopes but not inside a 
		// function or an anonymous scope. 
		short int can_look_inside=-1;
		if(ids->next)
		{
			switch(asb->astype)
			{   // Fall through switch: 
				case AST_Animation:
				case AST_Setting:
				case AST_Object:      can_look_inside=1;  break;
				case AST_AnonScope:
				case AST_UserFunc:
				case AST_UserVar:
				case AST_InternalVar:
				case AST_Incomplete:  can_look_inside=0;  break;
				case AST_InternalFunc:  assert(0);  break;  // if failure -> probably like AST_UserFunc
			}
		}
		else can_look_inside=1;
		assert(can_look_inside>=0);
		
		if(!can_look_inside)
		{
			if(write_errors)
			{  Error(ids->GetLocationRange(),
				"illegal attempt to resolve scope \"%s\" inside %s %s\n",
				ids->name.str(),
				asb->AniScopeTypeStr(),asb->CompleteName().str());  }
			return(1);
		}
		
		// We CAN look inside, so let's do it...
		ASB_List lul;
		asb->LookupName(ids->name,&lul,/*query_parent=*/0);
		if(lul.is_empty())
		{
			if(write_errors)
			{  Error(ids->GetLocationRange(),
				"no identifier \"%s\" inside %s\n",
				ids->name.str(),asb->CompleteName().str());  }
			return(1);
		}
		
		// See if we want to resolve more scopes: 
		if(ids->prev)
		{
			// Yes, want to look deeper. 
			if(lul.has_1_entry())
			{
				asb=lul.first()->asb;
				continue;
			}
			// List has more than 1 entry. 
			// This is normally only allowed for functions (overloading!) 
			// If this assert failed, then there is a bug in registration 
			// code: Two or more identical identifiers may only exist 
			// if they are all functions. 
			for(const ASB_List::Node *nn=lul.first(); nn; nn=nn->next)
			{
				if(nn->asb->astype!=AST_UserFunc && 
				   nn->asb->astype!=AST_InternalFunc )
				{  assert(0);  }
			}
			// Now... we want to look deeper but we may not look 
			// inside functions. 
			// This will write an error: (Only works because lul.first 
			// IS an AST_XxxFunc.) 
			asb=lul.first()->asb;
			continue;
		}
		else
		{
			// We're at the end of the lookup. 
			// Return the complete list -- or what we can use from it. 
			if(let==LET_Any)
			{
				ret_list->transfer_from(&lul);
				assert(!ret_list->is_empty());
				return(0);
			}
			int n_okay=0;
			while(!lul.is_empty())
			{
				ASB_List::Node *nn=lul.first();
				if(Match_LookupExpectedType(nn->asb,let))
				{  ret_list->transfer_from(&lul,nn);  ++n_okay;  }
				else
				{
					if(first_guess && !*first_guess) *first_guess=nn->asb;
					lul.dequeue(nn); /* Will be free'd. */
				}
			}
			if(!n_okay)
			{
				if(write_errors)
				{  Error(start_ids->parent()->GetLocationRange(),
					"unknown %s \"%s\" in %s\n",
					LookupExpectedTypeStr(let),
					((TNIdentifier*)start_ids->parent())->CompleteStr().str(),
					start_asb->CompleteName().str());  }
				return(1);
			}
			assert(!ret_list->is_empty());
			return(0);
		}
	}
	assert(0);
	return(-2);
}


int AniScopeBase::LookupIdentifierName(TNIdentifier *idf,
	AniScopeBase::ASB_List *ret_list,
	AniScopeBase::LookupExpectedType let,
	int write_errors,int accumulate_results)
{
	assert(idf && idf->NType()==TN_Identifier);
	
	// Do successive lookup: 
	TNIdentifierSimple *ids=(TNIdentifierSimple*)idf->down.last();
	assert(ids);  // Otherwise empty type; may not happen.
	
	// Check for special "built-in" names: 
	if(!ids->prev && !ids->next)  // Must be single. 
	{
		const RefString &iname=ids->name;
		if(iname=="this")
		{
			// "this" is handled somewhere else (a_incomplete.cc). 
			// If it is used here, then the use is invalid. 
			if((let & LET_VariableName))
			{  assert(0);  }  // <-- Must have been handeled by higher level. 
			else if((let & (LET_FunctionName|LET_TypeName)))
			{
				if(write_errors)
				{  Error(idf->GetLocationRange(),
					"cannot use \"this\" as %s\n",
					LookupExpectedTypeStr(let));  }
			}
			else if(write_errors)
			{
				// We should never reach here... but if we do at 
				// least yell at the user :)
				Error(idf->GetLocationRange(),
					"invlid use of \"this\"\n");
			}
			return(1);
		}
	}
	
	// Two cases: absolute spec begins with "::": 
	if(!ids->name)  // "::" spec
	{
		// Run up to animation root: 
		AniScopeBase *asb=this;
		for(; asb->parent; asb=asb->parent);
		// A single top-down lookup is enough in that case: 
		ASB_List lul;
		int rv=_TopDownLookupIdentifierName((TNIdentifierSimple*)ids->prev,
			asb,ret_list,let,write_errors,NULL);
		return(rv);
	}
	
	// Relative spec. 
	int sth_found=0;
	AniScopeBase *first_guess=NULL;
	for(AniScopeBase *start=this; start; start=start->parent)
	{
		// Look up name in *idf starting at *start. 
		int rv=_TopDownLookupIdentifierName(ids,start,ret_list,
			let,/*write_errors=*/0,&first_guess);
		if(rv)  continue;  // error; try one scope above
		
		sth_found=1;
		if(!accumulate_results)  break;
	}
	
	if(!sth_found)
	{
		if(write_errors)
		{
			if(first_guess)
			{  Error(idf->GetLocationRange(),
				"%s name %s where %s is expected\n",
				first_guess->AniScopeTypeStr(),
				first_guess->CompleteName().str(),
				LookupExpectedTypeStr(let));  }
			else
			{  Error(idf->GetLocationRange(),
				"unknown %s \"%s\" in %s\n",
				LookupExpectedTypeStr(let),idf->CompleteStr().str(),
				CompleteName().str());  }
		}
		return(1);
	}
	
	return(0);
}


int AniScopeBase::GetBasicDeclType(TNTypeSpecifier *tspec,ExprValueType *evt)
{
	assert(tspec->NType()==TN_TypeSpecifier);
	
	switch(tspec->ev_type_id)
	{
		case EVT_Unknown:  assert(0);  break;
		case EVT_Void:
			evt->SetVoid();
			break;
		case EVT_POD:
		{
			Value::CompleteType pod;
			pod.type=tspec->vtype;
			switch(pod.type)
			{
				case Value::VTInteger:  // fall through
				case Value::VTString:   // fall through
				case Value::VTScalar:  break;  // nothing to do
				case Value::VTRange:
					// What about range with "open" ends?
					//pod.rvalid=Value::RValidAB;
					// open ends: don't matter.
					break;
				case Value::VTVector:
					if(tspec->down.is_empty())
					{  pod.n=Vector::default_n;  }
					else
					{
						// Must mark the entry as not yet specified: 
						pod.n=-1;
					}
					break;
				case Value::VTMatrix:
					if(tspec->down.is_empty())
					{
						pod.r=Matrix::default_r;
						pod.c=Matrix::default_c;
					}
					else
					{
						// Must mark the entries as not yet specified: 
						pod.r=-1;
						pod.c=-1;
					}
					break;
				default:  assert(0);   // May not happen here. 
			}
			evt->SetPOD(pod);
		}  break;
		case EVT_AniScope:
		{
			// Must look up name: 
			ASB_List lul;
			int rv=LookupIdentifierName((TNIdentifier*)tspec->down.first(),
				&lul,LET_TypeName,/*write_errors=*/1,/*accumulate_results=*/0);
			
			if(rv || lul.is_empty())
			{  return(-2);  }
			// Type names may not collide. If the do registration code 
			// is probably buggy. 
			assert(lul.has_1_entry());
			
			AniScopeBase *asb=lul.first()->asb;
			evt->SetAniScope(asb);  // So that we can use TypeString() in the error. 
			
			// Especially make sure that there are no variables of 
			// type "setting" or "animation" floating around.
			// It would work somehow but these things are said to be 
			// static. 
			if(asb->astype!=AST_Object && 
			   asb->astype!=AST_UserVar &&  // <-- would be the case for pointers (?)
			   asb->astype!=AST_InternalVar)
			{
				Error(tspec->GetLocationRange(),
					"cannot use values of non-object/POD type %s\n",
					evt->TypeString().str());
				return(-2);
			}
		}  break;
		case EVT_Array:
		{
			// This happens for functions with array return type. 
			// TNTypeSpecifier with EVT_Array is always of 
			// "unspecified" size (i.e. dynamic). 
			ExprValueType tmp;
			int rv=GetBasicDeclType(
				(TNTypeSpecifier*)tspec->down.first(),&tmp);
			if(rv)  return(rv);
			evt->SetArray(tmp);
		}  break;
		default:  assert(0);  break;  // may not happen here
	}
	
	return(0);
}


ANI::ExprValueType AniScopeBase::GetDeclarationType(TNTypeSpecifier *tspec,
	TNDeclarator *decl,int /*is_automaitc_var*/,
	RefString *ret_name,TNIdentifier **ret_idf)
{
	assert(tspec->NType()==TN_TypeSpecifier);
	assert(decl->NType()==TN_Declarator);
	
	// Traverse tree: 
	TNDeclarator *di=decl;
	// Order top-down is: initializer -> array -> array -> name. 
	// First, skip initializer, if any: 
	
	TNDeclarator *initializer=NULL;
	if(di->GetDeclType()==TNDeclarator::DT_Initialize)
	{
		initializer=di;
		di=(TNDeclarator*)di->down.first();
		assert(di && di->NType()==TN_Declarator);
	}
	// Accumuate array types, if any: 
	// This works here, because the array is the only type which 
	// yields to a tree structure. 
	// Otherwise we would have to do it bottom-up. 
	int array_depth=0;
	TNDeclarator *array_decl=NULL;  // Get most deepy nested one. 
	while(di->GetDeclType()==TNDeclarator::DT_Array)
	{
		++array_depth;
		array_decl=di;
		di=(TNDeclarator*)di->down.first();
		assert(di && di->NType()==TN_Declarator);
	}
	// Okay, now we must find the name: 
	assert(di->GetDeclType()==TNDeclarator::DT_Name);
	TNIdentifier *idf=(TNIdentifier*)di->down.first();
	if(ret_idf)  *ret_idf=idf;
	assert(idf->NType()==TN_Identifier);
	
	// Complex names are not allowed here. 
	assert(!idf->down.is_empty());
	if(idf->down.first()!=idf->down.last())
	{
		Error(idf->GetLocationRange(),
			"cannot use scoped type \"%s\" as variable name\n",
			idf->CompleteStr().str());
		return ExprValueType(ExprValueType::CNullRef);
	}
	TNIdentifierSimple *idfs=(TNIdentifierSimple*)idf->down.first();
	assert(idfs->NType()==TN_IdentifierSimple);
	
	if(ret_name)
	{  *ret_name=idfs->name;  }
	
	// Extract basic type: 
	ExprValueType evt;
	if(GetBasicDeclType(tspec,&evt))
	{  return ExprValueType(ExprValueType::CNullRef);  }
	
	// Finally, complete arrays: 
	for(int i=0; i<array_depth; i++,
		array_decl=(TNDeclarator*)array_decl->parent())
	{
		if(array_decl->NType()!=TN_Declarator || 
		   array_decl->GetDeclType()!=TNDeclarator::DT_Array )
		{  assert(0);  }
		
		if(array_decl->down.first()!=array_decl->down.last())  // 2 children
		{
			Error(array_decl->down.last()->GetLocationRange(),
				"arrays must be allocated dynamically using \"new\"\n");
			return ExprValueType(ExprValueType::CNullRef);
		}
		
		ExprValueType tmp(evt);
		evt.SetArray(tmp);
	}
	
	return(evt);
}


int AniScopeBase::_CheckFuncArgRepetition(LookupFunctionInfo *lfi)
{	
	// Check if any of the named arguments in *lfi are given more than once. 
	int nerrors=0;
	for(int i=0; i<lfi->n_args; i++)
	{
		if(!lfi->arg_name[i]) continue;
		RefString name=lfi->arg_name[i]->CompleteStr();
		for(int j=i+1; j<lfi->n_args; j++)
		{
			if(!lfi->arg_name[j]) continue;
			if(lfi->arg_name[j]->CompleteStr()!=name) continue;
			TNLocation loc=lfi->arg_name[j]->GetLocationRange();
			Error(loc,
				"argument name \"%s\" previously specified @%s\n",
				name.str(),lfi->arg_name[i]->GetLocationRange().
					PosRangeStringRelative(loc).str());
			++nerrors;
			break;  // only innermost loop
		}
	}
	return(nerrors);
}


String AniScopeBase::FuncCallWithArgsString(String func_name,
	LookupFunctionInfo *lfi)
{
	String fname(func_name),tmp;
	fname+="(";
	for(int i=0; i<lfi->n_args; i++)
	{
		tmp.sprintf("%s%s%s%s",
			lfi->arg_name[i] ? lfi->arg_name[i]->CompleteStr().str() : "",
			lfi->arg_name[i] ? "=" : "",
			lfi->arg_type[i].TypeString().str(),
			i+1<lfi->n_args ? "," : "");
		fname+=tmp;
	}
	fname+=")";
	
	return(fname);
}


AniFunction *AniScopeBase::_LookupFunction(ASB_List *lul,
	LookupFunctionInfo *lfi,RefString name_for_error,TNLocation name_loc)
{
	// This will hold the looked-up function lateron: 
	AniFunction *afunc=NULL;
	int match_val=0;
	int nmatches=0;  // not really a counter: 0 -> none; 1 -> OK; 2 -> ambiguous
	for(ASB_List::Node *nn=lul->first(); nn; nn=nn->next)
	{
		//fprintf(stdout,"  %s  [0x%p]: ",nn->asb->CompleteName().str(),nn->asb);
		AniFunction *af=(AniFunction*)nn->asb;
		if(af->ASType()!=AST_UserFunc && af->ASType()!=AST_InternalFunc)
		{  assert(0);  }  // I specified LET_FunctionName above. 
		
		int rv=af->Match_FunctionArguments(lfi,/*store_evalhandler=*/0);
		//fprintf(stdout,"rv=%d\n",rv);
		
		assert(rv>=0);
		if(rv==0)  continue;   // try next function
		if(match_val==0)
		{
			// First match. 
			afunc=af;
			match_val=rv;
			nmatches=1;
		}
		else
		{
			// Second lookup match. 
			if(afunc->Parent()!=af->Parent())
			{
				// We're already one scope higher and the match is not 
				// better (-> rv val), then we have it. 
				if(match_val>=rv)  break;
				// Otherwise: ambiguous. 
				nmatches=2;  break;
			}
			else
			{
				// Same scope. See if we get a better match: 
				if(match_val<rv)
				{  afunc=af;  match_val=rv;  }  // matches stays 1. 
				else if(match_val>rv)
				{  /* still have better match; matches=1 stays. */  }
				else  // Otherwise: ambiguous. 
				{  nmatches=2;  break;  }
			}
		}
	}
	
	// Write error if needed: 
	if(nmatches!=1)
	{
		String fname(FuncCallWithArgsString(String(name_for_error.str()),lfi));
		
		if(nmatches>1)
		{  Error(name_loc,
			"call to function %s ambiguous; candidates:\n",fname.str());  }
		else
		{  Error(name_loc,
			"no matching call to function %s\n",fname.str());  }
		
		if(nmatches>1) for(ASB_List::Node *nn=lul->first(); nn; nn=nn->next)
		{
			AniFunction *af=(AniFunction*)nn->asb;
			int rv=af->Match_FunctionArguments(lfi,/*store_evalhandler=*/0);
			if(rv==0)  continue;   // try next function
			
			// The "(*)" in the error says that the function is in same scope 
			// as the first match, if it is "( )", then it is some higher scope. 
			Error(af->NameLocation(),
				"  (%s) %s\n",
				afunc->Parent()==af->Parent() ? "*" : " ",
				af->PrettyCompleteNameString().str());
		}
		
		afunc=NULL;
	}
	else
	{
		// Store eval handler (and return type) this time. 
		// May not fail because we checked it above (hende the assert()): 
		int rv=afunc->Match_FunctionArguments(lfi,/*store_evalhandler=*/1);
		assert(rv>0);
		
		// Some verbose code: 
		/*fprintf(stdout,"%s: calling %s @%s\n",
			name_loc.PosRangeString().str(),
			afunc->PrettyCompleteNameString().str(),
			afunc->NameLocation().PosRangeStringRelative(name_loc).str());*/
	}
	
	return(afunc);
}


// This is the exported function: 
ANI::AniGlue_ScopeBase *AniScopeBase::RegisterTN(ANI::TreeNode *tn,
	ANI::TRegistrationInfo *tri)
{
	switch(tn->NType())
	{
		case TN_Identifier:
			return(_DoRegisterTN_Identifier((TNIdentifier*)tn,tri));
		case TN_FunctionDef:
			return(_DoRegisterTN_Function((TNFunctionDef*)tn,tri));
		case TN_Object:  // fall through: 
		case TN_Setting:
			return(_DoRegisterTN_ScopeOrSetting(tn,tri));
		case TN_FuncDefArgDecl:
		{
			assert(astype=AST_UserFunc);
			AniUserFunction *thisfunc=(AniUserFunction*)this;
			return(thisfunc->RegisterTN_FuncDefArgDecl(
				(TNFuncDefArgDecl*)tn,tri));
		}
		case TN_Statement:
		{
			switch(((TNStatement*)tn)->StmtType())
			{
				case TNS_JumpStmt:
					return(_DoRegisterTN_JumpStmt((TNJumpStmt*)tn,tri));
				case TNS_CompoundStmt:
					return(_DoRegisterTN_CompoundStmt(
						(TNCompoundStmt*)tn,tri));
				case TNS_DeclarationStmt:
					return(_DoRegisterTN_DeclarationStmt(
						(TNDeclarationStmt*)tn,tri));
				case TNS_AniDescStmt:
					return(_DoRegisterTN_AniDescStmt(
						(TNAniDescStmt*)tn,tri));
				default:  assert(0);
			}
		}  break;
		case TN_Expression:
		{
			if(((TNExpression*)tn)->ExprType()==TNE_OperatorFunction)
			{  return(_DoRegisterTN_OpFunc((TNOperatorFunction*)tn,tri));  }
		}  break;
		case TN_Animation:
			// Special call. Performed when all registration steps are 
			// over. Just the right time to do various checks...
			assert(((TNAnimation*)tn)->animation==this);
			// NOTE: The RegisterTN_DoFinalChecks() function works 
			//       recursively, so it is important that we're at 
			//       the tree root here. Hence, the assert(!parent). 
			assert(!parent);
			return(RegisterTN_DoFinalChecks(tri));
		default:  assert(0);
	}
	return(NULL);
}


ANI::AniGlue_ScopeBase *AniScopeBase::CG_ReplaceAniIncomplete(
	AniGlue_ScopeBase *old_location,
	AniGlue_ScopeBase *ms_from,int ms_quality)
{
	// The old one must still be incomplete. 
	AniScopeBase *_old_asb=(AniScopeBase*)old_location;
	assert(_old_asb->astype==AST_Incomplete);  // IMPORTANT. 
	
	// Get the identifier we're talking about: 
	AniIncomplete *old_inc=(AniIncomplete*)_old_asb;
	TNIdentifier *idf=(TNIdentifier*)old_inc->treenode;
	assert(idf->NType()==TN_Identifier);
	
	// FIXME: hmmm.. 
	// The problem is: 
	//  - when we do member select from a scope below, no problem 
	//    because all vars in animation/setting/object must be 
	//    initialized. 
	//  - when we do member select inside assignments in the 
	//    animation/setting member vars and refer to a different 
	//    scope: no problem. 
	//  - when we do member select inside this setting/animation 
	//    and we're in an decl assignment, we HAVE A PROBLEM. 
	// THIS CAN CURRENTLY NOT HAPPEN (IMO). 
	
	int var_seq_num=0x7fffffff;
	AniIncomplete *ainc=new AniIncomplete(idf,this,var_seq_num,
		(AniScopeBase*)ms_from,ms_quality);
	old_inc->_TNUpdate_SelfRemoval(ainc);
	return(ainc);
}


/******************************************************************************/

void AniScopeBase::FillInInstanceInfo(AniScopeBase::InstanceInfo *iinfo,
	_AniComponentList<AniVariable> *varlist,int vtype,int used_at_beginning)
{
	// NOTE: 
	//  vtype=0 -> AniAniSetObj -> member vars
	//  vtype=1 -> AniAnonScope -> auto vars
	//  vtype=2 -> AniFunction -> function args, retval, this pointer
	
	for(int step=0; step<2; step++)
	{
		int cnt=used_at_beginning;
		
		for(AniVariable *_i=varlist->first(); _i; _i=_i->next)
		{
			if(_i->ASType()==AST_InternalVar)
			{
				//AniInternalVar *i=(AniInternalVar*)i;
				// Go on. Internal vars are handeled "internally"...
				// They do not need space on the stack or in the scope 
				// instance car array; they "live" somewhere else. 
				continue;
			}
			assert(_i->ASType()==AST_UserVar);
			
			AniUserVariable *i=(AniUserVariable*)_i;
			int attr=i->GetAttrib();
			if(vtype==0)
			{
				// Member vars. 
				if(attr & AniUserVariable::IS_Automatic)
				{  assert(0);  }  // Should not happen in case vtype=0. 
				// Skip static member vars. 
				if(attr & AniUserVariable::IS_Static)
				{  continue;  }
			}
			else if(vtype==1)
			{
				// Automatic vars. 
				if(!(attr & AniUserVariable::IS_Automatic))
				{  assert(0);  }  // Should not happen in case vtype=1. 
				// Skip static "auto" vars. 
				if(attr & AniUserVariable::IS_Static)
				{  continue;  }
			}
			else if(vtype==2)
			{
				// Automatic vars. 
				if(!(attr & AniUserVariable::IS_Automatic))
				{  assert(0);  }  // Should not happen in case vtype=2. 
				if(attr & AniUserVariable::IS_Static)
				{  assert(0);  }  // Should not happen in case vtype=2. 
			}
			else assert(0);
			
			if(step==1)
			{
				// Get variable type...
				iinfo->var_types[cnt]=i->GetType();
				if(iinfo->var_types[cnt].IsIncomplete() || 
				   iinfo->var_types[cnt].TypeID()==EVT_Unknown || 
				   iinfo->var_types[cnt].TypeID()==EVT_NULL )
				{  assert(0);  }
				// ...and set addressation index (vars are nonstatic here). 
				i->SetAddressationIndex(cnt);
			}
			
			++cnt;
		}
		
		if(step==0)
		{
			iinfo->n_stack_vars=cnt;
			iinfo->var_types=cnt ? new ExprValueType[cnt] : NULL;
		}
	}
	
	// Print info: 
	/*fprintf(stdout,"InstanceInfo for %s %s: %d vars:\n",
		AniScopeTypeStr(),CompleteName().str(),iinfo->n_stack_vars);
	for(int i=used_at_beginning; i<iinfo->n_stack_vars; i++)
	{
		fprintf(stdout,"  [%d]=%s\n",
			i,iinfo->var_types[i].TypeString().str());
	}*/
}
