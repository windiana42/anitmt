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

#include "operand_classes.hpp"

#include "val.hpp"
#include "error.hpp"

#include "property.hpp"
#include "solver.hpp"

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
    static User_Problem_Handler default_handler;
  protected:
    class Value_Reporter{
      friend Operand;
      virtual bool is_this_ok( T const &value, Solve_Run_Info const *info) = 0;
      virtual void use_it () = 0;
    };

    T value;			// to store the value
    bool solved;
    int last_test_run_id;

    Value_Reporter *reporter; 

    inline void report_value() {
      if( reporter ) reporter->use_it();
    }
    inline bool report_test_value( T res, Solve_Run_Info const *info ) {
      if( reporter ) 
	return reporter->is_this_ok( res, info );
      
      return true;
    }

    bool test_set_value( T res, Solve_Run_Info const *info );
  public:	
    inline bool is_solved_in_try( Solve_Run_Info const *info ) 
    { return solved || info->is_id_valid( last_test_run_id ); }
    inline bool is_solved() const { return solved; }
    inline const T& get_value() const { assert( is_solved() ); return value; }
    inline void use_value() 
    { solved = true; report_value(); }
    inline void set_value( T res, 
			   Solve_Problem_Handler *handler = &default_handler ) 
    { 
      Solve_Run_Info info( handler );
      if(test_set_value(res,&info)) report_value(); 
    }

    void init_reporter( Value_Reporter *reporter );

    Operand();
  };

  //*************************************************************************
  // Store_Operand_to_Property: stores the value of an operand to a property
  //*************************************************************************

  template<class T> 
  class Store_Operand_to_Property 
    : public Operand<T>::Value_Reporter, public Solver {
    Type_Property<T> *property;

    //** Operand Methods **

    virtual bool is_this_ok( T const &value, Solve_Run_Info const *info );
    virtual void use_it ();

    //** Solver Methods **

    // is called when property was solved
    virtual void do_when_prop_was_solved( Property *ID ) {}
    // calculates results of a solved Property (ID) and returns wheather
    // the solution is ok
    virtual bool check_prop_solution_and_results
    ( Property *ID, Solve_Run_Info const *info ){}

  public:
    Store_Operand_to_Property( Operand<T> &op, Type_Property<T> *prop ); 
  };

  template< class T >
  void assign( Type_Property<T> &property, Operand<T> &operand ) {
    new Store_Operand_to_Property<T>( operand, &property );
  }

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
    virtual bool is_this_ok( T_Operand const &value, 
			     Solve_Run_Info const *info );
    virtual void use_it();

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
    : public Operand<T_Result>{
    // Interface classes for operands to deliver result to
    class Operand_1_Interface : public Operand<T_Op1>::Value_Reporter{
      Basic_Operator_for_2_params *host;
      virtual bool is_this_ok( T_Op1 const &value, 
			       Solve_Run_Info const *info );
      virtual void use_it();
    public:
      Operand_1_Interface( Basic_Operator_for_2_params *h ) : host(h) {}
    };
    class Operand_2_Interface : public Operand<T_Op2>::Value_Reporter{
      Basic_Operator_for_2_params *host;
      virtual bool is_this_ok( T_Op2 const &value, 
			       Solve_Run_Info const *info );
      virtual void use_it();
    public:
      Operand_2_Interface( Basic_Operator_for_2_params *h ) : host(h) {}
    };
    friend class Operand_1_Interface; // both interface classes
    friend class Operand_2_Interface; // have direct access to host object

    bool just_solved;		// did operator just solve the result

    virtual bool is_operand1_ok( T_Op1 const &test_value ) { return true; }
    virtual bool is_operand2_ok( T_Op2 const &test_value ) { return true; }
    virtual bool are_operands_ok( T_Op1 const &test_value1, T_Op2 test_value2 )
    { return true; }
    virtual T_Result calc_result( T_Op1 const &value1, 
				  T_Op2 const &value2 ) = 0; 
				// may throw exception!

    Operand_1_Interface *interface1;
    Operand_2_Interface *interface2;
    Operand<T_Op1> &operand1;
    Operand<T_Op2> &operand2;

  protected:
    // must be called by constuctors of derived classes!!!
    void init(); 
  public:
    Basic_Operator_for_2_params( Operand<T_Op1> &operand1, 
				 Operand<T_Op2> &operand2 ); 
				// may throw exception if operands are not ok!
    ~Basic_Operator_for_2_params();
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

  // ! Operator with operand type as result type
  template< class T >
  Operand<T>& operator!( Operand<T> &op ) {
    return *new Not_Operator<T,T>( op );
  }

  //**********************************************************************
  // Add_Operator: operator for adding 2 operands of different types
  //**********************************************************************

  template<class T_Result, class T_Op1, class T_Op2>
  class Add_Operator
    : public Basic_Operator_for_2_params<T_Result, T_Op1, T_Op2> {

    virtual T_Result calc_result( T_Op1 const &value1, T_Op2 const &value2 ); 
				// may throw exception!

  public:
    Add_Operator( Operand<T_Op1> &operand1, Operand<T_Op2> &operand2 );
  };

  // + Operator with type of parameter 2 as result type
  template< class T1, class T2 >
  Operand<T2>& operator+( Operand<T1> &op1, Operand<T2> &op2 ) {
    return *new Add_Operator<T2,T1,T2>( op1, op2 );
  }

  // + Operator with type of parameter 2 as result type
  template< class T1 >
  Operand<T1>& operator+( Operand<T1> &op1, values::Scalar op2 ) {
    return *new Add_Operator<T1,T1,values::Scalar>
      ( op1, *(new Constant<values::Scalar>(op2)) );
  }

  // + Operator with type of parameter 2 as result type
  template< class T2 >
  Operand<T2>& operator+( values::Scalar op1, Operand<T2> &op2 ) {
    return *new Add_Operator<T2,values::Scalar,T2>
      ( *(new Constant<values::Scalar>(op1)), op2 );
  }

  // + Operator with type of parameter 2 as result type
  template< class T1 >
  Operand<T1>& operator+( Operand<T1> &op1, values::Vector op2 ) {
    return *new Add_Operator<T1,T1,values::Vector>
      ( op1, *(new Constant<values::Vector>(op2)) );
  }

  // + Operator with type of parameter 2 as result type
  template< class T2 >
  Operand<T2>& operator+( values::Vector op1, Operand<T2> &op2 ) {
    return *new Add_Operator<T2,values::Vector,T2>
      ( *(new Constant<values::Vector>(op1)), op2 );
  }

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operand_templ.cpp"

#endif

