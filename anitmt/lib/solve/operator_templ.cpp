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

#ifndef __Solve_Operator_Templateimlementation__
#define __Solve_Operator_Templateimlementation__

#include "operator.hpp"

#include <val/val.hpp>

namespace solve
{
  //***************************************************************
  // Basic_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  //*** Operand_Listener methods ***

  //! has to check the result of the operand with ID as pointer to operand
  //!!! very critical function !!! change only carefully !!!
  template<class T_Result, class T_Operand>
  bool Basic_Operator_for_1_Operand<T_Result,T_Operand>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &operand );
    bool just_solved = false;

    T_Operand op = operand.get_value( info );
    if( !is_operand_ok( op, info ) ) 
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
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &operand );
    if( result.is_solved_in_try( info ) ) // if result was just solved 
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
    throw()
  {
    if( operand.is_solved() ) 
      {
	const T_Operand &op = operand.get_value();

	User_Problem_Handler handler;
	Solve_Run_Info info( &handler );
	if( is_operand_ok( op, &info ) )
	{
	  if( is_operand_enough(op) )
	    result.set_value( calc_result(op) ); 
	}
      }
  }

  //*************************
  // Constructors/Destructor

  template<class T_Result, class T_Operand>
  Basic_Operator_for_1_Operand<T_Result,T_Operand>
  ::Basic_Operator_for_1_Operand ( Operand<T_Operand> &op ) throw() 
    : message::Message_Reporter( op.get_consultant() ), 
      operand( op ), result( op.get_consultant() )
  {
    operand.add_listener( this );
    // init() cannot be called here as it calls a virtual function
  }

  //********************************************************************
  // Basic_Dual_Solution_Operator_for_1_Operand: one parameter Operator
  //********************************************************************

  //*** Operand_Listener methods ***

  //! has to check the result of the operand with ID as pointer to operand
  //!!! very critical function !!! change only carefully !!!
  template<class T_Result, class T_Operand> bool 
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &operand );
    bool just_solved = false;

    const T_Operand &op = operand.get_value( info );
    if( !is_operand_ok( op, info ) ) 
      return false;

    if( is_operand_enough1( op ) )
    {
      // change test_id for first test run
      Solve_Run_Info::id_type test1_id = info->new_test_run_id();
      // start first test run
      info->set_trial_run(true); // first run is trial run
      just_solved = result.test_set_value( calc_result1(op), info );
      info->set_trial_run(false);
      if( just_solved )	
        return just_solved;	// return true if solving and test_set succeded
      else			
	info->remove_test_run_id( test1_id ); // undo first test run
    }
    if( is_operand_enough2( op ) )
    {
      // start test run with second solution
      just_solved = result.test_set_value( calc_result2(op), info );
      return just_solved;	// return true if solving and test_set succeded
    }

    return true;
  }
  
  // tells to use the result calculated by is_result_ok()
  template<class T_Result, class T_Operand> void 
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &operand );
    if( result.is_solved_in_try(info) ) // if result was just solved 
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
    throw()
  {
    if( operand.is_solved() ) 
    {
      const T_Operand &op = operand.get_value();

      User_Problem_Handler handler;
      Solve_Run_Info info( &handler );
      if( is_operand_ok( op, &info ) )
      {
	bool use_result2 = false;
	if( is_operand_enough1(op) )
	{
	  if( !result.set_value( calc_result1(op) ) ) 
	    use_result2 = true;
	}
	else
	  use_result2 = true;

	if( use_result2 && is_operand_enough2(op) )
	  result.set_value( calc_result2(op) ); 
      }
    }
  }

  //*************************
  // Constructors/Destructor

  template<class T_Result, class T_Operand>
  Basic_Dual_Solution_Operator_for_1_Operand<T_Result,T_Operand>
  ::Basic_Dual_Solution_Operator_for_1_Operand ( Operand<T_Operand> &op ) 
    throw() 
    : message::Message_Reporter( op.get_consultant() ), 
      operand( op ), result( op.get_consultant() )      
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
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    bool just_solved = false;	// reset flag
    if( ID == &operand1 )	// is operand1 solved?
    {
      T_Op1 op1 = operand1.get_value( info );	// get value
      if( !is_operand1_ok( op1, info ) ) // test value
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
      if( !is_operand2_ok( op2, info ) ) // check value
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
      if( !are_operands_ok( op1, op2, info ) ) 
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
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &operand1 || ID == &operand2 );
    if( result.is_solved_in_try( info ) ) // if result was just solved 
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
  void Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>::init() throw()
  {
    if( operand1.is_solved() )
    {
      const T_Op1 &op1 = operand1.get_value();

      if( is_operand1_enough( op1 ) )
      {
	result.set_value( calc_result_from_op1( op1 ) ); 
      }
    }
 
    if( operand2.is_solved() )
    {
      const T_Op2 &op2 = operand2.get_value();
      if( is_operand2_enough( op2 ) )
      {
	result.set_value( calc_result_from_op2( op2 ) ); 
      }
    }

    if( operand1.is_solved() && operand2.is_solved() ) 
    {
      result.set_value( calc_result( operand1.get_value(), 
				     operand2.get_value() ) ); 
    }
  }

  //***********************
  // Constructor/Destructor

  template<class T_Result, class T_Op1, class T_Op2>
  Basic_Operator_for_2_Operands<T_Result,T_Op1,T_Op2>
  ::Basic_Operator_for_2_Operands( Operand<T_Op1> &op1, 
				   Operand<T_Op2> &op2 ) throw()
    : message::Message_Reporter( op1.get_consultant() ),
      operand1( op1 ), operand2( op2 ), result( op1.get_consultant() )      
  { 
    
    operand1.add_listener( this );
    operand2.add_listener( this );

    // init() cannot be called here as it calls a virtual function
  }

  //***************************************************************************
  // Basic_Multi_Operand_Solver: base class of solvers with unlimited operands
  //***************************************************************************

  //! tries to solve result
  template< class OP, class RES >
  bool Basic_Multi_Operand_Operator<OP,RES>::try_solve( Solve_Run_Info *info )
  {
    if( result.is_solved() ) return true;

    if( info ) // is in solve run ?
    {
      if( num_solved_in_try >= num )	// are all operands solved yet?
      {
	// solve by running calc for each result
	RES res = initial_result();
	operands_type::iterator op;
	for( op = operands.begin(); op != operands.end(); ++op )
	{
	  if( !(*op)->is_solved_in_try(info) ) 
	  {
	    return true;	// oparand ok, but still some operands missing
	  }
	  res = calc( (*op)->get_value(info), res );
	}      
	bool solved = result.test_set_value( res, info );    
	return solved;
      }
    }
    else
    {
      if( num_solved >= num )	// are all operands solved yet?
      {
	// solve by running calc for each result
	RES res = initial_result();
	operands_type::iterator op;
	for( op = operands.begin(); op != operands.end(); ++op )
	{
	  if( !(*op)->is_solved() ) 
	  {
	    return true;	// oparand ok, but still some operands missing
	  }
	  res = calc( (*op)->get_value(), res );
	}      
	bool solved = result.set_value( res );    
	return solved;
      }
    }

    return true;
  }

  // has to check the result of the operand with ID as pointer to operand
  template< class OP, class RES >
  bool Basic_Multi_Operand_Operator<OP,RES>::
  is_result_ok( const void *ID, Solve_Run_Info *info ) throw()
  {
    if( ID == &result ) return true; // ignore other results

    if( !just_solving )
    {
      just_solving = true;

      // find solved operand
      operands_type::iterator op 
	= operands.find( static_cast<Operand<OP>*>(ID) );
      assert( (*op)->is_solved_in_try(info) );
      OP val = (*op)->get_value( info );
      if( !is_op_ok(val,result) )
      {
	just_solving = false;
	return false;
      }
      if( is_op_sufficient(val) ) // try to calc res with only this operand
      {
	just_solving = false;
	return result.test_set_value( calc(val, initial_result()), info );
      }
      ++num_solved_in_try;

      bool res = try_solve(info);
      just_solving = false;
      return res;
    }
    else
    {
      // find solved operand
      operands_type::iterator op 
	= operands.find( static_cast<Operand<OP>*>(ID) );
      assert( (*op)->is_solved_in_try(info) );
      OP val = (*op)->get_value( info );
      if( !is_op_ok(val,result) ) return false;
      ++num_solved_in_try;
    }
    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template< class OP, class RES >
  void Basic_Multi_Operand_Operator<OP,RES>::
  use_result( const void *ID, Solve_Run_Info *info ) throw()
  {
    if( !result.is_solved() )
    {
      if( ID != &result )
      {
	++num_solved;
	num_solved_in_try = num_solved;
 	verbose(4) << "solved " << num_solved << "/" << num;
	
	if( result.is_solved_in_try( info ) ) // if result was just solved 
	  result.use_test_value( info ); // tell result operand to accept value
      }
    }
  }

  // disconnect operand
  template< class OP, class RES >
  void Basic_Multi_Operand_Operator<OP,RES>::disconnect( const void *ID )
  {

    operands_type::iterator op;
    for( op = operands.begin(); op != operands.end(); ++op )
      if( (*op) != ID )
	(*op)->rm_listener(this);

    delete this;
  }
  
  template< class OP, class RES >
  void Basic_Multi_Operand_Operator<OP,RES>::add_operand( Operand<OP> 
							  &operand )
  {
    bool not_needed = false;

    if( operand.is_solved() )
    {
      OP op = operand.get_value();
      if( is_op_ok( op, result ) )
      {
	if( is_op_sufficient( op ) )
	{
	  result.set_value( calc( op, initial_result() ) );
	  not_needed = true;
	}
	else
	{
	  ++num_solved;
	  num_solved_in_try = num_solved;
	}
      }
      else
      {
	error() << "initial operand for multi and operator is invalid";
      }
    }

    if( !not_needed )
    {
      operand.add_listener( this );
      operands.insert( &operand );
      ++num;
    }
  }

  template< class OP, class RES >
  void Basic_Multi_Operand_Operator<OP,RES>::finish_adding()
  {
    try_solve();
  }
  
  //***********************
  // Constructor/Destructor

  template< class OP, class RES >
  Basic_Multi_Operand_Operator<OP,RES>::Basic_Multi_Operand_Operator
  ( message::Message_Consultant *c )
    : message::Message_Reporter(c), 
      num(0), num_solved(0), num_solved_in_try(0), result(c),
      just_solving(false) {}

  template< class OP, class RES >
  Basic_Multi_Operand_Operator<OP,RES>::~Basic_Multi_Operand_Operator()
  {}

//**********************************************
//**********************************************
//** Special Operators
//**********************************************
//**********************************************

//**********************************************
//**********************************************
//** Normal Calculation Operators
//**********************************************
//**********************************************

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
  ::is_operand_ok( const T_Operand &value, Solve_Run_Info *info ) 
  {
    // value under square root must be positive
    if( value < 0 )
    {
      if( !info->is_trial_run() )
	error() << "cannot calculate square root of " << value;
      return false;
    }
    return true;
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
  ::is_operand1_enough( const T_Op1 &value1 ) 
  {
    return !value1;		// if numerator is zero, result is also zero
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Div_Operator<T_Result,T_Op1,T_Op2>
  ::are_operands_ok( const T_Op1 &value1, const T_Op2 &value2, 
		     Solve_Run_Info *info ) 
  {
    // something diff zero is infinite
    if( (!(!value1)) && (!value2) )
    {
      if( !info->is_trial_run() )
	error() << "cannot devide " << value1 << " by zero";
      return false;
    }
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
