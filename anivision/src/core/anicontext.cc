/*
 * core/anicontext.cc
 * 
 * Animation context routines. 
 * An animation context saves the complete state of a sub-animation, 
 * i.e. an ani in a setting. 
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

#include "aniinstance.h"

// For the POV attach/detach: 
#include <ani-parser/treereg.h>
#include <ani-parser/exprtf.h>


using namespace ANI;


// NOTE: POV ATTACHMENTS: 
// The calling sequence may be a bit confusing: Here is comes: 
// In a_ifuncpov.cc, one of the Eval() functions gets called when 
// the POV attach/detach is executed in the code. 
//  -> AniInternalFunc_POV::_DO_POVAttachDetach()
//  -> AniInternalFunc_POV::_POVAttachDetach()
//  -> AniAnimation::POVAttach()
//      -> Animation_Context::ObjectInstanceNode::DoPOVAttach()
//  -> AniAnimation::POVDetach()
//      -> Animation_Context::ObjectInstanceNode::DoPOVDetach()


Animation_Context::POVAttachmentNode *Animation_Context::ObjectInstanceNode::FindAttachmentNodeByName(
	const RefString &objname)
{
	// Find passed object in the attachment list: 
	// Important: Reverse traversal. 
	for(POVAttachmentNode *i=attach_list.last(); i; i=i->prev)
	{
		if(i->osdef->oname==objname)
		{  return(i);  }
	}
	return(NULL);
}


void Animation_Context::ObjectInstanceNode::handlenotify(int /*ntype*/,
	AniObjectInstance *oi)
{
	assert(oi==obj_instance);
	
	if(context_back)
	{  context_back->_ForceDeleteInstanceNode(this);  }
	else
	{  assert(0);  }  // pov attach without context?!
	// NOTE: This DELETED us, do nothing more here!
}


int Animation_Context::ObjectInstanceNode::DoPOVAttach(POV::ObjectSpec *_ospec)
{
	// Need to clone the tree with the expressions, etc, 
	// which were specified in the POV command comment. 
	// This is done by the POVAttachmentNode() constructor: 
	Animation_Context::POVAttachmentNode *an=
		new Animation_Context::POVAttachmentNode(_ospec);
	
	// Get cloned object spec entry: 
	POV::PTNObjectSpec *ospec=an->osdef->CloneObjSpec();
	// Assigned to an->ospec lateron. 
	
	// Okay, cloned tree in ospec. 
	// Now, we have to do all the nice registration and TF stuff. 
	// As for the registration, the TRL_AllIdentifiers step is all 
	// we need. 
	// NOTE: The $pov statements are called inside a function 
	//       (normally) but the expressions in the object command 
	//       comment are to be interpreted at object level. I.e. 
	//       may not access automatic vars inside functions or 
	//       anonymous scopes. 
	// Consequences: read HOW IT WORKS in a_ifuncpov.cc. 
	
	int errors=0;
	bool ospec_added_to_compound=0;
	do {
		AniObject *asb=(AniObject*)obj_instance->GetASB();
		
		fprintf(stderr,"POV: %s: Attaching to object \"%s\":\n",
			asb->CompleteName().str(),
			an->osdef->oname.str());
		
		//----------------------------------------
		TRegistrationInfo tri;
		tri.who=TRegistrationInfo::TRL_AllIdentifiers;
		for(AniScopeBase *i=asb; i; i=i->Parent()) switch(i->ASType())
		{
			case AniScopeBase::AST_Animation:
				if(!tri.animation) tri.animation=(AniAnimation*)i;
				break;
			case AniScopeBase::AST_Setting:
				if(!tri.setting)  tri.setting=(AniSetting*)i;
				break;
			case AniScopeBase::AST_Object:
				if(!tri.object)  tri.object=(AniObject*)i;
				break;
			// default: go on
		}
		assert(tri.object);
		assert(tri.animation);
		// This is special: Use the POV function as function. 
		// (Read HOW IT WORKS in a_ifuncpov.cc). 
		tri.function=asb->pov_attach_detach;
		assert(tri.function);
		TNCompoundStmt *pov_hook_compound=obj_instance->_GetPOVCompound();
		tri.anonscope=pov_hook_compound->anonscope;
		tri.var_seq_num=0x7fffffff;  // Can see everything inside object. 
		
		// Read HOW IT WORKS in a_ifuncpov.cc...
		// This is a bit hack-like... we need to add it to the head 
		// nodes first for TreeNode::SetParent() and the like. 
		// It was removed from head nodes by CloneObjSpec(). 
		TreeNode::AppendToHeadNodes(ospec);
		// Add child to compound: 
		pov_hook_compound->AddChild(ospec);
		ospec_added_to_compound=1;
		
		fprintf(stderr,"POV:   Registration of obj expr for \"%s\"...\n",
			an->osdef->oname.str());
		ospec->DoRegistration(&tri);
		if(tri.n_errors)
		{
			fprintf(stderr,"POV:     %d errors\n",tri.n_errors);
			errors=tri.n_errors;
			break;
		}
		int n_left_out=ospec->CheckAniScopePresence();
		fprintf(stderr,"POV:     success (%d left out scopes)\n",n_left_out);
		if(n_left_out)
		{  ++errors;  break;  }
		
		{
			int n_incomplete=0;
			int n_scopes=asb->pov_attach_detach->CountScopes(
				n_incomplete);
			fprintf(stderr,"POV:   Subtree: %d nodes; Ani: %d + %d incomplete\n",
				ospec->CountNodes(),n_scopes,n_incomplete);
		}
		
		//----------------------------------------
		TFInfo ti;
		fprintf(stderr,"POV:   Type fixup / constant folding...\n");
		errors+=ospec->DoExprTF(&ti);
		fprintf(stderr,"POV:   TF: %d errors; %d node visitations\n",
			errors,ti.n_nodes_visited);
		if(errors)  break;
		
		// CLEANUP STEP 1
		asb->pov_attach_detach->PerformCleanup(/*cstep=*/1);
		
		{
			int n_incomplete=0;
			int n_scopes=asb->pov_attach_detach->CountScopes(
				n_incomplete);
			fprintf(stderr,"POV:   Subtree: %d nodes; Ani: %d + %d incomplete\n",
				ospec->CountNodes(),n_scopes,n_incomplete);
			// Ani scopes must be 2+0 here: 
			// 2 (the internal pov function scope (AniInternalFunc_POV) and 
			//   one anonymous scope inside it
			// 0 incomplete (after cleanup)
			assert(n_scopes==2);
			assert(n_incomplete==0);
		}
		
		ANI::CheckIfEvalInfo cinfo;
		errors+=ospec->CheckIfEvaluable(&cinfo);
		fprintf(stderr,"POV:   Evaluable check: %d errors "
			"(refs_to: this: %d, stack: %d, val: %d, static: %d)\n",
			errors,
			cinfo.nrefs_to_this_ptr,cinfo.nrefs_to_local_stack,
				cinfo.number_of_values,cinfo.nrefs_to_static);
		if(errors)  break;
	} while(0);
	
	if(errors)
	{
		if(ospec && ospec_added_to_compound)
		{
			//TNCompoundStmt *pov_hook_compound=_GetPOVCompound();
			//delete pov_hook_compound->RemoveChild(ospec);
			ospec->DeleteTree(); ospec=NULL;
		}
		
		DELETE(an);
		return(2);  // 2 -> "failed to attach"
	}
	
	// Important: Append at the end. 
	an->ospec=ospec; ospec=NULL;
	attach_list.append(an);
	
	// Some verbose message for debugging: 
	DumpAttachments(stderr);
	
	return(0);
}


