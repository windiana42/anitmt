/*
 * core/a_incomplete.cc
 * 
 * ANI incomplete - place holder in ANI scope tree. 
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

using namespace ANI;

static RefString *_incomplete_str=NULL;
static RefString *_incomplete_null_str=NULL;
static int _incomplete_str_refs=0;  // For any of them. 


AniGlue_ScopeBase::IncompleteType AniIncomplete::CG_IncompleteType() const
{
	// Not inline because virtual. 
	return(incomplete_type);
}


int AniIncomplete::CG_LookupFunction(LookupFunctionInfo *lfi)
{
	lfi->ani_function=NULL;
	
	TNIdentifier *our_idf=(TNIdentifier*)GetTreeNode();
	assert(our_idf->NType()==TN_Identifier);
	
	// Lookup: 
	assert(Parent());  // AniIncomplete always has parent. 
	ASB_List lul;   // "Look-Up-List"
	int rv=Parent()->LookupIdentifierName(our_idf,&lul,
		LET_FunctionName,/*write_errors=*/1,/*accumulate_results=*/1);
	if(rv)  return(0);  // error
	
	// Okay, now let's check the args. 
	// NOTE that the lookup was done bottom-up so that the functions in 
	// the innermost scopes are first in the lookup list lul. 
	
	// See if some named args are specified more than once: 
	int nerrors=_CheckFuncArgRepetition(lfi);
	if(nerrors)  return(0);
	
	/*fprintf(stderr,"%s: Lookup for %s:\n",
		our_idf->GetLocationRange().PosRangeString().str(),
		our_idf->CompleteStr().str());*/
	// This will hold the looked-up function: 
	AniFunction *afunc=_LookupFunction(&lul,lfi,
		our_idf->CompleteStr(),our_idf->GetLocationRange());
	
	if(afunc)
	{
		// Store the info for the caller: 
		lfi->ani_function=afunc;
		// The return type must have been set: 
		assert(!!lfi->ret_evt);
		
		// Before removing the AniIncomplete, make sure we process 
		// some saved work: 
		if(idf_to_update)
		{
			if(itu_for_fcall)
			{
				assert(!idf_to_update->evaladrfunc);
				if((afunc->GetAttrib() & AniFunction::IS_Static))
				{  idf_to_update->evaladrfunc=new AniUserVariable::EAF_Null();  }
				else
				{  idf_to_update->evaladrfunc=new AniUserVariable::EAF_Repeater();  }
				itu_for_fcall=0;
			}
			else assert(0);
			idf_to_update=NULL;
		}
		
		// Do the TreeNode update and self removal:
		_TNUpdate_SelfRemoval(afunc);
	}
	
	return(0);
}


// Return: -1 -> error; 0 -> no special idf, 
//          1 -> "this" found, 2 -> "NULL" found
int AniIncomplete::_LookupIdentifier_Special(
	LookupIdentifierInfo * /*lii*/,AniScopeBase **ret_ascope)
{
	TNIdentifier *our_idf=(TNIdentifier*)GetTreeNode();
	
	// Okay, check if this is a special case, the "this" keywoard: 
	do {
		TNIdentifierSimple *ids=(TNIdentifierSimple*)our_idf->down.first();
		if(!ids) break;
		if(ids->prev || ids->next) break;  // Must be single. 
		const RefString &iname=ids->name;
		if(iname=="this")
		{
			AniScopeBase *ascope=Parent();
			for(; ascope && ascope->ASType()!=AST_UserFunc && 
			                ascope->ASType()!=AST_InternalFunc;  // <-- "$$povhook"
				ascope=ascope->Parent());
			if(!ascope || (
					ascope->ASType()!=AST_UserFunc && 
					ascope->ASType()!=AST_InternalFunc
				) || 
				(((AniFunction*)ascope)->GetAttrib() & AniFunction::IS_Static) )
			{
				// This also happens if you use 
				// "object A { static A a=this; }"
				Error(our_idf->GetLocationRange(),
					"\"this\" can only be used in non-static member "
					"function context\n");
				return(-1);
			}
			
			// Look for object in hierarchy: 
			for(; ascope && ascope->ASType()!=AST_Object;
				ascope=ascope->Parent());
			if(!ascope)
			{
				Error(our_idf->GetLocationRange(),
					"\"this\" used outside object\n");
				return(-1);
			}
			
			*ret_ascope=ascope;
			return(1);
		}
		if(iname=="NULL")
		{
			*ret_ascope=NULL;
			return(2);
		}
	} while(0);
	
	return(0);
}


