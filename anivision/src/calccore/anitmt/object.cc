/*
 * calccore/anitmt/object.cc
 * 
 * AniTMT calc core object main file. 
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

#include "ccif_anitmt.h"
#include "factory.h"

#include <objdesc/omatrix.h>

#include <core/aniinstance.h>

#include "save_filled.hpp"

//#include <objdesc/curve_fixed.h>
//#include <objdesc/curve_lspline.h>
//#include <objdesc/curve_cspline.h>
//#include "curve_function.h"

//#include <objdesc/curvetmap_none.h>
//#include <objdesc/curvetmap_linear.h>

//#include <objdesc/frontspec_speed.h>

//#include <objdesc/upspec_gravity.h>


namespace CC { namespace AniTMT
{

int CCObject::_Pass1PropertyDeclaration(ADTNScopeEntry *scent,
	const RefString &name_str,const ANI::ExprValue &val)
{
	fprintf(stderr,"PropDecl(name=%s,val=%s)\n",
		name_str.str(),val.ToString().str());
	
	if(!name_str)
	{
		Error(scent->GetLocationRange(),
			"cannot use anonymous properties\n");
		++n_errors;
		return(0);
	}
	
	std::string name(name_str.str());
	
	proptree::Prop_Tree_Node *curr_chld=get_current_tree_node();
	if(curr_chld)
	{
		proptree::Property *prop = curr_chld->get_property(name);
		if(!prop)
		{
			Error(scent->GetLocationRange(),
				"unknown property \"%s\" in block of type %s\n",
				name.c_str(),curr_chld->get_type().c_str());
			++n_errors;
			return(0);
		}
		
		// FIXME: this should be put into some conversion function
		values::Valtype::Types vtype=prop->get_type();
		
		ANI::ExprValueType evt;
		val.GetExprValueType(&evt);
		Value::CompleteType *pod_evt=evt.PODType();
		
		bool set_val_rv=0;
		bool can_cast=0;
		const char *expected_type="?";
		switch(vtype)
		{
			case values::Valtype::scalar:
			{
				expected_type="scalar";
				values::Scalar v;
				if(pod_evt && pod_evt->type==Value::VTScalar)
				{  v=val.GetPODDouble();  can_cast=1;  }
				else if(pod_evt && pod_evt->type==Value::VTInteger)
				{  v=val.GetPODInt();  can_cast=1;  }
				if(can_cast)
				{  set_val_rv=dynamic_cast<proptree::Type_Property<values::Scalar>*>(
					prop)->set_value(v);  }
			}	break;
			case values::Valtype::vector:
			{
				if(pod_evt && pod_evt->type==Value::VTVector && pod_evt->n==3)
				{
					can_cast=1;
					Value tmp;
					val.GetPOD(&tmp);
					const Vector *tmp_v=tmp.GetPtr<Vector>();
					values::Vector v;
					for(int i=0; i<3; i++)
						v[i]=(*tmp_v)[i];
					set_val_rv=dynamic_cast<proptree::Type_Property<values::Vector>*>(
						prop)->set_value(v);
				}
				expected_type="vector";  // 3
			}	break;
			case values::Valtype::matrix:
			{
				expected_type="matrix"; // 4x4
assert(0);
			}	break;
			case values::Valtype::string:
			{
				expected_type="string";
assert(0);
			}	break;
			case values::Valtype::flag:
			{
				expected_type="flag";
				values::Flag v=!val.is_null();
				set_val_rv=dynamic_cast<proptree::Type_Property<values::Flag>*>(
					prop)->set_value(v);
				can_cast=1;
			}	break;
			default: assert(0);
		}
		
		if(!can_cast)
		{
			Error(scent->GetLocationRange(),
				"cannot convert type %s to %s for property \"%s\"\n",
				evt.TypeString().str(),
				expected_type,
				name.c_str());
			++n_errors;
		}
		else if(!set_val_rv)
		{
			Error(scent->GetLocationRange(),
				"property \"%s\" rejected due to conflict with "
				"previous input\n",
				name.c_str());
			++n_errors;
		}
	}
	
	return(0);
}

int CCObject::_Pass1OpenBlockDecl(ADTNTypedScope *tsc,
	const RefString &type_str,const RefString &name_str,int subpass)
{
	fprintf(stderr,"SCOPE(subpass=%d): type=%s, name=%s\n",
		subpass,type_str.str(),name_str.str());
	
	proptree::Prop_Tree_Node *curr_chld=get_current_tree_node();
	if(curr_chld)
	{
		std::string type(type_str.str());
		std::string name("");
		
		if(name_str)  name=name_str.str();
		
		if(subpass==0)
		{
			proptree::Prop_Tree_Node *node = 0;
			if( name != "" )	// if there is a name given -> search child
			{  node = curr_chld->get_child( name );  }
			if( node != 0 )		// if already found -> check type
			{
				if( node->get_type() != type )  // same block may be opened multiple times (useful?)
				{
					Error(tsc->GetLocationRange(),
						"child \"%s\" already exists with different type %s\n",
						name.c_str(),node->get_type().c_str());
					++n_errors;
				}
			}
			else			// else -> add new child
			{  node = curr_chld->add_child( type, name );  }

			if( node == 0 )		// was it not possible to add child?
			{
				Error(tsc->GetLocationRange(),
					"couldn't add tree node \"%s\" as type %s isn't allowed here\n",
					name.c_str(),type_str.str());
				++n_errors;
				// We need to push something to get the number of push/pop right: 
				set_new_tree_node( NULL );
			}
			else
			{
				set_new_tree_node( node );
			}
		}
		else if(subpass==1)
		{
			// For seconds pass this is easy...
			set_new_tree_node( child_manager->get_child() );
		}
	}
	else
	{
		// We need to push something to get the number of push/pop right: 
		set_new_tree_node( NULL );
	}
	
	fprintf(stderr,"SCOPE: done\n");
	
	return(0);
}

int CCObject::_Pass1FinishBlockDecl(int subpass)
{
	if(subpass==1)
	{  child_manager->child_finished();  }
	
	tree_node_done();
	
	return(0);
}


int CCObject::_PerformPass1(ADTNAniDescBlock *adb,ExecThread *et,int subpass)
{
	for(ANI::TreeNode *_tn=et->ess->popTN(adb->down.first()); 
		_tn; _tn=_tn->next)
	{
		if(_tn->NType()!=ANI::TN_AniDesc) assert(0);
		if(((ADTreeNode*)_tn)->ADNType()!=ADTN_ScopeEntry) assert(0);
		
		ADTNScopeEntry *scent=(ADTNScopeEntry*)_tn;
		
		// The core hook is attached during registration step. 
//ADTNScopeEntry_CoreHook *scent_ch=
//		(ADTNScopeEntry_CoreHook*)scent->ent_core_hook;
//assert(scent_ch);
//
// This is checked in ADTN's DoExprTF():  
//assert(!scent_ch->entry_name_tok);  // 0 -> not set
		
		int rv=_PerformPass1(scent,et,subpass);
		if(rv)
		{
			// Implement context switch. 
			assert(0);
		}
	}
	
	// Regular (i.e. non-context-switch) exit from pass1: 
	pt_root->hierarchy_final_init();
	
	return(0);
}

int CCObject::_PerformPass1(ADTNTypedScope *tsc,ExecThread *et,int subpass)
{
	RefString tsc_type(tsc->scope_type->name);
	RefString tsc_name;
	if(tsc->scope_name)
	{  tsc_name=tsc->scope_name->CompleteStr();  }
	
	_Pass1OpenBlockDecl(tsc,tsc_type,tsc_name,subpass);
	
	for(ANI::TreeNode *_tn=et->ess->popTN(tsc->first_entry); 
		_tn; _tn=_tn->next)
	{
		if(_tn->NType()!=ANI::TN_AniDesc) assert(0);
		if(((ADTreeNode*)_tn)->ADNType()!=ADTN_ScopeEntry) assert(0);
		
		ADTNScopeEntry *scent=(ADTNScopeEntry*)_tn;
		
		int rv=_PerformPass1(scent,et,subpass);
		if(rv)
		{
			// Implement context switch. 
			assert(0);
		}
	}
	
	_Pass1FinishBlockDecl(subpass);
	
	return(0);
}

int CCObject::_PerformPass1(ADTNScopeEntry *scent,ExecThread *et,int subpass)
{
	ANI::TNIdentifier *name = (scent->down.first()==scent->down.last() ? 
			NULL : (ANI::TNIdentifier*)scent->down.first());
	ANI::TreeNode *value=scent->down.last();
	
	RefString name_str;
	if(name)
	{  name_str=name->CompleteStr();  }
	
	if(value->NType()==ANI::TN_AniDesc && 
		((ADTreeNode*)value)->ADNType()==ADTN_TypedScope)
	{
		ADTNTypedScope *tsc=(ADTNTypedScope*)value;

		if(name)
		{
			Error(scent->GetLocationRange(),
				"cannot use lvalue name for scope entries\n");
			++n_errors;
			return(0);
		}
		
		int rv=_PerformPass1(tsc,et,subpass);
		if(rv)
		{
			// Implement context switch. 
			assert(0);
		}
	}
	else if(value->NType()==ANI::TN_Expression)
	{
		if(subpass==1)
		{
			// Evaluate the expression: 
			ANI::TNExpression *expr=(ANI::TNExpression*)value;
			
static int warned=0;
if(!warned)
{  Error(value->GetLocationRange(),
	"implement context switch\n");  warned=1;  }

			ANI::ExprValue val;
			int rv;
			do {
				rv=value->Eval(val,et);
			} while(rv);
			
			_Pass1PropertyDeclaration(scent,name_str,val);
		}
	}
	else
	{
		// This should have been caught during ADTN's DoExprTF(). 
fprintf(stderr,"value: %d,%d, %d\n",value->NType(),ANI::TN_AniDesc,
	((ADTreeNode*)value)->ADNType());
		assert(0);
		++n_errors;
	}
	
	return(0);
}


int CCObject::EvalAniDescBlock(ADTNAniDescBlock *adb,ExecThread *et)
{
	fprintf(stdout,"AniTMT: Interpreting ADB @%s\n",
		adb->GetLocationRange().PosRangeString().str());
	
	CCIF_Factory_AniTMT *factory=(CCIF_Factory_AniTMT*)setting->CoreFactory();
	assert(factory);
	
	// Allocate a prop tree root if there is no one yet: 
	if(!pt_root)
	{
		pt_root=new functionality::node_object(
			/*std::string name=*/ani_object_inst->GetASB()->GetName().str(),
			/*proptree::tree_info *info=*/factory->GetTreeInfo(),
			/*message::Message_Consultant *msg_consultant=*/
				factory->GetMessageConsultant() );
		
		// The root must be the bottom element on the ptn stack at 
		// all times. 
		ptn_stack.push(pt_root);
		
		// Add to child manager for subpass1 traversal. 
		child_manager=new proptree::Child_Manager();
		child_manager->set_root_node(pt_root);
	}	
	
	// Perform tree traversal into adb: 
	for(int subpass=0; subpass<2; subpass++)
	{
		int rv=_PerformPass1(adb,et,subpass);
		
		if(rv)
		{
			// Context switch. 
			if(et->schedule_status==ExecThread::SSRequest)
			{
				Error(adb->GetLocationRange(),
					"OOPS: Cloning from inside ADB forbidden\n");
				abort();
			}
			
			return(1);
		}
	}
	
	// If we come here, there is regular exit, i.e. no context switch. 
	
	// Calculate the complete animation: 
	// This should be done after pass2...
	// Maybe this should only be done if there were no errors. 
	factory->GetPrioritySystem()->invoke_all_Actions();
	
	// Write information to user: 
	std::cout << "-------<Complete dump>---------\n";
	anitmt::write_node( std::cout, pt_root, 0);
	std::cout << "-------------------------------\n";
	
	return(n_errors ? -1 : 0);
}

