/*
 * calccore/anitmt/factory.h
 * 
 * Simple calc core factory (= the calc core representation in anivision). 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_CALCCORE_ANITMT_FACTORY_H_
#define _ANIVISION_CALCCORE_ANITMT_FACTORY_H_ 1

#include <hlib/linkedlist.h>
#include <calccore/ccif.h>

//#include <core/execthread.h>

//#include <calccore/adtree.h>
////#include <numerics/la_basics.h>
//#include <numerics/spline/sl_base.h>

//#include <objdesc/move.h>

#include <ani-parser/treereg.h>
#include <ani-parser/exprvalue.h>

#include <message/message.hpp>

#include <proptree/proptree.hpp>
#include <solve/priority.hpp>
#include <functionality/object_base.hpp>
#include <animation.hpp>


namespace CC
{

namespace AniTMT
{
class CCSetting;
class CCObject;
class CCValue;

class SCC_ScopeTreeParser;
}

class CCIF_Factory_AniTMT : public CCIF_Factory
{
	private:
		// For error reporting...
		message::Stream_Message_Handler msg_handler;
		message::Message_Manager msg_manager;
		message::Message_Consultant msg_consultant;
		
		// AniTMT stuff... 
		proptree::Semi_Global semi_global;
		solve::Priority_System priority_system;
		proptree::tree_info pt_tree_info;
		
		int curr_depth;
		
	public:  _CPP_OPERATORS
		CCIF_Factory_AniTMT(int _core_index);
		~CCIF_Factory_AniTMT();
		
		// Get message consultant / tree info: 
		message::Message_Consultant *GetMessageConsultant() const
			{  return(&msg_consultant);  }
		proptree::tree_info *GetTreeInfo() const
			{  return(&pt_tree_info);  }
		solve::Priority_System *GetPrioritySystem() const
			{  return(&priority_system);  }
		proptree::Semi_Global *GetGlobalInfo() const
			{  return(&semi_global);  }
		
		// Return string with scope name hierarchy. 
		String TopScopeName(ADTNTypedScope *tn);
		
		// [overriding virtuals from CCIF_Factory:]
		CCIF_Setting *CreateCCIF_Setting();
		CCIF_Object *CreateCCIF_Object(CCIF_Setting *setting,int vflags,
			AniObjectInstance *obj_inst);
		CCIF_Value *CreateCCIF_Value(CCIF_Setting *setting,int dimension);
		
		// [overriding virtuals from CCIF_Factory:]
		void DoADTNRegsistration(ADTNAniDescBlock *tn,
			ANI::TRegistrationInfo *tri);
		void DoADTNRegsistration(ADTNTypedScope *tn,
			ANI::TRegistrationInfo *tri);
		void DoADTNRegsistration(ADTNScopeEntry *tn,
			ANI::TRegistrationInfo *tri);
		
		// [overriding virtuals from CCIF_Factory:]
		int DoADTNExprTF(ANI::TFInfo *ti,ADTNAniDescBlock *tn);
		int DoADTNExprTF(ANI::TFInfo *ti,ADTNTypedScope *tn);
		int DoADTNExprTF(ANI::TFInfo *ti,ADTNScopeEntry *tn);
};

}  // end of namespace CC

#endif  /* _ANIVISION_CALCCORE_ANITMT_FACTORY_H_ */
