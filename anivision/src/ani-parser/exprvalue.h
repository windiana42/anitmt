/*
 * ani-parser/exprvalue.h
 * 
 * The principal expression value used in AniVision. Can hold any 
 * type of value supported by the language. 
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

#ifndef _ANIVISION_ANI_EXPRVALUE_H_
#define _ANIVISION_ANI_EXPRVALUE_H_ 1

#include <ani-parser/exprtype.h>
#include <ani-parser/coreglue.h>
#include <ani-parser/vnotify.h>


// NOTE: These classes are a bit redundant because e.g.  
//       _InternalExprValueArray and ExprValue all contain pointers 
//       to some type of value objects, need allocation, dealloc 
//       assignment,... code. 
// The reason behind that is memory efficiency. I don't want to spend 
// lots of bytes for each single integer because that would dramatically 
// increase mem usage for simple integer arrays. Etc...

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-
// WARNING: JUST KEAP YOUR FINGERS OFF THIS FILE UNLESS YOU _R_E_A_L_L_Y_ 
//          KNOW WHAT YOU ARE DOING! 
//          THIS STUFF IS REAL PAIN TO DEBUG. 
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

namespace ANI
{

// This is provided by "coreglue.h": 
class AniGlue_ScopeInstanceBase;


// As defined below: 
class ScopeInstance;
class ExprValueArray;
class IndexHandler;
class InternalExprValue;
class ExprValue;

// Derived types from _InternalIndexHandler: 
class _InternalIndexHandler_Array;


// Internally uses AniGlue_ScopeInstanceBase as provided by 
// coreglue.h (and which does reference counting). 
// THIS IS the scope instance as meant to be used by the 
// ani-parser code. It is C++ safe and uses reference counting. 
// You can assign (=reference) different scopes here. 
class ScopeInstance
{
	public:
		// Special version of the new operator with placement as used 
		// by ExprValue: 
		void *operator new(size_t /*size*/,void *ptr)
			{  return(ptr);  }
		
		enum _None { None };
		enum _InternalCreate { InternalCreate };
	private:
		// Pointer to internally used instance; ref counting. 
		AniGlue_ScopeInstanceBase *sib;
		
		// Reference counting: 
		inline void _aqref()  {  if(sib)  sib->_aqref();  }
		inline void _deref()  {  if(sib)  sib->_deref();  }
		
	public:  _CPP_OPERATORS _CPP_OPERATORS_ARRAY
		// "Create new instance": Called by core after the passed 
		// internal instance has been created so that this class 
		// can take it over. (At this time, alloc has been done and 
		// vars have prototype val initialisation but constructor 
		// was not yet called.) 
		ScopeInstance(_InternalCreate,AniGlue_ScopeInstanceBase *_copysib)
			{  sib=_copysib;  _aqref();  }
		// Copy constructor: 
		ScopeInstance(const ScopeInstance &x)
			{  sib=x.sib;  _aqref();  }
		// Create scope instance which references nothing: 
		// Used by friends to create arrays. 
		ScopeInstance(_None=None)
			{  sib=NULL;  }
		// Destructor: 
		~ScopeInstance()
			{  _deref();  }
		
		// Assignment: 
		ScopeInstance &operator=(const ScopeInstance &b)
			{  _deref();  sib=b.sib;  _aqref();  return(*this);  }
		ScopeInstance &operator=(_None)
			{  _deref();  sib=NULL;  return(*this);  }
		
		// Get type: 
		void GetExprValueType(ExprValueType *store_here) const
			{  if(sib) sib->GetExprValueType(store_here);
				else store_here->SetNull();  }
		
		// Needed by core to get internal representation (which is, 
		// in turn, again provided by core): 
		const AniGlue_ScopeInstanceBase *GetASIB() const
			{  return(sib);  }
		AniGlue_ScopeInstanceBase *GetASIB()
			{  return(sib);  }
		
		// Check for NULL ref: 
		bool operator!() const
			{  return(!sib);  }
		
		// Return if this is logically 0 or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		int is_null() const
			{  return(sib ? 0 : 1);  }
		
		// Get string representation: 
		String ToString() const;
}__attribute__((__packed__));


// Interanally used by ExprValueArray; NO OTHER USE. 
class _InternalExprValueArray
{
	friend class ExprValueArray;
	friend class ExprValue;
	friend class _InternalIndexHandler_Array;
	private:
		// Reference counter: 
		int refcnt;
		