AniScopeBase *AniIncomplete::_LookupIdentifier_Normal(
	LookupIdentifierInfo * /*lii*/)
{
	TNIdentifier *our_idf=(TNIdentifier*)GetTreeNode();
	
	// We must accumulate the results because the var_seq_num check 
	// still has to be applied. 
	ASB_List lul;
	int rv=Parent()->LookupIdentifierName(our_idf,&lul,LET_Any,
		/*write_errors=*/1,/*accumulate_results=*/1);
	if(rv)  return(NULL);  // error
	
	// PLEASE READ COMMENT ON THIS FUNCTION IN <ani-parser/coreglue.h>
	// For variables and types, we return the associated ani scope, for 
	// functions we return the AniIncomplete. 
	
	// Verbose: 
	/*fprintf(stdout,"%s: looking up \"%s\" in %s:\n",
		our_idf->GetLocationRange().PosRangeString().str(),
		our_idf->CompleteStr().str(),
		Parent()->CompleteName().str());*/
	
	// Remove all nodes which cannot be seen from here. 
	for(ASB_List::Node *_nn=lul.first(); _nn; )
	{
		ASB_List::Node *nn=_nn;
		_nn=_nn->next;
		
		int can_see=-1;
		// This depends on where we are. 
		if(Parent()->ASType()==AST_UserFunc || 
		   Parent()->ASType()==AST_InternalFunc ||   // <-- will not happen
		   Parent()->ASType()==AST_AnonScope )
		{
			// We're in a function or an anonymous scope. 
			if(!nn->asb->Parent() || 
				nn->asb->Parent()->ASType()==AST_Animation || 
				nn->asb->Parent()->ASType()==AST_Setting || 
				nn->asb->Parent()->ASType()==AST_Object )
			{  can_see=1;  }
			else if(nn->asb->Parent()->ASType()==AST_UserFunc || 
			        nn->asb->Parent()->ASType()==AST_AnonScope )
			{
				switch(nn->asb->ASType())
				{
					// Fall through switch. 
					case AST_Animation:
					case AST_Setting:
					case AST_Object:
					case AST_UserFunc:
					case AST_InternalFunc:
						can_see=1;
						break;
					case AST_UserVar:
						can_see = 
							( ((AniUserVariable*)nn->asb)->
								GetVarSeqNum() < var_seq_num ) ? 1 : 0;
						break;
					case AST_InternalVar:
						can_see=1;
						break;
					default: assert(0);
				}
			}
			else assert(0);  // AST_InternalFunc MAY not happen
			
		}
		else if(Parent()->ASType()==AST_Animation || 
		        Parent()->ASType()==AST_Setting || 
		        Parent()->ASType()==AST_Object )
		{
			// We're in an object or setting or animation. 
			// Check if static variable or function. 
			if(( (nn->asb->ASType()==AST_UserVar || 
			      nn->asb->ASType()==AST_InternalVar) && 
			   (((AniVariable*)nn->asb)->GetAttrib() & AniVariable::IS_Static) ) || 
			   ( (nn->asb->ASType()==AST_UserFunc || 
			      nn->asb->ASType()==AST_InternalFunc) && 
			   (((AniFunction*)nn->asb)->GetAttrib() & AniFunction::IS_Static) ) )
			{
				// This happens for assignment decls of static (i.e. animation and 
				// setting) member vars (or static object vars). 
				// For same parent: respect var_seq_num, for other (higher) 
				// parents, we can access everything, for lower parents, 
				// we get a problem. NOTE: I talked about "access". 
				// We may not access them but we can "see" them. The access 
				// checks are done at the end of this function. 
				assert(nn->asb->ASType()!=AST_Incomplete);
				if(nn->asb->ASType()==AST_UserVar)
				{
					if(nn->asb->Parent()==Parent())
					{
						can_see = ( ((AniUserVariable*)nn->asb)->
							GetVarSeqNum() < var_seq_num ) ? 1 : 0;
					}
					else
					{
						// Let's solve that problem (see comment 
						// above) lateron. 
						can_see = 1;
					}
				}
				else if(nn->asb->ASType()==AST_UserFunc || 
				        nn->asb->ASType()==AST_InternalFunc )
				{
					// Functions (even static ones) can always be seen. 
					can_see=1;
				}
				else
				{  assert(0);  }  // <-- handle this if it happens
			}
			else
			{
				if(!nn->asb->Parent() || 
					nn->asb->Parent()->ASType()==AST_Animation || 
					nn->asb->Parent()->ASType()==AST_Setting || 
					nn->asb->Parent()->ASType()==AST_Object )
				{  can_see=1;  }
				else if(nn->asb->Parent()->ASType()==AST_UserFunc || 
				        nn->asb->Parent()->ASType()==AST_InternalFunc || 
			        	nn->asb->Parent()->ASType()==AST_AnonScope )
				{  assert(0);  }  // May not happen here. 
				else assert(0);
			}
		}
		else assert(0);
		
		assert(can_see>=0);  // Redundant... but oh well. 
		
		// For explicit member select, even fewer get visible: 
		if(can_see && ms_quality==2)
		{
			assert(ms_from);
			// For explicit member select, the nn->asb or its 
			// immediate parent must the the ms_from object. 
			// (ms_from can be a non-object here; complain lateron) 
			AniScopeBase *my_sc=NULL;
			switch(nn->asb->ASType())
			{
				// Fall through switch. 
				case AST_Animation:
				case AST_Setting:
				case AST_Object:
					//my_sc=nn->asb;
					break;
				case AST_UserFunc:
				case AST_InternalFunc:
				case AST_UserVar:
				case AST_InternalVar:
					// Immediate parent is object; --> member. 
					if(nn->asb->Parent()->ASType()==ms_from->ASType())
					{  my_sc=nn->asb->Parent();  }
					break;
				case AST_AnonScope:
				case AST_Incomplete:
					assert(0);   // cannot be looked up
					break;
				default:  assert(0);
			}
			if(my_sc!=ms_from)
			{  can_see=0;  }
		}
		
		// Some verbose messages: 
		//fprintf(stdout,"  %s: %svisible\n",
		//	nn->asb->CompleteName().str(), can_see ? "" : "in");
		
		if(!can_see)
		{  lul.dequeue(nn);  }
	}
	
	// See if lookup failed: 
	if(lul.is_empty())
	{
		Error(our_idf->GetLocationRange(),
			"unknown identifier \"%s\"%s%s\n",
			our_idf->CompleteStr().str(),
			ms_quality==2 ? " in member select from " : "",
			ms_quality==2 ? ms_from->CompleteName().str() : "");
		return(NULL);
	}
	
	// This will store the returned scope. 
	AniScopeBase *ascope=NULL;
	
	// First bogus match: 
	AniScopeBase *bogus_match=NULL;
	
	// Use "first match" (wrt parent hierarchy) and check type. 
	for(ASB_List::Node *nn=lul.first(); nn; nn=nn->next)
	{
		// "Real member", including static vars. 
		AniScopeBase *nonstatic_member_of=NULL;  // set if nonstatic
		AniScopeBase *static_member_of=NULL;     // set if static
		switch(nn->asb->ASType())
		{
			// Fall through switch. 
			case AST_Animation:
			case AST_Setting:
			case AST_Object:
				//is_member_of=nn->asb;
				break;
			case AST_InternalFunc:
			case AST_UserFunc:
				if(nn->asb->Parent() && 
				   ( nn->asb->Parent()->ASType()==AST_Object || 
				     nn->asb->Parent()->ASType()==AST_Setting || 
				     nn->asb->Parent()->ASType()==AST_Animation ) )
				{
					if(( ((AniFunction*)nn->asb)->GetAttrib() & AniFunction::IS_Static ))
					{  static_member_of=nn->asb->Parent();  }
					else
					{  nonstatic_member_of=nn->asb->Parent();  }
				}
				else assert(0);
				break;
			case AST_UserVar:
			case AST_InternalVar:
				if(nn->asb->Parent() && 
				   ( nn->asb->Parent()->ASType()==AST_Object || 
				     nn->asb->Parent()->ASType()==AST_Setting || 
				     nn->asb->Parent()->ASType()==AST_Animation ) )
				{
					if( (nn->asb->ASType()==AST_UserVar || 
					     nn->asb->ASType()==AST_InternalVar) && 
						( ((AniVariable*)nn->asb)->GetAttrib() & 
							AniVariable::IS_Static ) )
					{  static_member_of=nn->asb->Parent();  }
					else
					{  nonstatic_member_of=nn->asb->Parent();  }
				}
				break;
			case AST_AnonScope:
			case AST_Incomplete:
				// cannot be looked up. 
				assert(0);
				break;
			default:  assert(0);
		}
		
		if( (ms_quality==0 && nonstatic_member_of) || 
		    (ms_quality==1 && 
				((nonstatic_member_of && nonstatic_member_of!=ms_from) /*|| 
				 (static_member_of && static_member_of!=ms_from)*/) ) )
		{
			if(!bogus_match)  bogus_match=nn->asb;
			continue;
		}
		else if(ms_quality==2 && 
			(nonstatic_member_of!=ms_from && static_member_of!=ms_from) )
		{
			// Should have been caught above. 
			AniScopeBase *is_member_of=
				static_member_of ? static_member_of : nonstatic_member_of;
			fprintf(stderr,"OOPS: %s: from=%s, imo=%s\n",
				our_idf->GetLocationRange().PosRangeString().str(),
				ms_from->CompleteName().str(),
				is_member_of ? is_member_of->CompleteName().str() : "(null)");
			assert(0);
		}
		
		if(!ascope)
		{  ascope=nn->asb;  }
		else
		{
			// Second match. If same scope, it must be a function 
			// (otherwise error in registration code). 
			// If different scope, bail out unless constructor... 
			if(ascope->Parent()!=nn->asb->Parent())
			{
				if(ascope->Parent()==nn->asb && 
				   nn->asb->ASType()==AST_Object)
				{
					// Object type name; constructor lookup. 
					// Return object. 
					ascope=nn->asb;
				}
				break;
			}
			// If this assert fails, probably bug in registration code. 
			if(nn->asb->ASType()!=AST_UserFunc && 
			   nn->asb->ASType()!=AST_InternalFunc )
			{  assert(0);  }
		}
	}
	assert(ascope || bogus_match);
	
	if(ascope)
	{
		// Well, there are restrictions on the access of 
		// static vars... (not so for functions). 
		// (Must avoid access to net-yet-set-up static vars here.) 
		if( (ascope->ASType()==AST_UserVar || 
		     ascope->ASType()==AST_InternalVar) && 
			(((AniVariable*)ascope)->GetAttrib() & 
		   		AniVariable::IS_Static) )
		{
			// See if we may access ascope. 
			int may_access=-1;
			do {
				int our_hierarchy_depth=0,ascope_hierarchy_depth=0;
				for(AniScopeBase *asb=this; asb; 
					asb=asb->Parent(),our_hierarchy_depth++);
				for(AniScopeBase *asb=ascope; asb; 
					asb=asb->Parent(),ascope_hierarchy_depth++);
				// If the accessed var is higher in the hierarchy tree 
				// (less distance to root), we can access it. 
				// If further down, we cannot. 
				if(ascope_hierarchy_depth<our_hierarchy_depth)
				{  may_access=1;  break;  }
				else if(ascope_hierarchy_depth>our_hierarchy_depth)
				{  may_access=0;  break;  }
				// If same distance we must look closer. 
				// For same parent, the var_seq_num applies which 
				// affects visibility and was already checked. 
				if(Parent()==ascope->Parent())
				{  may_access=1;  break;  }
				// Otherwise, we must find the common parent of both 
				// scopes: 
				AniScopeBase *our_hier=this,*ascope_hier=ascope;
				for(;our_hier && ascope_hier;
					our_hier=our_hier->Parent(),
						ascope_hier=ascope_hier->Parent())
				{
					if(our_hier->Parent()==ascope_hier->Parent())
					{  goto found_common_parent;  }
				}
				// Common parent must exist. 
				assert(0);
				found_common_parent:;
				// Check who comes first in the parent list because 
				// who comes first is initialized first. 
				// (our_hier->Parent()=ascope_hier->Parent() here)
				AniAniSetObj *comm_par=(AniAniSetObj*)our_hier->Parent();
				if(comm_par->ASType()!=AST_Animation && 
				   comm_par->ASType()!=AST_Setting && 
				   comm_par->ASType()!=AST_Object )
				{  assert(0);  }
				for(const AniAniSetObj *ivar=
					comm_par->GetAniSetObjList()->first();
					ivar; ivar=ivar->next)
				{
					if(ivar==our_hier)  // we are first
					{  may_access=0;  goto found_us;  }
					if(ivar==ascope_hier)  // he is first
					{  may_access=1;  goto found_us;  }
				}
				assert(0);  // OOPS: Not found in parent list?!
				found_us:;
			} while(0);
			assert(may_access>=0);
			if(!may_access)
			{
				TNLocation loc=our_idf->GetLocationRange();
				Error(loc,
					"cannot access static %s @%s from here\n",
					ascope->CompleteName().str(),
					ascope->GetTreeNode()->GetLocationRange().PosRangeStringRelative(loc).str());
				return(NULL);
			}
		}
	}
	else  // (!ascope)
	{
		TNLocation loc=our_idf->GetLocationRange();
		Error(loc,
			"cannot use %s defined @%s without an object\n",
			bogus_match->CompleteName().str(),
			bogus_match->NameLocation().PosRangeStringRelative(loc).str());
		return(NULL);
	}
	
	/*fprintf(stdout,"  >>match: %s @%s\n",
		ascope->CompleteName().str(),
		ascope->ASType()==AST_Animation ? "[top level]" : 
		ascope->NameLocation().PosRangeStringRelative(
			our_idf->GetLocationRange()).str());*/
	
	return(ascope);
}


