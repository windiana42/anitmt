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

#ifndef __AniTMT_Operand__
#define __AniTMT_Operand__

#include "val.hpp"

namespace anitmt{
  class Basic_Operand;
  template<class T> class Operand;
  template<class T> class Constant;
  template<class T_Result, 
	   class T_Operand = T_Result> class Basic_Operator_for_1_param;
  template<class T_Result, class T_Op1 = T_Result, 
	   class T_Op2 = T_Op1> class Basic_Operator_for_2_params;
  template<class T_Result=values::Scalar, class T_Operand = T_Result> 
  class Not_Operator;
  template<class T_Result=values::Scalar, class T_Operand = T_Result> 
  class Add_Operator;
  template<class T_Result=values::Scalar, 
	   class T_Op1 = T_Result, class T_Op2 = T_Op1> 
  class Add_Operator_Diff;
}

#include "error.hpp"

namespace anitmt{

  //************
  // Exceptions:
  //************
  
  //*********************************************
  // Basic_Operand: basic class for all operands 
  //*********************************************

  class Basic_Operand{
  protected:
  public:
  };

  //**********************************************************
  // Operand: base class for operand values of a certain type
  //**********************************************************
  template<class T> class Operand : public Basic_Operand{
  protected:
    class Value_Reporter{
      friend Operand;
      virtual bool test_value( T const &value, int sender_ID, 
			       int test_ID ) = 0;
      virtual void use_value ( T const &value, int sender_ID ) = 0;
    };

    T value;			// to store the value
    bool value_valid;
    int value_test_ID;

    Value_Reporter *reporter; 
    int sender_ID;

    inline void report_value( T res ) {
      if( reporter ) reporter->use_value( res, sender_ID );
    }
    inline bool report_test_value( T res, int test_ID ) {
      if( reporter ) 
	return reporter->test_value( res, sender_ID, test_ID );
      
      return true;
    }

    bool test_set_value( T res, int test_ID );
  public:	
    inline bool is_value_valid() { return value_valid; }
    inline const T& get_value() { assert( is_value_valid() ); return value; }
    inline void set_value( T res ) 
    { value = res; value_valid = true; report_value( res ); }

    void init_reporter( Value_Reporter *reporter, int sender_ID );

    Operand();
  };

  //*****************************************
  // Constant: Operand for holding constants
  //*****************************************

  template<class T>
  class Constant : public Operand<T> {
  public:
    Constant( T val );
  };

  //***************************************************************
  // Basic_Operator_for_1_param: two parameter Operator
  //***************************************************************

  template<class T_Result, class T_Operand>
  class Basic_Operator_for_1_param 
    : public Operand<T_Result>, public Operand<T_Operand>::Value_Reporter {
    // for operand to deliver result
    virtual bool test_value( T_Operand const &value, int sender_ID, 
			     int test_ID );
    virtual void use_value( T_Operand const &value, int sender_ID );

    virtual bool is_operand_ok( T_Operand const &test_value ) { return true; };
    virtual T_Result calc_result( T_Operand const &value ) = 0;

    Operand<T_Operand> &operand;

  protected:
    // must be called by constuctors of derived classes!!!
    void init();
  public:
    Basic_Operator_for_1_param( Operand<T_Operand> &operand );
  };

  //***************************************************************
  // Basic_Operator_for_2_params: two parameter Operator
  //***************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Basic_Operator_for_2_params
    : public Operand<T_Result>, public Operand<T_Op1>::Value_Reporter, 
      public Operand<T_Op2>::Value_Reporter {
    enum{ sender_is_op1, sender_is_op2 };

    // for operands to deliver result
    virtual bool test_value( T_Op1 const &value, int sender_ID, int test_ID );
    virtual void use_value( T_Op1 const &value, int sender_ID );
    virtual bool test_value( T_Op2 const &value, int sender_ID, int test_ID );
    virtual void use_value( T_Op2 const &value, int sender_ID );

    virtual bool is_operand1_ok( T_Op1 const &test_value ) { return true; }
    virtual bool is_operand2_ok( T_Op2 const &test_value ) { return true; }
    virtual bool are_operands_ok( T_Op1 const &test_value1, T_Op2 test_value2 )
    { return true; }
    virtual T_Result calc_result( T_Op1 const &value1, 
				  T_Op2 const &value2 ) = 0; 
				// may throw exception!
    
    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;

  protected:
    // must be called by constuctors of derived classes!!!
    void init(); 
  public:
    Basic_Operator_for_2_params( Operand<T_Op1> &operand1, 
				 Operand<T_Op2> &operand2 ); 
				// may throw exception if operands are not ok!
  };

  //***************************************************************
  // Basic_Operator_for_2_same_params: two parameter Operator
  //***************************************************************

  template<class T_Result, class T_Operand>
  class Basic_Operator_for_2_same_params
    : public Operand<T_Result>, public Operand<T_Operand>::Value_Reporter {
    enum{ sender_is_op1, sender_is_op2 };

    // for operands to deliver result
    virtual bool test_value( T_Operand const &value, int sender_ID, 
			     int test_ID );
    virtual void use_value( T_Operand const &value, int sender_ID );

    virtual bool is_operand1_ok( T_Operand const &test_value ) { return true; }
    virtual bool is_operand2_ok( T_Operand const &test_value ) { return true; }
    virtual bool are_operands_ok( T_Operand const &test_value1, 
				  T_Operand const &test_value2 ) 
    { return true; }
    virtual T_Result calc_result( T_Operand const &value1, 
				  T_Operand const &value2 ) = 0; 
				// may throw exception!
    
    Operand<T_Operand> &operand1;
    Operand<T_Operand> &operand2;
  protected:
    // must be called by constuctors of derived classes!!!
    void init(); 
  public:
    Basic_Operator_for_2_same_params( Operand<T_Operand> &operand1, 
				      Operand<T_Operand> &operand2 ); 
				// may throw exception if operands are not ok!
  };

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  class Not_Operator
    : public Basic_Operator_for_1_param<T_Result, T_Operand> {

    virtual T_Result calc_result( T_Operand const &value ); 

  public:
    Not_Operator( Operand<T_Operand> &operand1 );
  };

  //**********************************************
  // Add_Operator: operator for adding 2 operands
  //**********************************************

  template<class T_Result, class T_Operand>
  class Add_Operator
    : public Basic_Operator_for_2_same_params<T_Result, T_Operand> {

    virtual T_Result calc_result( T_Operand const &value1, 
				  T_Operand const &value2 ); 
				// may throw exception!

  public:
    Add_Operator( Operand<T_Operand> &operand1, Operand<T_Operand> &operand2 );
  };

  //**********************************************************************
  // Add_Operator_Diff: operator for adding 2 operands of different types
  //**********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Add_Operator_Diff
    : public Basic_Operator_for_2_params<T_Result, T_Op1, T_Op2> {

    virtual T_Result calc_result( T_Op1 const &value1, T_Op2 const &value2 ); 
				// may throw exception!

  public:
    Add_Operator_Diff( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operand_templ.cpp"

#endif