		// Number of elements in the array; may be 0. 
		ssize_t arr_size;
		// Type of elements in the array: 
		ExprValueType arr_type;
		// The actual array: may be NULL if arr_size=0. 
		union {
			// For EVT_POD, the following is valid: 
			Value *pod;   // [arr_size, allocated]
			// For EVT_AniScope, the following is the object/setting/
			// function/... referred to: (see <coreglue.h>)
			// Valid types are animation, object, setting (and value 
			// if pointers are allowed). 
			ScopeInstance *scope;  // [arr_size, referenced]
			// For EVT_Array, this is valid: 
			ExprValueArray *array;  // [arr_size, referenced]
		}__attribute__((__packed__));
		
		// Reference counting: 
		inline void _aqref()  {  ++refcnt;  }
		inline void _deref()  {  if(--refcnt<=0) delete this;  }
		
		// Clear array and set size to 0. 
		void _cleararray();
		
		// Never use these: 
		_InternalExprValueArray(const _InternalExprValueArray &);
		_InternalExprValueArray &operator=(const _InternalExprValueArray &);
		
		// Create array: arr_size must be >=0. 
		_InternalExprValueArray(ssize_t arr_size,const ExprValueType &arr_type,
			bool set_prototype_vals);
	public:  _CPP_OPERATORS
		~_InternalExprValueArray();
}__attribute__((__packed__));


// Array type: Uses reference counting; only exists ONCE in memory. 
// NOTE: The array type is NOT copy-on-write. You can modify it. 
// It just exists once in memory and won't "detach". 
// This is C++ safe and uses reference counting. You may assign 
// (=ref) different arrays here. You need to use an IndexHandler 
// ExprValue to modify the elements of ExprValueArray; (non-lvalue) 
// access can be obtained via IndexGet()/IndexSet(). 
class ExprValueArray
{
	friend class _InternalExprValueArray;
	friend class _InternalIndexHandler_Array;
	public:
		// Special version of the new operator with placement as used 
		// by ExprValue: 
		void *operator new(size_t /*size*/,void *ptr)
			{  return(ptr);  }
		
		enum _None { None };
	private:
		// Pointer to internally used (ref-counting) array class: 
		_InternalExprValueArray *iva;
		
		// Reference counting: 
		inline void _aqref()  {  if(iva) iva->_aqref();  }
		inline void _deref()  {  if(iva) iva->_deref();  }
		
	public:  _CPP_OPERATORS _CPP_OPERATORS_ARRAY
		// Used by friends to create arrays: Be careful with it. 
		ExprValueArray(_None=None)
			{  iva=NULL;  }
		// Create an array: arr_size must be >=0. 
		// Set prototype values if set_prototype_vals is set. 
		ExprValueArray(ssize_t arr_size,const ExprValueType &arr_type,
			bool set_prototype_vals)
			{  iva=new _InternalExprValueArray(arr_size,arr_type,
				set_prototype_vals);  iva->_aqref();  }
		// Reference an array: 
		ExprValueArray(const ExprValueArray &arr)
			{  iva=arr.iva;  _aqref();  }
		// Derefernece an array: 
		~ExprValueArray()
			{  _deref();  }
		
		// Reference array: 
		ExprValueArray &operator=(const ExprValueArray &arr)
			{  _deref();  iva=arr.iva;  _aqref();  return(*this);  }
		ExprValueArray &operator=(_None)
			{  _deref();  iva=NULL;  return(*this);  }
		
		// Get array size: (-1 for NULL ref)
		ssize_t GetSize() const
			{  return(iva ? iva->arr_size : (-1));  }
		
		// Get type (this is always some array if non-NULL-ref): 
		// Returns a NULL reference if this array is a NULL ref. 
		void GetExprValueType(ExprValueType *save_here) const
			{  if(iva) save_here->SetArray(iva->arr_type);
				else save_here->SetNull();  }
		
		// Return if this is logically 0 or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		int is_null() const
			{  return(iva ? 0 : 1);  }
		
		// Get string representation: 
		String ToString() const;
		
		// Check if element is null: 
		// True for out of range index. 
		int is_null_elem(ssize_t idx) const;
		