int Animation_Context::ObjectInstanceNode::DoPOVDetach(
	Animation_Context::POVAttachmentNode *an)
{
	// Check the attach code. Need to un-do what was done there. 
	assert(an->ospec);
	
	AniObject *asb=(AniObject*)obj_instance->GetASB();
	
	fprintf(stderr,"POV: %s: Detaching from object \"%s\".\n",
		asb->CompleteName().str(),
		an->osdef->oname.str());
	
	//TNCompoundStmt *pov_compound=obj_instance->_GetPOVCompound();
	//delete pov_compound->RemoveChild(an->ospec);
	an->ospec->DeleteTree();  an->ospec=NULL;
	
	delete attach_list.dequeue(an);
	
	// Some verbose message for debugging: 
	DumpAttachments(stderr);
	
	return(0);
}


void Animation_Context::ObjectInstanceNode::DumpAttachments(FILE *out)
{
	fprintf(out,"POV: Attachments for %s: (%d entries)\n",
		obj_instance->GetASB()->CompleteName().str(),attach_list.count());
	for(POVAttachmentNode *i=attach_list.first(); i; i=i->next)
	{
		fprintf(out,"POV:   %s @%s\n",
			i->osdef->oname.str(),
			i->osdef->GetLocationRange().PosRangeString().str());
	}
}


Animation_Context::ObjectInstanceNode::ObjectInstanceNode(
	Animation_Context *_context,AniObjectInstance *_obj_instance) : 
	NotifyHandler<AniObjectInstance>(),
	attach_list()
{
	context_back=_context;
	obj_instance=_obj_instance;
	
	// Install delete notifier: 
	obj_instance->RegisterDeleteNotifier(this);
}

Animation_Context::ObjectInstanceNode::~ObjectInstanceNode()
{
	assert(attach_list.is_empty());
	
	// Uninstall delete notifier: 
	obj_instance->UnregisterDeleteNotifier(this);
	
	obj_instance=NULL;
	context_back=NULL;
}