void AniIncomplete::CG_LookupIdentifier(LookupIdentifierInfo *lii)
{
	lii->ani_scope=NULL;
	lii->evalfunc=NULL;
	
	TNIdentifier *our_idf=(TNIdentifier*)GetTreeNode();
	assert(our_idf->NType()==TN_Identifier);
	
	assert(Parent());  // We're AniIncomplete -- must have parent. 
	
	// Okay, first check specials: 
	AniScopeBase *ascope=NULL;
	int special_flag=_LookupIdentifier_Special(lii,&ascope);
	if(special_flag<0) return;  // error
	if(!special_flag)
	{
		ascope=_LookupIdentifier_Normal(lii);
		if(!ascope) return;  // error / not found
	}
	// HERE: special_flag: 0 -> no, ascope is the identifier. 
	//                     1 -> "this"; ascope is the object in question
	
	if(special_flag==1)
	{
		// Store the info for the caller: 
		lii->ani_scope=ascope;
		
		int scope_up_cnt=0;
		AniScopeBase *asb=Parent();
		// Find function scope in hierarchy. 
		for(;asb && asb->ASType()!=AST_UserFunc && 
		            asb->ASType()!=AST_InternalFunc;  // <-- "$$povhook"
		     asb=asb->Parent(),++scope_up_cnt);
		// If this fails, we are not inside a function 
		// which should have been checked by 
		// _LookupIdentifier_Special(). 
		// (Note that AST_InternalFunc may not happen in hierarchy.) 
		assert(asb);
		
		#if PRINT_EVFUNC_SEL
		fprintf(stderr,"EVALFUNC: %s: %s --this-#\n",
			our_idf->GetLocationRange().PosRangeString().str(),
			our_idf->CompleteStr().str());
		#endif
		lii->evalfunc=new AniUserVariable::EAF_This();
		
		// Do the TreeNode update and self removal:
		_TNUpdate_SelfRemoval(ascope);
	}
	else if(special_flag==2)
	{
		// Store the info for the caller: 
		lii->ani_scope=this;
		// For the assert, see _TNUpdate_SelfRemoval(). 
		assert(our_idf->ani_scope==this);
		
		// The eval func can not yet be stored as we do not 
		// know the type of the "NULL" expression. 
		// Anyways, NULL is handeled specially at time of 
		// assignment... but we must supply SOME eval func. 
		// Hence, use a special one: 
		lii->evalfunc=new AniUserVariable::EAF_NULL();
		
		// We now know the incomplete type: 
		this->incomplete_type=IT_NULL;
		this->name=*_incomplete_null_str;
		
		// NO _TNUpdate_SelfRemoval(). 
	}
	else if(ascope->ASType()==AST_UserFunc || 
	        ascope->ASType()==AST_InternalFunc )
	{
		// For functions, return the incomplete scope again. 
		lii->ani_scope=this;
		// For the assert, see _TNUpdate_SelfRemoval(). 
		assert(our_idf->ani_scope==this);
		
		// I do NOT return an eval func but the member select 
		// info has to be respected when calling the function. 
		// The problem is that we do not know the function here 
		// (argument overloading resolution has not yet been 
		// done). 
		lii->evalfunc=NULL;
		// Store it in the AniIncomplete so that we can update 
		// the identifier once the function is looked up: 
		assert(!our_idf->evaladrfunc);
		assert(!idf_to_update);
		this->idf_to_update=our_idf;
		this->itu_for_fcall=1;
		
		// We now know the incomplete type: 
		this->incomplete_type=IT_Function;
		RefString tmp;
		tmp.sprintf(0,"[incomplete function \"%s\"]",
			our_idf->CompleteStr().str());
		this->name=tmp;
		
		// NO _TNUpdate_SelfRemoval(). 
	}
	else if(ascope->ASType()==AST_UserVar || 
	        ascope->ASType()==AST_InternalVar)
	{
		// Store the info for the caller: 
		lii->ani_scope=ascope;
		
		// This is the just-looked-up variable: 
		AniVariable *avar=(AniVariable*)ascope;
		
		// Query the eval addressation function from the variable: 
		lii->evalfunc=avar->GetVarEvalAdrFuncFunc(our_idf,this);
		
		// Do the TreeNode update and self removal:
		_TNUpdate_SelfRemoval(ascope);
	}
	else if(ascope->ASType()==AST_Object || 
	        ascope->ASType()==AST_Setting || 
	        ascope->ASType()==AST_Animation )
	{
		// Store the info for the caller: 
		lii->ani_scope=ascope;
		
		//fprintf(stderr,"CG_LookupIdentifier->object->%s @%s\n",
		//	our_idf->CompleteStr().str(),
		//	our_idf->GetLocationRange().PosRangeString().str());
		
		// Do the TreeNode update and self removal:
		_TNUpdate_SelfRemoval(ascope);
	}
	/*else if(ascope->ASType()==AST_Animation || 
	        ascope->ASType()==AST_Setting || 
	        ascope->ASType()==AST_Object )
	{
		Error(our_idf->GetLocationRange(),
			"invalid use of %s %s\n",
			ascope->AniScopeTypeStr(),
			ascope->CompleteName().str());
		return;
	}*/
	else assert(0);  // <-- Add more handlers if that fails. (No longer...:)
}