		// Index array: Set value. 
		// Array stays unmodified as long as retval !=0. 
		// NOTE: TYPES MUST MATCH. THIS IS NOT CHECKED BY IndexSet(). 
		// Return value: 
		//  2 -> cannot assign Unknown/Void array. 
		//  0 -> OK
		// -2 -> index out of range
		// -3 -> index negative
		// -4 -> this is a NULL ref
		int IndexSet(ssize_t idx,const ExprValue &store_this);
		
		// Index array: Get value. 
		// Get element with passed index: 
		// If the return value is 0, the value is stored in store_here; 
		// otherwise it is left untouched. 
		// Return value: 
		//  0 -> OK
		// -2 -> index out of range
		// -3 -> index negative
		// -4 -> this is a NULL ref
		// NOT LVALUE CAPABLE. COPIES VALUE -- NO INDEX HANDLER RETURN. 
		int IndexGet(ssize_t idx,ExprValue *store_here);
		
}__attribute__((__packed__));


// This is the internal class; use IndexHandler instead. 
class _InternalIndexHandler
{
	friend class IndexHandler;
	friend class InternalExprValueArray;
	friend class InternalExprValue;
	protected:
		InternalExprValue *iev;
		union {
			ssize_t idx;                      // if is_matrix_idx==0
			struct { short int idxR,idxC; };  // if is_matrix_idx==1
		}__attribute__((__packed__));
		int is_matrix_idx : 1;
	private:
		int refcnt : (sizeof(int)*8-1);
		
		// Reference counting: 
		inline void _aqref()  {  ++refcnt;  }
		inline void _deref()  {  if(--refcnt<=0) delete this;  }
		
		// The details are hidden in the (more internal) 
		// derived classes. (Derived classes are only 
		// unsed internally and hidden here.) 
		
		// Do not use these: 
		_InternalIndexHandler &operator=(const _InternalIndexHandler &);
		_InternalIndexHandler(const _InternalIndexHandler &);
	
	protected:
		// Never use this constructor directly -- you need 
		// to create a derived class, e.g. by using the 
		// InternalExprValue(IndexArray/IndexPOD/..,...) constructor. 
		_InternalIndexHandler(InternalExprValue *_iev,ssize_t _idx,
			bool matrix=0);
	public:  _CPP_OPERATORS
		// No constructor -- see above. 
		virtual ~_InternalIndexHandler();
		
		// Virtual interface: 
		virtual ExprValueTypeID GetExprValueTypeID() const;
		virtual void GetExprValueType(ExprValueType *store_here) const;
		
		// Return if this is logically 0 or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		virtual int is_null() const;
		
		// Get string representation: 
		virtual String ToString() const;
		
		// See IndexHandler::assign(): 
		virtual void assign(const Value &v);
		virtual void assign(const ScopeInstance &v);
		virtual void assign(const ExprValueArray &v);
		
		// See IndexHandler::Get*(): 
		virtual void GetPOD(Value *store_here) const;
		virtual void GetAniScope(ScopeInstance *store_here) const;
		virtual void GetArray(ExprValueArray *store_here) const;
		virtual int GetPODInt() const;
		virtual double GetPODDouble() const;
}__attribute__((__packed__));


// Index handler. 
// When indexing arrays or PODs (vector,...), I cannot copy the 
// value and return it because the result would no longer be 
// lvalue-capable. Hence, this IndexHandler is returned as ExprValue. 
// The IndexHandler is an object created when indexing and stores just 
// the information needed to access the correct member. 
// NOTE: For member select of scopes, this is not needed as the 
//       scopes contain complete variables as members; member select 
//       from PODs (aka vector.x) is included in "indexing" above. 
// IndexHandler uses ref counting and is C++ safe. 
class IndexHandler
{
	friend class _InternalIndexHandler_Array;
	friend class InternalExprValue;
	public:
		// Special version of the new operator with placement as used 
		// by ExprValue: 
		void *operator new(size_t /*size*/,void *ptr)
			{  return(ptr);  }
	private:
		// Internally used class; does ref counting. 
		_InternalIndexHandler *iih;
		
		// Reference counting: 
		inline void _aqref()  {  if(iih) iih->_aqref();  }
		inline void _deref()  {  if(iih) iih->_deref();  }
		
		void _failassert() const;
		inline void _assertiih() const
			{  if(!iih) _failassert();  }
		
