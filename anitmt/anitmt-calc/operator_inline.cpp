/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __AniTMT_Operator_Inlineimplementation__
#define __AniTMT_Operator_Inlineimplementation__

#include "operator.hpp"

namespace anitmt
{

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  // ! Operator with operand type as result type
  template< class T >
  inline Operand<T>& operator!( Operand<T> &op ) 
  {
    return (new Not_Operator<T,T>( op ))->get_result();
  }

  //*****************************************************
  // Negative_Operator: operator for negative of operand 
  //*****************************************************

  // ! Operator with operand type as result type
  template< class T >
  inline Operand<T>& operator-( Operand<T> &op ) 
  {
    return (new Negative_Operator<T,T>( op ))->get_result();
  }

  //*************************************************************************
  // Abs_Operator: operator for calculating the absolute value of an operand 
  //*************************************************************************

  // ! Operator with operand type as result type
  template< class T >
  inline Operand<T>& abs( Operand<T> &op ) 
  {
    return (new Abs_Operator<T,T>( op ))->get_result();
  }

  //*************************************************************************
  // Sqrt_Operator: operator for calculating the sqare root of an operand 
  //*************************************************************************

  // ! Operator with operand type as result type
  template< class T >
  inline Operand<T>& sqrt( Operand<T> &op ) 
  {
    return (new Sqrt_Operator<T,T>( op ))->get_result();
  }

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  template< class T1, class T2 >
  inline Operand<T2>& operator+( Operand<T1> &op1, Operand<T2> &op2 ) 
  {
    return (new Add_Operator<T2,T1,T2>( op1, op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator+( Operand<T1> &op1, values::Scalar op2 ) 
  {
    return (new Add_Operator<T1,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator+( values::Scalar op1, Operand<T2> &op2 ) 
  {
    return (new Add_Operator<T2,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator+( Operand<T1> &op1, values::Vector op2 ) 
  {
    return (new Add_Operator<T1,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator+( values::Vector op1, Operand<T2> &op2 ) 
  {
    return (new Add_Operator<T2,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 ))->get_result();
  }

  //**********************************************************************
  // Sub_Operator: operator for subing 2 operands of different types
  //**********************************************************************

  template< class T1, class T2 >
  inline Operand<T2>& operator-( Operand<T1> &op1, Operand<T2> &op2 ) 
  {
    return (new Sub_Operator<T2,T1,T2>( op1, op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator-( Operand<T1> &op1, values::Scalar op2 ) 
  {
    return (new Sub_Operator<T1,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator-( values::Scalar op1, Operand<T2> &op2 ) 
  {
    return (new Sub_Operator<T2,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator-( Operand<T1> &op1, values::Vector op2 ) 
  {
    return (new Sub_Operator<T1,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator-( values::Vector op1, Operand<T2> &op2 ) 
  {
    return (new Sub_Operator<T2,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 ))->get_result();
  }

  //**********************************************************************
  // Mul_Operator: operator for muling 2 operands of different types
  //**********************************************************************

  //*****************
  // scalar * scalar

  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }

  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, values::Scalar op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, const_op(op2) ))->get_result();
  }

  inline Operand<values::Scalar>& 
  operator*( values::Scalar op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( const_op(op1), op2 ))->get_result();
  }

  //*****************
  // scalar * vector

  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( op1, op2 ))->get_result();
  }

  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, values::Vector op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( op1, const_op(op2) ))->get_result();
  }

  inline Operand<values::Vector>& 
  operator*( values::Scalar op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( const_op(op1), op2 ))->get_result();
  }

  //*****************
  // vector * scalar

  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, op2 ))->get_result();
  }

  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, values::Scalar op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, const_op(op2) ))->get_result();
  }

  inline Operand<values::Vector>& 
  operator*( values::Vector op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( const_op(op1), op2 ))->get_result();
  }

  //**********************************************************************
  // Div_Operator: operator for diving 2 operands of different types
  //**********************************************************************

  template< class T1, class T2 >
  inline Operand<T2>& operator/( Operand<T1> &op1, Operand<T2> &op2 ) 
  {
    return (new Div_Operator<T2,T1,T2>( op1, op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator/( Operand<T1> &op1, values::Scalar op2 ) 
  {
    return (new Div_Operator<T1,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator/( values::Scalar op1, Operand<T2> &op2 ) 
  {
    return (new Div_Operator<T2,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator/( Operand<T1> &op1, values::Vector op2 ) 
  {
    return (new Div_Operator<T1,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator/( values::Vector op1, Operand<T2> &op2 ) 
  {
    return (new Div_Operator<T2,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 ))->get_result();
  }

  //***************************************************
  // Equal_Operator: operator for comparing 2 operands
  //***************************************************

  template< class T1, class T2 >
  inline Operand<bool>& operator==( Operand<T1> &op1, Operand<T2> &op2 ) 
  {
    return (new Equal_Operator<bool,T1,T2>( op1, op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<bool>& operator==( Operand<T1> &op1, values::Scalar op2 ) 
  {
    return (new Equal_Operator<bool,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<bool>& operator==( values::Scalar op1, Operand<T2> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<bool>& operator==( Operand<T1> &op1, values::Vector op2 ) 
  {
    return (new Equal_Operator<bool,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<bool>& operator==( values::Vector op1, Operand<T2> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 ))->get_result();
  }
}

#endif

