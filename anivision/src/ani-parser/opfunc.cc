/*
 * ani-parser/opfunc.cc
 * 
 * Primitive operator functions (plus, minus,...). 
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

#include "opfunc.h"

// Standard operator functions but not thosewhich need special treatment. 
// NOT included: 
//  "?:" "&&" "||" because if we may eval one operand depends on 
//                 the result of some other operand 
//  "="            special; needs to use real C++ refenrene for the 
//                 lvalue (ExprValue::iev modification...)
//  "*=", "-=",... dealt with together with "=" in assfunc.cc. 

namespace ANI
{

struct _OpFuncID_Desc
{
	// Just for bug checks. 
	// There of ONE OpFuncID_Desc for every OperatorFuncID. 
	OperatorFuncID ofid;
	
	// Matching function: 
	// Query if this OperatorFuncDesc's (*opfunc)() can be used to 
	// compute the operation with nargs arguments of type argtypes[nargs]. 
	// Return value: 
	//  NULL -> no
	// !NULL -> pointer to function which can; the resulting value 
	//          type is stored in result_type. 
	OpFunc_Ptr (*matchfunc)(int nargs,const ExprValueType *argtypes,
		ExprValueType *result_type);
};

#warning "Speed improvement: Use GetPODInt() and GetPODDouble() when possible."

//******************************************************************************
//******************************************************************************

// This is a bit hacky... but more efficient than the 
// old version. 
static Value *podtmp=NULL;

inline const Value &OpFunc_Args::pod3(int i)  // i=0,1,2. 
{
	Value *v=&podtmp[i];
	arg(i)->GetPOD(v);
	return(*v);
}
inline const Value &OpFunc_Args::podU(int i,int use)  // use=0,1,2
{
	Value *v=&podtmp[use];
	arg(i)->GetPOD(v);
	return(*v);
}

void OpFunc_InitStaticVals()
{
	assert(!podtmp);
	podtmp=new Value[3];
}

void OpFunc_CleanupStaticVals()
{
	if(podtmp)
	{  delete[] podtmp;  podtmp=NULL;  }
}


//------------------------------------------------------------------------------
//  OF_Subscript,   // A[B]
//  OF_MembSel,     // A.B  (member select)  B=TNIdentifier
//  OF_FCall,       // A(B,C,D,...)          B,C,D,...=TNFCallArgument
// --> handeled externally. 

//------------------------------------------------------------------------------
//  OF_PODCast,     // A(B)                  A=TNTypeSpecifier
// NOTE: OpFunc_PODCast_FROM-TYPE_TO-TYPE
//       arg[0]: argument value 
//       arg[1]: dest type (value unused)
static int OpFunc_Cast_Integer_Scalar(OpFunc_Args *fa)
{
	fa->res->assign( double( fa->pod3(0).GetPtr<Integer>()->val() ) );
	return(0);
}
static int OpFunc_Cast_Scalar_Integer(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->pod3(0).GetPtr<Scalar>()->val() ) );
	return(0);
}
static int OpFunc_Cast_Integer_Vector(OpFunc_Args *fa)
{
	fa->res->assign( Vector(Vector::Promo,
		double(fa->pod3(0).GetPtr<Integer>()->val()),
		fa->pod3(1).GetPtr<Vector>()->dim() ) );
	return(0);
}
static int OpFunc_Cast_Scalar_Vector(OpFunc_Args *fa)
{
	fa->res->assign( Vector(Vector::Promo,
		fa->pod3(0).GetPtr<Scalar>()->val(),
		fa->pod3(1).GetPtr<Vector>()->dim() ) );
	return(0);
}
static int OpFunc_Cast_Integer_Matrix(OpFunc_Args *fa)
{
	const Matrix *desttype=fa->pod3(1).GetPtr<Matrix>();
	Matrix tmp(Matrix::NoInit,desttype->rows(),desttype->cols());
	double val = double( fa->pod3(0).GetPtr<Integer>()->val() );
	for(short int r=0; r<tmp.rows(); r++)
		for(short int c=0; c<tmp.cols(); c++)
			tmp[r][c]=val;
	fa->res->assign( tmp );
	return(0);
}
static int OpFunc_Cast_Scalar_Matrix(OpFunc_Args *fa)
{
	const Matrix *desttype=fa->pod3(1).GetPtr<Matrix>();
	Matrix tmp(Matrix::NoInit,desttype->rows(),desttype->cols());
	double val = fa->pod3(0).GetPtr<Scalar>()->val();
	for(short int r=0; r<tmp.rows(); r++)
		for(short int c=0; c<tmp.cols(); c++)
			tmp[r][c]=val;
	fa->res->assign( tmp );
	return(0);
}
static int OpFunc_Cast_Vector_Matrix(OpFunc_Args *fa)
{
	const Vector *srctype=fa->pod3(0).GetPtr<Vector>();
	const Matrix *desttype=fa->pod3(1).GetPtr<Matrix>();
	Matrix tmp(Matrix::NoInit,desttype->rows(),desttype->cols());
	if(desttype->cols()==1 && desttype->rows()==srctype->dim())
	{
		for(short int r=0; r<tmp.rows(); r++)
		{  tmp[r][0]=(*srctype)[r];  }
	}
	else if(desttype->rows()==1 && desttype->cols()==srctype->dim())
	{
		for(short int c=0; c<tmp.cols(); c++)
		{  tmp[0][c]=(*srctype)[c];  }
	}
	else assert(0);  // Failure -> bug in MatcFunc. 
	fa->res->assign( tmp );
	return(0);
}
static int OpFunc_Cast_Matrix_Vector(OpFunc_Args *fa)
{
	const Matrix *srctype=fa->pod3(0).GetPtr<Matrix>();
	const Vector *desttype=fa->pod3(1).GetPtr<Vector>();
	Vector tmp(Vector::NoInit,desttype->dim());
	if(srctype->cols()==1 && srctype->rows()==desttype->dim())
	{
		for(int r=0; r<tmp.dim(); r++)
		{  tmp[r]=(*srctype)[r][0];  }
	}
	else if(srctype->rows()==1 && srctype->cols()==desttype->dim())
	{
		for(int c=0; c<tmp.dim(); c++)
		{  tmp[c]=(*srctype)[0][c];  }
	}
	else assert(0);  // Failure -> bug in MatcFunc. 
	fa->res->assign( tmp );
	return(0);
}

// NOP: Simply copy. 
static int OpFunc_Cast_NOP(OpFunc_Args *fa)
{
	fa->res->assign( fa->pod3(0) );
	return(0);
}

#warning "Could speed it up by modifying one of the args instead of allocating new ones. In tree eval, this could be acceptable."

//------------------------------------------------------------------------------
//  OF_Mapping,     // A->B                  B=TNIdentifier

//------------------------------------------------------------------------------
//  OF_PostIncrement,  // A++
//  OF_PreIncrement,   // ++A
//  OF_PostDecrement,  // A--
//  OF_PreDecrement,   // --A
//  -> assfunc.cc

// Argh... Templates won't do the trick here -- at least with GCC. 

//------------------------------------------------------------------------------
//  OF_UnPlus,      // +A
#warning "Could be optimized away."
#define TEMPLATE(T,TMP) \
	static int OpFunc_UnPlus_ ## T(OpFunc_Args *fa) \
	{ \
		fa->res->assign( *(fa->pod3(0).GetPtr<T>()) ); \
		return(0); \
	}
TEMPLATE(Integer,int)      // -> Integer
TEMPLATE(Scalar,double)    // -> Scalar
TEMPLATE(Range,Range)      // -> Range
TEMPLATE(Vector,Vector)    // -> Vector
TEMPLATE(Matrix,Matrix)    // -> Matrix
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_UnMinus,     // -A
#define TEMPLATE(T,TMP) \
	static int OpFunc_UnMinus_ ## T(OpFunc_Args *fa) \
	{ \
		fa->res->assign( - *(fa->pod3(0).GetPtr<T>()) ); \
		return(0); \
	}
TEMPLATE(Integer,int)      // -> Integer
TEMPLATE(Scalar,double)    // -> Scalar
TEMPLATE(Range,Range)      // -> Range
TEMPLATE(Vector,Vector)    // -> Vector
TEMPLATE(Matrix,Matrix)    // -> Matrix
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_UnNot,       // !A
// Works for all POD types. 
static int OpFunc_UnNot(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->arg(0)->is_null() ) );
	return(0);
}

//------------------------------------------------------------------------------
//  OF_New,         // new A(B,C,D,...)   A=TNIdentifier, B,C,D,...=TNFCallArgument

//------------------------------------------------------------------------------
//  OF_Delete,      // delete A           A=TNExpression

//------------------------------------------------------------------------------
//  OF_Pow,         // A^B
static int OpFunc_Pow_Integer_Integer(OpFunc_Args *fa)  // -> Integer
{
	int y=*(fa->pod3(0).GetPtr<Integer>());
	int x=*(fa->pod3(1).GetPtr<Integer>());
	int r = x>=0 ? 1 : 0;
	for(;x>0;x--)
	{  r*=y;  }
	fa->res->assign( r );
	return(0);
}
#define TEMPLATE(TA,TB) \
	static int OpFunc_Pow_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( pow( *(fa->pod3(0).GetPtr<TA>()), \
			*(fa->pod3(1).GetPtr<TB>()) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Scalar)   // -> Scalar
TEMPLATE(Scalar,Integer)   // -> Scalar
TEMPLATE(Scalar,Scalar)    // -> Scalar
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Mult,        // A*B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Mult_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( TZ(  \
			*(fa->pod3(0).GetPtr<TA>()) * *(fa->pod3(1).GetPtr<TB>()) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)       // -> Integer
TEMPLATE(Integer,Scalar, double)    // -> Scalar
TEMPLATE(Scalar, Integer,double)    // -> Scalar
TEMPLATE(Scalar, Scalar, double)    // -> Scalar
TEMPLATE(Range,  Integer,)          // -> Range /*(only for RValidAB)*/ <- no
TEMPLATE(Range,  Scalar, )          // -> Range /*(only for RValidAB)*/   lon
TEMPLATE(Integer,Range,  )          // -> Range /*(only for RValidAB)*/   ger
TEMPLATE(Scalar, Range,  )          // -> Range /*(only for RValidAB)*/   now
TEMPLATE(Scalar, Vector, )          // -> Vector
TEMPLATE(Vector, Scalar, )          // -> Vector
TEMPLATE(Integer,Vector, )          // -> Vector
TEMPLATE(Vector, Integer,)          // -> Vector
TEMPLATE(Scalar, Matrix, )          // -> Matrix
TEMPLATE(Matrix, Scalar, )          // -> Matrix
TEMPLATE(Integer,Matrix, )          // -> Matrix
TEMPLATE(Matrix, Integer,)          // -> Matrix
TEMPLATE(Matrix, Vector, )          // -> Vector
TEMPLATE(Matrix, Matrix, )          // -> Matrix
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Div,         // A/B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Div_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( TZ(  \
			*(fa->pod3(0).GetPtr<TA>()) / *(fa->pod3(1).GetPtr<TB>()) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)       // -> Integer