		// This is currently private for safety: 
		IndexHandler()
			{  iih=NULL;  }
		// Used by InternalExprValue: (_iih must have been 
		// allocated via operator new). 
		IndexHandler(_InternalIndexHandler *_iih)
			{  iih=_iih;  _aqref();  }
	public:  _CPP_OPERATORS
		// Copy constructor (reference): 
		IndexHandler(const IndexHandler &v)
			{  iih=v.iih;  _aqref();  }
		~IndexHandler()
			{  _deref();  }
		
		// Assignment (reference): 
		IndexHandler &operator=(const IndexHandler &v)
			{  _deref();  iih=v.iih;  _aqref();  return(*this);  }
		
		// Get complete type and only type id of indexed value: 
		void GetExprValueType(ExprValueType *store_here) const
			{  if(iih) iih->GetExprValueType(store_here);
				else store_here->SetNull(); }
		ExprValueTypeID GetExprValueTypeID() const
			{  return(iih ? iih->GetExprValueTypeID() : EVT_NULL);  }
		
		// Get POD value result. Return null ref if invalid. 
		// Result can only be used as rvalue, NOT as lvalue. 
		void GetPOD(Value *store_here) const
			{  _assertiih();  iih->GetPOD(store_here);  }
		// Analoguous functions for ani scopes and arrays: 
		void GetAniScope(ScopeInstance *store_here) const
			{  _assertiih();  iih->GetAniScope(store_here);  }
		void GetArray(ExprValueArray *store_here) const
			{  _assertiih();  iih->GetArray(store_here);  }
		// Special versions for C++ PODs: 
		int GetPODInt() const
			{  _assertiih();  return(iih->GetPODInt());  }
		double GetPODDouble() const
			{  _assertiih();  return(iih->GetPODDouble());  }
		
		// Return if this is logically 0 or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		int is_null() const
			{  return(iih ? iih->is_null() : 2);  }
		
		// Get string representation: 
		String ToString() const;
		
		// Assign value; see ExprValue::assign(). 
		void assign(const Value &v)
			{  _assertiih();  iih->assign(v);  }
		void assign(const ScopeInstance &v)
			{  _assertiih();  iih->assign(v);  }
		void assign(const ExprValueArray &v)
			{  _assertiih();  iih->assign(v);  }
		// NOTE (FIXME): Could supply own versions for 
		// Integer, Scalar, Vector, Range, Matrix, String to 
		// avoid using temporary Value. 
		// Probably sufficient for Integer and Scalar. 
}__attribute__((__packed__));


// Compute the max of 4 values: 
/*#define _MAX4(a,b,c,d) \
	a>=b && a>=c && a>=d ? a : ( \
	b>=c && b>=d ? b : ( \
	c>=d ? c : d ))*/


// Okay, all the above types are the actual "types" provided by the 
// language. They are C++ safe and use ref counting (scope, array) 
// or copying ([pod] value). 
// Still need one central type which can hold any of these types. 
// That is InternalExprValue. InternalExprValue is NOT C++ safe and 
// provides ref-counting facilities. 
// ExprValue is C++ safe and uses a pointer to InternalExprValue (doing 
// ref counting). Now, while InternalExprValue actually stores the 
// value, ExprValue refers to exactly ONE InternalExprValue and 
// can either reference or copy an InternalExprValue. In the first 
// case, it stays an lvalue, in the latter one it does not: 
// 
// Example: 
//   ExprValue a(Value(10));
//   ExprValue b;
//   b.reference(a);
//   // a and b now refer to the same value. 
//   b.assign(Value(17));  // -> a and b are now 17
//   a.assign(Value(20));  // -> a and b are now 20
// -----
//   ExprValue a(Value(10));
//   ExprValue b;
//   b.assign(a);
//   // a and b are noe 10 but different vars
//   b.assign(Value(17));  // -> a=10, b=17
//   a.assign(Value(20));  // -> a=20, b=17

