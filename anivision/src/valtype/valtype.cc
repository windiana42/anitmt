/*
 * valtype/valtype.cc
 * 
 * Genuine value class implementation. 
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

#include "valtypes.h"
#include <assert.h>


bool operator==(
	const ValueNamespace::CompleteType &a,
	const ValueNamespace::CompleteType &b )
{
	if(a.type!=b.type) return(0);
	switch(a.type)
	{
		//case ValueNamespace::VTRange:
		//	return(a.rvalid==b.rvalid);
		case ValueNamespace::VTVector:
			// Value n<0 may have special meaning; in that case this 
			// has to be dealt with on a higher level. 
			assert(a.n>=0 && b.n>=0);
			return(a.n==b.n);
		case ValueNamespace::VTMatrix:
			// Value r,c<0 may have special meaning; in that case this 
			// has to be dealt with on a higher level. 
			assert(a.r>=0 && a.c>=0 && b.r>=0 && b.c>=0);
			return(a.c==b.c && a.r==b.r);
	}
	return(1);
}


Value::CompleteType Value::CType() const
{
	CompleteType ct;
	
	ct.type=type;
	
	switch(type)
	{
		case VTError:    // fall
		case VTNone:     // through
		case VTInteger:  // here
		case VTScalar:   break;
		case VTRange:    /*ct.rvalid=((Range*)val)->Valid();*/  break;
		case VTVector:   ct.n=((Vector*)val)->dim();  break;
		case VTMatrix:
			ct.r=((Matrix*)val)->rows();
			ct.c=((Matrix*)val)->cols();
			break;
		case VTString:  break;
		default:  assert(0);
	}
	
	return(ct);
}


Value ValueNamespace::ProtoTypeOf(const ValueNamespace::CompleteType &ct)
{
	Value v;
	
	switch(ct.type)
	{
		case VTError:    assert(0);  break;
		case VTNone:     break;
		case VTInteger:  v=0;  break;
		case VTScalar:   v=0.0;  break;
		case VTRange:
			// Meddle around with ct.rvalid... [no longer]
			v=Range(0.0,0.0);  break;
		case VTVector:   v=Vector(Vector::Null,ct.n);  break;
		case VTMatrix:   v=Matrix(Matrix::Null,ct.r,ct.c); break;
		case VTString:   v=String("");  break;
		default:  assert(0);
	}
	
	return(v);
}


void Value::_typeassert_failed(VType is,VType shall) const
{
	fprintf(stderr,"Type assertion failed: type=%d, should be %d\n",
		is,shall);
	abort();
}


// These functions are virtual, so there is no point in 
// trying to make them inline. 

ValueBase *Integer::clone() const
{  return(new Integer(*this));  }

ValueBase *Scalar::clone() const
{  return(new Scalar(*this));  }

ValueBase *Range::clone() const
{  return(new Range(*this));  }

ValueBase *Vector::clone() const
{  return(new Vector(*this));  }

ValueBase *Matrix::clone() const
{  return(new Matrix(*this));  }

ValueBase *String::clone() const
{  return(new String(*this));  }


void Integer::forceset(ValueBase *vb)
{  set(*(Integer*)vb);  }

void Scalar::forceset(ValueBase *vb)
{  set(*(Scalar*)vb);  }

void Range::forceset(ValueBase *vb)
{  set(*(Range*)vb);  }

void Vector::forceset(ValueBase *vb)
{  set(*(Vector*)vb);  }

void Matrix::forceset(ValueBase *vb)
{  set(*(Matrix*)vb);  }

void String::forceset(ValueBase *vb)
{  set(*(String*)vb);  }


// These may never be called: 
bool ValueBase::is_null() const
{  assert(0);  }

ValueBase *ValueBase::clone() const
{  assert(0);  }

void ValueBase::forceset(ValueBase *vb)
{  assert(0);  }
