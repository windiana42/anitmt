/*
 * ani-parser/exprvalue.cc
 * 
 * The main value representation in AniVision. 
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

#include "exprvalue.h"

namespace ANI
{

String ScopeInstance::ToString() const
{
	if(!sib)
	{  return(String("[NULL scope]"));  }
	return(sib->ToString());
}


/******************************************************************************/

void _InternalExprValueArray::_cleararray()
{
	switch(arr_type.TypeID())
	{
		case EVT_POD:
			if(pod)
			{  delete[] pod;  pod=NULL;  }
			break;
		case EVT_AniScope:
			if(scope)
			{  delete[] scope;  scope=NULL;  }
			break;
		case EVT_Array:
			if(array)
			{  delete[] array;  array=NULL;  }
			break;
		// All the rest: nothing to do. 
	}
}


_InternalExprValueArray &_InternalExprValueArray::operator=(
	const _InternalExprValueArray &)
{
	// forbidden.
	array=NULL;
	assert(0);
	return(*this);
}


_InternalExprValueArray::_InternalExprValueArray(
	const _InternalExprValueArray &) : 
	refcnt(0),arr_type(ExprValueType::CNullRef)
{
	// forbidden.
	array=NULL;
	assert(0);
}

_InternalExprValueArray::_InternalExprValueArray(
	ssize_t _arr_size,const ExprValueType &_arr_type,
	bool set_prototype_vals) : 
	refcnt(0),arr_size(_arr_size),arr_type(_arr_type)
{
	assert(arr_size>=0);
	
	switch(arr_type.TypeID())
	{
		case EVT_POD:
			pod = arr_size ? (new Value[arr_size]) : NULL;
			if(set_prototype_vals)
			{
				Value proto=Value::ProtoTypeOf(*arr_type.PODType());
				for(ssize_t i=0; i<arr_size; i++)
				{  pod[i]=proto;  }
			}
			break;
		case EVT_AniScope:
			scope = arr_size ? (new ScopeInstance[arr_size]) : NULL;
			// These are NULL scopes, so there is nothing to do if 
			// set_prototype_vals is set. 
			break;
		case EVT_Array:
			array = arr_size ? (new ExprValueArray[arr_size]) : NULL;
			// These are NULL arrays, so there is nothing to do if 
			// set_prototype_vals is set. 
			break;
		// All the rest: nothing to do. 
	}
}


_InternalExprValueArray::~_InternalExprValueArray()
{
	assert(refcnt==0);
	_cleararray();
}


/******************************************************************************/

int ExprValueArray::is_null_elem(ssize_t idx) const
{
	if(!iva) return(2);
	if(idx<0 || idx>=iva->arr_size)  return(1);
	switch(iva->arr_type.TypeID())
	{
		case EVT_Unknown:  return(2);
		case EVT_Void:     return(2);
		case EVT_POD:      return(iva->pod[idx].is_null());
		case EVT_AniScope: return(iva->scope[idx].is_null());
		case EVT_Array:    return(iva->array[idx].is_null());
		default:  assert(0);
	}
	return(1);
}


int ExprValueArray::IndexSet(ssize_t idx,const ExprValue &store_this)
{
	if(!iva)  return(-4);
	if(idx<0)  return(-3);
	if((ssize_t)idx>=iva->arr_size)  return(-2);
	
	switch(iva->arr_type.TypeID())
	{
		case EVT_Unknown:  return(2);
		case EVT_Void:     return(2);
		case EVT_POD:
			store_this.GetPOD(&iva->pod[idx]);
			break;
		case EVT_AniScope:
			store_this.GetAniScope(&iva->scope[idx]);
			break;
		case EVT_Array:
			store_this.GetArray(&iva->array[idx]);
			break;
		default:  assert(0);
	}
	
	return(0);
}


int ExprValueArray::IndexGet(ssize_t idx,ExprValue *store_here)
{
	if(!iva)  return(-4);
	if(idx<0)  return(-3);
	if((ssize_t)idx>=iva->arr_size)  return(-2);
	
	switch(iva->arr_type.TypeID())
	{
		case EVT_Unknown:  *store_here=ExprValue::CUnknownType;  break;
		case EVT_Void:     *store_here=ExprValue::CVoidType;     break;
		case EVT_POD:      store_here->assign(iva->pod[idx]);      break;
		case EVT_AniScope: store_here->assign(iva->scope[idx]);    break;
		case EVT_Array:    store_here->assign(iva->array[idx]);    break;
		default:  assert(0);
	}
	
	return(0);
}


