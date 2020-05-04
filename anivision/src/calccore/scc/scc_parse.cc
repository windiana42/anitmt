/*
 * calccore/scc/scc_parse.cc
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

#include "scc_parse.h"

#include <ani-parser/exprtype.h>

#include <assert.h>


namespace CC { namespace S
{

static const char *_scope_name[SCC_ScopeTreeParser::_SC_LAST]={
	"empty",
	"move",
	"move_front_speed",
	"move_up_gravity",
	"curve",
	"fixed3",
	"lspline",
	"lspline3",
	"lspline_vt",
	"lspline_xt",
	"cspline",
	"cspline3",
	"cspline_vt",
	"cspline_xt",
	"function3",
};

const char *SCC_ScopeTreeParser::ScopeID2Str(ScopeID s)
{
	return((s<0 || s>=_SC_LAST) ? "???" : _scope_name[s]);
}


const char *SCC_ScopeTreeParser::TypeID2Str(TypeID t)
{
	switch(t)
	{
		case TY_String:  return("string");
		case TY_Scalar:  return("scalar");
		case TY_ScalarArray:  return("scalar[]");
		case TY_Range:  return("range");
		case TY_Vector:  return("vector");
		case TY_VectorArray:  return("vector[]");
		case TY_Vector3:  return("vector<3>");
		case TY_Vector2Array:  return("vector<2>[]");
		case TY_Vector3Array:  return("vector<3>[]");
		case TY_Vector4Array:  return("vector<4>[]");
		case TY_StringOrScalarArray:  return("string/scalar[]");
	}
	return("???");
}


int SCC_ScopeTreeParser::CheckType(const ANI::ExprValueType &evt,int ty_xxx)
{
	const Value::CompleteType *pod=evt.PODType();
	ANI::ExprValueType elem_type(evt.ArrayType());
	const Value::CompleteType *elem_pod=elem_type.PODType();
	
	switch((SCC_ScopeTreeParser::TypeID)ty_xxx)
	{
		case TY_String:
			return(!(pod && pod->type==Value::VTString));
		case TY_Scalar:
			return(!(pod && 
				(pod->type==Value::VTScalar || pod->type==Value::VTInteger)));
		case TY_ScalarArray:
			return(!(elem_pod && (elem_pod->type==Value::VTScalar || 
				elem_pod->type==Value::VTInteger)));
		case TY_Range:
			return(!(pod && pod->type==Value::VTRange));
		case TY_Vector:
			return(!(pod && pod->type==Value::VTVector));
		case TY_VectorArray:
			return(!(elem_pod && elem_pod->type==Value::VTVector));
		case TY_Vector3:
			return(!(pod && pod->type==Value::VTVector && pod->n==3));
		case TY_Vector2Array:
			return(!(elem_pod && elem_pod->type==Value::VTVector && 
				elem_pod->n==2));
		case TY_Vector3Array:
			return(!(elem_pod && elem_pod->type==Value::VTVector && 
				elem_pod->n==3));
		case TY_Vector4Array:
			return(!(elem_pod && elem_pod->type==Value::VTVector && 
				elem_pod->n==4));
		case TY_StringOrScalarArray:
			return(!( (pod && pod->type==Value::VTString) || 
				(elem_pod && elem_pod->type==Value::VTScalar) ));
	}
	assert(0);
	return(-1);
}


SCC_ScopeTreeParser::SCC_ScopeTreeParser() : 
	ScopeTreeParser()
{
	fprintf(stdout,"SCC: Initializing grammar...");  fflush(stdout);
	
	// Empty scope:
	EntryList sc_empty;
	DefineScope(SC_empty,&( sc_empty )); // content will be moved 

	EntryList sc_move(
			  Entry::Scope(TID_pos)+SCEnt(TID_curve,SC_curve));
	sc_move,
	  Entry::Scope(TID_front)+SCEnt(TID_curve,SC_curve)+SCEnt(TID_speed,SC_move_front_speed),
	  Entry::Expr(TID_front_mode,TY_String),
	  Entry::Scope(TID_up)+SCEnt(TID_curve,SC_curve)+SCEnt(TID_gravity,SC_move_up_gravity),
	  Entry::Expr(TID_up_mode,TY_String);
	
	DefineScope(SC_move,&( sc_move )); // content will be moved
	
	EntryList sc_move_front_speed;
	DefineScope(SC_move_front_speed,&( sc_move_front_speed )); // content will be moved

	EntryList sc_move_up_gravity(Entry::Expr(TID_force_up,TY_Vector3));
	sc_move_up_gravity,
	  Entry::Expr(TID_force_center,TY_Vector3),
	  Entry::Expr(TID_accel_scale,TY_Scalar);
	DefineScope(SC_move_up_gravity,&( sc_move_up_gravity )); // content will be moved

        EntryList sc_curve(Entry::Scope(TID_pos)
			   +SCEnt(TID_fixed,SC_fixed3)
			   +SCEnt(TID_lspline,SC_lspline3)
			   +SCEnt(TID_cspline,SC_cspline3)
			   +SCEnt(TID_function,SC_function3));
        sc_curve,
	  Entry::Expr(TID_dt,TY_Scalar),
	  Entry::Expr(TID_t,TY_Range),
	  Entry::Scope(TID_length)
	  +SCEnt(TID_none,SC_empty)
	  +SCEnt(TID_lspline,SC_lspline_xt)
	  +SCEnt(TID_cspline,SC_cspline_xt),
	  Entry::Scope(TID_speed)
	  +SCEnt(TID_none,SC_empty)
	  +SCEnt(TID_constant,SC_empty)
	  +SCEnt(TID_lspline,SC_lspline_vt)
	  +SCEnt(TID_cspline,SC_cspline_vt);
  
	DefineScope(SC_curve,&( sc_curve )); // content will be moved

	EntryList sc_fixed3( Entry::Expr(TID_pos,TY_Vector3) );
		   
	DefineScope(SC_fixed3,&( sc_fixed3 ) ); // content will be moved

	EntryList sc_lspline(Entry::Expr(TID_pts,TY_VectorArray));

	sc_lspline,
	  Entry::Expr(TID_pts_t,TY_VectorArray),
	  Entry::Expr(TID_addpts,TY_String),
	  Entry::Expr(TID_tvals,TY_StringOrScalarArray);
	
	DefineScope(SC_lspline,&( sc_lspline )); // content will be moved

	EntryList sc_lspline3(Entry::Expr(TID_pts,TY_Vector3Array));

	sc_lspline3,
	  Entry::Expr(TID_pts_t,TY_Vector4Array),
	  Entry::Expr(TID_addpts,TY_String),
	  Entry::Expr(TID_tvals,TY_StringOrScalarArray);
	
	DefineScope(SC_lspline3,&( sc_lspline3 )); // content will be moved

	EntryList sc_lspline_vt( Entry::Expr(TID_pts_vt,TY_Vector2Array) );
		   
	DefineScope(SC_lspline_vt,&( sc_lspline_vt ) ); // content will be moved

	EntryList sc_lspline_xt( Entry::Expr(TID_pts_xt,TY_Vector2Array) );
		   
	DefineScope(SC_lspline_xt,&( sc_lspline_xt ) ); // content will be moved

	EntryList sc_cspline(Entry::Expr(TID_pts,TY_VectorArray));

	sc_cspline,
	  Entry::Expr(TID_pts_t,TY_VectorArray),
	  Entry::Expr(TID_addpts,TY_String),
	  Entry::Expr(TID_tvals,TY_StringOrScalarArray);
	
	DefineScope(SC_cspline,&( sc_cspline )); // content will be moved

	EntryList sc_cspline3(Entry::Expr(TID_pts,TY_Vector3Array));

	sc_cspline3,
	  Entry::Expr(TID_pts_t,TY_Vector4Array),
	  Entry::Expr(TID_addpts,TY_String),
	  Entry::Expr(TID_tvals,TY_StringOrScalarArray);
	
	DefineScope(SC_cspline3,&( sc_cspline3 )); // content will be moved

	EntryList sc_cspline_vt( Entry::Expr(TID_pts_vt,TY_Vector2Array) );
	
	DefineScope(SC_cspline_vt,&( sc_cspline_vt ) ); // content will be moved

	EntryList sc_scpline_xt( Entry::Expr(TID_pts_xt,TY_Vector2Array) );
	
	DefineScope(SC_cspline_xt,&( sc_scpline_xt ) ); // content will be moved

	EntryList sc_function3(Entry::Expr(TID_pos,TY_Vector3));

	sc_function3,
	  Entry::Expr(TID_t,TY_Range);
	
	DefineScope(SC_function3,&( sc_function3 )); // content will be moved
	
	int rv=InitGrammar();
	if(rv)
	{  fprintf(stdout,"OOPS: %d errors\n",rv);  }
	else
	{  fprintf(stdout,"OK\n");  }
	assert(rv==0);
}

SCC_ScopeTreeParser::~SCC_ScopeTreeParser()
{
}

}}  // end of namespace CC::S
