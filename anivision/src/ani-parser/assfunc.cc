/*
 * ani-parser/assfunc.cc
 * 
 * Assignment / implicit casting functions. 
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

#include "assfunc.h"
#include "exprvalue.h"

// Assignment functions. 
// That is: primarily "=" and based on that also "+=","-=",...

#warning "Implement mem improvement: Hash the ass functions and use ref counting."
// Maybe using new operator delete and destructor: 
//   operator delete(void *afb)
//   {  if(((AssignmentFuncBase*)afb)->refcnt<=0)  LFree(afb);  }
//   ~AFB()
//   {  --refcnt;  }
// PROBLEM: What does the compiler do with the virt function?

namespace ANI
{

#warning "Should verbose-dump these values at the end:"
int AssignmentFuncBase::ass_funcs_created=0;
int IncDecAssFuncBase::incdec_funcs_created=0;

void AssignmentFuncBase::Assign(ExprValue & /*lval*/,const ExprValue & /*rval*/)
{
	// Must be overridden if used. 
	assert(0);
}

AssignmentFuncBase::~AssignmentFuncBase()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------

// Direct assignment if the types match. 
struct AssFunc_Direct : AssignmentFuncBase
{
	AssFunc_Direct() : AssignmentFuncBase(1) { }
	// [overriding virtuals:]
	~AssFunc_Direct() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{
		#warning "Speed critical assert:"
		if(!!lval)
		{
			ExprValueType lt,rt;
			lval.GetExprValueType(&lt);
			rval.GetExprValueType(&rt);
			if(!!lt && !lt.IsEqualTo(rt))
			{
				fprintf(stderr,"OOPS: %s != %s\n",
					lt.TypeString().str(),rt.TypeString().str());
				assert(0);
			}
		}
		#warning "End of speed critical assert."
		lval.assign(rval);
	}
}__attribute__((__packed__));

// Assign NULL scope: 
struct AssFunc_Scope_Null : AssignmentFuncBase
{
	AssFunc_Scope_Null() : AssignmentFuncBase(0) { }
	// [overriding virtuals:]
	~AssFunc_Scope_Null() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{  lval.assign(ScopeInstance());  }
}__attribute__((__packed__));

// Assign NULL array: 
struct AssFunc_Array_Null : AssignmentFuncBase
{
	AssFunc_Array_Null() : AssignmentFuncBase(0) { }
	// [overriding virtuals:]
	~AssFunc_Array_Null() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{  lval.assign(ExprValueArray());  }
}__attribute__((__packed__));

//------------------------------------------------------------------------------
// Normal Naming convention: AssFunc_<LLvalue>_<RRvalue>()

struct AssFunc_Scalar_Integer : AssignmentFuncBase
{
	AssFunc_Scalar_Integer() : AssignmentFuncBase(1) { }
	// [overriding virtuals:]
	~AssFunc_Scalar_Integer() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{  lval.assign( (double)rval.GetPODInt() );  }
}__attribute__((__packed__));

struct AssFunc_Vector_Integer : AssignmentFuncBase
{
	int dest_dim;   // size of dest vector
	
	AssFunc_Vector_Integer(int _dest_dim) : AssignmentFuncBase(1)
		{  dest_dim=_dest_dim;  }
	// [overriding virtuals:]
	~AssFunc_Vector_Integer() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{  lval.assign(Vector(Vector::Promo, rval.GetPODInt(), dest_dim) );  }
}__attribute__((__packed__));

struct AssFunc_Vector_Scalar : AssignmentFuncBase
{
	int dest_dim;   // size of dest vector
	
	AssFunc_Vector_Scalar(int _dest_dim) : AssignmentFuncBase(1)
		{  dest_dim=_dest_dim;  }
	// [overriding virtuals:]
	~AssFunc_Vector_Scalar() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{  lval.assign( Vector(Vector::Promo, rval.GetPODDouble(), dest_dim) );  }
}__attribute__((__packed__));

struct AssFunc_Matrix_Integer : AssignmentFuncBase
{
	short int dest_r,dest_c;
	
	AssFunc_Matrix_Integer(short int _dest_r,short int _dest_c) : 
		AssignmentFuncBase(1)  {  dest_r=_dest_r;  dest_c=_dest_c;  }
	// [overriding virtuals:]
	~AssFunc_Matrix_Integer() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{
		double rv=(double)rval.GetPODInt();
		Matrix tmp(Matrix::NoInit,dest_r,dest_c);
		for(short int r=0; r<dest_r; r++)
			for(short int c=0; c<dest_c; c++)
				tmp[r][c]=rv;
		lval.assign(tmp);
	}
}__attribute__((__packed__));