String ExprValueArray::ToString() const
{
	if(!iva)
	{  return(String("[NULL array]"));  }
	String res("{");
	for(ssize_t i=0; i<iva->arr_size; i++)
	{
		switch(iva->arr_type.TypeID())
		{
			case EVT_Unknown:   res+="[unknown]";  break;
			case EVT_Void:      res+="[void]";     break;
			case EVT_POD:       res+=iva->pod[i].ToString();    break;
			case EVT_AniScope:  res+=iva->scope[i].ToString();  break;
			case EVT_Array:     res+=iva->array[i].ToString();  break;
			default:  assert(0);
		}
		if(i+1<iva->arr_size)
		{  res+=",";  }
	}
	res+="}";
	return(res);
}


/******************************************************************************/

static void _PODMatrixIndexOutOfRange_A(ssize_t idx)
{
	fprintf(stderr,"#### index %d out of range for matrix\n",idx);
	abort();
}

// Default implementations of various routines. 
// Most of them are simply "assert(0)" because they have to be 
// overridden in case they ever get called. 
ExprValueTypeID _InternalIndexHandler::GetExprValueTypeID() const
{
	assert(0);
}

void _InternalIndexHandler::GetExprValueType(ExprValueType *store_here) const
{
	store_here->SetUnknown();
	assert(0);
}

int _InternalIndexHandler::is_null() const
{
	assert(0);
	return(1);
}

String _InternalIndexHandler::ToString() const
{
	assert(0);
	return String();
}

void _InternalIndexHandler::assign(const Value &)
{
	assert(0);
}

void _InternalIndexHandler::assign(const ScopeInstance &)
{
	assert(0);
}

void _InternalIndexHandler::assign(const ExprValueArray &)
{
	assert(0);
}

void _InternalIndexHandler::GetPOD(Value *store_here) const
{
	assert(0);
	*store_here=Value::None;
}

void _InternalIndexHandler::GetAniScope(ScopeInstance *store_here) const
{
	assert(0);
	*store_here=ScopeInstance();
}

void _InternalIndexHandler::GetArray(ExprValueArray *store_here) const
{
	assert(0);
	*store_here=ExprValueArray();
}

int _InternalIndexHandler::GetPODInt() const
{
	assert(0);
	return(-1);
}

double _InternalIndexHandler::GetPODDouble() const
{
	assert(0);
	return(-1.0);
}


_InternalIndexHandler &_InternalIndexHandler::operator=(
	const _InternalIndexHandler &)
{
	// Forbidden. 
	assert(0);
	return(*this);
}

_InternalIndexHandler::_InternalIndexHandler(const _InternalIndexHandler &)
{
	// Forbidden. 
	assert(0);
}


_InternalIndexHandler::_InternalIndexHandler(
	InternalExprValue *_iev,ssize_t _idx,bool matrix)
{
	if(matrix)
	{
		if(_idx<0 && _idx>=0xffff)
		{  _PODMatrixIndexOutOfRange_A(_idx);  }
		idxR=_idx;
		idxC=-1;
		is_matrix_idx=1;
	}
	else
	{
		idx=_idx;
		is_matrix_idx=0;
	}
	refcnt=0;
	iev=_iev;
	if(iev)
	{  iev->_aqref();  }
}

_InternalIndexHandler::~_InternalIndexHandler()
{
	// Not inline because virtual. 
	assert(refcnt==0);
	if(iev)
	{  iev->_deref();  iev=NULL;  }
}


/******************************************************************************/

static void _NullArraySubscription()
{
	fprintf(stderr,"#### subscription of NULL array\n");
	abort();
}

static void _ArrayIndexOutOfRange(ssize_t idx,ssize_t arrsize)
{
	fprintf(stderr,"#### index %d out of range for array size %d\n",
		idx,arrsize);
	abort();
}

struct _InternalIndexHandler_Array : public _InternalIndexHandler
{
	// Get the array we shall do subscription on: 
	inline _InternalExprValueArray *_get_array() const
	{
		ExprValueArray tmp;
		iev->GetArray(&tmp);
		return(tmp.iva);
	}
	_InternalExprValueArray *_prepare_idx(ExprValueTypeID expect_evt) const
	{
		_InternalExprValueArray *iva = _get_array();
		if(!iva)  _NullArraySubscription();
		assert(iva->arr_type.TypeID()==expect_evt);
		if(idx<0 || idx>=iva->arr_size)  _ArrayIndexOutOfRange(idx,iva->arr_size);
		return(iva);
	}
	
	_InternalIndexHandler_Array(InternalExprValue *_iev,ssize_t _idx) : 
		_InternalIndexHandler(_iev,_idx)
	{
		// This is for arrays only. 
		assert(iev->GetExprValueTypeID()==EVT_Array);
		if(!_get_array())  _NullArraySubscription();
	}
	~_InternalIndexHandler_Array()  { }
	