struct _IEV_Namespace
{
	enum _ExprValueTypeID
	{
		EVT_Unknown=ANI::EVT_Unknown,
		EVT_Void=ANI::EVT_Void,
		EVT_POD=ANI::EVT_POD,
		EVT_AniScope=ANI::EVT_AniScope,
		EVT_Array=ANI::EVT_Array,
		EVT_IndexHdl=100,
	};
	// Used to select constructor: 
	enum _IndexArray  { IndexArray   };
	enum _IndexPOD    { IndexPODMatrix,IndexPODVector,
	                    IndexPODRange,IndexPODString };
	enum _UnknownVal  { CUnknownType };
	enum _VoidVal     { CVoidType    };
	enum _None        { None         };
	enum _Copy        { Copy         };
	enum _Prototype   { Prototype    };
};

class InternalExprValue : public _IEV_Namespace
{
	friend class ExprValueArray;
	friend class ExprValue;
	friend class _InternalIndexHandler;
	friend class _InternalIndexHandler_Array;
	private:
		// This is the memory used for the stored types: 
		// The complete union should be sizeof(int) which is the 
		// size of the used ref counting values 
		// (i.e. sizeof(refcnt)=sizeof(int)). 
		union
		{
			char _pod_mem[sizeof(Value)];
			char _scope_mem[sizeof(ScopeInstance)];
			char _array_mem[sizeof(ExprValueArray)];
			char _ihdl_mem[sizeof(IndexHandler)];
		}__attribute__((__packed__));
		
		// Value notifier list, NULL if not present. 
		ValueNotifierContainer *vnc;
		
		// The valid pointer depends on the type. 
		enum _ExprValueTypeID _evt;
		
		// Reference counter: 
		int refcnt;
		
		// Reference counting: 
		inline void _aqref()  {  ++refcnt;  }
		inline void _deref()  {  if(--refcnt<=0) delete this;  }
		
		// Only one of these is valid. 
		// Return the pointer to the value as "stored" in the union 
		// above. ONLY USE FOR THE CORRECT _evt. 
		// In this order: EVT_POD, EVT_AniScope, EVT_Array. 
		Value *pod()             {  return((Value*)_pod_mem);  }
		ScopeInstance *scope()   {  return((ScopeInstance*)_scope_mem);  }
		ExprValueArray *array()  {  return((ExprValueArray*)_array_mem);  }
		IndexHandler *ihdl()     {  return((IndexHandler*)_ihdl_mem);  }
		// Const version: 
		const Value *pod()            const { return((Value*)_pod_mem); }
		const ScopeInstance *scope()  const { return((ScopeInstance*)_scope_mem); }
		const ExprValueArray *array() const { return((ExprValueArray*)_array_mem); }
		const IndexHandler *ihdl()    const { return((IndexHandler*)_ihdl_mem); }
		
		// Internally used by constructor/destructor & assignment: 
		void _destroy();
		
		// Call change notifiers in notifier list: 
		inline void _EmitChangeNotify()
			{  if(vnc) vnc->EmitVN_Change();  }
		
		// Copy; get new copy and de-lvaluize the passed value. 
		// ONLY CALL FOR EVT_Unknown. 
		void _copy(const Value &v)
			{  _evt=EVT_POD;  new(pod()) Value(v);  }
		void _copy(const ScopeInstance &v)
			{  _evt=EVT_AniScope;  new(scope()) ScopeInstance(v);  }
		void _copy(const ExprValueArray &v)
			{  _evt=EVT_Array;  new(array()) ExprValueArray(v);  }
		void _copy(const IndexHandler &v)
			{  _evt=EVT_IndexHdl;  new(ihdl()) IndexHandler(v);  }
		void _copy(const InternalExprValue *v);
		// Special version: 
		void _copy(_InternalIndexHandler *_iih)
			{  _evt=EVT_IndexHdl;  new(ihdl()) IndexHandler(_iih);  }
		
		// Used by assign(): 
		// Template is private; all allowed versions are public. POD ONLY. 
		template<class T>void _assign(const T &v)
		{
			// Change the value inside Value *pod(): 
			if(_evt==EVT_POD)
			{  pod()->operator=(v);  }
			else if(_evt==EVT_IndexHdl)
			{  ihdl()->assign(v);  }
			//else if(_evt==EVT_Unknown)
			//{  _copy(v);  }  // Copy value; do NOT reference it. 
			else _assign_failassert();
			_EmitChangeNotify();
		}
		void _assign_failassert();
		
