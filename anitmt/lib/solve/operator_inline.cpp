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

#ifndef __Solve_Operator_Inlineimplementation__
#define __Solve_Operator_Inlineimplementation__

#include "operator.hpp"

namespace solve
{
  //*******************************************************************
  // Is_Solved_Operator: operator to test whether an operand is solved
  //*******************************************************************

  template< class OP >
  inline Operand<bool> &is_solved( Operand<OP> &op )
  {
    if( op.is_solved() ) return const_op( true, op.get_consultant() );
    return (new Is_Solved_Operator( op, op.get_consultant() ))->get_result();
  }

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<bool> &op ) 
  {
    return (new Not_Operator<bool,bool>( op ))->get_result();
  }

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<values::Scalar> &op ) 
  {
    return (new Not_Operator<bool,values::Scalar>( op ))->get_result();
  }

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<values::Vector> &op ) 
  {
    return (new Not_Operator<bool,values::Vector>( op ))->get_result();
  }

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<values::Matrix> &op ) 
  {
    return (new Not_Operator<bool,values::Matrix>( op ))->get_result();
  }

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<values::String> &op ) 
  {
    return (new Not_Operator<bool,values::String>( op ))->get_result();
  }

  //*****************************************************
  // Negative_Operator: operator for negative of operand 
  //*****************************************************

  // - Operator for operand expression trees
  inline Operand<values::Scalar>& operator-( Operand<values::Scalar> &op ) 
  {
    return (new Negative_Operator<values::Scalar,values::Scalar>( op ))
      ->get_result();
  }

  inline Operand<values::Vector>& operator-( Operand<values::Vector> &op ) 
  {
    return (new Negative_Operator<values::Vector,values::Vector>( op ))
      ->get_result();
  }

  //*************************************************************************
  // Abs_Operator: operator for calculating the absolute value of an operand 
  //*************************************************************************

  // abs Operator for operand expression trees
  inline Operand<values::Scalar>& abs( Operand<values::Scalar> &op ) 
  {
    return (new Abs_Operator<values::Scalar,values::Scalar>( op ))
      ->get_result();
  }

  inline Operand<values::Scalar>& abs( Operand<values::Vector> &op ) 
  {
    return (new Abs_Operator<values::Scalar,values::Vector>( op ))
      ->get_result();
  }

  //*************************************************************************
  // Sqrt_Operator: operator for calculating the sqare root of an operand 
  //*************************************************************************

  // sqrt function for operand expression trees
  inline Operand<values::Scalar>& sqrt( Operand<values::Scalar> &op ) 
  {
    return (new Sqrt_Operator<values::Scalar,values::Scalar>( op ))
      ->get_result();
  }

  //***************************************************************************
  // Plus_Minus_Operator: result is either the positive or the negative operand
  //***************************************************************************

  //! +- Operator for operand expression trees (+-scalar)
  inline Operand<values::Scalar>& plus_minus( Operand<values::Scalar> &op ) 
  {
    return (new Plus_Minus_Operator<values::Scalar,values::Scalar>( op ))
      ->get_result();
  }

  //! +- Operator for operand expression trees (+-vector)
  inline Operand<values::Vector>& plus_minus( Operand<values::Vector> &op ) 
  {
    return (new Plus_Minus_Operator<values::Vector,values::Vector>( op ))
      ->get_result();
  }

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  // scalar + scalar
  inline Operand<values::Scalar>& 
  operator+( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator+( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator+( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector + vector
  inline Operand<values::Vector>& 
  operator+( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Add_Operator<values::Vector,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator+( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Add_Operator<values::Vector,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator+( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Add_Operator<values::Vector,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string + string
  inline Operand<values::String>& 
  operator+( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Add_Operator<values::String,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::String>& 
  operator+( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Add_Operator<values::String,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::String>& 
  operator+( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Add_Operator<values::String,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //**********************************************************************
  // Sub_Operator: operator for subing 2 operands of different types
  //**********************************************************************

  // scalar - scalar
  inline Operand<values::Scalar>& 
  operator-( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Sub_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator-( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Sub_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator-( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Sub_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector - vector
  inline Operand<values::Vector>& 
  operator-( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Sub_Operator<values::Vector,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator-( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Sub_Operator<values::Vector,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator-( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Sub_Operator<values::Vector,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //**********************************************************************
  // Mul_Operator: operator for muling 2 operands of different types
  //**********************************************************************

  // scalar * scalar
  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator*( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // scalar * vector
  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, const values::Vector &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( const values::Scalar &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Scalar,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector * scalar
  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, const values::Scalar &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( const values::Vector &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Vector,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // matrix * matrix
  inline Operand<values::Matrix>& 
  operator*( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Mul_Operator<values::Matrix,values::Matrix,values::Matrix>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Matrix>& 
  operator*( Operand<values::Matrix> &op1, const values::Matrix &op2 ) 
  {
    return (new Mul_Operator<values::Matrix,values::Matrix,values::Matrix>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Matrix>& 
  operator*( const values::Matrix &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Mul_Operator<values::Matrix,values::Matrix,values::Matrix>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // matrix * vector
  inline Operand<values::Vector>& 
  operator*( Operand<values::Matrix> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Matrix,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( Operand<values::Matrix> &op1, const values::Vector &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Matrix,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator*( const values::Matrix &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Mul_Operator<values::Vector,values::Matrix,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //**********************************************************************
  // Div_Operator: operator for diving 2 operands of different types
  //**********************************************************************

  // scalar / scalar
  inline Operand<values::Scalar>& 
  operator/( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Div_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator/( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Div_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Scalar>& 
  operator/( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Div_Operator<values::Scalar,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector / scalar
  inline Operand<values::Vector>& 
  operator/( Operand<values::Vector> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Div_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator/( Operand<values::Vector> &op1, const values::Scalar &op2 ) 
  {
    return (new Div_Operator<values::Vector,values::Vector,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<values::Vector>& 
  operator/( const values::Vector &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Div_Operator<values::Vector,values::Vector,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //***************************************************
  // Equal_Operator: operator for comparing 2 operands
  //***************************************************

  // bool == bool
  inline Operand<bool>& operator==( Operand<bool> &op1, Operand<bool> &op2 ) 
  {
    return (new Equal_Operator<bool,bool,bool>( op1, op2 ))->get_result();
  }
  inline Operand<bool>& operator==( Operand<bool> &op1, bool op2 ) 
  {
    return (new Equal_Operator<bool,bool,bool>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>& operator==( bool op1, Operand<bool> &op2 ) 
  {
    return (new Equal_Operator<bool,bool,bool>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // scalar == scalar
  inline Operand<bool>&
  operator==( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator==( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Equal_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator==( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector == vector
  inline Operand<bool>&
  operator==( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator==( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Equal_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator==( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // matrix == matrix
  inline Operand<bool>&
  operator==( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Matrix,values::Matrix>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator==( Operand<values::Matrix> &op1, const values::Matrix &op2 ) 
  {
    return (new Equal_Operator<bool,values::Matrix,values::Matrix>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator==( const values::Matrix &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Equal_Operator<bool,values::Matrix,values::Matrix>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string == string
  inline Operand<bool>&
  operator==( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Equal_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator==( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Equal_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator==( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Equal_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //***************************************************
  // Unequal_Operator: operator for comparing 2 operands
  //***************************************************

  // bool != bool
  inline Operand<bool>& operator!=( Operand<bool> &op1, Operand<bool> &op2 ) 
  {
    return (new Unequal_Operator<bool,bool,bool>( op1, op2 ))->get_result();
  }
  inline Operand<bool>& operator!=( Operand<bool> &op1, bool op2 ) 
  {
    return (new Unequal_Operator<bool,bool,bool>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>& operator!=( bool op1, Operand<bool> &op2 ) 
  {
    return (new Unequal_Operator<bool,bool,bool>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // scalar != scalar
  inline Operand<bool>&
  operator!=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator!=( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator!=( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector != vector
  inline Operand<bool>&
  operator!=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator!=( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator!=( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // matrix != matrix
  inline Operand<bool>&
  operator!=( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Matrix,values::Matrix>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator!=( Operand<values::Matrix> &op1, const values::Matrix &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Matrix,values::Matrix>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator!=( const values::Matrix &op1, Operand<values::Matrix> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::Matrix,values::Matrix>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string != string
  inline Operand<bool>&
  operator!=( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator!=( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Unequal_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator!=( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Unequal_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //***************************************************
  // Less_Operator: operator for comparing 2 operands
  //***************************************************

  // scalar < scalar
  inline Operand<bool>&
  operator<( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Less_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Less_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Less_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector < vector
  inline Operand<bool>&
  operator<( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Less_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Less_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Less_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string < string
  inline Operand<bool>&
  operator<( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Less_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Less_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Less_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //***************************************************
  // Greater_Operator: operator for comparing 2 operands
  //***************************************************

  // scalar > scalar
  inline Operand<bool>&
  operator>( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector > vector
  inline Operand<bool>&
  operator>( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Greater_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Greater_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Greater_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string > string
  inline Operand<bool>&
  operator>( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Greater_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Greater_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Greater_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //******************************************************
  // Not_Greater_Operator: operator for comparing 2 operands
  //******************************************************

  // scalar <= scalar
  inline Operand<bool>&
  operator<=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<=( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<=( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector <= vector
  inline Operand<bool>&
  operator<=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<=( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<=( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string <= string
  inline Operand<bool>&
  operator<=( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator<=( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator<=( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Not_Greater_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  //******************************************************
  // Not_Less_Operator: operator for comparing 2 operands
  //******************************************************

  // scalar >= scalar
  inline Operand<bool>&
  operator>=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>=( Operand<values::Scalar> &op1, const values::Scalar &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Scalar,values::Scalar>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>=( const values::Scalar &op1, Operand<values::Scalar> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Scalar,values::Scalar>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // vector >= vector
  inline Operand<bool>&
  operator>=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Vector,values::Vector>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>=( Operand<values::Vector> &op1, const values::Vector &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Vector,values::Vector>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>=( const values::Vector &op1, Operand<values::Vector> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::Vector,values::Vector>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

  // string >= string
  inline Operand<bool>&
  operator>=( Operand<values::String> &op1, Operand<values::String> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::String,values::String>
	    ( op1, op2 ))->get_result();
  }
  inline Operand<bool>&
  operator>=( Operand<values::String> &op1, const values::String &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::String,values::String>
	    ( op1, const_op(op2,op1.get_consultant()) ))->get_result();
  }
  inline Operand<bool>&
  operator>=( const values::String &op1, Operand<values::String> &op2 ) 
  {
    return (new Not_Less_Operator<bool,values::String,values::String>
	    ( const_op(op1,op2.get_consultant()), op2 ))->get_result();
  }

}

#endif

