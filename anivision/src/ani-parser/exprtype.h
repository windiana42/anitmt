/*
 * ani-parser/exprtype.h
 * 
 * Expression type representation. 
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

#ifndef _ANIVISION_ANI_EXPRTYPE_H_
#define _ANIVISION_ANI_EXPRTYPE_H_ 1

#include <valtype/valtypes.h>
#include <ani-parser/assfunc.h>

namespace ANI
{

// This is provided by "coreglue.h": 
class AniGlue_ScopeBase;


// Expression valtypes to build type-fixed trees: */
enum ExprValueTypeID
{
	EVT_NULL=-2,     // Only for special cases. 
	EVT_Unknown=-1,  // type unknown
	EVT_Void=0,      // void type
	EVT_POD,         // POD-type (int,scalar,range,vector,matrix,string)
	EVT_AniScope,    // animation, setting, object, function
	EVT_Array,       // array type
};


// Internally used class; not C++ save; reference counting. 
// ONLY FOR INERNAL USE BY ExprValueType. 
class _InternalExprValueType
{
	friend class ExprValueType;
	friend int _LAF_GetAssFunc(
		const _InternalExprValueType *lval,const _InternalExprValueType *rval,
		AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op);
	friend int _LAIDF_GetIncDecFunc(
		const _InternalExprValueType *ltype,
		IncDecAssFuncBase **ret_incdecfunc,int inc_or_dec);
	enum _ArrayType { CArrayType };
	private:
		// Reference counter: 
		int refcnt;
		
		// Major type id: 
		ExprValueTypeID type_id;  // !=EVT_NULL always. 
		
		// Type information detail: 
		union
		{
			// For EVT_POD, the following is valid: 
			Value::CompleteType pod;
			// For EVT_AniScope: 
			// This can be casted to AniAnimation, AniSetting, ...
			// depending on scope->ASType(). 
			// Especially, this also provides the "function" for the 
			// function call operator. 
			struct {
				AniGlue_ScopeBase *scope;
				int scope_flags;  // UNUSED currently. 
			};
			// For EVT_Array: 
			// Points to array element type, e.g. an array of integers 
			// has EVT_Array and if you follow arrtype you get 
			// {EVT_POD,VTInteger}. 
			struct {
				//ssize_t arr_size;  // Must be SIGNED -- UNUSED. 
				_InternalExprValueType *arrtype;
			};
		};
		
		inline void _aqref()  {  ++refcnt;  }
		inline void _deref()  {  if(--refcnt<=0)  delete this;  }
		
		// Compare two types exactly: Requires b!=NULL. 
		// NOTE that two unknown types are never equal. 
		bool _exact_compare(const _InternalExprValueType *b);
		
		// Get string representation of type: 
		String _typestring() const;
		
		// See IsIncomplete(). 
		int _is_incomplete() const;
		
		// See CanBoolConvert(): 
		int _can_boolcv() const;
		
		// Be careful not to set illegal values here!
		_InternalExprValueType(ExprValueTypeID _tid) : 
			refcnt(0),type_id(_tid) {}
		_InternalExprValueType(const Value::CompleteType &_pod) : 
			refcnt(0),type_id(EVT_POD)  {  pod=_pod;  }
		_InternalExprValueType(AniGlue_ScopeBase *_scope,int _scope_flags=0) : 
			refcnt(0),type_id(EVT_AniScope)
			{  scope=_scope;  scope_flags=_scope_flags;  }
		// For array only. 
		_InternalExprValueType(_ArrayType,
			_InternalExprValueType *_arrtype) : 
			refcnt(0),type_id(EVT_Array)
			{  arrtype=_arrtype;  arrtype->_aqref();  }
		
		// Copy constructor: MAY NEVER BE USED. 
		_InternalExprValueType(const _InternalExprValueType &);
		// Assignment: may never be used: 
		_InternalExprValueType &operator=(const _InternalExprValueType &);
		
		// Get a fresh copy of this type: 
		_InternalExprValueType *clone();
		
	public:  _CPP_OPERATORS
		~_InternalExprValueType();
		
		static int _is_pod_incomplete(const Value::CompleteType &ct);
};

