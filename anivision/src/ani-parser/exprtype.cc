/*
 * ani-parser/exprtype.cc
 * 
 * Describing the type of an expression. 
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

#include "exprtype.h"
#include "coreglue.h"

namespace ANI
{

String _InternalExprValueType::_typestring() const
{
	String tmp;
	
	switch(type_id)
	{
		case EVT_Unknown:  tmp="[unknown type]";  break;
		case EVT_Void:     tmp="[void]";  break;
		case EVT_POD:
			switch(pod.type)
			{
				case Value::VTInteger:  tmp="int";     break;
				case Value::VTScalar:   tmp="scalar";  break;
				case Value::VTRange:
					tmp="range";
					/*if(pod.rvalid & Value::RValidA)
					{  tmp+="A";  }
					else
					{  tmp+="-";  }
					if(pod.rvalid & Value::RValidB)
					{  tmp+="B";  }
					else
					{  tmp+="-";  }*/
					break;
				case Value::VTVector:
					if(pod.n==-1)  // not (yet) specified
					{  tmp.sprintf("<?>");  }
					else if(pod.n!=Vector::default_n)
					{  tmp.sprintf("<%d>",pod.n);  }
					tmp.prepend("vector");
					break;
				case Value::VTMatrix:
					if(pod.r==-1 && pod.c==-1)  // not (yet) specified
					{  tmp.sprintf("<?,?>");  }
					else if(pod.r!=Matrix::default_r || 
					   pod.c!=Matrix::default_c )
					{  tmp.sprintf("<%d,%d>",(int)pod.r,(int)pod.c);  }
					tmp.prepend("matrix");
					break;
				case Value::VTString:   tmp="string";  break;
				default: assert(0);
			}
			break;
		case EVT_AniScope:
			if(scope)  tmp=scope->CG_GetName().str(); // scope->name;
			else tmp="[(nullscope)]";
			break;
		case EVT_Array:
		{
			if(arrtype) tmp=arrtype->_typestring();
			else tmp="[(nullarray)]";
			tmp+="[]";
		}	break;
		//case EVT_NULL:  tmp="[EVT_NULL]";  break;
		default: assert(0);
	}
	
	return(tmp);
}


int _InternalExprValueType::_is_pod_incomplete(const Value::CompleteType &pod)
{
	if(pod.type==Value::VTVector && pod.n==-1)  return(1);
	if(pod.type==Value::VTMatrix && (pod.r==-1 || pod.c==-1) )  return(1);
	return(0);
}


int _InternalExprValueType::_can_boolcv() const
{
	// Every expression is that unless unknown or void. 
	switch(type_id)
	{
		// Fall through switch. 
		case EVT_POD:
		case EVT_AniScope:
		case EVT_Array:
			return(1);
	}
	return(0);
}


bool _InternalExprValueType::_exact_compare(const _InternalExprValueType *b)
{
	if(type_id!=b->type_id)  return(0);
	switch(type_id)
	{
		case EVT_Unknown:  return(0);
		case EVT_Void:     return(1);
		case EVT_POD:      return(pod==b->pod);
		case EVT_AniScope:
			/*// What about scope_flag?
			assert(!scope_flags && !b->scope_flags); <-- no scope flags*/
			return(scope==b->scope);
		case EVT_Array:
			return(arrtype->_exact_compare(b->arrtype));
	}
	assert(0);
}


int _InternalExprValueType::_is_incomplete() const
{
	switch(type_id)
	{
		case EVT_Unknown:   return(0);
		case EVT_Void:      return(0);
		case EVT_POD:       return(_is_pod_incomplete(pod));
		case EVT_AniScope:
			if(scope && scope->CG_ASType()==AniGlue_ScopeBase::AST_Incomplete)
			{  assert(0);  }  // <-- If it happens, check what to do in this case. 
			return(0);
		case EVT_Array:     return(arrtype->_is_incomplete());
	}
	assert(0);
}