TEMPLATE(Integer,Scalar, double)    // -> Scalar
TEMPLATE(Scalar, Integer,double)    // -> Scalar
TEMPLATE(Scalar, Scalar, double)    // -> Scalar
TEMPLATE(Range,  Integer,)          // -> Range (only for RValidAB)  <- no
TEMPLATE(Range,  Scalar, )          // -> Range (only for RValidAB)  <- longer
TEMPLATE(Vector, Scalar, )          // -> Vector
TEMPLATE(Vector, Integer,)          // -> Vector
TEMPLATE(Matrix, Scalar, )          // -> Matrix
TEMPLATE(Matrix, Integer,)          // -> Matrix
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Modulo,      // A%B
inline int _compute_modulo(int x,int y)  {  return(x%y);  }
inline double _compute_modulo(double x,double y)  {  return(fmod(x,y));  }
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Modulo_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( _compute_modulo( \
			TZ( *(fa->pod3(0).GetPtr<TA>()) ), \
			TZ( *(fa->pod3(1).GetPtr<TB>()) ) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)       // -> Integer
TEMPLATE(Integer,Scalar, double)    // -> Scalar
TEMPLATE(Scalar, Integer,double)    // -> Scalar
TEMPLATE(Scalar, Scalar, double)    // -> Scalar
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Add,         // A+B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Add_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( TZ(  \
			*(fa->pod3(0).GetPtr<TA>()) + *(fa->pod3(1).GetPtr<TB>()) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)       // -> Integer
