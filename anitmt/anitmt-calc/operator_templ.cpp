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

#include "val.hpp"

namespace anitmt
{
  //***************************************************************
  // Basic_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  //! has to check the result of the operand with ID as pointer to operand
  //!!! very critical function !!! change only carefully !!!
  template<class T_Result, class T_Operand>
  bool Basic_Operator_for_1_Operand<T_Result,T_Operand>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    just_solved = false;

    T_Operand op = operand.get_value( info );
    if( !is_operand_ok( op ) ) 
      return false;
    if( is_operand_enough( op ) )
    {
      just_solved = result.test_set_value( calc_result(op), info );
      return just_solved;	// return true if solving and test_set succeded
    }

    return true;
  }
  
  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_Operand<T_Result,T_Operand>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    if( just_solved )		// if result was just solved 
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
    T_Operand op = operand.get_value();
    if( operand.is_solved() ) 
      {
	if( !is_operand_ok(op) )
	  throw EX_Initial_Operand_Not_Valid();
	if( is_operand_enough(op) )
  	  result.set_value( calc_result(op) ); 
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
  // Basic_Dual_Solution_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  //! has to check the result of the operand with ID as pointer to operand
  //!!! very critical function !!! change only carefully !!!
  template<class T_Result, class T_Operand> bool 
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    just_solved = false;

    T_Operand op = operand.get_value( info );
    if( !is_operand_ok( op ) ) 
      return false;

    if( is_operand_enough1( op ) )
    {
      Solve_Run_Info::id_type test1_id = info->new_test_run_id();
      just_solved = result.test_set_value( calc_result1(op), info );
      if( !just_solved )
	info->remove_test_run_id( test1_id );
      else
        return just_solved;	// return true if solving and test_set succeded
    }
    if( is_operand_enough2( op ) )
    {
      just_solved = result.test_set_value( calc_result2(op), info );
      return just_solved;	// return true if solving and test_set succeded
    }

    return true;
  }
  
  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Operand> void 
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID == &operand );
    if( just_solved )		// if result was just solved 
      result.use_test_value( info );
  }


  // disconnect operand
  template<class T_Result, class T_Operand> void 
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::disconnect
  ( const void *ID )
  {
    assert( ID == &operand );
    delete this;		// !!! no further commands !!!
  }

  template<class T_Result, class T_Operand>
  void Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::init()
    throw( EX )
  {
    if( operand.is_solved() ) 
    {
      const T_Operand &op = operand.get_value();

      if( !is_operand_ok(op) )
	throw EX_Initial_Operand_Not_Valid();

      bool use_result2 = false;
      if( is_operand_enough1(op) )
      {
	try
        {
	  if( !result.set_value( calc_result1(op) ) ) 
		 		// could throw exception !
	    use_result2 = true;
        }
	catch( EX )
	{
	  use_result2 = true;
	}
      }
      else
	use_result2 = true;

      if( use_result2 && is_operand_enough2(op) )
	if( !result.set_value( calc_result2(op) ) ) 
				// could throw exception !
	  throw EX_Initial_Operand_Not_Valid();	  
    }
  }

  //*************************
  // Constructors/Destructor

  template<class T_Result, class T_Operand>
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>
  ::Basic_Dual_Solution_Operator_for_1_Operand ( Operand<T_Operand> &op ) 
    throw(EX) 
    : operand( op ) 
  {
    operand.add_listener( this );
    // init() cannot be called here as it calls virtual functions
  }

  //***************************************************************
  // Basic_Operator_for_2_Operands: two Operand Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  //! has to check the result of the operand with ID as pointer to operand
  //!!! very critical function !!! change only carefully !!!
  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
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
      just_solved = false;	// reset flag used for another purpose

      T_Op1 op1 = operand1.get_value( info ); // get value1
      T_Op2 op2 = operand2.get_value( info ); // get value2
      // check operands
      if( !are_operands_ok( op1, op2 ) ) 
 	return false;
 
      // calculate result and test it
      if( are_operands_enough( op1, op2 ) )
      {
	just_solved = result.test_set_value( calc_result( op1, op2 ), info );
	return just_solved;	// return true if solving and test_set succeded
      }
    }

    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
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

    // init() cannot be called here as it calls a virtual function
  }

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  T_Result Not_Operator<T_Result,T_Operand>
  ::calc_result( const T_Operand &value ) 
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
  T_Result Negative_Operator<T_Result,T_Operand>
  ::calc_result( const T_Operand &value ) 
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

  //*************************************************************************
  // Abs_Operator: operator for calculating the absolute value of an operand 
  //*************************************************************************

  template<class T_Result, class T_Operand>
  T_Result Abs_Operator<T_Result,T_Operand>
  ::calc_result( const T_Operand &value ) 
  {
    return abs( value );
  }

  template<class T_Result, class T_Operand>
  Abs_Operator<T_Result,T_Operand>::Abs_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Operator_for_1_Operand<T_Result, T_Operand>( operand1 )
  {
    init();
  }

  //*************************************************************************
  // Sqrt_Operator: operator for calculating the sqare root of an operand 
  //*************************************************************************

  template<class T_Result, class T_Operand>
  T_Result Sqrt_Operator<T_Result,T_Operand>
  ::calc_result( const T_Operand &value ) 
  {
    return sqrt( value );
  }

  template<class T_Result, class T_Operand>
  bool Sqrt_Operator<T_Result,T_Operand>
  ::is_operand_ok( const T_Operand &value ) 
  {
    return value >= 0;		// value under square root must be positive
  }

  template<class T_Result, class T_Operand>
  Sqrt_Operator<T_Result,T_Operand>::Sqrt_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Operator_for_1_Operand<T_Result, T_Operand>( operand1 )
  {
    init();
  }

  //***************************************************************************
  // Plus_Minus_Operator: result is either the positive or the negative operand
  //***************************************************************************

  template<class T_Result, class T_Operand>
  T_Result Plus_Minus_Operator<T_Result,T_Operand>
  ::calc_result1( const T_Operand &value ) 
  {
    return value;
  }

  template<class T_Result, class T_Operand>
  T_Result Plus_Minus_Operator<T_Result,T_Operand>
  ::calc_result2( const T_Operand &value ) 
  {
    return -value;
  }

  template<class T_Result, class T_Operand>
  Plus_Minus_Operator<T_Result,T_Operand>::Plus_Minus_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Dual_Solution_Operator_for_1_Operand<T_Result, T_Operand>(operand1)
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

  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Mul_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 * value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Mul_Operator<T_Result,T_Op1,T_Op2>
  ::is_operand1_enough( const T_Op1 &value1 ) 
  {
    return !value1;		// return true if value1 is zero equivalent
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Mul_Operator<T_Result,T_Op1,T_Op2>
  ::is_operand2_enough( const T_Op2 &value2 ) 
  {
    return !value2;		// return true if value1 is zero equivalent
  }

  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Mul_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result_from_op1( const T_Op1 &value1 ) 
  {
    assert( !value1 );
    return T_Result();		// return 0 equavalent
  }

  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Mul_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result_from_op2( const T_Op2 &value2 ) 
  {
    assert( !value2 );
    return T_Result();		// return 0 equavalent
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

  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Div_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 / value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Div_Operator<T_Result,T_Op1,T_Op2>
  ::is_operand2_ok( const T_Op2 &value2 ) 
  {
    return !(!value2);		// value2 has to be different from zero
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Div_Operator<T_Result,T_Op1,T_Op2>
  ::is_operand1_enough( const T_Op1 &value1 ) 
  {
    return !value1;		// if numerator is zero, result is also zero
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Div_Operator<T_Result,T_Op1,T_Op2>
  ::are_operands_ok( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    // something diff zero is infinite
    if( (!(!value1)) && (!value2) ) return false;
    return true;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Div_Operator<T_Result,T_Op1,T_Op2>
  ::are_operands_enough( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    // zero diff zero isn't rejected, but the result cannot be calculated
    return !!value2; 
  }

  /*! has to calculate result only with operand1 when is_operand1_enough 
    returns true */
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Div_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result_from_op1( const T_Op1 &value1 ) 
  {
    assert( !value1 );
    return T_Result();		// return zero equivalent
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Div_Operator<T_Result,T_Op1,T_Op2>::Div_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //***************************************************
  // Equal_Operator: operator for comparing 2 operands 
  //***************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Equal_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 == value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Equal_Operator<T_Result,T_Op1,T_Op2>::Equal_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //***************************************************
  // Unequal_Operator: operator for comparing 2 operands 
  //***************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Unequal_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 != value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Unequal_Operator<T_Result,T_Op1,T_Op2>::Unequal_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //***************************************************
  // Less_Operator: operator for comparing 2 operands 
  //***************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Less_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 < value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Less_Operator<T_Result,T_Op1,T_Op2>::Less_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //***************************************************
  // Greater_Operator: operator for comparing 2 operands 
  //***************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Greater_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 > value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Greater_Operator<T_Result,T_Op1,T_Op2>::Greater_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //*********************************************************
  // Not_Greater_Operator: operator for comparing 2 operands 
  //*********************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Not_Greater_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 <= value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Not_Greater_Operator<T_Result,T_Op1,T_Op2>::Not_Greater_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

  //*********************************************************
  // Not_Less_Operator: operator for comparing 2 operands 
  //*********************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Not_Less_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( const T_Op1 &value1, const T_Op2 &value2 ) 
  {
    return value1 >= value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Not_Less_Operator<T_Result,T_Op1,T_Op2>::Not_Less_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>( operand1,
							   operand2 ) 
  {
    init();
  }

}

#endif