_InternalExprValueType *_InternalExprValueType::clone()
{
	switch(type_id)
	{
		case EVT_Unknown:   // fall through
		case EVT_Void:
			return(new _InternalExprValueType(type_id));
		case EVT_POD:
			return(new _InternalExprValueType(pod));
		case EVT_AniScope:
			return(new _InternalExprValueType(scope,scope_flags));
		case EVT_Array:
			return(new _InternalExprValueType(
				_InternalExprValueType::CArrayType,arrtype));
	}
	assert(0);
	return(NULL);
}


_InternalExprValueType &_InternalExprValueType::operator=(
	const _InternalExprValueType &)
{
	// May never be used. 
	assert(0);
	return(*this);
}


_InternalExprValueType::_InternalExprValueType(
	const _InternalExprValueType &)
{
	// May never be used. 
	assert(0);
}

_InternalExprValueType::~_InternalExprValueType()
{
	assert(refcnt==0);
	if(type_id==EVT_Array && arrtype)
	{  arrtype->_deref();  }
	type_id=EVT_Unknown;
}


/******************************************************************************/

// These are the allocation functions for the internal data 
// structures. These functions may be smart and return already 
// existing references from somewhere else. 
#warning "Make these smarter."
_InternalExprValueType *ExprValueType::_AllocIVT(
	const Value::CompleteType &_pod)
{
	_InternalExprValueType *ivt;
	switch(_pod.type)
	{
		// error and none may not happen here. 
		// I may allow error, but "none" is not a type; 
		// use EVT_Void or EVT_Unknown. 
		case Value::VTError:    // fall through
		case Value::VTNone:     ivt=NULL;  assert(0);  break;
		case Value::VTInteger:  ivt=S_pod_integer;  break;
		case Value::VTScalar:   ivt=S_pod_scalar;   break;
		case Value::VTRange:    ivt=S_pod_range;    break;  // <-- Possible because rvalid no longer in CompleteType
		case Value::VTString:   ivt=S_pod_string;   break;
		default:  ivt=new _InternalExprValueType(_pod);  break;
	}
	return(ivt);
}

_InternalExprValueType *ExprValueType::_AllocIVT(
	AniGlue_ScopeBase *_scope,int _scope_flags)
{
	assert(_scope);
	_InternalExprValueType *ivt;
	ivt=new _InternalExprValueType(_scope,_scope_flags);
	return(ivt);
}

_InternalExprValueType *ExprValueType::_AllocIVT(
	ExprValueType::_ArrayType,const ExprValueType &_arrtype)
{
	assert(!!_arrtype);
	_InternalExprValueType *ivt;
	ivt=new _InternalExprValueType(_InternalExprValueType::CArrayType,
		_arrtype.ivt);
	return(ivt);
}


String ExprValueType::TypeString() const
{
	if(!ivt)
	{
		String tmp("[(null)]");
		return(tmp);
	}
	else
	{  return(ivt->_typestring());  }
}


#warning "Could inline fast path of SetXXX functions."
void ExprValueType::SetPOD(const Value::CompleteType &_pod)
{
	// The commented part could as well be commented out but 
	// this is probably faster while not requiring more 
	// memory. 
	if(!ivt || ivt->type_id!=EVT_POD || ivt->refcnt>1 /*|| 
	   _pod.type==Value::VTInteger || 
	   _pod.type==Value::VTScalar || 
	   _pod.type==Value::VTString*/ )
	{
		_deref();
		ivt=_AllocIVT(_pod);
		ivt->_aqref();
	}
	else  // fast path
	{  ivt->pod=_pod;  }
}

void ExprValueType::SetAniScope(AniGlue_ScopeBase *_scope,int _scope_flags)
{
	if(!ivt || ivt->type_id!=EVT_AniScope || ivt->refcnt>1)
	{
		_deref();
		ivt=_AllocIVT(_scope,_scope_flags);
		ivt->_aqref();
	}
	else  // fast path
	{  ivt->scope=_scope;  }
}

void ExprValueType::SetArray(const ExprValueType &_arrtype)
{
	if(!ivt || ivt->type_id!=EVT_Array || ivt->refcnt>1)
	{
		_deref();
		ivt=_AllocIVT(CArrayType,_arrtype);
		ivt->_aqref();
	}
	else  // fast path
	{
		assert(ivt->arrtype);
		ivt->arrtype->_deref();
		ivt->arrtype=_arrtype.ivt;
		assert(ivt->arrtype);
		ivt->arrtype->_aqref();
	}
}