	// [overriding virtuals]
	ExprValueTypeID GetExprValueTypeID() const
	{
		ExprValueType evt;
		iev->GetExprValueType(&evt);
		return(evt.ArrayType().TypeID());
	}
	void GetExprValueType(ExprValueType *store_here) const
	{
		ExprValueType evt;
		iev->GetExprValueType(&evt);
		*store_here=evt.ArrayType();
	}
	int is_null() const
	{
		ExprValueArray tmp;
		iev->GetArray(&tmp);
		return(tmp.is_null_elem(idx));
	}
	String ToString() const
	{
		ExprValueArray tmp;
		iev->GetArray(&tmp);
		ExprValue val;
		int rv=tmp.IndexGet(idx,&val);
		switch(rv)
		{
			case 0:  return(val.ToString());
			case -2:
			case -3:  // (both)
			{
				String s;
				s.sprintf("[idx %d out of range [%d]]",idx,tmp.GetSize());
				return(s);
			}
			case -4:  return(String("[subscription of NULL array]"));
			default:  assert(0);
		}
		return String();
	}
	void assign(const Value &v)
	{
		_prepare_idx(EVT_POD)->pod[idx]=v;
	}
	void assign(const ScopeInstance &v)
	{
		_prepare_idx(EVT_AniScope)->scope[idx]=v;
	}
	void assign(const ExprValueArray &v)
	{
		_prepare_idx(EVT_Array)->array[idx]=v;
	}
	void GetPOD(Value *store_here) const
	{
		_InternalExprValueArray *iva=_prepare_idx(EVT_POD);
		*store_here=iva->pod[idx];
	}
	void GetAniScope(ScopeInstance *store_here) const
	{
		_InternalExprValueArray *iva=_prepare_idx(EVT_AniScope);
		*store_here=iva->scope[idx];
	}
	void GetArray(ExprValueArray *store_here) const
	{
		_InternalExprValueArray *iva=_prepare_idx(EVT_Array);
		*store_here=iva->array[idx];
	}
	int GetPODInt() const
	{
		_InternalExprValueArray *iva=_prepare_idx(EVT_POD);
		return(iva->pod[idx].GetPtr<Integer>()->val());
	}
	double GetPODDouble() const
	{
		_InternalExprValueArray *iva=_prepare_idx(EVT_POD);
		return(iva->pod[idx].GetPtr<Scalar>()->val());
	}
};

/*----------------------------------------------------------------------------*/

static void _PODVectorIndexOutOfRange(ssize_t idx,int vsize)
{
	fprintf(stderr,"#### index %d out of range for vector of size %d\n",
		idx,vsize);
	abort();
}

struct _InternalIndexHandler_PODVector : public _InternalIndexHandler
{
	_InternalIndexHandler_PODVector(InternalExprValue *_iev,ssize_t _idx) : 
		_InternalIndexHandler(_iev,_idx)
	{
		// This is for POD/Vector only. 
		assert(iev->GetExprValueTypeID()==EVT_POD);
	}
	~_InternalIndexHandler_PODVector()  { }
	
	inline double _get_value() const
	{
		Value tmp;
		iev->GetPOD(&tmp);
		const Vector *v=tmp.GetPtr<Vector>();
		if(idx<0 || idx>=v->dim())
		{  _PODVectorIndexOutOfRange(idx,v->dim());  }
		return((*v)[idx]);
	}
	