TEMPLATE(Integer,Scalar, double)    // -> Scalar
TEMPLATE(Scalar, Integer,double)    // -> Scalar
TEMPLATE(Scalar, Scalar, double)    // -> Scalar
TEMPLATE(Range,  Integer,)          // -> Range (only for RValidAB)  <-  no
TEMPLATE(Range,  Scalar, )          // -> Range (only for RValidAB)     lon
TEMPLATE(Integer,Range,  )          // -> Range (only for RValidAB)     ger
TEMPLATE(Scalar, Range,  )          // -> Range (only for RValidAB)  <- now
TEMPLATE(Vector, Vector, )          // -> Vector
TEMPLATE(Matrix, Matrix, )          // -> Matrix
TEMPLATE(String, String, )          // -> String
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Subtract,    // A-B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Subtract_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( TZ(  \
			*(fa->pod3(0).GetPtr<TA>()) - *(fa->pod3(1).GetPtr<TB>()) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)       // -> Integer
TEMPLATE(Integer,Scalar, double)    // -> Scalar
TEMPLATE(Scalar, Integer,double)    // -> Scalar
TEMPLATE(Scalar, Scalar, double)    // -> Scalar
TEMPLATE(Range,  Integer,)          // -> Range (only for RValidAB)  <-  no
TEMPLATE(Range,  Scalar, )          // -> Range (only for RValidAB)     lon
TEMPLATE(Integer,Range,  )          // -> Range (only for RValidAB)     ger
TEMPLATE(Scalar, Range,  )          // -> Range (only for RValidAB)  <- now
TEMPLATE(Vector, Vector, )          // -> Vector
TEMPLATE(Matrix, Matrix, )          // -> Matrix
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Lesser,      // A<B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Lesser_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( ( \
			TZ(*(fa->pod3(0).GetPtr<TA>())) < \
			TZ(*(fa->pod3(1).GetPtr<TB>())) ) ? 1 : 0 ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)      // -> Integer
TEMPLATE(Integer,Scalar, double)   // -> Integer
TEMPLATE(Scalar, Integer,double)   // -> Integer
TEMPLATE(Scalar, Scalar, double)   // -> Integer
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Greater,     // A>B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_Greater_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( ( \
			TZ(*(fa->pod3(0).GetPtr<TA>())) > \
			TZ(*(fa->pod3(1).GetPtr<TB>())) ) ? 1 : 0 ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)      // -> Integer
TEMPLATE(Integer,Scalar, double)   // -> Integer
TEMPLATE(Scalar, Integer,double)   // -> Integer
TEMPLATE(Scalar, Scalar, double)   // -> Integer
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_LesserEq,    // A<=B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_LesserEq_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( ( \
			TZ(*(fa->pod3(0).GetPtr<TA>())) <= \
			TZ(*(fa->pod3(1).GetPtr<TB>())) ) ? 1 : 0 ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)      // -> Integer
TEMPLATE(Integer,Scalar, double)   // -> Integer
TEMPLATE(Scalar, Integer,double)   // -> Integer
TEMPLATE(Scalar, Scalar, double)   // -> Integer
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_GreaterEq,   // A>=B
#define TEMPLATE(TA,TB,TZ) \
	static int OpFunc_GreaterEq_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( ( \
			TZ(*(fa->pod3(0).GetPtr<TA>())) >= \
			TZ(*(fa->pod3(1).GetPtr<TB>())) ) ? 1 : 0 ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer,int)      // -> Integer
TEMPLATE(Integer,Scalar, double)   // -> Integer
TEMPLATE(Scalar, Integer,double)   // -> Integer
TEMPLATE(Scalar, Scalar, double)   // -> Integer
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Equal,       // A==B
#define TEMPLATE(TA,TB) \
	static int OpFunc_Equal_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( \
			(fa->pod3(0).GetPtr<TA>())->operator==( \
			*(fa->pod3(1).GetPtr<TB>()) ) )  ); \
		return(0); \
	}
TEMPLATE(Integer,Integer)   // -> Integer
TEMPLATE(Integer,Scalar)    // -> Integer
TEMPLATE(Scalar, Integer)   // -> Integer
TEMPLATE(Scalar, Scalar)    // -> Integer
TEMPLATE(Range,  Range)     // -> Integer
TEMPLATE(Vector, Vector)    // -> Integer
TEMPLATE(Matrix, Matrix)    // -> Integer
TEMPLATE(String, String)    // -> Integer
#undef TEMPLATE