ExprValueType ExprValueType::RIntegerType()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTInteger;
	return(ExprValueType(tmp));
}

ExprValueType ExprValueType::RScalarType()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTScalar;
	return(ExprValueType(tmp));
}

ExprValueType ExprValueType::RRangeType()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTRange;
	return(ExprValueType(tmp));
}

ExprValueType ExprValueType::RStringType()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTString;
	return(ExprValueType(tmp));
}

ExprValueType ExprValueType::RVectorType(int n)
{
	Value::CompleteType tmp;
	tmp.type=Value::VTVector;
	tmp.n=n;
	return(ExprValueType(tmp));
}

ExprValueType ExprValueType::RStringArrayType()
{
	Value::CompleteType tmp;
	tmp.type=Value::VTString;
	ExprValueType elemtype(tmp);
	return(ExprValueType(ExprValueType::CArrayType,elemtype));
}


//------------------------------------------------------------------------------

// Init static data: 
_InternalExprValueType *ExprValueType::S_unknown_type=NULL;
_InternalExprValueType *ExprValueType::S_void_type=NULL;
_InternalExprValueType *ExprValueType::S_pod_integer=NULL;
_InternalExprValueType *ExprValueType::S_pod_scalar=NULL;
_InternalExprValueType *ExprValueType::S_pod_range=NULL;
_InternalExprValueType *ExprValueType::S_pod_string=NULL;

// Important initialisation routine: 
void ExprValueType::_InitStaticVals()
{
	fprintf(stderr,"ExprValueType::_InitStaticVals()...");
	
	Value::CompleteType ctype;
	
	S_unknown_type=new _InternalExprValueType(EVT_Unknown);
		S_unknown_type->_aqref();
	S_void_type=new _InternalExprValueType(EVT_Void);
		S_void_type->_aqref();
	
	ctype.type=Value::VTInteger;
		S_pod_integer=new _InternalExprValueType(ctype);
		S_pod_integer->_aqref();
	ctype.type=Value::VTScalar;
		S_pod_scalar=new _InternalExprValueType(ctype);
		S_pod_scalar->_aqref();
	ctype.type=Value::VTRange;
		S_pod_range=new _InternalExprValueType(ctype);
		S_pod_range->_aqref();
	ctype.type=Value::VTString;
		S_pod_string=new _InternalExprValueType(ctype);
		S_pod_string->_aqref();
	
	fprintf(stderr,"done\n");
	
	//fprintf(stderr,"ExprValueType::statics=%p %p %p %p %p %p\n",
	//	S_unknown_type,S_void_type,
	//	S_pod_integer,S_pod_scalar,S_pod_range,S_pod_string);
}

// Cleanup function: 
void ExprValueType::_CleanupStaticVals()
{
	fprintf(stderr,"ExprValueType::_CleanupStaticVals()...");
	
	if(S_unknown_type->refcnt!=1 || 
	   S_void_type->refcnt!=1 || 
	   S_pod_integer->refcnt!=1 || 
	   S_pod_scalar->refcnt!=1 || 
	   S_pod_range->refcnt!=1 || 
	   S_pod_string->refcnt!=1 )
	{
		fprintf(stderr," OOPS: %d %d %d %d %d %d!!",
			S_unknown_type->refcnt,S_void_type->refcnt,S_pod_integer->refcnt,
			S_pod_scalar->refcnt,S_pod_range->refcnt,S_pod_string->refcnt);
		assert(0);
	}
	
	// This is easy here...
	S_unknown_type->_deref(); S_unknown_type=NULL;
	S_void_type->_deref();    S_void_type=NULL;
	S_pod_integer->_deref();  S_pod_integer=NULL;
	S_pod_scalar->_deref();   S_pod_scalar=NULL;
	S_pod_range->_deref();    S_pod_range=NULL;
	S_pod_string->_deref();   S_pod_string=NULL;
	
	fprintf(stderr,"done\n");
}

}  // end of namespace ANI
