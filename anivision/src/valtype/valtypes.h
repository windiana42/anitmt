/*
 * valtype/valtypes.h
 * 
 * Value types; definition of genuine (POD) Value class holding 
 * integer, scalar, range, vector, matrix or string value. 
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

#ifndef _VALTYPES_VALTYPES_H_
#define _VALTYPES_VALTYPES_H_ 1

#include <hlib/cplusplus.h>

#include <math.h>
#include <assert.h>


class Value;

struct ValueNamespace
{
	enum VType
	{
		VTError=-1,
		VTNone=0,
		VTInteger,
		VTScalar,
		VTRange,
		VTVector,
		VTMatrix,
		VTString
	};
	
	// Range validity flags: 
	enum RValidity
	{
		RInvalid=0x0,
		RValidA= 0x1,
		RValidB= 0x2,
		RValidAB=0x3
	};
	
	struct CompleteType
	{
		VType type;
		union {
			int n;                       // VTVector
			struct { short int r,c; };   // VTMatrix
			// RValidity rvalid;            // VTRange
		};
		// operator== and operator!= available. 
	}__attribute__((__packed__));
	
	// Allocation/deallocation routines; see valalloc.cc. 
	// Never use directly; use ValAlloc(), ValFree() below instead. 
	private:
		static void *_ValAlloc(size_t size);
		static void *_ValRealloc(void *ptr,size_t size);
		static void _ValFree(void *ptr);
		static void _illsize(int size);  // Called if size<0. 
	public:
		static inline void *ValAlloc(size_t size)
			{  return(size ? _ValAlloc(size) : NULL);  }
		static inline void *ValRealloc(void *ptr,size_t size)
			{  return(size ? _ValRealloc(ptr,size) : NULL);  }
		static inline void *ValFree(void *ptr)
			{  if(ptr) _ValFree(ptr);  return(NULL);  }
	
	// This is used by the vector and matrix types to 
	// allocate the arrays: 
	#warning "This needs to be fixed: use LMalloc() and wrapper and alloc check."
	template<class T>static inline T *ALLOC(int size)
		{  if(size<0) _illsize(size); return((T*)ValAlloc(size*sizeof(T)));  }
	template<class T>static inline T *FREE(T *ptr)
		{  return((T*)ValFree(ptr));  }
	template<class T>static inline T *FREEALLOC(T *ptr,int size)
		{  FREE(ptr);  if(size<0) _illsize(size);  return(ALLOC<T>(size));  }
	template<class T>static inline T *REALLOC(T *ptr,int newsize)
		{  if(newsize<0) _illsize(newsize);
			return((T*)ValRealloc(ptr,newsize*sizeof(T)));  }
	
	template<class T>static inline void CopyArray(T *dest,const T *src,int n)
		{  for(int i=0; i<n; i++)  *(dest++)=*(src++);  }
	
	// This returns a value which has the passed type and is normally 
	// initialized to zero. 
	static Value ProtoTypeOf(const CompleteType &ct);
};


bool operator==(const ValueNamespace::CompleteType &a,
	const ValueNamespace::CompleteType &b);
inline bool operator!=(const ValueNamespace::CompleteType &a,
	const ValueNamespace::CompleteType &b)
	{  return(!operator==(a,b));  }


class String;

// Do not use ValueBase directly; instead use Value. 
// ValueBase is for intenral use, only. 
struct ValueBase : public ValueNamespace
{
	// Default epsilon value: 
	static const double default_eps=1.0e-10;
	
	// fabs() version used by value types: 
	static inline double fabs(double x)
		{  return(::fabs(x));  }
	
	inline ValueBase() {}
	virtual ~ValueBase() {}
	
	inline void *operator new(size_t size)
		{  return(ValAlloc(size));  }
	inline void operator delete(void *ptr)
		{  ValFree(ptr);  }
	
	// Make string representation of value: 
	virtual String ToString() const;
	
	// Check if the value is null. Meaning of the "!" operator, 
	virtual bool is_null() const;
	
	// This creates a new copy of the value and returns it: 
	virtual ValueBase *clone() const;
	// Assign the passed value to this value; no type check! 
	virtual void forceset(ValueBase *vb);
};


#include "vtinteger.h"
#include "vtscalar.h"
#include "vtrange.h"
#include "vtvector.h"
#include "vtmatrix.h"
#include "vtstring.h"

inline String ValueBase::ToString() const
	{  String s; return(s);  }


// This checks various important things; do not disable. 
// Uncomment the following to disable the checks: 
//#define _typeassert(x,y)  do{}while(0)


// NOTE: Value is a container for different values, such as integer, 
//       scalar, range, vector, matrix, string. 
//       Value is NOT meant to be a class you can do arithmetics with; 
//       if you want to do that, convert it first to a Scalar, Vector 
//       or whatever (but make sure the Value actually holds a value of 
//       the desired type). 

class Value : public ValueNamespace
{
	public:
		enum _None { None };
		
		// (De)allocation: 
		inline void *operator new(size_t size)    {  return(ValAlloc(size));  }
		inline void *operator new[](size_t size)  {  return(ValAlloc(size));  }
		inline void operator delete(void *ptr)    {  ValFree(ptr);  }
		inline void operator delete[](void *ptr)  {  ValFree(ptr);  }
		// Allow for explicit placement: 
		inline void *operator new(size_t /*size*/,void *ptr)  {  return(ptr);  }
		
	private:
		// Value type: 
		VType type;
		// Pointer to stored value or NULL: 
		ValueBase *val;
		
		// Replace value by new one: 
		template<class T>inline void _replace(const T &v)
			{  if(val) delete val;  val=new T(v);  type=v.vtype;  }
		// Assignment template: 
		template<class T>inline void _assign(const T &v)
			{  if(type==v.vtype)  ((T*)val)->set(v);  else  _replace(v);  }
		// Remove value: 
		inline void _zap()
			{  if(val) { delete val; val=NULL; }  type=VTNone;  }
		// Type assertion function; inline for speed but in case of 
		// failure, the error routine is extern. 
		void _typeassert_failed(VType is,VType shall) const;
		inline void _typeassert(VType is,VType shall) const
			{  if(is!=shall) _typeassert_failed(is,shall);  }
	public:
		inline Value(int v)             { val=new Integer(v); type=VTInteger; }
		inline Value(double v)          { val=new Scalar(v);  type=VTScalar;  }
		inline Value(const Integer &v)  { val=new Integer(v); type=v.vtype; }
		inline Value(const Scalar &v)	{ val=new Scalar(v);  type=v.vtype; }
		inline Value(const Range &v)    { val=new Range(v);   type=v.vtype; }
		inline Value(const Vector &v)	{ val=new Vector(v);  type=v.vtype; }
		inline Value(const Matrix &v)	{ val=new Matrix(v);  type=v.vtype; }
		inline Value(const String &v)	{ val=new String(v);  type=v.vtype; }
		inline Value(_None=None)
			{  type=VTNone;  val=NULL;  }
		inline Value(const Value &v)
			{  type=v.type;  if(!v.val) val=NULL; else val=v.val->clone();  }
		inline ~Value()
			{  if(val) delete val;  val=NULL;  }
		
		// Get value type: 
		inline VType Type() const
			{  return(type);  }
		// Check value type: 
		inline bool operator==(VType vt) const  {  return(type==vt);  }
		inline bool operator!=(VType vt) const  {  return(type==vt);  }
		
		// Get complete type: 
		CompleteType CType() const;
		
		// Get size of vector or -1 in case this is not a vector: 
		inline int Dim() const
			{  return(type==VTVector ? ((const Vector*)val)->dim() : (-1));  }
		// Get size of matrix or -1 in case this is not a matrix: 
		inline short int Rows() const
			{  return(type==VTMatrix ? ((const Matrix*)val)->rows() : (-1));  }
		inline short int Cols() const
			{  return(type==VTMatrix ? ((const Matrix*)val)->cols() : (-1));  }
		
		// Assign value: 
		inline Value &operator=(int v)
		{
			if(type==VTInteger)  ((Integer*)val)->set(v);
			else  { if(val) delete val;  val=new Integer(v); type=VTInteger; }
			return(*this);
		}
		inline Value &operator=(double v)
		{
			if(type==VTScalar)  ((Scalar*)val)->set(v);
			else  { if(val) delete val;  val=new Scalar(v); type=VTScalar; }
			return(*this);
		}
		inline Value &operator=(const Integer &v)  {  return(operator=(v.val()));  }
		inline Value &operator=(const Scalar &v)   {  return(operator=(v.val()));  }
		inline Value &operator=(const Range &v)    {  _assign(v);  return(*this);  }
		inline Value &operator=(const Vector &v)   {  _assign(v);  return(*this);  }
		inline Value &operator=(const Matrix &v)   {  _assign(v);  return(*this);  }
		inline Value &operator=(const String &v)   {  _assign(v);  return(*this);  }
		// Special assignment: "clear" value. void return, of course. 
		inline void operator=(_None)  {  _zap();  }
		inline Value &operator=(const Value &v)
		{
			if(v.type==VTNone || v.type==VTError)  {  _zap(); type=v.type;  }
			else if(type==v.type)  {  val->forceset(v.val);  }
			else  {  if(val) delete val;  val=v.val->clone(); type=v.type;  }
			return(*this);
		}
		
		// Get value: 
		// These will abort in case *this holds a value of different type. 
		// Only automatic conversion from integer to scalar will be done. 
		// These functions return a copy of the value, so the original 
		// content will not be modified. 
		inline Integer GetInteger() const
			{ _typeassert(type,VTInteger); return(Integer(*(const Integer*)val)); }
		inline Range GetRange() const
			{ _typeassert(type,VTRange);   return(Range(*(const Range*)val));     }
		inline Vector GetVector() const
			{ _typeassert(type,VTVector);  return(Vector(*(const Vector*)val));   }
		inline Matrix GetMatrix() const
			{ _typeassert(type,VTMatrix);  return(Matrix(*(const Matrix*)val));   }
		inline String GetString() const
			{ _typeassert(type,VTString);  return(String(*(const String*)val));   }
		inline Scalar GetScalar() const
			{  if(type==VTInteger)  // Do "widening" if needed. 
			   { return(double(Scalar(((const Integer*)val)->val()))); }
			   _typeassert(type,VTScalar);  return(Scalar(*(const Scalar*)val));   }
		
		// Helper for shift operator: 
		template<class T>inline void _opshift(T &dest) const
			{  _typeassert(type,dest.vtype);  dest.set(*(const T*)val);  }
		friend Scalar &operator<<(Scalar &dest,const Value &src);
		
		// Functions like the GetXXX() above but returning the pointer 
		// to the object managed by this Value. READ-ONLY!
		// And of course, NO CONVERSION from integer to scalar. 
		inline const Integer *GetIntegerPtr() const
			{ _typeassert(type,VTInteger); return((const Integer*)val); }
		inline const Scalar *GetScalarPtr() const
			{ _typeassert(type,VTScalar); return((const Scalar*)val); }
		inline const Range *GetRangePtr() const
			{ _typeassert(type,VTRange);   return((const Range*)val);     }
		inline const Vector *GetVectorPtr() const
			{ _typeassert(type,VTVector);  return((const Vector*)val);   }
		inline const Matrix *GetMatrixPtr() const
			{ _typeassert(type,VTMatrix);  return((const Matrix*)val);   }
		inline const String *GetStringPtr() const
			{ _typeassert(type,VTString);  return((const String*)val);   }
		//-- Special version for templates: --
		template<class T> inline const T *GetPtr() const
			{  _typeassert(type,T::vtype);  return((const T*)val);  }
		//template<class T> inline const T *GetPtr(const T* /*for_gcc_sake*/) const
		//	{  _typeassert(type,T::vtype);  return((const T*)val);  }
		
		// Format value into string: 
		String ToString() const
			{  return(val ? val->ToString() : String("[none]"));  }
		
		// Check if value is null; that's the meaning of the ! operator: 
		// Returns 2 if this is a null ref. 
		int is_null() const
			{  return(val ? int(val->is_null()) : 2);  }
};

// The operator<< provide are short versions of the 
// Value::GetXXX() functions. 

inline Integer &operator<<(Integer &dest,const Value &src)
	{  src._opshift(dest);  return(dest);  }
inline Range &operator<<(Range &dest,const Value &src)
	{  src._opshift(dest);  return(dest);  }
inline Vector &operator<<(Vector &dest,const Value &src)
	{  src._opshift(dest);  return(dest);  }
inline Matrix &operator<<(Matrix &dest,const Value &src)
	{  src._opshift(dest);  return(dest);  }
inline String &operator<<(String &dest,const Value &src)
	{  src._opshift(dest);  return(dest);  }
inline Scalar &operator<<(Scalar &dest,const Value &src)
	{  if(src==Value::VTInteger)  // Do "widening" if needed. 
	   { dest.set(double(((const Integer*)(src.val))->val())); }
	   else src._opshift(dest);  return(dest); }

#endif  /* _VALTYPES_VALTYPES_H_ */