// This type is C++-save, and should be fairly fast because 
// it uses reference counting. 
// THIS IS THE COMPLETE EXPRESSION VALUE TYPE DESCRIPTION. 
class ExprValueType
{
	friend class _InternalExprValueType;
	friend int LookupAssignmentFunction(
		const ExprValueType &lval,const ExprValueType &rval,
		AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op);
	friend int LookupAssIncDecFunction(
		const ExprValueType &type,
		IncDecAssFuncBase **ret_incdecfunc,int inc_or_dec);
	private:
		// Some types which are always needed: 
		static _InternalExprValueType *S_unknown_type;
		static _InternalExprValueType *S_void_type;
		static _InternalExprValueType *S_pod_integer;
		static _InternalExprValueType *S_pod_scalar;
		static _InternalExprValueType *S_pod_range;
		static _InternalExprValueType *S_pod_string;
	public:
		// Must be called in the beginning: 
		static void _InitStaticVals();
		// ...and at the end: 
		static void _CleanupStaticVals();
	public:
		enum _VoidType    { CVoidType };
		enum _UnknownType { CUnknownType };
		enum _ArrayType   { CArrayType };
		enum _NullRef     { CNullRef };
	private:
		// Pointer to internal data structure. May be NULL. 
		_InternalExprValueType *ivt;
		
		inline void _aqref()  {  if(ivt) ivt->_aqref();  }
		inline void _deref()  {  if(ivt) ivt->_deref();  }
		
		// Get private copy: 
		void _detach()
			{	if(!ivt || ivt->refcnt==1)  return;
				ivt->_deref(); ivt=ivt->clone(); ivt->_aqref();  }
		
		// These are the allocation functions for the internal data 
		// structures. These functions may be smart and return already 
		// existing references from somewhere else. 
		static _InternalExprValueType *_AllocIVT(const Value::CompleteType &);
		static _InternalExprValueType *_AllocIVT(AniGlue_ScopeBase *,int sf);
		static _InternalExprValueType *_AllocIVT(_ArrayType,
			const ExprValueType &_arrtype);
		
		// Internally used constructor: 
		inline ExprValueType(_InternalExprValueType *_ivt)
			{  ivt=_ivt;  _aqref();  }
	public:  _CPP_OPERATORS  _CPP_OPERATORS_ARRAY
		// Construct unknown type: 
		inline ExprValueType(_UnknownType=CUnknownType)
			{  ivt=S_unknown_type;  ivt->_aqref();  }
		// Construct void type: 
		inline ExprValueType(_VoidType)
			{  ivt=S_void_type;  ivt->_aqref();  }
		// Construct POD type: 
		ExprValueType(const Value::CompleteType &_pod)
			{  ivt=_AllocIVT(_pod);  _aqref();  }
		// Construct ani scope type: 
		inline ExprValueType(AniGlue_ScopeBase *_scope,int _scope_flags=0)
			{  ivt=_AllocIVT(_scope,_scope_flags);  ivt->_aqref();  }
		// Construct an array type with passed type as array elements: 
		inline ExprValueType(_ArrayType,const ExprValueType &_arrtype)
			{  ivt=_AllocIVT(CArrayType,_arrtype);  ivt->_aqref();  }
		// Copy constructor: 
		inline ExprValueType(const ExprValueType &e)
			{  ivt=e.ivt;  _aqref();  }
		// This is for "internal" use only. Well, sort of...
		inline ExprValueType(_NullRef)
			{  ivt=NULL;  }
		// Destructor...
		inline ~ExprValueType()
			{  _deref();  ivt=NULL;  }
		
		// Assignment operator: 
		inline ExprValueType &operator=(const ExprValueType &e)
			{  _deref();  ivt=e.ivt;  _aqref();  return(*this);  }
		
		// Check for NULL ref: 
		inline bool operator!() const
			{  return(!ivt);  }
		
		// Exactly compare types: 
		// NOTE that two unknown types are never equal. 
		bool IsEqualTo(const ExprValueType &b) const
			{  return((ivt==b.ivt) ? 1 : 
				((ivt && b.ivt) ? ivt->_exact_compare(b.ivt) : 0));  }
		