struct AssFunc_Matrix_Scalar : AssignmentFuncBase
{
	short int dest_r,dest_c;
	
	AssFunc_Matrix_Scalar(short int _dest_r,short int _dest_c) : 
		AssignmentFuncBase(1)  {  dest_r=_dest_r;  dest_c=_dest_c;  }
	// [overriding virtuals:]
	~AssFunc_Matrix_Scalar() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{
		double rv=rval.GetPODDouble();
		Matrix tmp(Matrix::NoInit,dest_r,dest_c);
		for(short int r=0; r<dest_r; r++)
			for(short int c=0; c<dest_c; c++)
				tmp[r][c]=rv;
		lval.assign(tmp);
	}
}__attribute__((__packed__));

struct AssFunc_Matrix_Vector : AssignmentFuncBase
{
	short int dest_r,dest_c;
	
	AssFunc_Matrix_Vector(short int _dest_r,short int _dest_c) : 
		AssignmentFuncBase(1)  {  dest_r=_dest_r;  dest_c=_dest_c;  }
	// [overriding virtuals:]
	~AssFunc_Matrix_Vector() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{
		Value _rv;
		rval.GetPOD(&_rv);
		const Vector *rv=_rv.GetPtr<Vector>();
		Matrix tmp(Matrix::NoInit,dest_r,dest_c);
		if(dest_c==1 && dest_r==rv->dim())
		{
			for(short int r=0; r<dest_r; r++)
			{  tmp[r][0]=(*rv)[r];  }
		}
		else if(dest_r==1 && dest_c==rv->dim())
		{
			for(short int c=0; c<dest_c; c++)
			{  tmp[0][c]=(*rv)[c];  }
		}
		else assert(0);  // Failure -> bug somewehere else!
		lval.assign(tmp);
	}
}__attribute__((__packed__));

//------------------------------------------------------------------------------
// For operators such as "+=", "-=", "*=", "/=": 
#define TEMPLATE(OP,OPSTR) \
	struct AssFunc_Integer_Integer_ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_Integer_Integer_ ## OPSTR() : AssignmentFuncBase(1) { } \
		/* [overriding virtuals:] */ \
		~AssFunc_Integer_Integer_ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ lval.assign( lval.GetPODInt() OP rval.GetPODInt() ); } \
	}__attribute__((__packed__));
TEMPLATE(+,Add)
TEMPLATE(-,Subtract)
TEMPLATE(*,Multiply)
TEMPLATE(/,Divide)
#undef TEMPLATE

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR) \
	struct AssFunc_Scalar_Scalar_ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_Scalar_Scalar_ ## OPSTR() : AssignmentFuncBase(1) { } \
		/* [overriding virtuals:] */ \
		~AssFunc_Scalar_Scalar_ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ lval.assign( lval.GetPODDouble() OP rval.GetPODDouble() ); } \
	}__attribute__((__packed__));
TEMPLATE(+,Add)
TEMPLATE(-,Subtract)
TEMPLATE(*,Multiply)
TEMPLATE(/,Divide)
#undef TEMPLATE

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR) \
	struct AssFunc_Scalar_Integer_ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_Scalar_Integer_ ## OPSTR() : AssignmentFuncBase(1) { } \
		/* [overriding virtuals:] */ \
		~AssFunc_Scalar_Integer_ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ lval.assign( lval.GetPODDouble() OP double(rval.GetPODInt()) ); } \
	}__attribute__((__packed__));
TEMPLATE(+,Add)
TEMPLATE(-,Subtract)
TEMPLATE(*,Multiply)
TEMPLATE(/,Divide)
#undef TEMPLATE

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR,TYPE,GET) \
	struct AssFunc_Vector_ ## TYPE ## _ ## OPSTR : AssignmentFuncBase \
	{ \
		int dest_dim;   /* size of dest vector */  \
		\
		AssFunc_Vector_ ## TYPE ## _ ## OPSTR(int _dest_dim) : \
			AssignmentFuncBase(1)  {  dest_dim=_dest_dim;  } \
		/* [overriding virtuals:] */ \
		~AssFunc_Vector_ ## TYPE ## _ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ \
			Value _tmp; lval.GetPOD(&_tmp); \
			lval.assign( *_tmp.GetPtr<Vector>() OP \
				Vector(Vector::Promo, rval.GET(), dest_dim) ); \
		} \
	}__attribute__((__packed__));
