/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __Solve_Operator__
#define __Solve_Operator__

#include <set>

#include <val/val.hpp>
#include <message/message.hpp>

#include "operand.hpp"

namespace solve
{
  //***************************************************************
  // Basic_Operator_for_1_Operand: one parameter Operator
  //***************************************************************

  /*! Base class for expression tree operators, that calculate the results
    from one operand
    name in afd: one_operand_operator */
  template<class T_Result, class T_Operand>
  class Basic_Operator_for_1_Operand 
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

    //*** virtual Operator methods ***

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value,
				Solve_Run_Info* ) { return true; }
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
    void init() throw();
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Operator_for_1_Operand( Operand<T_Operand> &operand ) throw();
    virtual ~Basic_Operator_for_1_Operand() {}
  };

  //********************************************************************
  // Basic_Dual_Solution_Operator_for_1_Operand: one parameter Operator
  //********************************************************************

  /*! Base class for expression tree operators, that have two possible results
    from one operand
    name in afd: one_operand_dual_solution_operator*/
  template<class T_Result, class T_Operand>
  class Basic_Dual_Solution_Operator_for_1_Operand 
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, Solve_Run_Info *info ) 
      throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

    //*** virtual Operator methods ***

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value,
				Solve_Run_Info* ) { return true; }
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
    void init() throw();
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Dual_Solution_Operator_for_1_Operand( Operand<T_Operand> &operand ) 
      throw();
    virtual ~Basic_Dual_Solution_Operator_for_1_Operand() {}
  };

  //***************************************************************
  // Basic_Operator_for_2_Operands: two parameter Operator
  //***************************************************************

  /*! Base class for expression tree operators, that calculate the result
    from two operands
    name in afd: two_operands_operator */
  template<class T_Result, class T_Op1, class T_Op2>
  class Basic_Operator_for_2_Operands
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

    //*** virtual Operator methods ***

    /*! has to calculate the result when both are_operands_ok and 
      are_operands_enough return true */
    virtual T_Result calc_result( const T_Op1 &value1, 
				  const T_Op2 &value2 ) = 0; 

    //! is operand1 ok, or should it be rejected
    virtual bool is_operand1_ok( const T_Op1 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand2 ok, or should it be rejected
    virtual bool is_operand2_ok( const T_Op2 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/*val*/ ) { return false;}
    //! can result be calculated only with operand2
    virtual bool is_operand2_enough( const T_Op2 &/*val*/ ) { return false;}
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2,
				  Solve_Run_Info* ) { return true; }
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2 )
    { return true; }
    /*! has to calculate result only with operand1 when is_operand1_enough 
      returns true */
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 ) 
    { assert(0); return result(); } // function doesn't need to be implemented

    /*! has to calculate result only with operand2 when is_operand2_enough 
      returns true */
    virtual T_Result calc_result_from_op2( const T_Op2 &value2 ) 
    { assert(0); return result(); } // function doesn't need to be implemented

    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;

    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw();
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Operator_for_2_Operands( Operand<T_Op1> &operand1, 
				   Operand<T_Op2> &operand2 ) throw(); 
    virtual ~Basic_Operator_for_2_Operands() {}
  };


  //*****************************************************************
  // Basic_Simple_Operator_for_3_Operands: three parameter Operator
  //					   without early calculation
  //*****************************************************************

  /*! Base class for expression tree operators, that calculate the result
    from three operands
    name in afd: simple_three_operands_operator */
  template<class T_Result, class T_Op1, class T_Op2, class T_Op3>
  class Basic_Simple_Operator_for_3_Operands
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

    //*** virtual Operator methods ***

    /*! has to calculate the result when both are_operands_ok and 
      are_operands_enough return true */
    virtual T_Result calc_result( const T_Op1 &value1, 
				  const T_Op2 &value2, 
				  const T_Op3 &value3 ) = 0; 

    //! is operand1 ok, or should it be rejected
    virtual bool is_operand1_ok( const T_Op1 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand2 ok, or should it be rejected
    virtual bool is_operand2_ok( const T_Op2 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand3 ok, or should it be rejected
    virtual bool is_operand3_ok( const T_Op3 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2,
				  const T_Op3 &test_value3,
				  Solve_Run_Info* ) { return true; }
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2, 
				      const T_Op3 &test_value3 )
    { return true; }

    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;
    Operand<T_Op3> &operand3;

    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw();
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Simple_Operator_for_3_Operands( Operand<T_Op1> &operand1, 
					  Operand<T_Op2> &operand2, 
					  Operand<T_Op3> &operand3 ) throw(); 
    virtual ~Basic_Simple_Operator_for_3_Operands() {}
  };

  //*****************************************************************
  // Basic_Simple_Operator_for_4_Operands: three parameter Operator
  //					   without early calculation
  //*****************************************************************

  /*! Base class for expression tree operators, that calculate the result
    from three operands
    name in afd: simple_four_operands_operator */
  template<class T_Result, class T_Op1, class T_Op2, class T_Op3, class T_Op4>
  class Basic_Simple_Operator_for_4_Operands
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

    //*** virtual Operator methods ***

    /*! has to calculate the result when both are_operands_ok and 
      are_operands_enough return true */
    virtual T_Result calc_result( const T_Op1 &value1, 
				  const T_Op2 &value2, 
				  const T_Op3 &value3, 
				  const T_Op4 &value4 ) = 0; 

    //! is operand1 ok, or should it be rejected
    virtual bool is_operand1_ok( const T_Op1 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand2 ok, or should it be rejected
    virtual bool is_operand2_ok( const T_Op2 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand3 ok, or should it be rejected
    virtual bool is_operand3_ok( const T_Op3 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! is operand4 ok, or should it be rejected
    virtual bool is_operand4_ok( const T_Op3 &test_value,
				 Solve_Run_Info* ) { return true; }
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2,
				  const T_Op3 &test_value3,
				  const T_Op4 &test_value4,
				  Solve_Run_Info* ) { return true; }
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2, 
				      const T_Op3 &test_value3, 
				      const T_Op4 &test_value4 )
    { return true; }

    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;
    Operand<T_Op3> &operand3;
    Operand<T_Op4> &operand4;

    Operand<T_Result> result;

  protected:
    /*! must be called from constructors of derived classes 
      (calls virtual functions) */
    void init() throw();
  public:
    inline Operand<T_Result> &get_result() { return result; }

    Basic_Simple_Operator_for_4_Operands( Operand<T_Op1> &operand1, 
					  Operand<T_Op2> &operand2, 
					  Operand<T_Op3> &operand3, 
					  Operand<T_Op4> &operand4 ) throw(); 
    virtual ~Basic_Simple_Operator_for_4_Operands() {}
  };

  //***************************************************************************
  // Basic_Multi_Operand_Operator: base class of operators with an unlimited 
  //				   number of operands
  //***************************************************************************

  //! base class for operators for a sequence of calculations with any number
  //! of arguments
  template< class OP, class RES >
  class Basic_Multi_Operand_Operator 
    : public Operand_Listener, public message::Message_Reporter
  {
  private:
    typedef std::set<Operand<OP>*> operands_type;
    operands_type operands;
    int num, num_solved, num_solved_in_try;
    Operand<RES> result;

    bool just_solving;           // is set while testing result of solved value

    bool try_solve( Solve_Run_Info *info = 0 ); // tries to solve result

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const Basic_Operand *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const Basic_Operand *ID, Solve_Run_Info *info ) throw();
    // disconnect operand
    void disconnect( const Basic_Operand *ID );

    //***************************************
    // Virtual methods for concrete operands

    //! return initial result for calculation sequence
    virtual RES initial_result() = 0;
    //! return result when no operand was added
    virtual RES no_operands_result() = 0;
    //! returns new result for one operand and the old result (sequence calc)
    virtual RES calc( const OP &op, const RES &old_res ) = 0;

    //! is operand valid?
    virtual bool is_op_ok( const OP &op, Operand<RES> &result ){ return true; }
    //! is operand sufficient to calc result?
    virtual bool is_op_sufficient( const OP &op ) { return false; }

  public:
    void add_operand( Operand<OP> &operand ); 
    void finish_adding();
    //! get result when all operands are added
    inline Operand<RES> &get_result() { return result; }

    Basic_Multi_Operand_Operator( message::Message_Consultant *c );
    virtual ~Basic_Multi_Operand_Operator();
  };



