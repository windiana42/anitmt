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

  template< class T1, class T2 >
  inline Operand<T2>& operator*( Operand<T1> &op1, Operand<T2> &op2 ) 
  {
    return (new Mul_Operator<T2,T1,T2>( op1, op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator*( Operand<T1> &op1, values::Scalar op2 ) 
  {
    return (new Mul_Operator<T1,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator*( values::Scalar op1, Operand<T2> &op2 ) 
  {
    return (new Mul_Operator<T2,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 ))->get_result();
  }

  template< class T1 >
  inline Operand<T1>& operator*( Operand<T1> &op1, values::Vector op2 ) 
  {
    return (new Mul_Operator<T1,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) ))->get_result();
  }

  template< class T2 >
  inline Operand<T2>& operator*( values::Vector op1, Operand<T2> &op2 ) 
  {
    return (new Mul_Operator<T2,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 ))->get_result();
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

}

#endif