		// Check if one can assign the passed type to this one. 
		// Return value: 
		//  0 -> no (or NULL ref or void type)
		//  1 -> with type conversion (WIDENING, ONLY)
		//  2 -> yes, directly
		// -2 -> type incomplete or AniIncomplete scope 
		// If you pass non-NULL for the AssignmentFuncBase pointer, 
		// a handler is returned; see LookupAssignmentFunction() in 
		// assfunc.h for details (e.g. deallocation). 
		int CanAssignType(const ExprValueType &r_evt,
			AssignmentFuncBase **ret_assfunc,AssignmentOpID ass_op=AO_Set) const
			{  return(ANI::LookupAssignmentFunction(*this,r_evt,
				ret_assfunc,ass_op));  }
		
		// Can convert this type to bool for if(), while(),...
		// Return value: 
		//  0 -> no (or NULL ref)
		//  1 -> yes
		int CanBoolConvert() const
			{  return(ivt ? ivt->_can_boolcv() : 0);  }
		
		// Can use increment/decrement operators on that type?
		// If ret_aid_func is non-NULL, a handler is returned; see 
		// LookupAssIncDecFunction() in assfunc.h for details 
		// (e.g. deallocation). 
		// inc_dec specifies if increment (+1) or decrement (-1) is 
		// wanted and is only needed if ret_aid_func!=NULL. 
		// Return value: 
		//  0 -> no (or NULL ref)
		//  1 -> yes
		int CanIncDec(IncDecAssFuncBase **ret_aid_func=NULL,int inc_dec=0) const
			{  return(LookupAssIncDecFunction(*this,ret_aid_func,inc_dec));  }
		
		// Query functions: 
		// Returns type or EVT_NULL for NULL ref. 
		inline ExprValueTypeID TypeID() const
			{  return(ivt ? ivt->type_id : EVT_NULL);  }
		
		// Get string representation of complete type: 
		String TypeString() const;
		
		// THE RETURN VALUES ARE CONST AND MEANT DO BE CONST!
		// Return POD type or NULL if this is not a POD type. 
		inline const Value::CompleteType *PODType() const
			{  return((ivt && ivt->type_id==EVT_POD) ? &ivt->pod : NULL);  }
		// Return AniGlue_ScopeBase node or NULL if not object type: 
		inline AniGlue_ScopeBase *AniScope() const
			{  return((ivt && ivt->type_id==EVT_AniScope) ? ivt->scope : NULL);  }
		// Get scope flags; only call if it is a scope; will -1 return 
		// otherwise. 
		inline int AniScopeFlags() const
			{  return((ivt && ivt->type_id==EVT_AniScope) ? 
				ivt->scope_flags : (-1));  }
		// Return array type subnode or NULL ref if not array type: 
		inline ExprValueType ArrayType() const
			{  return(ExprValueType((ivt && ivt->type_id==EVT_Array) ? 
				ivt->arrtype : NULL));  }
		
		// Check if the type is incomplete: 
		// Returns 1 if so and 0 if complete. 
		// A type is incomplete if the vector or matrix size is not 
		// yet known (=-1) -- because the constant folding has not yet 
		// been done. 
		// Return value: 0 -> complete; 1 -> incomplete; -2 -> null ref
		int IsIncomplete() const
			{  return(ivt ? ivt->_is_incomplete() : (-2));  }
		
		// Set type: 
		void SetNull()
			{  _deref();  ivt=NULL;  }
		void SetUnknown()
			{  _deref();  ivt=S_unknown_type;  ivt->_aqref();  }
		void SetVoid()
			{  _deref();  ivt=S_void_type;  ivt->_aqref();  }
		void SetPOD(const Value::CompleteType &_pod);
		void SetAniScope(AniGlue_ScopeBase *_scope,int _scope_flags=0);
		void SetArray(const ExprValueType &_arrtype);
		
		// Quick way to get objects of some basic type: 
		static ExprValueType RIntegerType();
		static ExprValueType RScalarType();
		static ExprValueType RRangeType();
		static ExprValueType RStringType();
		static ExprValueType RVectorType(int n);
		static ExprValueType RStringArrayType();
		
}__attribute__((__packed__));

}  // end of namespace ANI

#endif  /* _ANIVISION_ANI_EXPRTYPE_H_ */