		// FORBIDDEN; Use explicit functions/constructors. 
		InternalExprValue(const InternalExprValue &v);
		InternalExprValue &operator=(const InternalExprValue &v);
		// Array/vector/matrix subscription: 
		InternalExprValue(_IndexArray,InternalExprValue *iev,ssize_t idx);
		InternalExprValue(_IndexPOD,InternalExprValue *iev,ssize_t idx);
	public:  _CPP_OPERATORS
		// Copy the passed values: 
		InternalExprValue(_Copy,const Value &v)
			{  _copy(v);  refcnt=0;  vnc=NULL;  }
		InternalExprValue(_Copy,const ScopeInstance &v)
			{  _copy(v);  refcnt=0;  vnc=NULL;  }
		InternalExprValue(_Copy,const ExprValueArray &v)
			{  _copy(v);  refcnt=0;  vnc=NULL;  }
		InternalExprValue(_Copy,const InternalExprValue *v)
			{  _copy(v);  refcnt=0;  vnc=NULL;  }
		// Special: create unknown/void value: 
		// First one used for arrays of ExprValues. 
		InternalExprValue(_UnknownVal=CUnknownType)
			{  _evt=EVT_Unknown;  refcnt=0;  vnc=NULL;  }
		InternalExprValue(_VoidVal)
			{  _evt=EVT_Void;  refcnt=0;  vnc=NULL;  }
		~InternalExprValue();
		
		// Get complete expr value type: 
		void GetExprValueType(ExprValueType *store_here) const;
		// Get the type ID: 
		ExprValueTypeID GetExprValueTypeID() const
			{  return(_evt==EVT_IndexHdl ? ihdl()->GetExprValueTypeID() : 
				ExprValueTypeID(_evt));  }
		
		// If this is an EVT_POD/EVT_AniScope/EVT_Array (according to 
		// GetExprValueTypeID() (!)), get the value. 
		// Will do indexing if needed. -- NO LVALUE!
		void GetPOD(Value *store_here) const
			{  if(_evt==EVT_POD)  *store_here=*pod(); else 
			   if(_evt==EVT_IndexHdl)  ihdl()->GetPOD(store_here); else
			   *store_here=Value::None;  }
		void GetAniScope(ScopeInstance *store_here) const
			{  if(_evt==EVT_AniScope)  *store_here=*scope(); else
			   if(_evt==EVT_IndexHdl)  ihdl()->GetAniScope(store_here); else
			   *store_here=ScopeInstance::None;  }
		void GetArray(ExprValueArray *store_here) const
			{  if(_evt==EVT_Array)  *store_here=*array(); else
			   if(_evt==EVT_IndexHdl)  ihdl()->GetArray(store_here); else
			   *store_here=ExprValueArray::None;  }
		// Special versions for C PODs: 
		void _getpod_failassert() const;
		int GetPODInt() const
			{  return(_evt==EVT_POD ? pod()->GetPtr<Integer>()->val() : 
				(_evt==EVT_IndexHdl ? ihdl()->GetPODInt() : 
					(_getpod_failassert(),-1) ));  }
		double GetPODDouble() const
			{  return(_evt==EVT_POD ? pod()->GetPtr<Scalar>()->val() : 
				(_evt==EVT_IndexHdl ? ihdl()->GetPODDouble() : 
					(_getpod_failassert(),-1.0) ));  }
		
		// Assign value. Not reference but "copy". 
		// For POD, that means: copy content, for scope and array 
		// that means: reference a different ScopeInstance/ExprValueArray 
		// (i.e. copy "pointer" because in the language, scope and array 
		// are pointer-like). 
		void assign(const InternalExprValue *v);
		void assign(const Value &v)    {  _assign(v);  }
		void assign(const ScopeInstance &v);
		void assign(const ExprValueArray &v);
		// Even more: 
		void assign(int v)             {  _assign(v);  }
		void assign(double v)          {  _assign(v);  }
		void assign(const Integer &v)  {  _assign(v);  }
		void assign(const Scalar &v)   {  _assign(v);  }
		void assign(const Range &v)    {  _assign(v);  }
		void assign(const Vector &v)   {  _assign(v);  }
		void assign(const Matrix &v)   {  _assign(v);  }
		void assign(const String &v)   {  _assign(v);  }
		
		// Check for unknown type: 
		//bool operator==(_UnknownVal)
		//	{  return(_evt==EVT_Unknown);  }
		
		// This checks if the type is LOGICALLY null or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		int is_null() const;
		