TEMPLATE(+,Add,Integer,GetPODInt)
TEMPLATE(-,Subtract,Integer,GetPODInt)
TEMPLATE(+,Add,Scalar,GetPODDouble)
TEMPLATE(-,Subtract,Scalar,GetPODDouble)
#undef TEMPLATE

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR,TYPE,GET,LTYPE) \
	struct AssFunc_ ## LTYPE ## _ ## TYPE ## _ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_ ## LTYPE ## _ ## TYPE ## _ ## OPSTR() : AssignmentFuncBase(1)  { } \
		/* [overriding virtuals:] */ \
		~AssFunc_ ## LTYPE ## _ ## TYPE ## _ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ \
			Value _tmp; lval.GetPOD(&_tmp); \
			lval.assign( *_tmp.GetPtr<LTYPE>() OP rval.GET() ); \
		} \
	}__attribute__((__packed__));
TEMPLATE(*,Multiply,Integer,GetPODInt,Vector)
TEMPLATE(/,Divide,Integer,GetPODInt,Vector)
TEMPLATE(*,Multiply,Scalar,GetPODDouble,Vector)
TEMPLATE(/,Divide,Scalar,GetPODDouble,Vector)
TEMPLATE(*,Multiply,Integer,GetPODInt,Matrix)
TEMPLATE(/,Divide,Integer,GetPODInt,Matrix)
TEMPLATE(*,Multiply,Scalar,GetPODDouble,Matrix)
TEMPLATE(/,Divide,Scalar,GetPODDouble,Matrix)
#undef TEMPLATE

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR,TYPE) \
	struct AssFunc_ ## TYPE ## _ ## TYPE ## _ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_ ## TYPE ## _ ## TYPE ## _ ## OPSTR() : \
			AssignmentFuncBase(1) { } \
		/* [overriding virtuals:] */ \
		~AssFunc_ ## TYPE ## _ ## TYPE ## _ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ \
			Value _ltmp; lval.GetPOD(&_ltmp); \
			Value _rtmp; rval.GetPOD(&_rtmp); \
			lval.assign( *_ltmp.GetPtr<TYPE>() + *_rtmp.GetPtr<TYPE>() ); \
		} \
	}__attribute__((__packed__));
TEMPLATE(+,Add,Vector)
TEMPLATE(-,Subtract,Vector)
TEMPLATE(+,Add,Matrix)
TEMPLATE(-,Subtract,Matrix)
#undef TEMPLATE

//------------------------------------------------------------------------------
struct AssFunc_String_String_Add : AssignmentFuncBase
{
	AssFunc_String_String_Add() : AssignmentFuncBase(1) { }
	/* [overriding virtuals:] */
	~AssFunc_String_String_Add() {}
	void Assign(ExprValue &lval,const ExprValue &rval)
	{
		Value ltmp;  lval.GetPOD(&ltmp);
		Value rtmp;  rval.GetPOD(&rtmp);
		lval.assign( *ltmp.GetPtr<String>() + *rtmp.GetPtr<String>() ); }
}__attribute__((__packed__));

//------------------------------------------------------------------------------
#define TEMPLATE(OP,OPSTR,TYPE,GET) \
	struct AssFunc_Range ## _ ## TYPE ## _ ## OPSTR : AssignmentFuncBase \
	{ \
		AssFunc_Range ## _ ## TYPE ## _ ## OPSTR() : AssignmentFuncBase(1)  { } \
		/* [overriding virtuals:] */ \
		~AssFunc_Range ## _ ## TYPE ## _ ## OPSTR() {} \
		void Assign(ExprValue &lval,const ExprValue &rval) \
		{ \
			Value _tmp; lval.GetPOD(&_tmp); \
			lval.assign( *_tmp.GetPtr<Range>() OP rval.GET() ); \
		} \
	}__attribute__((__packed__));
TEMPLATE(+,Add,Integer,GetPODInt)
TEMPLATE(-,Subtract,Integer,GetPODInt)
TEMPLATE(*,Multiply,Integer,GetPODInt)
TEMPLATE(/,Divide,Integer,GetPODInt)
TEMPLATE(+,Add,Scalar,GetPODDouble)
TEMPLATE(-,Subtract,Scalar,GetPODDouble)
TEMPLATE(*,Multiply,Scalar,GetPODDouble)
TEMPLATE(/,Divide,Scalar,GetPODDouble)
#undef TEMPLATE

