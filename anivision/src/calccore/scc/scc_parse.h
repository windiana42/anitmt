/*
 * calccore/scc/scc_parse.h
 * 
 * Simple calc core scope tree parser. 
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

#ifndef _ANIVISION_CALCCORE_SCC_SCOPETREEPARSER_H_
#define _ANIVISION_CALCCORE_SCC_SCOPETREEPARSER_H_ 1

#include <calccore/sctparse.h>
#include "scc_token.h"

namespace ANI { class ExprValueType; }

namespace CC { namespace S
{

class SCC_ScopeTreeParser : public ScopeTreeParser
{
	public:
		// Scope identifiers: 
		enum ScopeID
		{
			// NOTE: !! DO NOT FORGET TO ADD SCOPE TO _scope_name[]. !!
			SC_empty=0,
			SC_move,
			SC_move_front_speed,
			SC_move_up_gravity,
			SC_curve,
			SC_fixed3,
			SC_lspline,
			SC_lspline3,
			SC_lspline_vt,
			SC_lspline_xt,
			SC_cspline,
			SC_cspline3,
			SC_cspline_vt,
			SC_cspline_xt,
			SC_function3,
			
			_SC_LAST  // See note above!
		};
		
		static const char *ScopeID2Str(ScopeID s);
		
		// Type identifiers: 
		enum TypeID
		{
			// DO NOT FORGET TO ADD IN TypeID2Str(). 
			TY_String=1,
			TY_Scalar,
			TY_ScalarArray,
			TY_Range,
			TY_Vector,
			TY_VectorArray,
			TY_Vector3,
			TY_Vector2Array,
			TY_Vector3Array,
			TY_Vector4Array,
			TY_StringOrScalarArray,
		};
		
		static const char *TypeID2Str(TypeID t);
		
	private:
		
	public:  _CPP_OPERATORS
		SCC_ScopeTreeParser();
		~SCC_ScopeTreeParser();
		
		// Check if the passed expression value type is compatible 
		// with TY_* from above. 
		// Return value: 
		//   0 -> OK
		//   1 -> error
		int CheckType(const ANI::ExprValueType &evt,int ty_xxx);
};

}}  // end of namespace CC::S

#endif  /* _ANIVISION_CALCCORE_SCC_SCOPETREEPARSER_H_ */