	// [overriding virtuals]
	ExprValueTypeID GetExprValueTypeID() const
	{
		return(EVT_POD);
	}
	void GetExprValueType(ExprValueType *store_here) const
	{
		Value::CompleteType ctype;
		ctype.type=Value::VTScalar;
		store_here->SetPOD(ctype);
	}
	int is_null() const
	{
		Scalar s(_get_value());
		return(s.is_null());
	}
	String ToString() const
	{
		Scalar s(_get_value());
		return(s.ToString());
	}
	void assign(const Value &val)
	{
		const Scalar *s=val.GetPtr<Scalar>();
		// For now, assign works like this: get value, modify it 
		// and then set it again. 
		#warning "Optimize me!!!"
		Value tmp;
		iev->GetPOD(&tmp);
		Vector *v=(Vector*)tmp.GetPtr<Vector>();
		if(idx<0 || idx>=v->dim())
		{  _PODVectorIndexOutOfRange(idx,v->dim());  }
		(*v)[idx]=*s;
		iev->assign(tmp);
	}
	void assign(const ScopeInstance &/*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void assign(const ExprValueArray &/*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void GetPOD(Value *store_here) const
	{
		*store_here=_get_value();
	}
	void GetAniScope(ScopeInstance * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	void GetArray(ExprValueArray * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	int GetPODInt() const
	{
		// Can only get scalar/double. 
		assert(0);
	}
	double GetPODDouble() const
	{
		return(_get_value());
	}
};

/*----------------------------------------------------------------------------*/

static void _PODMatrixIndexOutOfRange(ssize_t idx,int vsize)
{
	fprintf(stderr,"#### index %d out of range for vector of size %d\n",
		idx,vsize);
	abort();
}

static void _PODMatrixIndexOutOfRange(short int idxR,short int idxC,
	short int R,short int C)
{
	fprintf(stderr,"#### index %d,%d out of range for matrix of size %dx%d\n",
		idxR,idxC,R,C);
	abort();
}

static void _PODMatrixIndexOutOfRange(short int idxR,short int R,short int C)
{
	fprintf(stderr,"#### row index %d out of range for matrix of size %dx%d\n",
		idxR,R,C);
	abort();
}

struct _InternalIndexHandler_PODMatrix : public _InternalIndexHandler
{
	_InternalIndexHandler_PODMatrix(InternalExprValue *_iev,ssize_t _idxR) : 
		_InternalIndexHandler(_iev,_idxR,1)
	{
		// This is for POD/Matrix only. 
		assert(iev->GetExprValueTypeID()==EVT_POD);
	}
	_InternalIndexHandler_PODMatrix(InternalExprValue *_iev,ssize_t _idxR,
		ssize_t _idxC) : _InternalIndexHandler(_iev,_idxR,1)
	{
		if(_idxC<0 && _idxC>=0xffff)
		{  _PODMatrixIndexOutOfRange_A(_idxC);  }
		idxC=_idxC;
		// This is for POD/Matrix only. 
		assert(iev->GetExprValueTypeID()==EVT_POD);
	}
	~_InternalIndexHandler_PODMatrix()  { }
	
	inline double _get_valueD() const
	{
		Value tmp;
		iev->GetPOD(&tmp);
		const Matrix *m=tmp.GetPtr<Matrix>();
		if(idxR<0 || idxC<0 || idxR>=m->rows() || idxC>=m->cols())
		{  _PODMatrixIndexOutOfRange(idxR,idxC,m->rows(),m->cols());  }
		return((*m)[idxR][idxC]);
	}
	inline void _get_valueV(Vector *dest) const
	{
		Value tmp;
		iev->GetPOD(&tmp);
		const Matrix *m=tmp.GetPtr<Matrix>();
		if(idxR<0 || idxR>=m->rows())
		{  _PODMatrixIndexOutOfRange(idxR,m->rows(),m->cols());  }
		*dest=Vector((*m)[idxR],m->cols());
	}
	
	// [overriding virtuals]
	ExprValueTypeID GetExprValueTypeID() const
	{
		return(EVT_POD);
	}
	void GetExprValueType(ExprValueType *store_here) const
	{
		Value::CompleteType ctype;
		if(idxC<0)
		{
			Value tmp;
			iev->GetPOD(&tmp);
			const Matrix *m=tmp.GetPtr<Matrix>();
			ctype.type=Value::VTVector;
			ctype.n=(int)m->cols();
		}
		else
		{  ctype.type=Value::VTScalar;  }
		store_here->SetPOD(ctype);
	}
	int is_null() const
	{
		if(idxC<0)
		{
			Vector v;
			_get_valueV(&v);
			return(v.is_null());
		}
		else
		{
			Scalar s(_get_valueD());
			return(s.is_null());
		}
	}
	String ToString() const
	{
		if(idxC<0)
		{
			Vector v;
			_get_valueV(&v);
			return(v.ToString());
		}
		else
		{
			Scalar s(_get_valueD());
			return(s.ToString());
		}
	}
	void assign(const Value &val)
	{
		// For now, assign works like this: get value, modify it 
		// and then set it again. 
		#warning "Optimize me!!!"
		Value tmp;
		iev->GetPOD(&tmp);
		Matrix *m=(Matrix*)tmp.GetPtr<Matrix>();
		if(idxC<0)
		{
			if(idxR<0 || idxR>=m->rows())
			{  _PODMatrixIndexOutOfRange(idxR,m->rows(),m->cols());  }
			const Vector *v=val.GetPtr<Vector>();
			assert(m->cols()==v->dim());
			for(short int i=0; i<m->cols(); i++)
			{  (*m)[idxR][i]=(*v)[i];  }
		}
		else
		{
			if(idxR<0 || idxC<0 || idxR>=m->rows() || idxC>=m->cols())
			{  _PODMatrixIndexOutOfRange(idxR,idxC,m->rows(),m->cols());  }
			const Scalar *s=val.GetPtr<Scalar>();
			(*m)[idxR][idxC]=s->val();
		}
		iev->assign(tmp);
	}
	void assign(const ScopeInstance & /*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void assign(const ExprValueArray & /*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void GetPOD(Value *store_here) const
	{
		if(idxC<0)
		{
			Vector tmp;
			_get_valueV(&tmp);
			*store_here=tmp;
		}
		else
		{  *store_here=_get_valueD();  }
	}
	void GetAniScope(ScopeInstance * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	void GetArray(ExprValueArray * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	int GetPODInt() const
	{
		// Can only get scalar/double or vector. 
		assert(0);
	}
	double GetPODDouble() const
	{
		assert(idxC>=0);
		return(_get_valueD());
	}
};

/*----------------------------------------------------------------------------*/

struct _InternalIndexHandler_PODRange : public _InternalIndexHandler
{
	_InternalIndexHandler_PODRange(InternalExprValue *_iev,ssize_t _idx) : 
		_InternalIndexHandler(_iev,_idx)
	{
		// This is for POD/Range only. 
		assert(iev->GetExprValueTypeID()==EVT_POD);
	}
	~_InternalIndexHandler_PODRange()  { }
	
	inline double _get_value() const
	{
		Value tmp;
		iev->GetPOD(&tmp);
		const Range *r=tmp.GetPtr<Range>();
		switch(idx)
		{
			case 0:  return((r->Valid() & Value::RValidA) ? r->val_a() : NAN);
			case 1:  return((r->Valid() & Value::RValidB) ? r->val_b() : NAN);
			default: assert(0);
		}
	}
	
	// [overriding virtuals]
	ExprValueTypeID GetExprValueTypeID() const
	{
		return(EVT_POD);
	}
	void GetExprValueType(ExprValueType *store_here) const
	{
		Value::CompleteType ctype;
		ctype.type=Value::VTScalar;
		store_here->SetPOD(ctype);
	}
	int is_null() const
	{
		Scalar s(_get_value());
		return(s.is_null());
	}
	String ToString() const
	{
		Scalar s(_get_value());
		return(s.ToString());
	}
	void assign(const Value &val)
	{
		const Scalar *s=val.GetPtr<Scalar>();
		// For now, assign works like this: get value, modify it 
		// and then set it again. 
		#warning "Optimize me!!!"
		Value tmp;
		iev->GetPOD(&tmp);
		Range *r=(Range*)tmp.GetPtr<Range>();
		switch(idx)
		{
			case 0:  r->set_a(s->val());  break;
			case 1:  r->set_b(s->val());  break;
			default:  assert(0);
		}
		iev->assign(tmp);
	}
	void assign(const ScopeInstance & /*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void assign(const ExprValueArray & /*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void GetPOD(Value *store_here) const
	{
		*store_here=_get_value();
	}
	void GetAniScope(ScopeInstance * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	void GetArray(ExprValueArray * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	int GetPODInt() const
	{
		// Can only get scalar/double. 
		assert(0);
	}
	double GetPODDouble() const
	{
		return(_get_value());
	}
};

/*----------------------------------------------------------------------------*/

static void _PODStringIndexOutOfRange(ssize_t idx,ssize_t len)
{
	fprintf(stderr,"#### index %d out of range for string of length %d\n",
		idx,len);
	abort();
}

struct _InternalIndexHandler_PODString : public _InternalIndexHandler
{
	_InternalIndexHandler_PODString(InternalExprValue *_iev,ssize_t _idx) : 
		_InternalIndexHandler(_iev,_idx)
	{
		// This is for POD/String only. 
		assert(iev->GetExprValueTypeID()==EVT_POD);
	}
	~_InternalIndexHandler_PODString()  { }
	
	inline void _get_value(String *dest) const
	{
		Value tmp;
		iev->GetPOD(&tmp);
		const String *s=tmp.GetPtr<String>();
		// For index out of range, fail silently returning null string. 
		if(idx>=0 && idx<s->len())
		{
			char tstr[2]={s->str()[idx],'\0'};
			*dest=String(tstr);
		}
	}
	
	// [overriding virtuals]
	ExprValueTypeID GetExprValueTypeID() const
	{
		return(EVT_POD);
	}
	void GetExprValueType(ExprValueType *store_here) const
	{
		Value::CompleteType ctype;
		ctype.type=Value::VTString;
		store_here->SetPOD(ctype);
	}
	int is_null() const
	{
		String s;
		_get_value(&s);
		return(s.is_null());
	}
	String ToString() const
	{
		String s;
		_get_value(&s);
		return(s.ToString());
	}
	void assign(const Value &val)
	{
		const String *s=val.GetPtr<String>();
		Value tmp;
		iev->GetPOD(&tmp);
		const String *v=tmp.GetPtr<String>();
		String dest(*v);  // "reference"
		if(dest.replace_idx(idx,*s))
		{  _PODStringIndexOutOfRange(idx,v->len());  }
		iev->assign(dest);
	}
	void assign(const ScopeInstance &/*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void assign(const ExprValueArray &/*v*/)
	{
		// ONLY POD type!
		assert(0);
	}
	void GetPOD(Value *store_here) const
	{
		String s;
		_get_value(&s);
		*store_here=s;
	}
	void GetAniScope(ScopeInstance * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	void GetArray(ExprValueArray * /*store_here*/) const
	{
		// ONLY POD type!
		assert(0);
	}
	int GetPODInt() const
	{
		// string type
		assert(0);
	}
	double GetPODDouble() const
	{
		// string type
		assert(0);
	}
};

/******************************************************************************/

String IndexHandler::ToString() const
{
	if(!iih)
	{  return(String("[NULL ihdl]"));  }
	return(iih->ToString());
}


void IndexHandler::_failassert() const
{
	assert(0);
}


/******************************************************************************/

InternalExprValue::InternalExprValue(_IEV_Namespace::_IndexArray,
	InternalExprValue *_iev,ssize_t _idx)
{
	// This is not inline to protect the internals from doing namespace 
	// pollution. However, all the called functions here ARE inline. 
	_copy(new _InternalIndexHandler_Array(_iev,_idx));
	refcnt=0;
	vnc=NULL;
}

InternalExprValue::InternalExprValue(_IEV_Namespace::_IndexPOD ip,
	InternalExprValue *_iev,ssize_t _idx)
{
	// There is a special case: if this vector is due to 
	// matrix subscription: 
	switch(ip)
	{
		case IndexPODVector:
			if(_iev->_evt==EVT_IndexHdl && _iev->ihdl()->iih->is_matrix_idx)
			{
				_InternalIndexHandler_PODMatrix *iihm=
					(_InternalIndexHandler_PODMatrix*)_iev->ihdl()->iih;
				
				_copy(new _InternalIndexHandler_PODMatrix(
					/*_iev=*/iihm->iev,/*idxR=*/iihm->idxR,/*idxC=*/_idx));
			}
			else
			{  _copy(new _InternalIndexHandler_PODVector(_iev,_idx));  }
			break;
		case IndexPODMatrix:
			_copy(new _InternalIndexHandler_PODMatrix(_iev,_idx));
			break;
		case IndexPODRange:
			_copy(new _InternalIndexHandler_PODRange(_iev,_idx));
			break;
		case IndexPODString:
			_copy(new _InternalIndexHandler_PODString(_iev,_idx));
			break;
	}
	refcnt=0;
	vnc=NULL;
}


void InternalExprValue::_destroy()
{
	if(vnc)
	{
		vnc->EmitVN_Delete();
		delete vnc;
		vnc=NULL;
	}
	switch(_evt)
	{
		case EVT_POD:       pod()->Value::~Value();                      break;
		case EVT_AniScope:  scope()->ScopeInstance::~ScopeInstance();    break;
		case EVT_Array:     array()->ExprValueArray::~ExprValueArray();  break;
		case EVT_IndexHdl:  ihdl()->IndexHandler::~IndexHandler();       break;
		// Rest: nothing to do
	}
	_evt=EVT_Unknown;
}

void InternalExprValue::_copy(const InternalExprValue *v)
{
	// NOTE: We need to de-index if necessary. I do not use 
	//       GetExprValueTypeID() here because this version 
	//       allows for some optimization: the IndexHandler 
	//       case is dealt with separately below. 
	switch(v->_evt)
	{
		case EVT_Unknown:
			_evt=EVT_Unknown;
			break;
		case EVT_Void:
			_evt=EVT_Void;
			break;
		case EVT_POD:
			// Direct POD, not via index handler: 
			_copy(*v->pod());
			break;
		case EVT_AniScope:
			// Direct ani scope, not via index handler: 
			_copy(*v->scope());
			break;
		case EVT_Array:
			// Direct ani array, not via index handler: 
			_copy(*v->array());
			break;
		case EVT_IndexHdl:
		{
			// Need to call index handler to get value...
			switch(v->ihdl()->GetExprValueTypeID())
			{
				case EVT_Unknown:
					_evt=EVT_Unknown;
					break;
				case EVT_Void:
					_evt=EVT_Void;
					break;
				case EVT_POD:
				{
					Value tmp;
					v->ihdl()->GetPOD(&tmp);
					_copy(tmp);
				}	break;
				case EVT_AniScope:
				{
					ScopeInstance tmp;
					v->ihdl()->GetAniScope(&tmp);
					_copy(tmp);
				}	break;
				case EVT_Array:
				{
					ExprValueArray tmp;
					v->ihdl()->GetArray(&tmp);
					_copy(tmp);
				}	break;
				case EVT_IndexHdl:
					// May not happen here (we already de-indexed)
					assert(0);  break;
				default:
					fprintf(stderr,"OOPS: _evt=%d -> assert\n",
						v->ihdl()->GetExprValueTypeID());
					assert(0);
			}
		}	break;
		default:  assert(0);  // As usual...
	}
}


void InternalExprValue::assign(const ScopeInstance &v)
{
	if(_evt==EVT_AniScope)
	{  scope()->operator=(v);  }  // Reference another scope instance. 
	else if(_evt==EVT_IndexHdl)
	{  ihdl()->assign(v);  }
	//else if(_evt==EVT_Unknown)
	//{  _copy(v);  }  // Copy value; do NOT reference it. 
	else assert(0);
	_EmitChangeNotify();
}

void InternalExprValue::assign(const ExprValueArray &v)
{
	if(_evt==EVT_Array)
	{  array()->operator=(v);  }  // Reference another array instance. 
	else if(_evt==EVT_IndexHdl)
	{  ihdl()->assign(v);  }
	//else if(_evt==EVT_Unknown)
	//{  _copy(v);  }  // Copy value; do NOT reference it. 
	else assert(0);
	_EmitChangeNotify();
}

void InternalExprValue::assign(const InternalExprValue *v)
{
	// NOTE: We need to de-index if necessary. I do not use 
	//       GetExprValueTypeID() here because this version 
	//       allows for some optimization: the IndexHandler 
	//       case is dealt with separately below. 
	switch(v->_evt)
	{
		case EVT_Unknown:  // Cannot assign unknown value. 
			assert(0);  break;
		case EVT_Void:  // Cannot assign void value. 
			assert(0);  break;
		case EVT_POD:
			// Direct POD, not via index handler: 
			assign(*v->pod());
			break;
		case EVT_AniScope:
			// Direct ani scope, not via index handler: 
			assign(*v->scope());
			break;
		case EVT_Array:
			// Direct ani array, not via index handler: 
			assign(*v->array());
			break;
		case EVT_IndexHdl:
		{
			// Need to call index handler to get value...
			switch(v->ihdl()->GetExprValueTypeID())
			{
				case EVT_Unknown:  // Cannot assign unknown value. 
					assert(0);  break;
				case EVT_Void:  // Cannot assign void value. 
					assert(0);  break;
				case EVT_POD:
				{
					Value tmp;
					v->ihdl()->GetPOD(&tmp);
					assign(tmp);
				}	break;
				case EVT_AniScope:
				{
					ScopeInstance tmp;
					v->ihdl()->GetAniScope(&tmp);
					assign(tmp);
				}	break;
				case EVT_Array:
				{
					ExprValueArray tmp;
					v->ihdl()->GetArray(&tmp);
					assign(tmp);
				}	break;
				case EVT_IndexHdl:
					// May not happen here (we already de-indexed)
					assert(0);  break;
				default:  assert(0);
			}
		}	break;
		default:  assert(0);  // As usual...
	}
}

void InternalExprValue::_assign_failassert()
{
	assert(0);
}


void InternalExprValue::_getpod_failassert() const
{
	// GetPODInt() or GetPODDouble() "failed". 
	assert(0);
}


void InternalExprValue::GetExprValueType(ExprValueType *dest) const
{
	switch(_evt)
	{
		case EVT_Unknown:   dest->SetUnknown();  break;
		case EVT_Void:      dest->SetVoid();     break;
		case EVT_POD:       dest->SetPOD(pod()->CType());     break;
		case EVT_AniScope:  scope()->GetExprValueType(dest);  break;
		case EVT_Array:     array()->GetExprValueType(dest);  break;
		case EVT_IndexHdl:  ihdl()->GetExprValueType(dest);   break;
		default:  assert(0);
	}
}


// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
int InternalExprValue::is_null() const
{
	switch(_evt)
	{
		case EVT_Unknown:  assert(0);  break;
		case EVT_Void:     assert(0);  break;
		case EVT_POD:      return(pod()->is_null());
		case EVT_AniScope: return(scope()->is_null());
		case EVT_Array:    return(array()->is_null());
		case EVT_IndexHdl: return(ihdl()->is_null());
	}
	assert(0);
	return(0);
}


String InternalExprValue::ToString() const
{
	switch(_evt)
	{
		case EVT_Unknown:  return(String("[unknown type]"));
		case EVT_Void:     return(String("[void]"));
		case EVT_POD:      return(pod()->ToString());
		case EVT_AniScope: return(scope()->ToString());
		case EVT_Array:    return(array()->ToString());
		case EVT_IndexHdl: return(ihdl()->ToString());
	}
	assert(0);
	return(String("???"));
}


int InternalExprValue::InstallVNH(ValueNotifyHandler *hdl,void *hook)
{
	if(!vnc)
	{  vnc=new ValueNotifierContainer();  }
	return(vnc->InstallNotifier(hdl,hook));
}


InternalExprValue &InternalExprValue::operator=(const InternalExprValue &)
{
	// Forbidden. 
	assert(0);
	return(*this);
}


InternalExprValue::InternalExprValue(const InternalExprValue &)
{
	// Forbidden. 
	assert(0);
}

InternalExprValue::~InternalExprValue()
{
	assert(refcnt==0);
	_destroy();
}


/******************************************************************************/

ExprValue::ExprValue(_Prototype,const ExprValueType &evt)
{
	switch(evt.TypeID())
	{
		case EVT_NULL:      iev=NULL;  break;
		case EVT_Unknown:   iev=new InternalExprValue(CUnknownType);    break;
		case EVT_Void:      iev=new InternalExprValue(CVoidType);       break;
		case EVT_POD:       iev=new InternalExprValue(Copy,
		                        Value::ProtoTypeOf(*evt.PODType()));    break;
		case EVT_AniScope:  iev=new InternalExprValue(Copy,
		                        ScopeInstance(ScopeInstance::None));    break;
		case EVT_Array:     iev=new InternalExprValue(Copy,
		                        ExprValueArray(ExprValueArray::None));  break;
		default:  assert(0);
	}
	_aqref();
}


String ExprValue::ToString() const
{
	if(!iev)
	{  return(String("[NULL EV]"));  }
	return(iev->ToString());
}


void ExprValue::_fail_ievassert() const
{
	assert(0);
}

String getstring()
{
	//return String();
	return(String("[NULL EV]"));
}

#if 0
int _test()
{
	//fprintf(stderr,"sizeof(Value)=%d;  sizeof(ExprValue)=%d\n",
	//	sizeof(Value),sizeof(ExprValue));
	
	Value::CompleteType ctype;
	ctype.type=Value::VTInteger;
	ExprValueArray arr1(2,ExprValueType(ctype),0);
	arr1.IndexSet(0,ExprValue(ExprValue::Copy,Value(11)));
	arr1.IndexSet(1,ExprValue(ExprValue::Copy,Value(22)));
	ExprValue aa(ExprValue::Copy,arr1);
	arr1=ExprValueArray(2,ExprValueType(ctype),0);
	arr1.IndexSet(0,ExprValue(ExprValue::Copy,Value(33)));
	arr1.IndexSet(1,ExprValue(ExprValue::Copy,Value(44)));
	ExprValue ab(ExprValue::Copy,arr1);
	arr1=ExprValueArray(2,ExprValueType(ctype),0);
	arr1.IndexSet(0,ExprValue(ExprValue::Copy,Value(55)));
	arr1.IndexSet(1,ExprValue(ExprValue::Copy,Value(66)));
	ExprValue ac(ExprValue::Copy,arr1);
	
	fprintf(stderr,"aa=%s;  ab=%s;  acv=%s\n",
		aa.ToString().str(),ab.ToString().str(),ac.ToString().str());
	
	ExprValueType tmp_evt;
	arr1.GetExprValueType(&tmp_evt);
	ExprValueArray arr2(3,tmp_evt,0);
	arr2.IndexSet(0,aa);
	arr2.IndexSet(1,ab);
	arr2.IndexSet(2,ac);
	ExprValue a(ExprValue::Copy,arr2);
	
	fprintf(stderr,"a=%s\n",a.ToString().str());
	
	ExprValue elem(ExprValue::IndexArray,ExprValue(ExprValue::IndexArray,a,1),0);
	fprintf(stderr,"elem=%s\n",elem.ToString().str());
	//elem.assign(Value(333));
	//fprintf(stderr,"a=%s;  elem=%s\n",a.ToString().str(),elem.ToString().str());
	
	fprintf(stderr,"TEST DONE--------------------\n");
	
	return(0);
}
#endif

}  // end of namespace ANI
