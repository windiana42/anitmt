/*
 * core/aniscope.cc
 * 
 * ANI scope tree base class implentation. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "aniscope.h"

using namespace ANI;


static int enable_color_stderr=1;


void AniScopeBase::CG_DeleteNotify(ANI::TreeNode *tn)
{
	// The main TN tree is deleted before the ANI scope tree. 
	//fprintf(stderr,"DeleteNotify: %s -> %s<<\n",CompleteName().str(),
	//	tn->GetLocationRange().PosRangeString().str());
	
	// NOTE: We may not simply "delete this" because an ani 
	// scope may correspond to more than one tree node 
	// (like functions, object (names)...). This only seems to 
	// be safe for AST_Incomplete. 
	// OTOH, upon POV detach, no ani scopes need to be deleted 
	// (incomplete are no longer left and others were not created). 
	
	if(treenode && treenode==tn)
	{
		treenode=NULL;
		
		if(astype==AST_Incomplete)
		{  delete this;  }
	}
}


const char *AniScopeBase::LookupExpectedTypeStr(LookupExpectedType let)
{
	if(let==LET_Any)
	{  return("indentifier name");  }
	
	static char tmp[64];
	char *end=tmp;
	int cnt=0;
	if(let & LET_TypeName)
	{  strcpy(end,"type");  end+=4;  ++cnt;  }
	if(let & LET_FunctionName)
	{
		if(cnt)  *(end++)='/';
		strcpy(end,"function");  end+=8;  ++cnt;
	}
	if(let & LET_VariableName)
	{
		if(cnt)  *(end++)='/';
		strcpy(end,"variable");  end+=8;  ++cnt;
	}
	if(!let)
	{  strcpy(end,"???");  end+=3;  }
	strcpy(end," name");
	return(tmp);
}


int AniScopeBase::Match_LookupExpectedType(AniScopeBase *asb,
	AniScopeBase::LookupExpectedType letype)
{
	switch(asb->ASType())
	{
		case AST_Animation:    return(letype & LET_TypeName);
		case AST_Setting:      return(letype & LET_TypeName);
		case AST_Object:       return(letype & LET_TypeName);
		case AST_UserFunc:     return(letype & LET_FunctionName);
		case AST_InternalFunc: return(letype & LET_FunctionName);
		case AST_AnonScope:    return(0);
		case AST_UserVar:      return(letype & LET_VariableName);
		case AST_InternalVar:  return(letype & LET_VariableName);
		case AST_Incomplete:   assert(0);  // may not happen -- just a placeholder
		default:  assert(0);
	}
	return(0);
}


String AniScopeBase::CompleteName() const
{
	String tmp;
	if(astype==AST_Animation)
	{  tmp="::";  }
	else for(const AniScopeBase *asb=this; asb; asb=asb->parent)
	{
		#if 1  /* normal case */
		if(asb->astype==AST_Animation)  break;
		if(!asb->name)  continue;  // anonymous scopes
		tmp.prepend(asb->name.str());
		tmp.prepend("::");
		#else  /* for debugging */
		if(!asb->name)  tmp.prepend("[anonymous]");
		else
		{
			String tmp2;
			tmp2.sprintf("(%s)",AniScopeTypeStr(asb->ASType()));
			tmp.prepend(tmp2);
			tmp.prepend(asb->name);
		}
		tmp.prepend("::");
		#endif
	}
	//RefString rv;
	//rv.set(tmp.str());
	//return(rv);
	return(tmp);
}

String AniScopeBase::CG_GetName() const
{
	// Not inline beause virtual. 
	return(CompleteName());
}


ANI::TNLocation AniScopeBase::NameLocation() const
{
	// This function must be overridden. 
	assert(0);
	return TNLocation();
}


void AniScopeBase::LookupName(const RefString & /*name*/,
	AniScopeBase::ASB_List * /*ret_list*/,int /*query_parent*/)
{
	// This function must be overridden. 
	assert(0);
}


void AniScopeBase::QueueChild(AniScopeBase * /*asb*/,int /*do_queue*/)
{
	// This function must be overridden. 
	assert(0);
}


void AniScopeBase::AniTreeDump(StringTreeDump * /*d*/)
{
	// This function must be overridden. 
	assert(0);
}


int AniScopeBase::CountScopes(int &) const
{
	// This function must be overridden. 
	assert(0);
	return(0);
}


int AniScopeBase::FixIncompleteTypes()
{
	// This function must be overridden. 
	assert(0);
	return(1);  // <-- number of errors
}


LinkedList<AniScopeBase::TypeFixEntry> *AniScopeBase::GetTypeFixList()
{
	// Needs only be overridden by derived classes which 
	// do have a typefix_list. 
	return(NULL);
}


int AniScopeBase::PerformCleanup(int /*cstep*/)
{
	// This function must be overridden. 
	assert(0);
	return(1);  // <-- number of errors
}


AniScopeBase::InstanceInfo *AniScopeBase::GetInstanceInfo()
{
	// Default implementation will return NULL. 
	return(NULL);
}


int AniScopeBase::InitStaticVariables(ExecThreadInfo * /*info*/,int /*flag*/)
{
	assert(0);
	return(1);
}


AniScopeBase &AniScopeBase::operator=(const AniScopeBase &)
{
	// Forbidden. 
	assert(0);
	return(*this);
}

AniScopeBase::AniScopeBase(const AniScopeBase &)
{
	// Forbidden. 
	assert(0);
}

AniScopeBase::AniScopeBase(AniScopeType _t,TreeNode *_treenode,
	const RefString &_name,AniScopeBase *_parent) : 
	astype(_t),name(_name)
{
	parent=_parent;
	treenode=_treenode;
	if(!parent && astype!=AST_Animation)
	{  assert(0);  }
	
	if(astype!=AST_Incomplete && 
	   astype!=AST_InternalFunc && 
	   astype!=AST_InternalVar )
	{  assert((int)treenode->NType()==(int)astype);  }
	
	// DUE TO INHERICANCE ISSUES; THIS HAS TO BE DONE 
	// IN THE DERIVED CLASS' CONSTRUCTOR. 
	//if(parent)
	//{  parent->QueueChild(this,/*do_queue=*/+1);  }
}

AniScopeBase::~AniScopeBase()
{
	// DUE TO INHERICANCE ISSUES; THIS HAS TO BE DONE 
	// IN THE DERIVED CLASS' DESTRUCTOR. 
	//if(parent)
	//{  parent->QueueChild(this,/*do_queue=*/-1);  }
}


/******************************************************************************/

void AniScopeBase::ASB_List::_forbidden()
{
	assert(0);
}


/******************************************************************************/

AniScopeBase::InstanceInfo::InstanceInfo()
{
	n_stack_vars=0;
	var_types=NULL;
	instance_counter=0;
}

AniScopeBase::InstanceInfo::~InstanceInfo()
{
	if(var_types)
	{  delete[] var_types;  var_types=NULL;  }
	n_stack_vars=0;
	if(instance_counter)
	{
		fprintf(stderr,"OOPS: instance_counter=%d\n",instance_counter);
		assert(0);
	}
}
