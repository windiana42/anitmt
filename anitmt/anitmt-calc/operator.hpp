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

#ifndef __AniTMT_Operator__
#define __AniTMT_Operator__

#include "val.hpp"
#include "error.hpp"
#include "operand.hpp"

namespace anitmt
{
  //************
  // Exceptions:
  //************

  class EX_Initial_Operand_Not_Valid : public EX 
  {
  public:
    EX_Initial_Operand_Not_Valid() : EX( "initial operand not valid" ) {}
  };
  
  //***************************************************************
  // Basic_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  /*! Base class for expression tree operators, that calculate the results
    from one operand*/
  template<class T_Result, class T_Operand>
  class Basic_Operator_for_1_Operand 
    : public Operand_Listener 
  {
    bool just_solved;		// did operator just solve the result

    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, 
			       Solve_Run_Info *info ) throw(EX);
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, Solve_Run_Info *info )
      throw(EX);

    //! disconnect operand
    virtual void disconnect( const void *ID );

    //*** virtual Operator methods ***

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value ) { return true; }
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool is_operand_enough( const T_Operand &test_value ) 
    { return true; }
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Operand &value ) = 0;

    Operand<T_Operand> &operand;
    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw( EX );
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Operator_for_1_Operand( Operand<T_Operand> &operand ) throw(EX);
    virtual ~Basic_Operator_for_1_Operand() {}
  };

  //********************************************************************
  // Basic_Dual_Solution_Operator_for_1_Operand: one parameter Operator
  //********************************************************************

  /*! Base class for expression tree operators, that have two possible results
    from one operand*/
  template<class T_Result, class T_Operand>
  class Basic_Dual_Solution_Operator_for_1_Operand 
    : public Operand_Listener 
  {
    bool just_solved;		// did operator just solve the result

    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, Solve_Run_Info *info ) 
      throw(EX);
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, Solve_Run_Info *info )
      throw(EX);

    //! disconnect operand
    virtual void disconnect( const void *ID );

    //*** virtual Operator methods ***

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value ) { return true; }
    //! can result1 be calculated? operand won't be rejected when this is false
    virtual bool is_operand_enough1( const T_Operand &test_value ) 
    { return true; }
    //! can result2 be calculated? operand won't be rejected when this is false
    virtual bool is_operand_enough2( const T_Operand &test_value ) 
    { return true; }
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough1 return true */
    virtual T_Result calc_result1( const T_Operand &value ) = 0;
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough2 return true */
    virtual T_Result calc_result2( const T_Operand &value ) = 0;

    Operand<T_Operand> &operand;
    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw( EX );
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Dual_Solution_Operator_for_1_Operand( Operand<T_Operand> &operand ) 
      throw(EX);
    virtual ~Basic_Dual_Solution_Operator_for_1_Operand() {}
  };

  //***************************************************************
  // Basic_Operator_for_2_Operands: two parameter Operator
  //***************************************************************

  /*! Base class for expression tree operators, that calculate the result
    from two operands*/
  template<class T_Result, class T_Op1, class T_Op2>
  class Basic_Operator_for_2_Operands
    : public Operand_Listener
  {
    bool just_solved;		// did operator just solve the result

    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, 
			       Solve_Run_Info *info ) throw(EX);
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, Solve_Run_Info *info )
      throw(EX);

    //! disconnect operand
    virtual void disconnect( const void *ID );

    //*** virtual Operator methods ***

    /*! has to calculate the result when both are_operands_ok and 
      are_operands_enough return true */
    virtual T_Result calc_result( const T_Op1 &value1, 
				  const T_Op2 &value2 ) = 0; 

    //! is operand1 ok, or should it be rejected
    virtual bool is_operand1_ok( const T_Op1 &test_value ) { return true; }
    //! is operand2 ok, or should it be rejected
    virtual bool is_operand2_ok( const T_Op2 &test_value ) { return true; }
    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/*val*/ ) { return false;}
    //! can result be calculated only with operand2
    virtual bool is_operand2_enough( const T_Op2 &/*val*/ ) { return false;}
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2 ) { return true; }
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2 )
    { return true; }
    /*! has to calculate result only with operand1 when is_operand1_enough 
      returns true */
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 ) 
    { assert(0); }	// function doesn't need to be implemented

    /*! has to calculate result only with operand2 when is_operand2_enough 
      returns true */
    virtual T_Result calc_result_from_op2( const T_Op2 &value2 ) 
    { assert(0); }	// function doesn't need to be implemented

    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;

    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw( EX );
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Operator_for_2_Operands( Operand<T_Op1> &operand1, 
				   Operand<T_Op2> &operand2 ) throw(EX); 
    virtual ~Basic_Operator_for_2_Operands() {}
  };

  //**********************************************
  // Not_Operator: operator for inverting operand
  //**********************************************

  template<class T_Result, class T_Operand>
  class Not_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Operand &value ); 

  public:
    Not_Operator( Operand<T_Operand> &operand1 );
  };

  // ! Operator for operand expression trees
  inline Operand<bool>& operator!( Operand<bool> &op );
  inline Operand<bool>& operator!( Operand<values::Scalar> &op );
  inline Operand<bool>& operator!( Operand<values::Vector> &op );
  inline Operand<bool>& operator!( Operand<values::Matrix> &op );
  inline Operand<bool>& operator!( Operand<values::String> &op );

  //*****************************************************
  // Negative_Operator: operator for negative of operand 
  //*****************************************************

  template<class T_Result, class T_Operand>
  class Negative_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Operand &value ); 

  public:
    Negative_Operator( Operand<T_Operand> &operand1 );
  };

  // - Operator for operand expression trees
  inline Operand<values::Scalar>& operator-( Operand<values::Scalar> &op );
  inline Operand<values::Vector>& operator-( Operand<values::Vector> &op );

  //*************************************************************************
  // Abs_Operator: operator for calculating the absolute value of an operand 
  //*************************************************************************

  template<class T_Result, class T_Operand>
  class Abs_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Operand &value ); 

  public:
    Abs_Operator( Operand<T_Operand> &operand1 );
  };

  // abs function for operand expression trees
  inline Operand<values::Scalar>& abs( Operand<values::Scalar> &op );
  inline Operand<values::Scalar>& abs( Operand<values::Vector> &op );

  //*************************************************************************
  // Sqrt_Operator: operator for calculating the square root of an operand 
  //*************************************************************************

  template<class T_Result, class T_Operand>
  class Sqrt_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Operand &value ); 

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value );
  public:
    Sqrt_Operator( Operand<T_Operand> &operand1 );
  };

  // sqrt function for operand expression trees
  inline Operand<values::Scalar>& sqrt( Operand<values::Scalar> &op );

  //***************************************************************************
  // Plus_Minus_Operator: result is either the positive or the negative operand
  //***************************************************************************

  template<class T_Result, class T_Operand>
  class Plus_Minus_Operator
    : public Basic_Dual_Solution_Operator_for_1_Operand<T_Result, T_Operand> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough1 return true */
    virtual T_Result calc_result1( const T_Operand &value ); 

    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough2 return true */
    virtual T_Result calc_result2( const T_Operand &value ); 

  public:
    Plus_Minus_Operator( Operand<T_Operand> &operand1 );
  };

  //! +- Operator for operand expression trees (+-scalar)
  inline Operand<values::Scalar>& plus_minus( Operand<values::Scalar> &op );
  //! +- Operator for operand expression trees (+-vector)
  inline Operand<values::Vector>& plus_minus( Operand<values::Vector> &op );

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Add_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Add_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar + scalar
  inline Operand<values::Scalar>& 
  operator+( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Scalar>& 
  operator+( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<values::Scalar>& 
  operator+( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector + vector
  inline Operand<values::Vector>& 
  operator+( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<values::Vector>& 
  operator+( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<values::Vector>& 
  operator+( const values::Vector &op1, Operand<values::Vector> &op2 );

  // string + string
  inline Operand<values::String>& 
  operator+( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<values::String>& 
  operator+( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<values::String>& 
  operator+( const values::String &op1, Operand<values::String> &op2 );

  //**********************************************************************
  // Sub_Operator: operator for subtracting 2 operands of different types
  //**********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Sub_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Sub_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar - scalar
  inline Operand<values::Scalar>& 
  operator-( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Scalar>& 
  operator-( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<values::Scalar>& 
  operator-( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector - vector
  inline Operand<values::Vector>& 
  operator-( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<values::Vector>& 
  operator-( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<values::Vector>& 
  operator-( const values::Vector &op1, Operand<values::Vector> &op2 );

  //**********************************************************************
  // Mul_Operator: operator for multiplying 2 operands of different types
  //**********************************************************************

  // !!! Mul Operator might not work with some types !!!

  template<class T_Result, class T_Op1, class T_Op2>
  class Mul_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/*val*/ );
    //! can result be calculated only with operand2
    virtual bool is_operand2_enough( const T_Op2 &/*val*/ );
    /*! has to calculate result only with operand1 when is_operand1_enough 
      returns true */
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 );
    /*! has to calculate result only with operand2 when is_operand2_enough 
      returns true */
    virtual T_Result calc_result_from_op2( const T_Op2 &value2 );
  public:
    Mul_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar * scalar
  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Scalar>& 
  operator*( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<values::Scalar>& 
  operator*( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // scalar * vector
  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, Operand<values::Vector> &op2 );
  inline Operand<values::Vector>& 
  operator*( Operand<values::Scalar> &op1, const values::Vector &op2 );
  inline Operand<values::Vector>& 
  operator*( const values::Scalar &op1, Operand<values::Vector> &op2 );

  // vector * scalar
  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Vector>& 
  operator*( Operand<values::Vector> &op1, const values::Scalar &op2 );
  inline Operand<values::Vector>& 
  operator*( const values::Vector &op1, Operand<values::Scalar> &op2 );

  // matrix * matrix
  inline Operand<values::Matrix>& 
  operator*( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 );
  inline Operand<values::Matrix>& 
  operator*( Operand<values::Matrix> &op1, const values::Matrix &op2 );
  inline Operand<values::Matrix>& 
  operator*( const values::Matrix &op1, Operand<values::Matrix> &op2 );

  // matrix * vector
  inline Operand<values::Vector>& 
  operator*( Operand<values::Matrix> &op1, Operand<values::Vector> &op2 );
  inline Operand<values::Vector>& 
  operator*( Operand<values::Matrix> &op1, const values::Vector &op2 );
  inline Operand<values::Vector>& 
  operator*( const values::Matrix &op1, Operand<values::Vector> &op2 );

  //**********************************************************************
  // Div_Operator: operator for dividing 2 operands of different types
  //**********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Div_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

    //! is operand2 ok, or should it be rejected
    virtual bool is_operand2_ok( const T_Op2 &test_value );
    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/*val*/ ); 
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2 );
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2 );
    /*! has to calculate result only with operand1 when is_operand1_enough 
      returns true */
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 );
  public:
    Div_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar / scalar
  inline Operand<values::Scalar>& 
  operator/( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Scalar>& 
  operator/( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<values::Scalar>& 
  operator/( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector / scalar
  inline Operand<values::Vector>& 
  operator/( Operand<values::Vector> &op1, Operand<values::Scalar> &op2 );
  inline Operand<values::Vector>& 
  operator/( Operand<values::Vector> &op1, const values::Scalar &op2 );
  inline Operand<values::Vector>& 
  operator/( const values::Vector &op1, Operand<values::Scalar> &op2 );

  //***************************************************
  // Equal_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Equal_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Equal_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // bool == bool
  inline Operand<bool>& 
  operator==( Operand<bool> &op1, Operand<bool> &op2 );
  inline Operand<bool>& 
  operator==( Operand<bool> &op1, bool op2 );
  inline Operand<bool>& 
  operator==( bool op1, Operand<bool> &op2 );

  // scalar == scalar
  inline Operand<bool>&
  operator==( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator==( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator==( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector == vector
  inline Operand<bool>&
  operator==( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator==( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator==( const values::Vector &op1, Operand<values::Vector> &op2 );

  // matrix == matrix
  inline Operand<bool>&
  operator==( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 );
  inline Operand<bool>&
  operator==( Operand<values::Matrix> &op1, const values::Matrix &op2 );
  inline Operand<bool>&
  operator==( const values::Matrix &op1, Operand<values::Matrix> &op2 );

  // string == string
  inline Operand<bool>&
  operator==( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator==( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator==( const values::String &op1, Operand<values::String> &op2 );

  //***************************************************
  // Unequal_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Unequal_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Unequal_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // bool != bool
  inline Operand<bool>& 
  operator!=( Operand<bool> &op1, Operand<bool> &op2 );
  inline Operand<bool>& 
  operator!=( Operand<bool> &op1, bool op2 );
  inline Operand<bool>& 
  operator!=( bool op1, Operand<bool> &op2 );

  // scalar != scalar
  inline Operand<bool>&
  operator!=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator!=( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator!=( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector != vector
  inline Operand<bool>&
  operator!=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator!=( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator!=( const values::Vector &op1, Operand<values::Vector> &op2 );

  // matrix != matrix
  inline Operand<bool>&
  operator!=( Operand<values::Matrix> &op1, Operand<values::Matrix> &op2 );
  inline Operand<bool>&
  operator!=( Operand<values::Matrix> &op1, const values::Matrix &op2 );
  inline Operand<bool>&
  operator!=( const values::Matrix &op1, Operand<values::Matrix> &op2 );

  // string != string
  inline Operand<bool>&
  operator!=( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator!=( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator!=( const values::String &op1, Operand<values::String> &op2 );

  //***************************************************
  // Less_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Less_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Less_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar < scalar
  inline Operand<bool>&
  operator<( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator<( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator<( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector < vector
  inline Operand<bool>&
  operator<( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator<( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator<( const values::Vector &op1, Operand<values::Vector> &op2 );

  // string < string
  inline Operand<bool>&
  operator<( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator<( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator<( const values::String &op1, Operand<values::String> &op2 );

  //***************************************************
  // Greater_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Greater_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Greater_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar > scalar
  inline Operand<bool>&
  operator>( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator>( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator>( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector > vector
  inline Operand<bool>&
  operator>( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator>( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator>( const values::Vector &op1, Operand<values::Vector> &op2 );

  // string > string
  inline Operand<bool>&
  operator>( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator>( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator>( const values::String &op1, Operand<values::String> &op2 );

  //***************************************************
  // Not_Greater_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Not_Greater_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Not_Greater_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar <= scalar
  inline Operand<bool>&
  operator<=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator<=( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator<=( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector <= vector
  inline Operand<bool>&
  operator<=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator<=( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator<=( const values::Vector &op1, Operand<values::Vector> &op2 );

  // string <= string
  inline Operand<bool>&
  operator<=( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator<=( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator<=( const values::String &op1, Operand<values::String> &op2 );

  //***************************************************
  // Not_Less_Operator: operator for comparing 2 operands 
  //***************************************************

  template<class T_Result=bool, class T_Op1, class T_Op2>
  class Not_Less_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
    /*! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true */
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Not_Less_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  //**********
  // Operators

  // scalar >= scalar
  inline Operand<bool>&
  operator>=( Operand<values::Scalar> &op1, Operand<values::Scalar> &op2 );
  inline Operand<bool>&
  operator>=( Operand<values::Scalar> &op1, const values::Scalar &op2 );
  inline Operand<bool>&
  operator>=( const values::Scalar &op1, Operand<values::Scalar> &op2 );

  // vector >= vector
  inline Operand<bool>&
  operator>=( Operand<values::Vector> &op1, Operand<values::Vector> &op2 );
  inline Operand<bool>&
  operator>=( Operand<values::Vector> &op1, const values::Vector &op2 );
  inline Operand<bool>&
  operator>=( const values::Vector &op1, Operand<values::Vector> &op2 );

  // string >= string
  inline Operand<bool>&
  operator>=( Operand<values::String> &op1, Operand<values::String> &op2 );
  inline Operand<bool>&
  operator>=( Operand<values::String> &op1, const values::String &op2 );
  inline Operand<bool>&
  operator>=( const values::String &op1, Operand<values::String> &op2 );

  //***************
  // test function
  //***************
  int operator_test();
}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operator_templ.cpp"

// include implementation to enable inline functions to be expanded
#include "operator_inline.cpp"

#endif