		// Get string representation of value: 
		String ToString() const;
		
		// (Un)Install ValueNotifyHandler; see class ValueNotifyHandler. 
		// NEVER USE DIRECTLY. 
		int InstallVNH(ValueNotifyHandler *hdl,void *hook);
		int UninstallVNH(ValueNotifyHandler *hdl)
			{  return(vnc ? vnc->UninstallNotifier(hdl) : 1);  }
}__attribute__((__packed__));


class ExprValue : public _IEV_Namespace
{
	friend class IndexHandler;
	private:
		// Pointer to internally used value; ref counting...
		InternalExprValue *iev;
		
		// Reference counting: 
		inline void _aqref()  {  if(iev) iev->_aqref();  }
		inline void _deref()  {  if(iev) iev->_deref();  }
		
		void _fail_ievassert() const;
		inline void _ievassert(bool x) const
			{  if(!x)  _fail_ievassert();  }
		
		// Internally used by "copy" constructur and assign: 
		// ONLY CALL FOR NULL REFS. 
		void _copy(const Value &v)
			{  iev=new InternalExprValue(Copy,v);  _aqref();  }
		void _copy(const ScopeInstance &v)
			{  iev=new InternalExprValue(Copy,v);  _aqref();  }
		void _copy(const ExprValueArray &v)
			{  iev=new InternalExprValue(Copy,v);  _aqref();  }
		void _copy(const ExprValue &v)  {  _ievassert(v.iev);
			   iev=new InternalExprValue(Copy,v.iev);  _aqref();  }
		
	public:  _CPP_OPERATORS _CPP_OPERATORS_ARRAY
		// Constuct NULL ref: 
		ExprValue(_None=None)
			{  iev=NULL;  }
		// The usual reference suff: 
		ExprValue(const ExprValue &v)
			{  iev=v.iev;  _aqref();  }
		// Create new ExprValue holding the passed value. 
		ExprValue(_Copy,const Value &v)           {  _copy(v);  }
		ExprValue(_Copy,const ScopeInstance &v)   {  _copy(v);  }
		ExprValue(_Copy,const ExprValueArray &v)  {  _copy(v);  }
		ExprValue(_Copy,const ExprValue &v)       {  _copy(v);  }
		// Special ones: 
		ExprValue(_UnknownVal)
			{  iev=new InternalExprValue(CUnknownType);  _aqref();  }
		ExprValue(_VoidVal)
			{  iev=new InternalExprValue(CVoidType);  _aqref();  }
		// Get prototype of passed type, i.e. value of passed type 
		// set up to NULL or 0 or 0.0 or <0,0,0>, etc. 
		ExprValue(_Prototype,const ExprValueType &evt);
		// Array subscription: 
		ExprValue(_IndexArray,const ExprValue &v,ssize_t idx)  { _ievassert(v.iev);
			  iev=new InternalExprValue(IndexArray,v.iev,idx); _aqref(); }
		// Vector/matrix/range subscription: 
		ExprValue(_IndexPOD ip,const ExprValue &v,ssize_t idx)  { _ievassert(v.iev);
			  iev=new InternalExprValue(ip,v.iev,idx); _aqref(); }
		// Destructor (easy): 
		~ExprValue()
			{  _deref();  }
		
		// Assignment (ref): 
		ExprValue &operator=(const ExprValue &v)
			{  _deref();  iev=v.iev;  _aqref();  return(*this);  }
		// Special assignment: 
		ExprValue &operator=(_None)
			{  _deref();  iev=NULL;  return(*this);  }
		
		// Check for NULL ref: 
		bool operator!() const
			{  return(!iev);  }
		bool operator==(_None) const
			{  return(!iev);  }
		
		// Get complete expr value type: 
		void GetExprValueType(ExprValueType *store_here) const
			{  if(iev) iev->GetExprValueType(store_here);
				else store_here->SetNull();  }
		// Get the type ID: 
		ExprValueTypeID GetExprValueTypeID() const
			{  return(iev ? iev->GetExprValueTypeID() : EVT_NULL);  }
		