void AniIncomplete::_TNUpdate_SelfRemoval(AniScopeBase *update_base)
{
	if(!update_base)  return;
	
	// Lookup was successful
	// Replace the AniIncomplete in the TNIdentifier node 
	// with the correct ani scope (function). 
	// *this is then useless and should be deleted. 
	TNIdentifier *tn_idf=(TNIdentifier*)GetTreeNode();
	assert(tn_idf->NType()==TN_Identifier);
	// If that assert fails, somebody else must have assigned 
	// some different ani scope to the TNIdentifer. STRANGE. 
	assert(((AniScopeBase*)tn_idf->ani_scope)->ASType()==AST_Incomplete);
	// This may not fail if the previous did not fail. 
	assert(tn_idf->ani_scope==this);
	
	// Update...
	tn_idf->ani_scope=update_base;
	delete this;  // Will dequeue us from parent. 
}


int AniIncomplete::FixIncompleteTypes()
{
	int nerrors=0;
	// No children to pass on...
	
	// Otherwise, nothing to do either. 
	// AniIncomplete is resolved later. 
	assert(0);  // may not be called
	
	return(nerrors);
}


AniScopeBase *AniIncomplete::RegisterTN_DoFinalChecks(
	ANI::TRegistrationInfo * /*tri*/)
{
	// Function works recursively. Tell all our children: 
	// --oops, we have none. Okay. 
	
	// Perform checks...
	// -> Currently, nothing to do. 
	assert(0);  // May not be called. This is just a ani scope tree placeholder. 
	
	return(NULL);  // Always NULL; that is okay. 
}