int CCObject::GetVal(OVal ov,int *store_here,CCIF_ExecThread *cet)
{
	fprintf(stderr,"CC:AniTMT: Implement GetVal(int*)\n");
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


int CCObject::GetVal(OVal ov,double *store_here,CCIF_ExecThread *cet)
{
	// First, get the current movement: 
	double setting_time = setting->CurrentSettingTime();
	
	fprintf(stderr,"CC:AniTMT: Calculating ov=%d...\n",ov);
	
	CCIF_Factory_AniTMT *factory=(CCIF_Factory_AniTMT*)setting->CoreFactory();
	proptree::Semi_Global *semi_global=factory->GetGlobalInfo();
	
	bool val_ok=0;
	do {
		if(!pt_root) break;
		values::Vector result;
		switch(ov)
		{
			case ValTime:
				// FIXME: Should return object time(?) For now, I am using setting time. 
				// Only used in povwriter to write a comment, currently. 
				*store_here=setting_time;
				return(0);
			case ValPos:
			{
				std::pair< bool,functionality::position > rv=
					pt_root->_rf_object_component_position_time(setting_time,
						&semi_global->final_run_info);
				val_ok=rv.first;
				result=rv.second;
			} break;
			case ValFront:
			{
				std::pair< bool,functionality::front > rv=
					pt_root->_rf_object_component_front_time(setting_time,
						&semi_global->final_run_info);
				val_ok=rv.first;
				result=rv.second;
			} break;
			case ValUp:
			{
				std::pair< bool,functionality::up_vector > rv=
					pt_root->_rf_object_component_up_vector_time(setting_time,
						&semi_global->final_run_info);
				val_ok=rv.first;
				result=rv.second;
			} break;
			case ValSpeed:
			case ValAccel:
				assert(!"!implemented");
				return(0);
			// NO: default: assert(0); NO!
		}
		if(!val_ok)  break;
		
		// Value is available. 
		for(int i=0; i<3; i++)
			store_here[i]=result[i];
		
		fprintf(stderr,"ANITMT: %d=<%g,%g,%g> ******\n",
			ov,store_here[0],store_here[1],store_here[2]);
		
		return(0);
	} while(0);
	
	// Return correct error value: 
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


int CCObject::GetVal(OVal ov,NUM::ObjectMatrix *store_here,CCIF_ExecThread *cet)
{
	if(ov==ValMatrix)
	{
		fprintf(stderr,"CC:AniTMT: Implement GetVal(matrix)\n");
		
		// This is an ugly hack since it does not interprete the return values!!!
		
		double pos[3],front[3],up[3];
		
		GetVal(ValPos,pos,cet);
		GetVal(ValFront,front,cet);
		GetVal(ValUp,up,cet);
		
		// FIXME: (a) allow right vector; (b) use obj_{front,right,up}
		store_here->SetObjPos(pos,front,up,NULL);
		return(0);
	}
	
	// *store_here=NUM::ObjectMatrix::IdentMat;
	
	// Return correct error value: 
	return(CCIF_Object::GetVal(ov,store_here,cet));
}


CCObject::CCObject(CCSetting *_setting,int _vflags,AniObjectInstance *_inst) : 
	CCIF_Object(_setting,_vflags,_inst),
	LinkedListBase<CCObject>()
{
	pt_root=NULL;
	child_manager=NULL;
	
	n_errors=0;
	
	Setting()->RegisterObject(this);
}

CCObject::~CCObject()
{
	Setting()->UnregisterObject(this);
	
	ptn_stack.pop();
	assert(ptn_stack.empty());
	
	if(child_manager)
	{  delete child_manager;  child_manager=NULL;  }
	// FIXME: detetion: no problem since objects are not connected. 
	if(pt_root)
	{  delete pt_root;  pt_root=NULL;  }
}

}}  // end of namespace CC::S
