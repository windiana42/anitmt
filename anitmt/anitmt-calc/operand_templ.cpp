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
  bool Operand<T>::test_set_value( T res, int test_ID ){
    // was value already solved in test run?
    if( value_valid || (value_test_ID == test_ID) )
      {
	return res == value;	// test ok, when same value
      }
    
    value = res; 
    value_test_ID = test_ID; 
    if( reporter ) 
      return report_test_value( res, test_ID );

    return true;
  }

  template<class T>
  void Operand<T>::init_reporter( Value_Reporter *reporter ) {
    this->reporter = reporter;
  }

  template<class T>
  Operand<T>::Operand() : value_valid(false), reporter(0) {}

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
  ::use_value( T_Operand const &value ) {
    set_value( calc_result( value ) );
  }

  template<class T_Result, class T_Operand>
  bool Basic_Operator_for_1_param<T_Result,T_Operand>
  ::test_value( T_Operand const &value, int test_ID ) {
    if( !is_operand_ok( value ) ) return false;

    return test_set_value( calc_result( value ), test_ID );
  }

  template<class T_Result, class T_Operand>
  void Basic_Operator_for_1_param<T_Result,T_Operand>::init() {

    if( operand.is_value_valid() ) 
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
  ::Operand_1_Interface::use_value( T_Op1 const &value ) {
    if( host->operand2.is_value_valid() )
      host->set_value( host->calc_result( value, host->operand2.get_value() ));
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_1_Interface::test_value( T_Op1 const &value, int test_ID ) {

    if( !host->is_operand1_ok( value ) ) return false;

    // are both operands solved now?
    if( host->operand2.is_value_valid() )
      {
	if( !host->are_operands_ok( value, host->operand2.get_value() ) ) 
	  return false;
	
	return host->test_set_value
	  ( host->calc_result( value, host->operand2.get_value() ), test_ID );
      }
    
    return true;
  }


  // for operand to deliver result
  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_2_Interface::use_value( T_Op2 const &value ) {
    if( host->operand1.is_value_valid() )
      host->set_value( host->calc_result( host->operand1.get_value(), value ));
  }

  template<class T_Result, class T_Op1, class T_Op2>
  bool Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::Operand_2_Interface::test_value( T_Op2 const &value, int test_ID ) {
    if( !host->is_operand2_ok( value ) ) return false;

    // are both operands solved now?
    if( host->operand1.is_value_valid() )
      {
	if( !host->are_operands_ok( host->operand1.get_value(), value ) ) 
	  return false;

	return host->test_set_value
	  ( host->calc_result( host->operand1.get_value(), value ), test_ID );
      }

    return true;
  }

  template<class T_Result, class T_Op1, class T_Op2>
  void Basic_Operator_for_2_params<T_Result,T_Op1,T_Op2>
  ::init() { 
    
    if( operand1.is_value_valid() && operand2.is_value_valid() ) 
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