int AniIncomplete::PerformCleanup(int cstep)
{
	int nerr=0;
	
	switch(cstep)
	{
		case 1:  break;  // Nothing to do (incomplete cleanup done by parent). 
		case 2:  break;  // Nothing to do for InstanceInfo creation step. 
		default:  assert(0);
	}
	
	return(nerr);
}


void AniIncomplete::LookupName(const RefString &name,
	AniScopeBase::ASB_List *ret_list,int query_parent)
{
	// There are no children, so we cannot look up anything: 
	if(parent && query_parent)
	{  parent->LookupName(name,ret_list,query_parent-1);  }
}


void AniIncomplete::QueueChild(AniScopeBase * /*asb*/,int /*do_queue*/)
{
	// AniIncomplete has no children. 
	assert(0);
}

int AniIncomplete::CountScopes(int &n_incomplete) const
{
	++n_incomplete;
	return(0);
}


ANI::TNLocation AniIncomplete::NameLocation() const
{
	return(GetTreeNode()->GetLocationRange());
}


const char *AniIncomplete::CG_AniScopeTypeStr() const
{
	//return("[incomplete]");
	return(name.str());
}


void AniIncomplete::AniTreeDump(StringTreeDump *d)
{
	// It is probably useful to hide the incomplete types. 
	// For debugging, show them. 
	d->Append(AniScopeTypeStr());
	d->Append(" ");
	d->Append(((TNIdentifier*)GetTreeNode())->CompleteStr().str());
	char tmp[32];
	snprintf(tmp,32," (seq=%d)",var_seq_num);
	d->Append(tmp);
	if(ms_quality)
	{
		d->Append(" MS: ");
		if(ms_quality==1)
		{  d->Append("implicit");  }
		else if(ms_quality==2)
		{  d->Append("explicit");  }
		else assert(0);
		d->Append(" from ");
		assert(ms_from);
		d->Append(ms_from->CompleteName().str());
	}
	else assert(!ms_from);
	d->Append("\n");
}