//==============================================================================

static int _LAF_GetAssFunc_POD_POD(
	const Value::CompleteType &lval,const Value::CompleteType &rval,
	AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op)
{
	//-- ONLY Non-Incomplete types. --
	if(_InternalExprValueType::_is_pod_incomplete(rval) || 
	   _InternalExprValueType::_is_pod_incomplete(lval) )
	{  return(-2);  }
	
	// NOTE: WIDENING, ONLY. 
	// (Could introduce new rv for "narrowing".) 
	// AND: Need complete widening chains without holes!
	
	switch(lval.type)
	{
		case Value::VTInteger:
			if(rval.type==Value::VTInteger)
			{
				if(ret_assfunc) switch(ass_op)
				{
					case AO_Set:
						*ret_assfunc=new AssFunc_Direct();  break;
					case AO_Add:
						*ret_assfunc=new AssFunc_Integer_Integer_Add();  break;
					case AO_Subtract:
						*ret_assfunc=new AssFunc_Integer_Integer_Subtract();  break;
					case AO_Multiply:
						*ret_assfunc=new AssFunc_Integer_Integer_Multiply();  break;
					case AO_Divide:
						*ret_assfunc=new AssFunc_Integer_Integer_Divide();  break;
					default: assert(0);
				}
				return(2);
			}
			//if(rval.type==Value::VTScalar)   // "narrowing"
			//{
			//	if(ret_assfunc)
			//	{  *ret_assfunc=new AssFunc_???();  }
			//	return(1);
			//}
			break;
		case Value::VTScalar:
			if(rval.type==Value::VTScalar)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Direct();  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Scalar_Add();  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Scalar_Subtract();  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Scalar_Multiply();  break;
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Scalar_Divide();  break;
					default: assert(0);
				}
				return(2);
			}
			else if(rval.type==Value::VTInteger)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Integer();  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Integer_Add();  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Integer_Subtract();  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Integer_Multiply();  break;
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Scalar_Integer_Divide();  break;
					default: assert(0);
				}
				return(1);
			}
			break;
		case Value::VTRange:
			if(rval.type==Value::VTRange && ass_op==AO_Set)
			{
				//if(rval.rvalid!=lval.rvalid)
				//{  assert(0);  /* What should we do in this case? */  }
				if(ret_assfunc)
				{  *ret_assfunc=new AssFunc_Direct();  }
				return(2);
			}
			else if(rval.type==Value::VTInteger)
			{
				switch(ass_op)
				{
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Integer_Add();  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Integer_Subtract();  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Integer_Multiply();  break;
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Integer_Divide();  break;
					default: return(0);
				}
				return(2);
			}
			else if(rval.type==Value::VTScalar)
			{
				switch(ass_op)
				{
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Scalar_Add();  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Scalar_Subtract();  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Scalar_Multiply();  break;
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Range_Scalar_Divide();  break;
					default: return(0);
				}
				return(2);
			}
			break;
		case Value::VTVector:
			if(lval.n>=1 && rval.type==Value::VTInteger)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Integer(lval.n);  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Integer_Add(lval.n);  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Integer_Subtract(lval.n);  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Integer_Multiply();
						return(2);
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Integer_Divide();
						return(2);
					default: return(0);  // Yes, RETURN!!
				}
				return(1);
			}
			else if(lval.n>=1 && rval.type==Value::VTScalar)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Scalar(lval.n);  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Scalar_Add(lval.n);  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Scalar_Subtract(lval.n);  break;
					case AO_Multiply: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Scalar_Multiply();
						return(2);
					case AO_Divide: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Scalar_Divide();
						return(2);
					default: return(0);  // Yes, RETURN!!
				}
				return(1);
			}
			else if(rval.type==Value::VTVector && rval.n==lval.n)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Direct();  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Vector_Add();  break;
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Vector_Vector_Subtract();  break;
					default: return(0);  // Yes, RETURN!!
				}
				return(2);
			}
			break;
		case Value::VTMatrix:
			if(rval.type==Value::VTMatrix && 
			   rval.r==lval.r && rval.c==lval.c)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Direct();
						return(2);
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Matrix_Matrix_Add();
						return(2);
					case AO_Subtract: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Matrix_Matrix_Subtract();
						return(2);
					//default: go on...
				}
			}
			if(ass_op==AO_Set)
			{
				if(rval.type==Value::VTVector && 
				   ((rval.n==lval.r && lval.c==1) || 
			    	(rval.n==lval.c && lval.r==1) ) )
				{
					if(ret_assfunc)
					{  *ret_assfunc=new AssFunc_Matrix_Vector(lval.r,lval.c);  }
					return(1);
				}
				else if(rval.type==Value::VTInteger && 
				   ((lval.r>=1 && lval.c==1) || 
			    	(lval.c>=1 && lval.r==1) ) )
				{
					if(ret_assfunc)
					{  *ret_assfunc=new AssFunc_Matrix_Integer(lval.r,lval.c);  }
					return(1);
				}
				else if(rval.type==Value::VTScalar && 
				   ((lval.r>=1 && lval.c==1) || 
			    	(lval.c>=1 && lval.r==1) ) )
				{
					if(ret_assfunc)
					{  *ret_assfunc=new AssFunc_Matrix_Scalar(lval.r,lval.c);  }
					return(1);
				}
			}
			else
			{
				if(rval.type==Value::VTInteger)
				{
					switch(ass_op)
					{
						case AO_Multiply: if(ret_assfunc)
							*ret_assfunc=new AssFunc_Matrix_Integer_Multiply();
							return(2);
						case AO_Divide: if(ret_assfunc)
							*ret_assfunc=new AssFunc_Matrix_Integer_Divide();
							return(2);
						//default: go on...
					}
				}
				else if(rval.type==Value::VTScalar)
				{
					switch(ass_op)
					{
						case AO_Multiply: if(ret_assfunc)
							*ret_assfunc=new AssFunc_Matrix_Scalar_Multiply();
							return(2);
						case AO_Divide: if(ret_assfunc)
							*ret_assfunc=new AssFunc_Matrix_Scalar_Divide();
							return(2);
						//default: go on...
					}
				}
			}
			break;
		case Value::VTString:
			if(rval.type==Value::VTString)
			{
				switch(ass_op)
				{
					case AO_Set: if(ret_assfunc)
						*ret_assfunc=new AssFunc_Direct();  break;
					case AO_Add: if(ret_assfunc)
						*ret_assfunc=new AssFunc_String_String_Add();  break;
					default:  return(0);
				}
				return(2);
			}
			break;
		default: assert(0);
	}
	
	return(0);
}