// Always true: 
static int OpFunc_Equal_Always(OpFunc_Args *fa)
{
	fa->res->assign( int(1) );
	return(0);
}
// Compare with NULL: 
static int OpFunc_Equal_Null_ASB(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->arg(1)->is_null() ) );
	return(0);
}
static int OpFunc_Equal_ASB_Null(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->arg(0)->is_null() ) );
	return(0);
}

//------------------------------------------------------------------------------
//  OF_NotEqual,    // A!=B
#define TEMPLATE(TA,TB) \
	static int OpFunc_NotEqual_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( int( \
			(fa->pod3(0).GetPtr<TA>())->operator!=( \
			*(fa->pod3(1).GetPtr<TB>()) ) )  ); \
		return(0); \
	}
TEMPLATE(Integer,Integer)   // -> Integer
TEMPLATE(Integer,Scalar)    // -> Integer
TEMPLATE(Scalar, Integer)   // -> Integer
TEMPLATE(Scalar, Scalar)    // -> Integer
TEMPLATE(Range,  Range)     // -> Integer
TEMPLATE(Vector, Vector)    // -> Integer
TEMPLATE(Matrix, Matrix)    // -> Integer
TEMPLATE(String, String)    // -> Integer
#undef TEMPLATE

// Always false: 
static int OpFunc_NotEqual_Always(OpFunc_Args *fa)
{
	fa->res->assign( int(0) );
	return(0);
}
// Compare with NULL: 
static int OpFunc_NotEqual_Null_ASB(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->arg(1)->is_null() ? 0 : 1 ) );
	return(0);
}
static int OpFunc_NotEqual_ASB_Null(OpFunc_Args *fa)
{
	fa->res->assign( int( fa->arg(0)->is_null() ? 0 : 1 ) );
	return(0);
}

//------------------------------------------------------------------------------
//  OF_LogicalAnd,  // A&&B
//  OF_LogicalOr,   // A||B
// -> Dealt with separately because B may not be evaluated if result is 
//    clear from A. 

//------------------------------------------------------------------------------
//  OF_IfElse,      // A?B:C
//  OF_IfElse2,     // A?:B
// -> Dealt with separately because C may not be evaluated if A is false, etc. 

//------------------------------------------------------------------------------
//  OF_Range,       // A..B
#define TEMPLATE(TA,TB) \
	static int OpFunc_Range_ ## TA ## _ ## TB(OpFunc_Args *fa) \
	{ \
		fa->res->assign( Range( \
			double( *(fa->pod3(0).GetPtr<TA>()) ), \
			double( *(fa->pod3(1).GetPtr<TB>()) ) ) ); \
		return(0); \
	}
TEMPLATE(Integer,Integer)   // -> Range (RValidAB)
TEMPLATE(Integer,Scalar)    // -> Range (RValidAB)
TEMPLATE(Scalar, Integer)   // -> Range (RValidAB)
TEMPLATE(Scalar, Scalar)    // -> Range (RValidAB)
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_RangeNoA,    // ..A
#define TEMPLATE(TA) \
	static int OpFunc_RangeNoA_ ## TA(OpFunc_Args *fa) \
	{ \
		fa->res->assign( Range( Range::RInvalid, \
			double( *(fa->pod3(0).GetPtr<TA>()) ) ) ); \
		return(0); \
	}
TEMPLATE(Integer)   // -> Range (RValidB)
TEMPLATE(Scalar)    // -> Range (RValidB)
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_RangeNoB,    // A..
#define TEMPLATE(TA) \
	static int OpFunc_RangeNoB_ ## TA(OpFunc_Args *fa) \
	{ \
		fa->res->assign( Range( \
			double( *(fa->pod3(0).GetPtr<TA>()) ), Range::RInvalid ) ); \
		return(0); \
	}
TEMPLATE(Integer)   // -> Range (RValidA)
TEMPLATE(Scalar)    // -> Range (RValidA)
#undef TEMPLATE

//------------------------------------------------------------------------------
//  OF_Vector,      // <A,B,C,...>
// All arguments must be Integer or Scalar. 
static int OpFunc_Vector(OpFunc_Args *fa)
{
	double tmp[fa->nargs];
	for(int i=0; i<fa->nargs; i++)
	{
		const Value &a=fa->podU(i,0);
		switch(a.Type())
		{
			case Value::VTInteger:  tmp[i]=a.GetIntegerPtr()->val();  break;
			case Value::VTScalar:   tmp[i]=a.GetScalarPtr()->val();  break;
			default: assert(0);
		}
	}
	fa->res->assign( Vector(tmp,fa->nargs) );
	return(0);
}

//******************************************************************************
//******************************************************************************

