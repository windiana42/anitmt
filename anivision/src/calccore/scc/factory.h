/*
 * calccore/scc/factory.h
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

#ifndef _ANIVISION_CALCCORE_SCC_FACTORY_H_
#define _ANIVISION_CALCCORE_SCC_FACTORY_H_ 1

#include <hlib/linkedlist.h>
#include <calccore/ccif.h>

//#include <core/execthread.h>

//#include <calccore/adtree.h>
////#include <numerics/la_basics.h>
//#include <numerics/spline/sl_base.h>

//#include <objdesc/move.h>

#include "scc_token.h"

#include <ani-parser/treereg.h>
#include <ani-parser/exprvalue.h>


namespace CC
{

namespace S
{
class CCSetting;
class CCObject;
class CCValue;

class SCC_ScopeTreeParser;
}

class CCIF_Factory_Simple : public CCIF_Factory
{
	private:
		// Translate (known) strings into integer token IDs (inside ADBs). 
		S::SCCTokenTranslator toktrans;
		
		// Represents a special identifier which we may want to 
		// attach during registration step (for function{$t} etc.). 
		// This is just temporarily allocated to make the registration 
		// step; we use the core hook (CCIF_SpecialIdf_CoreHook below) 
		// to attach to the AniInternalVariable_ADB which stays there 
		// permanently. 
		struct DynIdf : ANI::TRegistrationInfo::SpecialIdf
		{
			DynIdf() : ANI::TRegistrationInfo::SpecialIdf() {}
			~DynIdf() {}
		};
		
		S::SCC_ScopeTreeParser *stp;
		int curr_depth;
		
	public:  _CPP_OPERATORS
		CCIF_Factory_Simple(int _core_index);
		~CCIF_Factory_Simple();
		
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

#endif  /* _ANIVISION_CALCCORE_SCC_FACTORY_H_ */