//**********************************************
//**********************************************
//** Special Operators
//**********************************************
//**********************************************

  //***************************************************************************
  // Multi_And_Operator: Logical and with any number of operands
  //***************************************************************************

  //! Logical and with any number of operands
  class Multi_And_Operator 
    : public Basic_Multi_Operand_Operator<values::Flag,values::Flag>
  {
  private:
    typedef values::Flag RES;
    typedef values::Flag OP;

    virtual RES initial_result();	
    virtual RES no_operands_result();
    virtual RES calc( const OP &op, const RES &old_res );

    virtual bool is_op_sufficient( const OP &op );
  public:
    Multi_And_Operator( message::Message_Consultant *c );
  };

  //*******************************************************************
  // Is_Solved_Operator: operator to test whether an operand is solved
  //*******************************************************************

  //! tests whether operands are solved (but not when this is already the case)
  class Is_Solved_Operator : public Operand_Listener
  {
    Operand<values::Flag> result;

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const Basic_Operand *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const Basic_Operand *ID, Solve_Run_Info *info ) throw();
    // disconnect operand
    void disconnect( const Basic_Operand *ID );

  public:
    inline Operand<values::Flag> &get_result() { return result; }
    Is_Solved_Operator( Basic_Operand &op, 
			message::Message_Consultant *c );
  };

  template< class OP >
  inline Operand<values::Flag> &is_solved( Operand<OP> &op );

