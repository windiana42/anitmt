/*
 * value.cpp
 * 
 * Value abstraction for input library / exparse. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "value.hpp"

#include <stdio.h>


namespace exparse
{

//! Detete current value (set VTNone). 
void Value::_zapval()
{
	if(val.none)
	{
		switch(vtype)
		{
			case VTNone:    break;
			case VTFlag:    if(val.flag)  delete val.flag;  break;
			case VTScalar:  if(val.scal)  delete val.scal;  break;
			case VTVector:  if(val.vect)  delete val.vect;  break;
			case VTString:  if(val.str)   delete val.str;   break;
			default:        assert(0);  abort();
		}
		val.none=NULL;
	}
	vtype=VTNone;
	vstate=VSOK;
}


//! Normal way to overwrite an old value with a new one. 
//! Fast if the old and new value is of same type, slower 
//! if they are of different type. 
void Value::_setval(const void *v,ValueType vt)
{
	if(vt==vtype)  // Assign same type. 
	{              // This is easy. 
		switch(vtype)
		{
			case VTNone:    assert(!v); assert(!val.none);  break;   // pretty senseless...
			case VTFlag:    *val.flag = *(const Flag*)v;    break;
			case VTScalar:  *val.scal = *(const Scalar*)v;  break;
			case VTVector:  *val.vect = *(const Vector*)v;  break;
			case VTString:  *val.str  = *(const String*)v;  break;
			default:        assert(0);  abort();
		}
		vstate=VSOK;
		assert((!val.none && vtype==VTNone) || (val.none && vtype!=VTNone));
	}
	else  // Assign different type
	{
		assert((!v && vt==VTNone) || (v && vt!=VTNone));
		// First get rid of old value...
		_zapval();
		// ...then assign new one: 
		vtype=vt;
		switch(vtype)
		{
			case VTNone:    ;   break;   // pretty senseless...
			case VTFlag:    val.flag = new Flag  (*(const Flag  *)v);  break;
			case VTScalar:  val.scal = new Scalar(*(const Scalar*)v);  break;
			case VTVector:  val.vect = new Vector(*(const Vector*)v);  break;
			case VTString:  val.str  = new String(*(const String*)v);  break;
			default:        assert(0);  abort();
		}
		vstate=VSOK;
		assert((!val.none && vtype==VTNone) || (val.none && vtype!=VTNone));
	}
}

// Very special function used by convert(): 
// Used to set a value with different vtype and already 
// suitably allocated via new. 
inline void Value::_set_different_new_val(void *new_val,ValueType vt)
{
	// First get rid of old value...
	_zapval();
	// ...then assign new one: 
	val.none=new_val;
	vtype=vt;
	vstate=VSOK;
	assert((!val.none && vtype==VTNone) || (val.none && vtype!=VTNone));
}


//! Convert the current value to the specified type vt. 
//! Return value: 
//!  0 -> success 
//!  1 -> cannot convert 
// DONT FORGET TO UPDATE can_convert() AS WELL. 
int Value::convert(ValueType vt)
{
	if(vt==vtype)  return(0);
	switch(vtype)
	{
		case VTNone:  break;
		case VTFlag:
			if(vt==VTScalar)  // flag -> scalar
			{
				Scalar *tmp=new Scalar(val.flag->val() ? 1.0 : 0.0);
				_set_different_new_val(tmp,VTScalar);
				return(0);
			}
			break;
		case VTScalar:
			if(vt==VTFlag)  // scalar -> flag
			{
				Flag *tmp=new Flag((*val.scal==0.0) ? false : true);
				_set_different_new_val(tmp,VTFlag);
				return(0);
			}
			#warning make it switchable 
			if(vt==VTVector)  // scalar -> vector
			{
				fprintf(stderr,"warning: scalar -> vector promotion\n");
				Vector *tmp=new Vector(val.scal->val(),
					val.scal->val(),val.scal->val());
				_set_different_new_val(tmp,VTVector);
			}
			break;
		case VTVector:
			if(vt==VTFlag)  // vector -> flag
			{
				Flag *tmp=new Flag(val.vect->is_null() ? false : true);
				_set_different_new_val(tmp,VTFlag);
				return(0);
			}
			break;
		case VTString:
			if(vt==VTFlag)
			{
				Flag *tmp=new Flag((val.str->length()>0) ? true : false);
				_set_different_new_val(tmp,VTFlag);
				return(0);
			}
			break;
		default:  assert(0);  abort();
	}
	return(1);
}

int Value::can_convert(ValueType vt) const
{
	if(vt==vtype)  return(1);
	switch(vtype)
	{
		case VTNone:
			break;
		case VTFlag:
			if(vt==VTScalar)  return(1);
			break;
		case VTScalar:
			#warning make it switchable 
			if(vt==VTFlag || vt==VTVector)  return(1);
			break;
		case VTVector:
			if(vt==VTFlag)  return(1);
			break;
		case VTString:
			if(vt==VTFlag)  return(1);
			break;
		default:  assert(0);  abort();
	}
	return(0);
}


ostream &operator<<(ostream &os,const Value &v)
{
	using namespace values;
	switch(v.vtype)
	{
		case VTNone:    os << "[none]";     break;
		case VTFlag:    os << (*v.val.flag);  break;
		case VTScalar:  os << (*v.val.scal);  break;
		case VTVector:  os << (*v.val.vect);  break;
		#warning string output does not do escape stuff...
		case VTString:  os << (*v.val.str);   break;
	}
	return(os);
}

}  // namespace end 