static OpFunc_Ptr MatchFunc_PODCast(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	// arg0: source value; arg1: dest type. 
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	switch(ptype1->type)
	{
		case Value::VTInteger:
			if(ptype0->type==Value::VTInteger)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			else if(ptype0->type==Value::VTScalar)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Scalar_Integer);
			}
			break;
		case Value::VTScalar:
			if(ptype0->type==Value::VTScalar)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			else if(ptype0->type==Value::VTInteger)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Integer_Scalar);
			}
			break;
		case Value::VTRange:
			if(ptype0->type==Value::VTRange)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			break;
		case Value::VTVector:
			if(ptype0->type==Value::VTVector && 
			   ptype1->n==ptype0->n )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			else if(ptype0->type==Value::VTInteger)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Integer_Vector);
			}
			else if(ptype0->type==Value::VTScalar)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Scalar_Vector);
			}
			else if(ptype0->type==Value::VTMatrix && 
				((ptype0->r==1 && ptype0->c==ptype1->n) || 
				 (ptype0->c==1 && ptype0->r==ptype1->n)))
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Matrix_Vector);
			}
			break;
		case Value::VTMatrix:
			if(ptype0->type==Value::VTMatrix && 
			   ptype1->r==ptype0->r && ptype1->c==ptype0->c )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			else if(ptype0->type==Value::VTInteger && 
				((ptype1->r==1 && ptype1->c>=1) || 
				 (ptype1->c==1 && ptype1->r>=1)) )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Integer_Matrix);
			}
			else if(ptype0->type==Value::VTScalar && 
				((ptype1->r==1 && ptype1->c>=1) || 
				 (ptype1->c==1 && ptype1->r>=1)) )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Scalar_Matrix);
			}
			else if(ptype0->type==Value::VTVector && 
				((ptype1->r==1 && ptype1->c==ptype0->n) || 
				 (ptype1->c==1 && ptype1->r==ptype0->n)) )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_Vector_Matrix);
			}
			break;
		case Value::VTString:
			if(ptype0->type==Value::VTString)
			{
				*result_type=argtypes[1];
				return(&OpFunc_Cast_NOP);
			}
			break;
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_UnPlus(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=1)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	if(!ptype0) return(NULL);
	switch(ptype0->type)
	{
		case Value::VTInteger:
			*result_type=argtypes[0];
			return(&OpFunc_UnPlus_Integer);
		case Value::VTScalar:
			*result_type=argtypes[0];
			return(&OpFunc_UnPlus_Scalar);
		case Value::VTRange:
			*result_type=argtypes[0];
			return(&OpFunc_UnPlus_Range);
		case Value::VTVector:
			*result_type=argtypes[0];
			return(&OpFunc_UnPlus_Vector);
		case Value::VTMatrix:
			*result_type=argtypes[0];
			return(&OpFunc_UnPlus_Matrix);
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_UnMinus(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=1)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	if(!ptype0) return(NULL);
	switch(ptype0->type)
	{
		case Value::VTInteger:
			*result_type=argtypes[0];
			return(&OpFunc_UnMinus_Integer);
		case Value::VTScalar:
			*result_type=argtypes[0];
			return(&OpFunc_UnMinus_Scalar);
		case Value::VTRange:
			*result_type=argtypes[0];
			return(&OpFunc_UnMinus_Range);
		case Value::VTVector:
			*result_type=argtypes[0];
			return(&OpFunc_UnMinus_Vector);
		case Value::VTMatrix:
			*result_type=argtypes[0];
			return(&OpFunc_UnMinus_Matrix);
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_UnNot(int nargs,
	const ExprValueType * /*argtypes*/,ExprValueType *result_type)
{
	if(nargs!=1)  return(NULL);
	Value::CompleteType rtype;
	rtype.type=Value::VTInteger;
	result_type->SetPOD(rtype);
	return(&OpFunc_UnNot);
}

static OpFunc_Ptr MatchFunc_Pow(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Pow_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Pow_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Pow_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Pow_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Mult(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	switch(ptype0->type)
	{
		case Value::VTInteger:
		{
			Value::CompleteType rtype;
			switch(ptype1->type)
			{
				case Value::VTInteger:
					rtype.type=Value::VTInteger;
					result_type->SetPOD(rtype);
					// *commutative=&OpFunc_Mult_Integer_Integer;
					return(&OpFunc_Mult_Integer_Integer);
				case Value::VTScalar:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					// *commutative=&OpFunc_Mult_Scalar_Integer;
					return(&OpFunc_Mult_Integer_Scalar);
				case Value::VTRange:
					/*if(ptype1->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[1];
						// *commutative=&OpFunc_Mult_Range_Integer;
						return(&OpFunc_Mult_Integer_Range);
					/*} break;*/
				case Value::VTVector:
					*result_type=argtypes[1];
					// *commutative=&OpFunc_Mult_Vector_Integer;
					return(&OpFunc_Mult_Integer_Vector);
				case Value::VTMatrix:
					*result_type=argtypes[1];
					// *commutative=&OpFunc_Mult_Matrix_Integer;
					return(&OpFunc_Mult_Integer_Matrix);
			}
		} break;
		case Value::VTScalar:
		{
			Value::CompleteType rtype;
			switch(ptype1->type)
			{
				case Value::VTInteger:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					// *commutative=&OpFunc_Mult_Integer_Scalar;
					return(&OpFunc_Mult_Scalar_Integer);
				case Value::VTScalar:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					// *commutative=&OpFunc_Mult_Scalar_Scalar;
					return(&OpFunc_Mult_Scalar_Scalar);
				case Value::VTRange:
					/*if(ptype1->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[1];
						// *commutative=&OpFunc_Mult_Range_Scalar;
						return(&OpFunc_Mult_Scalar_Range);
					/*} break;*/
				case Value::VTVector:
					*result_type=argtypes[1];
					// *commutative=&OpFunc_Mult_Vector_Scalar;
					return(&OpFunc_Mult_Scalar_Vector);
				case Value::VTMatrix:
					*result_type=argtypes[1];
					// *commutative=&OpFunc_Mult_Matrix_Scalar;
					return(&OpFunc_Mult_Scalar_Matrix);
			}
		} break;
		case Value::VTRange:
		{
			switch(ptype1->type)
			{
				case Value::VTInteger:
					/*if(ptype0->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[0];
						// *commutative=&OpFunc_Mult_Integer_Range;
						return(&OpFunc_Mult_Range_Integer);
					/*} break;*/
				case Value::VTScalar:
					/*if(ptype0->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[0];
						// *commutative=&OpFunc_Mult_Scalar_Range;
						return(&OpFunc_Mult_Range_Scalar);
					/*} break;*/
			}
		} break;
		case Value::VTVector:
		{
			switch(ptype1->type)
			{
				case Value::VTInteger:
					*result_type=argtypes[0];
					// *commutative=&OpFunc_Mult_Integer_Vector;
					return(&OpFunc_Mult_Vector_Integer);
				case Value::VTScalar:
					*result_type=argtypes[0];
					// *commutative=&OpFunc_Mult_Scalar_Vector;
					return(&OpFunc_Mult_Vector_Scalar);
			}
		} break;
		case Value::VTMatrix:
		{
			switch(ptype1->type)
			{
				case Value::VTInteger:
					*result_type=argtypes[0];
					// *commutative=&OpFunc_Mult_Integer_Matrix;
					return(&OpFunc_Mult_Matrix_Integer);
				case Value::VTScalar:
					*result_type=argtypes[0];
					// *commutative=&OpFunc_Mult_Scalar_Matrix;
					return(&OpFunc_Mult_Matrix_Scalar);
				case Value::VTVector:
					if(ptype0->c==ptype1->n)
					{
						Value::CompleteType rtype;
						rtype.type=Value::VTVector;
						rtype.n=ptype0->r;
						result_type->SetPOD(rtype);
						return(&OpFunc_Mult_Matrix_Vector);
					}
					break;
				case Value::VTMatrix:
					if(ptype0->c==ptype1->r)
					{
						Value::CompleteType rtype;
						rtype.type=Value::VTMatrix;
						rtype.r=ptype0->r;
						rtype.c=ptype1->c;
						result_type->SetPOD(rtype);
						return(&OpFunc_Mult_Matrix_Matrix);
					}
					break;
			}
		} break;
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Div(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	switch(ptype1->type)
	{
		case Value::VTInteger:
		{
			Value::CompleteType rtype;
			switch(ptype0->type)
			{
				case Value::VTInteger:
					rtype.type=Value::VTInteger;
					result_type->SetPOD(rtype);
					return(&OpFunc_Div_Integer_Integer);
				case Value::VTScalar:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					return(&OpFunc_Div_Scalar_Integer);
				case Value::VTRange:
					/*if(ptype0->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[0];
						return(&OpFunc_Div_Range_Integer);
					/*} break;*/
				case Value::VTVector:
					*result_type=argtypes[0];
					return(&OpFunc_Div_Vector_Integer);
				case Value::VTMatrix:
					*result_type=argtypes[0];
					return(&OpFunc_Div_Matrix_Integer);
			}
		} break;
		case Value::VTScalar:
		{
			Value::CompleteType rtype;
			switch(ptype0->type)
			{
				case Value::VTInteger:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					return(&OpFunc_Div_Integer_Scalar);
				case Value::VTScalar:
					rtype.type=Value::VTScalar;
					result_type->SetPOD(rtype);
					return(&OpFunc_Div_Scalar_Scalar);
				case Value::VTRange:
					/*if(ptype0->rvalid==Value::RValidAB)
					{*/
						*result_type=argtypes[0];
						return(&OpFunc_Div_Range_Scalar);
					/*} break;*/
				case Value::VTVector:
					*result_type=argtypes[0];
					return(&OpFunc_Div_Vector_Scalar);
				case Value::VTMatrix:
					*result_type=argtypes[0];
					return(&OpFunc_Div_Matrix_Scalar);
			}
		} break;
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Modulo(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Modulo_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Modulo_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Modulo_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTScalar;
			result_type->SetPOD(rtype);
			return(&OpFunc_Modulo_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Add(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==ptype1->type)
	{
		switch(ptype0->type)
		{
			case Value::VTInteger:
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Integer_Integer;
				return(&OpFunc_Add_Integer_Integer);
			case Value::VTScalar:
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Scalar_Scalar;
				return(&OpFunc_Add_Scalar_Scalar);
			case Value::VTVector:
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Vector_Vector;
				return(&OpFunc_Add_Vector_Vector);
			case Value::VTMatrix:
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Matrix_Matrix;
				return(&OpFunc_Add_Matrix_Matrix);
			case Value::VTString:
				*result_type=argtypes[0];
				return(&OpFunc_Add_String_String);
		}
	}
	else
	{
		if(ptype0->type==Value::VTInteger)
		{
			if(ptype1->type==Value::VTScalar)
			{
				Value::CompleteType rtype;
				rtype.type=Value::VTScalar;
				result_type->SetPOD(rtype);
				// *commutative=&OpFunc_Add_Scalar_Integer;
				return(&OpFunc_Add_Integer_Scalar);
			}
			else if(ptype1->type==Value::VTRange /*&& 
			   ptype1->rvalid==Value::RValidAB*/ )
			{
				*result_type=argtypes[1];
				// *commutative=&OpFunc_Add_Range_Integer;
				return(&OpFunc_Add_Integer_Range);
			}
		}
		else if(ptype0->type==Value::VTScalar)
		{
			if(ptype1->type==Value::VTInteger)
			{
				Value::CompleteType rtype;
				rtype.type=Value::VTScalar;
				result_type->SetPOD(rtype);
				// *commutative=&OpFunc_Add_Integer_Scalar;
				return(&OpFunc_Add_Scalar_Integer);
			}
			else if(ptype1->type==Value::VTRange /*&& 
			   ptype1->rvalid==Value::RValidAB*/ )
			{
				*result_type=argtypes[1];
				// *commutative=&OpFunc_Add_Range_Scalar;
				return(&OpFunc_Add_Scalar_Range);
			}
		}
		else if(ptype0->type==Value::VTRange)
		{
			if(ptype1->type==Value::VTInteger)
			{
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Integer_Range;
				return(&OpFunc_Add_Range_Integer);
			}
			else if(ptype1->type==Value::VTScalar)
			{
				*result_type=argtypes[0];
				// *commutative=&OpFunc_Add_Scalar_Range;
				return(&OpFunc_Add_Range_Scalar);
			}
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Subtract(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==ptype1->type)
	{
		switch(ptype0->type)
		{
			case Value::VTInteger:
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Integer_Integer);
			case Value::VTScalar:
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Scalar_Scalar);
			case Value::VTVector:
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Vector_Vector);
			case Value::VTMatrix:
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Matrix_Matrix);
		}
	}
	else
	{
		if(ptype0->type==Value::VTInteger)
		{
			if(ptype1->type==Value::VTScalar)
			{
				Value::CompleteType rtype;
				rtype.type=Value::VTScalar;
				result_type->SetPOD(rtype);
				return(&OpFunc_Subtract_Integer_Scalar);
			}
			else if(ptype1->type==Value::VTRange /*&& 
			   ptype1->rvalid==Value::RValidAB*/ )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Subtract_Integer_Range);
			}
		}
		else if(ptype0->type==Value::VTScalar)
		{
			if(ptype1->type==Value::VTInteger)
			{
				Value::CompleteType rtype;
				rtype.type=Value::VTScalar;
				result_type->SetPOD(rtype);
				return(&OpFunc_Subtract_Scalar_Integer);
			}
			else if(ptype1->type==Value::VTRange /*&& 
			   ptype1->rvalid==Value::RValidAB*/ )
			{
				*result_type=argtypes[1];
				return(&OpFunc_Subtract_Scalar_Range);
			}
		}
		else if(ptype0->type==Value::VTRange)
		{
			if(ptype1->type==Value::VTInteger)
			{
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Range_Integer);
			}
			else if(ptype1->type==Value::VTScalar)
			{
				*result_type=argtypes[0];
				return(&OpFunc_Subtract_Range_Scalar);
			}
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Lesser(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Lesser_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Lesser_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Lesser_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Lesser_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Greater(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Greater_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Greater_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Greater_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_Greater_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_LesserEq(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_LesserEq_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_LesserEq_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_LesserEq_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_LesserEq_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_GreaterEq(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_GreaterEq_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_GreaterEq_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_GreaterEq_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTInteger;
			result_type->SetPOD(rtype);
			return(&OpFunc_GreaterEq_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Equal(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	
	// First, the non-POD section: 
	AniGlue_ScopeBase *asb0=argtypes[0].AniScope();
	AniGlue_ScopeBase *asb1=argtypes[1].AniScope();
	bool arg0_is_null=asb0 && 
		asb0->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
		asb0->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL;
	bool arg1_is_null=asb1 && 
		asb1->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
		asb1->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL;
	ExprValueType arr0=argtypes[0].ArrayType();
	ExprValueType arr1=argtypes[1].ArrayType();
	
	Value::CompleteType rtype;
	if(arg0_is_null && arg1_is_null)
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_Equal_Always);  // always 1
	}
	if(arg0_is_null && (asb1 || !!arr1))
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_Equal_Null_ASB);  // also works for array
	}
	if(arg1_is_null && (asb0 || !!arr0))
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_Equal_ASB_Null);  // also works for array
	}
	
	// Compare ASBs: FIXME, MISSING. 
	
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==ptype1->type)
	{
		switch(ptype0->type)
		{
			case Value::VTInteger:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Integer_Integer);
			case Value::VTScalar:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Scalar_Scalar);
			case Value::VTRange:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Range_Range);
			case Value::VTVector:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Vector_Vector);
			case Value::VTMatrix:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Matrix_Matrix);
			case Value::VTString:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_String_String);
		}
	}
	else
	{
		if(ptype0->type==Value::VTInteger)
		{
			if(ptype1->type==Value::VTScalar)
			{
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Integer_Scalar);
			}
		}
		else if(ptype0->type==Value::VTScalar)
		{
			if(ptype1->type==Value::VTInteger)
			{
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_Equal_Scalar_Integer);
			}
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_NotEqual(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	
	// First, the non-POD section: 
	AniGlue_ScopeBase *asb0=argtypes[0].AniScope();
	AniGlue_ScopeBase *asb1=argtypes[1].AniScope();
	bool arg0_is_null=asb0 && 
		asb0->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
		asb0->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL;
	bool arg1_is_null=asb1 && 
		asb1->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete && 
		asb1->CG_IncompleteType()==AniGlue_ScopeBase::IT_NULL;
	ExprValueType arr0=argtypes[0].ArrayType();
	ExprValueType arr1=argtypes[1].ArrayType();
	
	Value::CompleteType rtype;
	if(arg0_is_null && arg1_is_null)
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_NotEqual_Always);  // always 0
	}
	if(arg0_is_null && (asb1 || !!arr1))
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_NotEqual_Null_ASB);  // also works for array
	}
	if(arg1_is_null && (asb0 || !!arr0))
	{
		rtype.type=Value::VTInteger;
		result_type->SetPOD(rtype);
		return(&OpFunc_NotEqual_ASB_Null);  // also works for array
	}
	
	// Compare ASBs: FIXME, MISSING. 
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==ptype1->type)
	{
		switch(ptype0->type)
		{
			case Value::VTInteger:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Integer_Integer);
			case Value::VTScalar:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Scalar_Scalar);
			case Value::VTRange:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Range_Range);
			case Value::VTVector:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Vector_Vector);
			case Value::VTMatrix:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Matrix_Matrix);
			case Value::VTString:
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_String_String);
		}
	}
	else
	{
		if(ptype0->type==Value::VTInteger)
		{
			if(ptype1->type==Value::VTScalar)
			{
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Integer_Scalar);
			}
		}
		else if(ptype0->type==Value::VTScalar)
		{
			if(ptype1->type==Value::VTInteger)
			{
				rtype.type=Value::VTInteger;
				result_type->SetPOD(rtype);
				return(&OpFunc_NotEqual_Scalar_Integer);
			}
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Range(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=2)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	const Value::CompleteType *ptype1=argtypes[1].PODType();
	if(!ptype0 || !ptype1) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTRange;
			//rtype.rvalid=Value::RValidAB;
			result_type->SetPOD(rtype);
			return(&OpFunc_Range_Integer_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTRange;
			//rtype.rvalid=Value::RValidAB;
			result_type->SetPOD(rtype);
			return(&OpFunc_Range_Integer_Scalar);
		}
	}
	else if(ptype0->type==Value::VTScalar)
	{
		if(ptype1->type==Value::VTInteger)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTRange;
			//rtype.rvalid=Value::RValidAB;
			result_type->SetPOD(rtype);
			return(&OpFunc_Range_Scalar_Integer);
		}
		else if(ptype1->type==Value::VTScalar)
		{
			Value::CompleteType rtype;
			rtype.type=Value::VTRange;
			//rtype.rvalid=Value::RValidAB;
			result_type->SetPOD(rtype);
			return(&OpFunc_Range_Scalar_Scalar);
		}
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_RangeNoA(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=1)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	if(!ptype0) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		Value::CompleteType rtype;
		rtype.type=Value::VTRange;
		//rtype.rvalid=Value::RValidB;
		result_type->SetPOD(rtype);
		return(&OpFunc_RangeNoA_Integer);
	}
	else if(ptype0->type==Value::VTScalar)
	{
		Value::CompleteType rtype;
		rtype.type=Value::VTRange;
		//rtype.rvalid=Value::RValidB;
		result_type->SetPOD(rtype);
		return(&OpFunc_RangeNoA_Scalar);
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_RangeNoB(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	if(nargs!=1)  return(NULL);
	const Value::CompleteType *ptype0=argtypes[0].PODType();
	if(!ptype0) return(NULL);
	if(ptype0->type==Value::VTInteger)
	{
		Value::CompleteType rtype;
		rtype.type=Value::VTRange;
		//rtype.rvalid=Value::RValidA;
		result_type->SetPOD(rtype);
		return(&OpFunc_RangeNoB_Integer);
	}
	else if(ptype0->type==Value::VTScalar)
	{
		Value::CompleteType rtype;
		rtype.type=Value::VTRange;
		//rtype.rvalid=Value::RValidA;
		result_type->SetPOD(rtype);
		return(&OpFunc_RangeNoB_Scalar);
	}
	return(NULL);
}

static OpFunc_Ptr MatchFunc_Vector(int nargs,
	const ExprValueType *argtypes,ExprValueType *result_type)
{
	for(int i=0; i<nargs; i++)
	{
		const Value::CompleteType *ptypeI=argtypes[i].PODType();
		if(!ptypeI) return(NULL);
		switch(ptypeI->type)
		{
			case Value::VTInteger:
			case Value::VTScalar:
				break;
			default: return(NULL);
		}
	}
	Value::CompleteType rtype;
	rtype.type=Value::VTVector;
	rtype.n=nargs;
	result_type->SetPOD(rtype);
	return(&OpFunc_Vector);
}


//******************************************************************************
//******************************************************************************

struct _OpFuncID_Desc OpFuncID_Desc[_OF_LAST]=
{
	{ OF_None, NULL },
	
	{ OF_Subscript, NULL },
	{ OF_FCall, NULL },
	{ OF_PODCast, &MatchFunc_PODCast },
	{ OF_MembSel, NULL },
	{ OF_Mapping, NULL },
	
	{ OF_PostIncrement, NULL },
	{ OF_PostDecrement, NULL },
	{ OF_PreIncrement, NULL },
	{ OF_PreDecrement, NULL },
	
	{ OF_UnPlus, &MatchFunc_UnPlus },
	{ OF_UnMinus, &MatchFunc_UnMinus },
	{ OF_UnNot, &MatchFunc_UnNot },
	
	{ OF_New, NULL },
	{ OF_NewArray, NULL },
	{ OF_Delete, NULL },
	
	{ OF_Pow, &MatchFunc_Pow },
	{ OF_Mult, &MatchFunc_Mult },
	{ OF_Div, &MatchFunc_Div },
	{ OF_Modulo, &MatchFunc_Modulo },
	{ OF_Add, &MatchFunc_Add },
	{ OF_Subtract, &MatchFunc_Subtract },
	{ OF_Lesser, &MatchFunc_Lesser },
	{ OF_Greater, &MatchFunc_Greater },
	{ OF_LesserEq, &MatchFunc_LesserEq },
	{ OF_GreaterEq, &MatchFunc_GreaterEq },
	{ OF_Equal, &MatchFunc_Equal },
	{ OF_NotEqual, &MatchFunc_NotEqual },
	{ OF_LogicalAnd, NULL },
	{ OF_LogicalOr, NULL },
	
	{ OF_IfElse, NULL },
	{ OF_IfElse2, NULL },
	
	{ OF_Range, &MatchFunc_Range },
	{ OF_RangeNoA, &MatchFunc_RangeNoA },
	{ OF_RangeNoB, &MatchFunc_RangeNoB },
	{ OF_Vector, &MatchFunc_Vector },
};


// Primary function: 
// See opfunc.h. 
OpFunc_Ptr LookupOperatorFunction(OperatorFuncID of_id,
	int nargs,const ExprValueType *argtypes,
	ExprValueType *result_type)
{
	if(of_id<0 || of_id>=_OF_LAST)
	{  assert(0);  return(NULL);  }
	_OpFuncID_Desc *desc=&OpFuncID_Desc[of_id];
	assert(desc->ofid==of_id);
	if(!desc->matchfunc)  return(NULL);
	result_type->SetNull();
	OpFunc_Ptr fptr=(*(desc->matchfunc))(nargs,argtypes,result_type);
	assert(!fptr || !!result_type);
	return(fptr);
}

}  // end of namespace ANI