int _LAF_GetAssFunc(
	const _InternalExprValueType *lval,const _InternalExprValueType *rval,
	AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op)
{
	// Can assign *rval to *lval?
	switch(lval->type_id)
	{
		case EVT_Unknown:
			return(0);
		case EVT_Void:
			return(0);
		case EVT_POD:
			if(rval->type_id==EVT_POD)
			{  return(_LAF_GetAssFunc_POD_POD(
				lval->pod,rval->pod,ret_assfunc,ass_op));  }
			return(0);
		case EVT_AniScope:
			if(ass_op!=AO_Set)  return(0);
			if(rval->type_id!=EVT_AniScope)  return(0);
			switch(lval->scope->CG_ASType())
			{
				case AniGlue_ScopeBase::AST_Animation:  // fall 
				case AniGlue_ScopeBase::AST_Setting:    //    through
				case AniGlue_ScopeBase::AST_Object:
					// Can assign NULL: 
					if(rval->scope->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
					   rval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL )
					{
						if(ret_assfunc)
						{  *ret_assfunc=new AssFunc_Scope_Null();  }
						return(1);
					}
					// Could allow conversion to extract higher scope 
					// instances from lower ones. 
					else if(lval->scope==rval->scope)
					{
						if(ret_assfunc)
						{  *ret_assfunc=new AssFunc_Direct();  }
						return(2);
					}
					return(0);
				case AniGlue_ScopeBase::AST_AnonScope:
					return(0);
				case AniGlue_ScopeBase::AST_UserFunc:
				case AniGlue_ScopeBase::AST_InternalFunc:
					// No pointers to function...
					return(0);
				case AniGlue_ScopeBase::AST_UserVar:
				case AniGlue_ScopeBase::AST_InternalVar:
					// No pointers to variables. 
					// AND WHAT ABOUT ARRAYS?
					return(0);
				case AniGlue_ScopeBase::AST_Incomplete:
					if(lval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL && 
					   rval->scope->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
					   rval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL )
					{
						// This is needed to check for type matches. However, 
						// we cannot actually DO such an assignment: 
						if(ret_assfunc)
						{  *ret_assfunc=NULL;  /* really! */  }
						return(2);
					}
					if(lval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL || 
					   (rval->scope->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
					    rval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL) )
					{
						return(0);
					}
					return(-2);
				default: assert(0);
			}
			break;
		case EVT_Array:
			if(ass_op!=AO_Set)  return(0);
			if(rval->type_id==EVT_AniScope && 
			   rval->scope->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
			   rval->scope->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL )
			{
				if(ret_assfunc)
				{  *ret_assfunc=new AssFunc_Array_Null();  }
				return(1);
			}
			else
			{
				if(rval->type_id!=EVT_Array)  return(0);
				int rv=_LAF_GetAssFunc(lval->arrtype,rval->arrtype,NULL,ass_op);
				if(rv==2)  // No conversion necessary, good. 
				{
					if(ret_assfunc)
					{  *ret_assfunc=new AssFunc_Direct();  }
					return(2);
				}
				else if(rv==1)  // Conversion necessary. 
				{  return(0);  }  // no conversion at array assign (ref only!)
				else return(rv);
			}
			break;
		default: assert(0);
	}
	
	assert(0);  // Never reaced. 
	return(0);
}


