/*
 * value.hpp
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

#ifndef _NS_ExParse_Value_HPP_
#define _NS_ExParse_Value_HPP_ 1

#include <val/val.hpp>
#include <iostream>

namespace exparse
{

typedef values::Flag   Flag;
typedef values::Scalar Scalar;
typedef values::Vector Vector;
typedef values::String String;

//! This parser uses `Values' which may hold several differnet types of 
//! values discriminated by ValueType. 
//! The expression tree consists of `Operator's which are either 
//! values (OTValue) or real operators (like +,*,...) (OTAdd,OTMult,...)

enum ValueType
{
	VTNone=0,  //! no value assigned 
	VTFlag,
	VTScalar,
	VTVector,
	VTString
};

enum ValueState
{
	VSOK=0
};

class Value;

extern std::ostream &operator<<(std::ostream &os,const Value &v);

class Value
{
	private:
		ValueType vtype;
		ValueState vstate;
		union
		{
			void   *none;
			Flag   *flag;
			Scalar *scal;
			Vector *vect;
			String *str;
		} val;
		
		//! Normal way to overwrite an old value with a new one. 
		//! Fast if the old and new value is of same type, slower 
		//! if they are of different type. 
		void _setval(const void *v,ValueType vt);
		
		//! Delete current value (set VTNone). 
		void _zapval();
		
		// Very special function used by convert(): 
		// Used to set a value with different vtype and already 
		// suitably allocated via new. 
		inline void _set_different_new_val(void *new_val,ValueType vt);
		
		inline void init0()
			{  vtype=VTNone;  vstate=VSOK;  val.none=NULL;  }
	public:
		Value(const Value &v)  //! copy constructor 
			{  init0();  _setval(v.val.none,v.vtype);  vstate=v.vstate;  }
		Value()   //! VTNone
			{  init0();  }
		Value(const Flag   &flag)  //! VTFlag
			{  init0();  _setval(&flag,VTFlag);    }
		Value(const Scalar &scal)  //! VTScalar
			{  init0();  _setval(&scal,VTScalar);  }
		Value(const Vector &vect)  //! VTVector
			{  init0();  _setval(&vect,VTVector);  }
		Value(const String &str )  //! VTString
			{  init0();  _setval(&str,VTString);   }
		~Value()
			{  _zapval();  }
		
		//! Get the current value type: 
		ValueType type()  const  {  return(vtype);  }
		
		//! Assign type VTNone: 
		void zap()  {  _zapval();  }
		
		//! Get the current value: 
		//! The r* variants return references (undefined if 
		//! wrong type!), the p* variants pointers or NULL 
		//! if different vtype. 
		//! Return REFERENCE values (both const and non-const): 
		Flag   &rflag()    {  return(*val.flag);  }
		Scalar &rscalar()  {  return(*val.scal);  }
		Vector &rvector()  {  return(*val.vect);  }
		String &rstring()  {  return(*val.str);   }
		
		const Flag   &rflag()   const  {  return(*val.flag);  }
		const Scalar &rscalar() const  {  return(*val.scal);  }
		const Vector &rvector() const  {  return(*val.vect);  }
		const String &rstring() const  {  return(*val.str);   }
		
		//! Return POINTER values (both const and non-const): 
		Flag   *pflag()    {  return((vtype==VTFlag  ) ? val.flag : NULL);  }
		Scalar *pscalar()  {  return((vtype==VTScalar) ? val.scal : NULL);  }
		Vector *pvector()  {  return((vtype==VTVector) ? val.vect : NULL);  }
		String *pstring()  {  return((vtype==VTString) ? val.str  : NULL);  }
		
		const Flag   *pflag()   const  {  return((vtype==VTFlag  ) ? val.flag : NULL);  }
		const Scalar *pscalar() const  {  return((vtype==VTScalar) ? val.scal : NULL);  }
		const Vector *pvector() const  {  return((vtype==VTVector) ? val.vect : NULL);  }
		const String *pstring() const  {  return((vtype==VTString) ? val.str  : NULL);  }
		
		//! COPY values: 
		Flag   cflag()   const  {  return(Flag(*val.flag));  }
		Scalar cscalar() const  {  return(Scalar(*val.scal));  }
		Vector cvector() const  {  return(Vector(*val.vect));  }
		String cstring() const  {  return(String(*val.str));   }
		
		//! Assign the passed value; pass NULL to set none (VTNone). 
		void assign(const Value *v)
			{  _setval(v->val.none,v->vtype);  vstate=v->vstate;  }
		
		//! Assign a value: 
		void assign(const Flag   *v)  {  _setval(v,VTFlag);    }
		void assign(const Scalar *v)  {  _setval(v,VTScalar);  }
		void assign(const Vector *v)  {  _setval(v,VTVector);  }
		void assign(const String *v)  {  _setval(v,VTString);  }
		
		Value &operator=(const Value &v)
			{  assign(&v);  return(*this);  }
		Value *operator=(const Value *v)
			{  assign(v);  return(this);  }
		
		Value &operator=(const Flag &v)    {  assign(&v);  return(*this);  }
		Value &operator=(const Scalar &v)  {  assign(&v);  return(*this);  }
		Value &operator=(const Vector &v)  {  assign(&v);  return(*this);  }
		Value &operator=(const String &v)  {  assign(&v);  return(*this);  }
		
		Value *operator=(const Flag *v)    {  assign(v);  return(this);  }
		Value *operator=(const Scalar *v)  {  assign(v);  return(this);  }
		Value *operator=(const Vector *v)  {  assign(v);  return(this);  }
		Value *operator=(const String *v)  {  assign(v);  return(this);  }
		
		//! Convert the current value to the specified type vt. 
		//! Return value: 
		//!  0 -> success 
		//!  1 -> cannot convert 
		int convert(ValueType vt);
		
		//! Checks if you can convert *this to the specified value 
		//! type; does nothing more. 
		//! Return value: 1 -> can convert; 0 -> cannot convert. 
		int can_convert(ValueType vt) const;
		
		friend std::ostream &operator<<(std::ostream &os,const Value &v);
};

}  // namespace end 


#endif  /* _NS_ExParse_Value_HPP_ */
