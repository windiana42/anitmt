/*****************************************************************************/
/**   This file offers operators to solve an operand from others            **/
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

#ifndef __AniTMT_Operator_Templateimlementation__
#define __AniTMT_Operator_Templateimlementation__

#include "operator.hpp"

namespace anitmt
{
  //***************************************************************
  // Basic_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  // has to check the result of the operand with ID as pointer to operand
  template<class T_Result, class T_Operand>
  bool Basic_Operator_for_1_Operand<T_Result,T_Operand>::is_result_ok
  ( const void *ID, const Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    if( !is_operand_ok( operand.get_value( info ) ) ) 
      return false;
    if( !result.test_set_value( calc_result(operand.get_value(info)), info ) )
      return false;

    return true;
  }
  
  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_Operand<T_Result,T_Operand>::use_result
  ( const void *ID, const Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    result.use_test_value( info );
  }


  // disconnect operand
  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_Operand<T_Result,T_Operand>::disconnect
  ( const void *ID )
  {
    assert( ID == &operand );
    delete this;		// !!! no further commands !!!
  }

  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_Operand<T_Result,T_Operand>::init()
    throw( EX )
  {
    if( operand.is_solved() ) 
      {
	result.set_value( calc_result( operand.get_value() ) ); 
				// could throw exception !
      }
  }

  //*************************
  // Constructors/Destructor

  template<class T_Result, class T_Operand>
  Basic_Operator_for_1_Operand<T_Result,T_Operand>
  ::Basic_Operator_for_1_Operand ( Operand<T_Operand> &op ) throw(EX) 
    : operand( op ) 
  {
    operand.add_listener( this );
    // init() cannot be called here as it calls a virtual function
  }

  //***************************************************************
  // Basic_Operator_for_2_Operands: two Operand Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  // has to check the result of the operand with ID as pointer to operand
  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::is_result_ok
  ( const void *ID, const Solve_Run_Info *info ) throw(EX)
  {
    just_solved = false;	// reset flag
    if( ID == &operand1 )	// is operand1 solved?
    {
      T_Op1 op1 = operand1.get_value( info );	// get value
      if( !is_operand1_ok(op1) ) // test value
	return false;

      if( is_operand1_enough(op1) ) // is this operand enough to calc result
      {
	// test the calculated result
	just_solved = result.test_set_value( calc_result_from_op1(op1), info );
	return just_solved;
      }
	
      if( operand2.is_solved_in_try( info ) ) // is other operand also solved
	just_solved = true;	// mark that both operands are solved
    }
    else
    {
      assert( ID == &operand2 );
      T_Op2 op2 = operand2.get_value( info ); // get value
      if( !is_operand2_ok(op2) ) // check value
	return false;

      if( is_operand2_enough(op2) ) // is operand2 enough to calc result
      {
	// test the calculated result
	just_solved = result.test_set_value( calc_result_from_op2(op2), info );
	return just_solved;
      }

      if( operand1.is_solved_in_try( info ) ) // is other operand also solved
	just_solved = true;	// mark both operands to be solved
    }

    if( just_solved )		// are both operands solved?
    {
      // check operands
      if( !are_operands_ok( operand1.get_value(info), 
			    operand2.get_value( info ) ) ) 
	return false;

      // calculate result and test it
      if( !result.test_set_value( calc_result( operand1.get_value(info), 
					       operand2.get_value(info) ), 
				  info ) )
      {
	just_solved = false;	// if failed: reset flag
	return false;
      }
    }

    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::use_result
  ( const void *ID, const Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand1 || ID == &operand2 );
    if( just_solved )		// if result was just solved 
      result.use_test_value( info ); // tell result operand to accept value
  }


  // disconnect operand
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::disconnect
  ( const void *ID )
  {
    if( ID == &operand1 )
    {
      operand2.rm_listener( this );
    }
    else
    {
      assert( ID == &operand2 );
      operand1.rm_listener( this );
    }

    delete this;		// !!! no further commands !!!
  }

  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::init() throw( EX )
  {
    if( operand1.is_solved() && operand2.is_solved() ) 
    {
      result.set_value( calc_result( operand1.get_value(), 
				     operand2.get_value() ) ); 
				// could throw exception !
    }
  }

  //***********************
  // Constructor/Destructor

  template<class T_Result, class T_Op1, class T_Op2>
  Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>
  ::Basic_Operator_for_2_Operands( Operand<T_Op1> &op1, 
				   Operand<T_Op2> &op2 ) throw( EX )
    : operand1( op1 ), operand2( op2 ) { 
    
    operand1.add_listener( this );
    operand2.add_listener( this );

    if( operand1.is_solved() )
      if( is_operand1_enough( operand1.get_value() ) )
      {
	result.set_value( calc_result_from_op1( operand1.get_value() ) ); 
				// could throw exception !
      }
 
    if( operand2.is_solved() )
      if( is_operand2_enough( operand2.get_value() ) )
      {
	result.set_value( calc_result_from_op2( operand2.get_value() ) ); 
				// could throw exception !
      }
    // init() cannot be called here as it calls a virtual function
  }

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  T_Result Not_Operator<T_Result,T_Operand>::calc_result( const T_Operand
							  &value ) 
  {
    return !value;
  }

  template<class T_Result, class T_Operand>
  Not_Operator<T_Result,T_Operand>::Not_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Operator_for_1_Operand<T_Result, T_Operand>( operand1 )
  {
    init(); 
  }

  //**********************************************
  // Negative_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  T_Result Negative_Operator<T_Result,T_Operand>::calc_result( const T_Operand
							       &value ) 
  {
    return -value;
  }

  template<class T_Result, class T_Operand>
  Negative_Operator<T_Result,T_Operand>::Negative_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Operator_for_1_Operand<T_Result, T_Operand>( operand1 )
  {
    init();
  }

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Add_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 + value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Add_Operator<T_Result,T_Op1,T_Op2>::Add_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }


  //**********************************************************************
  // Sub_Operator: operator for subtracting 2 operands of different types
  //**********************************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Sub_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 - value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Sub_Operator<T_Result,T_Op1,T_Op2>::Sub_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //**********************************************************************
  // Mul_Operator: operator for multiplying 2 operands of different types
  //**********************************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Mul_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 - value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Mul_Operator<T_Result,T_Op1,T_Op2>::Mul_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //**********************************************************************
  // Div_Operator: operator for dividing 2 operands of different types
  //**********************************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Div_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 - value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Div_Operator<T_Result,T_Op1,T_Op2>::Div_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

}

#endif