//------------------------------------------------------------------------------

Animation_Context::POVAttachmentNode::POVAttachmentNode(POV::ObjectSpec *_osdef) : 
	LinkedListBase<POVAttachmentNode>()
{
	osdef=_osdef;
	ospec=NULL;  // Clone done by attach code. 
}

Animation_Context::POVAttachmentNode::~POVAttachmentNode()
{
	osdef=NULL;  // No DELETE(); managed by POV::CommandParser in AniAnimation. 
	
	// But... this one was cloned: It must have been cleaned 
	// up by the detach code. REALLY NOT HERE. 
	assert(!ospec);
}

//------------------------------------------------------------------------------

Animation_Context::ObjectInstanceNode *Animation_Context::_FindAllocObjectInstanceNode(
	AniObjectInstance *obj_inst,ssize_t *ret_idx,int alloc_new)
{
	// Find associated object instance node or create new one: 
	ssize_t idx=pov_instlist.FindKeyIdx(obj_inst);
	if(idx>=0)
	{
		if(ret_idx) *ret_idx=idx;
		return(pov_instlist[idx]);
	}
	if(!alloc_new)
	{
		if(ret_idx) *ret_idx=-1;
		return(NULL);
	}
	// Allocate new one: 
	ObjectInstanceNode *oin=new ObjectInstanceNode(this,obj_inst);
	int rv=pov_instlist.Insert(oin,0,&idx);
	assert(rv==0);
	if(ret_idx) *ret_idx=idx;
	return(oin);
}


int Animation_Context::_CheckDeleteObjectInstanceNode(ssize_t idx)
{
	if(idx<0 || (size_t)idx>=pov_instlist.NElem()) assert(0);
	ObjectInstanceNode *oin=pov_instlist[idx];
	
	// See if we still need the node; if so, do nothing. 
	if(!oin->attach_list.is_empty())  return(0);
	
	// Node no longer needed. Remove it. 
	pov_instlist.RemoveIdx(idx);
	delete oin;
	return(1);
}


void Animation_Context::_ForceDeleteInstanceNode(ObjectInstanceNode *oin)
{
	// We must know the node: 
	ssize_t idx=pov_instlist.FindIdx(oin);
	assert(idx>=0);
	
	// Detach from all POV objects: 
	while(!oin->attach_list.is_empty())
	{
		oin->DoPOVDetach(oin->attach_list.last());
	}
	
	// The code above MUST make sure that the deletion is now 
	// regularly possible. 
	int rv=_CheckDeleteObjectInstanceNode(idx);
	assert(rv==1);  // deleted
}


ExecThread *Animation_Context::LookupThreadByID(int thread_id)
{
	for(ExecThread *et=thread_list.first(); et; et=et->next)
	{
		if(et->thread_id==thread_id)  return(et);
	}
	return(NULL);
}


Animation_Context::Animation_Context(const String &_name,AniSetting *_setting,
	int eval_thread_id) : 
	name(_name),
	thread_list(),
	pov_instlist()
{
	setting=_setting;
	
	// Create an ExecThread for the evaluation of the 
	// POV expressions and (simple) ADB functions: 
	eval_thread=new ExecThread(/*Animation_Context=*/this,eval_thread_id);
	eval_thread->state=ExecThread::Running;
	eval_thread->schedule_status=ExecThread::SSDisabled;
	
	ccif=CC::CCIF_Factory::CreateCCIF_SettingArray();
}

Animation_Context::~Animation_Context()
{
	DELETE(eval_thread);
	while(!thread_list.is_empty())
	{  delete thread_list.popfirst();  }
	// After having deleted all threads, there may not be 
	// any object instance nodes left: 
	if(pov_instlist.NElem())
	{
		fprintf(stderr,"OOPS: pov_instlist still contains %d elements:\n",
			pov_instlist.NElem());
		// This normally means that an object instance is till existing 
		// although it should not. 
		for(ssize_t i=0; i<pov_instlist.NElem(); i++)
		{
			Animation_Context::ObjectInstanceNode *oin=pov_instlist[i];
			fprintf(stderr,"  [%d] attach_list (%d entries)  "
				"obj_instance=%p (%s): \n",
				i,oin->attach_list.count(),
				oin->obj_instance,
				oin->obj_instance->GetASB()->CompleteName().str());
			int j=0;
			for(Animation_Context::POVAttachmentNode *pan=oin->attach_list.first();
				pan; pan=pan->next,j++)
			{
				fprintf(stderr,"     [%d] %s (%s)\n",
					j,pan->osdef->oname.str(),
					POV::ObjectSpec::ObjectTypeStr(pan->osdef->otype));
			}
		}
		abort();
	}
	ccif=CC::CCIF_Factory::DestroyCCIF_SettingArray(ccif);
	setting=NULL;
}