//**********************************************
//**********************************************
//** Normal Calculation Operators
//**********************************************
//**********************************************
  /* see solver.afd 
  // *********************************************
  // Not_Operator: operator for inverting operand
  // *********************************************

  template<class T_Result, class T_Operand>
  class Not_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
  private:
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

  // ****************************************************
  // Negative_Operator: operator for negative of operand 
  // ****************************************************

  template<class T_Result, class T_Operand>
  class Negative_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Operand &value ); 

  public:
    Negative_Operator( Operand<T_Operand> &operand1 );
  };

  // - Operator for operand expression trees
  inline Operand<values::Scalar>& operator-( Operand<values::Scalar> &op );
  inline Operand<values::Vector>& operator-( Operand<values::Vector> &op );

  // ************************************************************************
  // Abs_Operator: operator for calculating the absolute value of an operand 
  // ************************************************************************

  template<class T_Result, class T_Operand>
  class Abs_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Operand &value ); 

  public:
    Abs_Operator( Operand<T_Operand> &operand1 );
  };

  // abs function for operand expression trees
  inline Operand<values::Scalar>& abs( Operand<values::Scalar> &op );
  inline Operand<values::Scalar>& abs( Operand<values::Vector> &op );

  // ************************************************************************
  // Sqrt_Operator: operator for calculating the square root of an operand 
  // ************************************************************************

  template<class T_Result, class T_Operand>
  class Sqrt_Operator
    : public Basic_Operator_for_1_Operand<T_Result, T_Operand> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Operand &value ); 

    //! is operand ok, or should it be rejected
    virtual bool is_operand_ok( const T_Operand &test_value,
				Solve_Run_Info* );
  public:
    Sqrt_Operator( Operand<T_Operand> &operand1 );
  };

  // sqrt function for operand expression trees
  inline Operand<values::Scalar>& sqrt( Operand<values::Scalar> &op );

  // **************************************************************************
  // Plus_Minus_Operator: result is either the positive or the negative operand
  // **************************************************************************

  template<class T_Result, class T_Operand>
  class Plus_Minus_Operator
    : public Basic_Dual_Solution_Operator_for_1_Operand<T_Result, T_Operand> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough1 return true * /
    virtual T_Result calc_result1( const T_Operand &value ); 

    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough2 return true * /
    virtual T_Result calc_result2( const T_Operand &value ); 

  public:
    Plus_Minus_Operator( Operand<T_Operand> &operand1 );
  };

  //! +- Operator for operand expression trees (+-scalar)
  inline Operand<values::Scalar>& plus_minus( Operand<values::Scalar> &op );
  //! +- Operator for operand expression trees (+-vector)
  inline Operand<values::Vector>& plus_minus( Operand<values::Vector> &op );

  // *********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  // *********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Add_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Add_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // *********************************************************************
  // Sub_Operator: operator for subtracting 2 operands of different types
  // *********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Sub_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Sub_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // *********************************************************************
  // Mul_Operator: operator for multiplying 2 operands of different types
  // *********************************************************************

  // !!! Mul Operator might not work with some types !!!

  template<class T_Result, class T_Op1, class T_Op2>
  class Mul_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/ *val* / );
    //! can result be calculated only with operand2
    virtual bool is_operand2_enough( const T_Op2 &/ *val* / );
    / *! has to calculate result only with operand1 when is_operand1_enough 
      returns true * /
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 );
    / *! has to calculate result only with operand2 when is_operand2_enough 
      returns true * /
    virtual T_Result calc_result_from_op2( const T_Op2 &value2 );
  public:
    Mul_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // *********************************************************************
  // Div_Operator: operator for dividing 2 operands of different types
  // *********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Div_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

    //! can result be calculated only with operand1
    virtual bool is_operand1_enough( const T_Op1 &/ *val* / ); 
    //! are both operands ok, or should one be rejected
    virtual bool are_operands_ok( const T_Op1 &test_value1, 
				  const T_Op2 &test_value2,
				  Solve_Run_Info* );
    //! can result be calculated? operand won't be rejected when this is false
    virtual bool are_operands_enough( const T_Op1 &test_value1, 
				      const T_Op2 &test_value2 );
    / *! has to calculate result only with operand1 when is_operand1_enough 
      returns true * /
    virtual T_Result calc_result_from_op1( const T_Op1 &value1 );
  public:
    Div_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Equal_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Equal_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Equal_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Unequal_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Unequal_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Unequal_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Less_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Less_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Less_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Greater_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Greater_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Greater_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Not_Greater_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Not_Greater_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Not_Greater_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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

  // **************************************************
  // Not_Less_Operator: operator for comparing 2 operands 
  // **************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Not_Less_Operator
    : public Basic_Operator_for_2_Operands<T_Result, T_Op1, T_Op2> 
  {
  private:
    / *! has to calculate the result when both is_operand_ok and 
      is_operand_enough  return true * /
    virtual T_Result calc_result( const T_Op1 &value1, const T_Op2 &value2 ); 

  public:
    Not_Less_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // *********
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
  */

  // **************
  // test function
  // **************
  int operator_test();
}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operator_templ.cpp"

// include implementation to enable inline functions to be expanded
#include "operator_inline.cpp"

#endif