AniIncomplete::AniIncomplete(ANI::TreeNode *_treenode,AniScopeBase *_parent,
	int _var_seq_num,AniScopeBase *_ms_from,int _ms_quality) : 
	AniScopeBase(AST_Incomplete,_treenode,
		// Take your time to parse this :)  C++ rocks, hehe...
		// (N.B.: It's now more tidied up since no longer 
		//  cluttered with CheckMalloc()...)
		*(_incomplete_str ? _incomplete_str : 
		  (	_incomplete_str=NEW<RefString>(),
			 _incomplete_str->set("[incomplete]"),
			 _incomplete_null_str=NEW<RefString>(),
			 _incomplete_null_str->set("NULL"), 
			_incomplete_str)),
		_parent)
{
	assert(_treenode->NType()==TN_Identifier);
	var_seq_num=_var_seq_num;
	assert(var_seq_num>=0);
	
	// Member select info: 
	ms_from=_ms_from;
	ms_quality=_ms_quality;
	
	incomplete_type=IT_Unknown;
	
	itu_for_fcall=0;
	idf_to_update=NULL;
	
	++_incomplete_str_refs;
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniIncomplete::~AniIncomplete()
{
	TreeNode *tn=(TreeNode*)GetTreeNode();
	// Remove us from the (no longer) associated identifier. 
	if(tn) switch(tn->NType())
	{
		case TN_Identifier:
			if( ((TNIdentifier*)tn)->ani_scope==this )
			{  ((TNIdentifier*)tn)->ani_scope=NULL;  }
			break;
		default:
			fprintf(stderr,"T=%d<\n",tn->NType());
			assert(0);
	}
	
	// no children 
	if(--_incomplete_str_refs==0)
	{
		DELETE(_incomplete_str);
		DELETE(_incomplete_null_str);
	}
	
	if(parent)
	{  parent->QueueChild(this,/*do_queue=*/-1);  }
}