		// If this is an EVT_POD/EVT_AniScope/EVT_Array (according to 
		// GetExprValueTypeID() (!)), get the value. 
		// (Will do indexing if needed.)
		// NO LVALUE!
		void GetPOD(Value *store_here) const
			{  if(iev) iev->GetPOD(store_here);
			   else *store_here=Value::None;  }
		void GetAniScope(ScopeInstance *store_here) const
			{  if(iev) iev->GetAniScope(store_here);
			   else *store_here=ScopeInstance::None;  }
		void GetArray(ExprValueArray *store_here) const
			{  if(iev) iev->GetArray(store_here);
			   else *store_here=ExprValueArray::None;  }
		// Special versions for C PODs: 
		int GetPODInt() const
			{  _ievassert(iev);  return(iev->GetPODInt());  }
		double GetPODDouble() const
			{  _ievassert(iev);  return(iev->GetPODDouble());  }
		
		// Assign value to stored value. 
		// If you simply use ExprValue1 = ExprValue2, then the value 2 
		// is referenced by the value 1. 
		// If ExprValue1 is actually an lvalue (which is assumed), then 
		// assign() will take the value of the passed argument and assign it 
		// to *this value. This is only valid if the types match, otherwise 
		// an assertion will fail. 
		// Type match: you may not assign with NULL refs. 
		void assign(const ExprValue &v)
			{  iev ? (_ievassert(v.iev),iev->assign(v.iev)) : _copy(v);  }
		// This are special versions which can be used instead of creating a 
		// temporary ExprValue: 
		void assign(const Value &v)    { iev ? iev->assign(v) : _copy(v); }
		void assign(const ScopeInstance &v)  { iev ? iev->assign(v) : _copy(v); }
		void assign(const ExprValueArray &v) { iev ? iev->assign(v) : _copy(v); }
		// Even more: 
		void assign(int v)             { iev ? iev->assign(v) : _copy(v); }
		void assign(double v)          { iev ? iev->assign(v) : _copy(v); }
		void assign(const Integer &v)  { iev ? iev->assign(v) : _copy(v); }
		void assign(const Scalar &v)   { iev ? iev->assign(v) : _copy(v); }
		void assign(const Range &v)    { iev ? iev->assign(v) : _copy(v); }
		void assign(const Vector &v)   { iev ? iev->assign(v) : _copy(v); }
		void assign(const Matrix &v)   { iev ? iev->assign(v) : _copy(v); }
		void assign(const String &v)   { iev ? iev->assign(v) : _copy(v); }
		
		// This checks if the type is LOGICALLY null or not. 
		// THIS IS LIKE AN INVERTED BOOL CONVERSION. 
		int is_null() const
			{  return(iev ? iev->is_null() : 2);  }
		
		// Get string representation of value: 
		String ToString() const;
		
		// (Un)Install ValueNotifyHandler. DO NOT USE DIRECTLY. 
		// USE ValueNotifyHandler::VN(Un)Install() INSTEAD. 
		// Return value: See ValueNotifyHandler. 
		int InstallVNH(ValueNotifyHandler *hdl,void *hook)
			{  return(iev ? iev->InstallVNH(hdl,hook) : -1);  }
		int UninstallVNH(ValueNotifyHandler *hdl)
			{  return(iev ? iev->UninstallVNH(hdl) : 1);  }
}__attribute__((__packed__));


/*------------------------------------------------------------------------------
 * Application example:
 * (Yeah... nobody would use exactly that layout -- this is ExprValue demo 
 * code and not tree demo code.)
 * 
 * ExprValue TreeNode::eval(EvalInfo *info)
 * {
 *     if(nodetype==NodeType.:Value)
 *     {  return(val.ev);  }
 *     else if(nodetype==NodeType::OperatorPlus)
 *     {
 *         EvprValue a=child[0]->eval(info);
 *         ExprValue b=child[1]->eval(info);
 *         ExprValue res=<compute a+b>;
 *         return(res);
 *     }
 *     else if(nodetype==NodeType::Assignment)
 *     {  child[0]->eval(info).assign(child[1]->eval(info));  }
 *     else if(nodetype==NodeType::OperatorSubscript)
 *     {
 *         ExprValue i=child[1]->eval(info);
 *         int idx=i.PODValue().GetValuePtr()->GetIntegerPtr()->val();
 *         // check idx...
 *         ExprValue a=child[0]->eval(info);
 *         return(ExprValue(ExprValue::IndexArray,a,idx));
 *     }
 *     // ...
 * }
 *----------------------------------------------------------------------------*/

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_EXPRVALUE_H_ */
