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

#ifndef __AniTMT_Operand_Templateimlementation__
#define __AniTMT_Operand_Templateimlementation__

#include "operand.hpp"

namespace anitmt{

  //************
  // Exceptions:
  //************
  
  //***************************************************************
  // Operand: base class for operand values of a certain type
  //***************************************************************
  
  template<class T>
  User_Problem_Handler Operand<T>::default_handler;

  template<class T>
  bool Operand<T>::test_set_value( T res, Solve_Run_Info const *info ){
    // was value already solved in test run?
    if( is_solved_in_try( info ) )
      {
	if( res == value )	// test ok, when same value
	  {
	    return true;
	  }
	else
	  {
	    std::list<Property*> bad_props;
	    // !!! should add Operand and not property to bad_probs...
	    info->problem_handler->property_collision_occured( bad_props );
	  }
      }
    
    value = res; 
    last_test_run_id = info->get_test_run_id(); 
    if( reporter ) 
      return report_test_value( res, info );

    return true;
  }

  template<class T>
  void Operand<T>::init_reporter( Value_Reporter *reporter ) {
    this->reporter = reporter;
  }

  template<class T>
  Operand<T>::Operand() : solved(false), reporter(0) {}

  //**************************************************************************
  // Store_Operand_to_Property: reports the value of an operand to a property
  //**************************************************************************
  
  template<class T>
  bool Store_Operand_to_Property<T>::is_this_ok
  ( T const &value, Solve_Run_Info const *info ) {

    return property->is_this_ok( value, this, info );
  }

  template<class T>
  void Store_Operand_to_Property<T>::use_it() {
    property_use_it( property );
  }

  template<class T>
  Store_Operand_to_Property<T>::Store_Operand_to_Property
  ( Operand<T> &op, Type_Property<T> *prop ) : property(prop) {
    // Solver initialization
    add_Property( prop );

    // Operand initialization
    op.init_reporter( this );
  }

  //*****************************************
  // Constant: Operand for holding constants
  //*****************************************

  template<class T>
  Constant<T>::Constant( T val )
    : Operand<T>() {
    set_value( val );
  }

  //***************************************************************
  // Basic_Operator_for_1_param: two parameter Operator
  //***************************************************************

  // for operand to deliver result
  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_param<T_Result,T_Operand>
  ::use_it() {
    use_value();
  }

  template<class T_Result, class T_Operand>
  bool Basic_Operator_for_1_param<T_Result,T_Operand>
  ::is_this_ok( T_Operand const &value, Solve_Run_Info const *info ) {
    if( !is_operand_ok( value ) ) return false;

    return test_set_value( calc_result( value ), info );
  }

  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_param<T_Result,T_Operand>::init() {

    if( operand.is_solved() ) 
      {
	set_value( calc_result( operand.get_value() ) ); 
				// could throw exception 
      }
  }

  template<class T_Result, class T_Operand>
  Basic_Operator_for_1_param<T_Result,T_Operand>::Basic_Operator_for_1_param
  ( Operand<T_Operand> &op ) : operand( op ) {

    operand.init_reporter( this );
  }

  //***************************************************************
  // Basic_Operator_for_2_params: two parameter Operator
  //***************************************************************

  // for operand to deliver result
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_1_Interface::use_it() {
    if( host->just_solved )
      host->use_value();
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_1_Interface::is_this_ok( T_Op1 const &value, 
				     Solve_Run_Info const *info ) {
    if( !host->is_operand1_ok( value ) ) return false;

    // are both operands solved now?
    if( host->operand2.is_solved_in_try( info ) )
      {
	if( !host->are_operands_ok( value, host->operand2.get_value() ) ) 
	  return false;
	
	host->just_solved = true;
	return host->test_set_value
	  ( host->calc_result( value, host->operand2.get_value() ), info );
      }
    
    host->just_solved = false;
    return true;
  }


  // for operand to deliver result
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_2_Interface::use_it( ) {
    if( host->just_solved )
      host->use_value();
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_2_Interface::is_this_ok( T_Op2 const &value, 
				     Solve_Run_Info const *info ) {
    if( !host->is_operand2_ok( value ) ) return false;

    // are both operands solved now?
    if( host->operand1.is_solved_in_try( info ) )
      {
	if( !host->are_operands_ok( host->operand1.get_value(), value ) ) 
	  return false;

	host->just_solved = true;
	return host->test_set_value
	  ( host->calc_result( host->operand1.get_value(), value ), info );
      }

    host->just_solved = false;
    return true;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::init() { 
    
    if( operand1.is_solved() && operand2.is_solved() ) 
      {
	set_value( calc_result(operand1.get_value(), operand2.get_value()) ); 
				// could throw exception !!!
      }
  }

  //***********************
  // Constructor/Destructor

  template<class T_Result, class T_Op1, class T_Op2>
  Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Basic_Operator_for_2_params( Operand<T_Op1> &op1, 
				 Operand<T_Op2> &op2 ) 
    : operand1( op1 ), operand2( op2 ) { 
    
    interface1 = new Operand_1_Interface(this);
    interface2 = new Operand_2_Interface(this);

    operand1.init_reporter( interface1 );
    operand2.init_reporter( interface2 );
  }
  template<class T_Result, class T_Op1, class T_Op2>
  Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::~Basic_Operator_for_2_params() {
    delete interface1;
    delete interface2;
  } 

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  T_Result Not_Operator<T_Result,T_Operand>::calc_result( T_Operand const
							  &value ) {
    return !value;
  }

  template<class T_Result, class T_Operand>
  Not_Operator<T_Result,T_Operand>::Not_Operator
  ( Operand<T_Operand> &operand1 ) 
    : Basic_Operator_for_1_param<T_Result, T_Operand>( operand1 ){

    init();
  }

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  // may throw exception!
  template<class T_Result, class T_Op1, class T_Op2>
  T_Result Add_Operator<T_Result,T_Op1,T_Op2>
  ::calc_result( T_Op1 const &value1, T_Op2 const &value2 ) {
    return value1 + value2;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  Add_Operator<T_Result,T_Op1,T_Op2>::Add_Operator
  ( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 ) 
    : Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>( operand1,
							 operand2 ) {
    init();
  }

}

#endif