// Primary function: 
// See assfunc.h. 
int LookupAssignmentFunction(
	const ExprValueType &lval,const ExprValueType &rval,
	AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op)
{
	if(ret_assfunc) *ret_assfunc=NULL;
	if(!lval.ivt || !rval.ivt)  return(0);
	return(_LAF_GetAssFunc(lval.ivt,rval.ivt,ret_assfunc,ass_op));
}


/******************************************************************************/


void IncDecAssFuncBase::IncDec(ExprValue &/*val*/)
{
	// Must be overridden. 
	assert(0);
}

IncDecAssFuncBase::~IncDecAssFuncBase()
{
	// Not inline because virtual. 
}

//------------------------------------------------------------------------------

#define TEMPLATE(OP,OPSTR,TYPE) \
	struct IncDecFunc_ ## OPSTR ## TYPE : IncDecAssFuncBase \
	{ \
		IncDecFunc_ ## OPSTR ## TYPE() {} \
		/* [overriding virtuals:] */ \
		~IncDecFunc_ ## OPSTR ## TYPE() {} \
		void IncDec(ExprValue &val)  \
		{  val.assign( val.GetPOD ## TYPE() OP );  } \
	};
TEMPLATE(+1,Inc,Int)
TEMPLATE(-1,Dec,Int)
TEMPLATE(+1.0,Inc,Double)
TEMPLATE(-1.0,Dec,Double)
#undef TEMPLATE

//==============================================================================

int _LAIDF_GetIncDecFunc(
	const _InternalExprValueType *ltype,
	IncDecAssFuncBase **ret_incdecfunc,int inc_or_dec)
{
	assert(inc_or_dec!=0);
	
	if(ltype->type_id==EVT_POD)
	{
		if(ltype->pod.type==Value::VTInteger)
		{
			if(ret_incdecfunc)
			{
				if(inc_or_dec>0)
				{  *ret_incdecfunc=new IncDecFunc_IncInt();  }
				else if(inc_or_dec<0)
				{  *ret_incdecfunc=new IncDecFunc_DecInt();  }
			}
			return(1);
		}
		if(ltype->pod.type==Value::VTScalar)
		{
			if(ret_incdecfunc)
			{
				if(inc_or_dec>0)
				{  *ret_incdecfunc=new IncDecFunc_IncDouble();  }
				else if(inc_or_dec<0)
				{  *ret_incdecfunc=new IncDecFunc_DecDouble();  }
			}
			return(1);
		}
	}
	return(0);
}


int LookupAssIncDecFunction(
	const ExprValueType &ltype,
	IncDecAssFuncBase **ret_incdecfunc,int inc_or_dec)
{
	// Note: inc_or_dec!=0 only needed if caller wants to get an 
	// assignment func handler, i.e. ret_incdecfunc!=NULL. 
	if(ret_incdecfunc)
	{  *ret_incdecfunc=NULL;  assert(inc_or_dec!=0);  }
	if(!ltype.ivt)  return(0);
	return(_LAIDF_GetIncDecFunc(ltype.ivt,ret_incdecfunc,inc_or_dec));
}

}
