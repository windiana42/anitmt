/*****************************************************************************/
/**   This file offers solver for operands          			    **/
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

#ifndef __AniTMT_Solver__
#define __AniTMT_Solver__

#include "val.hpp"
#include "error.hpp"
#include "operand.hpp"

namespace anitmt{
  // Methods
  template< class T_A, class T_B, class T_C >
  void establish_sum_solver( Operand<T_A> &a, Operand<T_A> &b, 
			     Operand<T_A> &c );
  template< class T_A, class T_B, class T_C >
  void establish_product_solver( Operand<T_A> &a, Operand<T_A> &b, 
				 Operand<T_A> &c );
  void establish_accel_solver( Operand<values::Scalar> &d, 
			       Operand<values::Scalar> &t, 
			       Operand<values::Scalar> &a, 
			       Operand<values::Scalar> &v0, 
			       Operand<values::Scalar> &ve );
}

namespace anitmt{

  //*************************************************************************
  // Basic_Solver_for_2_Operands: base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  template< class T_A, class T_B >
  class Basic_Solver_for_2_Operands : public Operand_Listener
  {
    Operand<T_A> &a;
    Operand<T_B> &b;

    bool just_solving;           // is set while testing result of solved value
    bool just_solved_a, just_solved_b;

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw(EX);
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw(EX);
    // disconnect operand
    void disconnect( const void *ID );

    //*************************************
    // Virtual methods for concrete solvers

    virtual T_A calc_a( const T_B &b ) = 0;
    virtual T_B calc_b( const T_A &a ) = 0;

    virtual bool is_a_ok( const T_A &a, const T_B &b, bool avail_b ) 
    { return true; }
    virtual bool is_b_ok( const T_A &a, const T_B &b, bool avail_a ) 
    { return true; }

    virtual bool is_a_calcable( const T_B & ) { return true; }
    virtual bool is_b_calcable( const T_A & ) { return true; }

  public:
    Basic_Solver_for_2_Operands( Operand<T_A> &a, Operand<T_B> &b );
  };

  //*************************************************************************
  // Basic_Solver_for_3_Operands: base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  template< class T_A, class T_B, class T_C >
  class Basic_Solver_for_3_Operands : public Operand_Listener
  {
    Operand<T_A> &a;
    Operand<T_B> &b;
    Operand<T_C> &c;

    bool just_solving;           // is set while testing result of solved value
    bool just_solved_a, just_solved_b, just_solved_c;

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw(EX);
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw(EX);
    // disconnect operand
    void disconnect( const void *ID );

    //*************************************
    // Virtual methods for concrete solvers

    virtual T_A calc_a( const T_B &b, const T_C &c ) = 0;
    virtual T_B calc_b( const T_A &a, const T_C &c ) = 0;
    virtual T_C calc_c( const T_A &a, const T_B &b ) = 0;

    virtual bool is_a_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_b, bool avail_c ) { return true; }
    virtual bool is_b_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_a, bool avail_c ) { return true; }
    virtual bool is_c_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_a, bool avail_b ) { return true; }

    virtual bool is_a_calcable( const T_B &, const T_C & ) { return true; }
    virtual bool is_b_calcable( const T_A &, const T_C & ) { return true; }
    virtual bool is_c_calcable( const T_A &, const T_B & ) { return true; }

    virtual bool is_b_calcable_from_a( const T_A &/*a*/ ) { return false; }
    virtual bool is_c_calcable_from_a( const T_A &/*a*/ ) { return false; }
    virtual bool is_a_calcable_from_b( const T_B &/*b*/ ) { return false; }
    virtual bool is_c_calcable_from_b( const T_B &/*b*/ ) { return false; }
    virtual bool is_a_calcable_from_c( const T_C &/*c*/ ) { return false; }
    virtual bool is_b_calcable_from_c( const T_C &/*c*/ ) { return false; }

    virtual T_A calc_a_from_b( const T_B &/*b*/ ) { assert(0); }
    virtual T_A calc_a_from_c( const T_C &/*c*/ ) { assert(0); }
    virtual T_B calc_b_from_a( const T_A &/*a*/ ) { assert(0); }
    virtual T_B calc_b_from_c( const T_C &/*c*/ ) { assert(0); }
    virtual T_C calc_c_from_a( const T_A &/*a*/ ) { assert(0); }
    virtual T_C calc_c_from_b( const T_B &/*b*/ ) { assert(0); }
  public:
    Basic_Solver_for_3_Operands( Operand<T_A> &a, Operand<T_B> &b,
				 Operand<T_C> &c );
    };

  //************************************************
  // Equal_Solver: solves a = b + c in any direction
  //************************************************

  template< class T_A, class T_B >
  class Equal_Solver : public Basic_Solver_for_2_Operands<T_A,T_B>
  {
    virtual T_A calc_a( const T_B &b ) { return b; }
    virtual T_B calc_b( const T_A &a ) { return a; }
  public:
    // Equal solver a = b
    Equal_Solver( Operand<T_A> &a, Operand<T_B> &b )
      : Basic_Solver_for_2_Operands<T_A,T_B>(a,b) {}
  };

  // a = b 
  template< class T_A, class T_B >
  void establish_equal_solver( Operand<T_A> &a, Operand<T_B> &b )
  {
    new Equal_Solver<T_A,T_B>( a, b );
  }

  //************************************************
  // Sum_Solver: solves a = b + c in any direction
  //************************************************

  template< class T_A, class T_B, class T_C >
  class Sum_Solver : public Basic_Solver_for_3_Operands<T_A,T_B,T_C>
  {
    virtual T_A calc_a( const T_B &b, const T_C &c );
    virtual T_B calc_b( const T_A &a, const T_C &c );
    virtual T_C calc_c( const T_A &a, const T_B &b );
  public:
    // Sum  solver a = b + c
    // Diff solver c = a - b
    Sum_Solver( Operand<T_A> &a, Operand<T_B> &b, Operand<T_C> &c );
  };

  // a = b + c
  template< class T_A, class T_B, class T_C >
  void establish_sum_solver( Operand<T_A> &a, Operand<T_B> &b, 
			     Operand<T_C> &c )
  {
    new Sum_Solver<T_A,T_B,T_C>( a, b, c );
  }

  //***************************************************
  // Product_Solver: solves a = b * c in any direction
  //***************************************************

  class Product_Solver : public Basic_Solver_for_3_Operands
  <values::Scalar, values::Scalar, values::Scalar>
  {
    typedef values::Scalar T_A;
    typedef values::Scalar T_B;
    typedef values::Scalar T_C;

    virtual T_A calc_a( const T_B &b, const T_C &c );
    virtual T_B calc_b( const T_A &a, const T_C &c );
    virtual T_C calc_c( const T_A &a, const T_B &b );

    virtual bool is_a_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_b, bool avail_c );
    virtual bool is_b_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_a, bool avail_c );
    virtual bool is_c_ok( const T_A &a, const T_B &b, const T_C &c, 
			  bool avail_a, bool avail_b );

    virtual bool is_b_calcable( const T_A &, const T_C & );
    virtual bool is_c_calcable( const T_A &, const T_B & );

    virtual bool is_a_calcable_from_b( const T_B &b );
    virtual bool is_a_calcable_from_c( const T_C &c );

    virtual T_A calc_a_from_b( const T_B &/*b*/ );
    virtual T_A calc_a_from_c( const T_C &/*c*/ );
  public:
    // Product  solver a = b * c
    // Relation solver c = a / b
    Product_Solver( Operand<T_A> &a, Operand<T_B> &b, Operand<T_C> &c );
  };

  // a = b * c
  template< class T_A, class T_B, class T_C >
  void establish_product_solver( Operand<T_A> &a, Operand<T_B> &b, 
				 Operand<T_C> &c )
  {
    new Product_Solver( a, b, c );
  }

  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************
  /*
    class Accel_Solver : public Solver{
    Scalar_Property &d;		// differance
    Scalar_Property &t;		// duration
    Scalar_Property &a;		// acceleration
    Scalar_Property &v0;	// startspeed
    Scalar_Property &ve;	// endspeed
    bool s_d, s_t, s_a, s_v0, s_ve; // indicates wheater a property was solved
				    // while checking a solution
  public:
    // Properties call that if they want to validate their results
    virtual bool check_prop_solution_and_results
    ( Property *ID, Solve_Run_Info const *info );

    Accel_Solver( Scalar_Property *d, Scalar_Property *t, Scalar_Property *a,
		  Scalar_Property *v0, Scalar_Property *ve );
  };
  */
  void establish_accel_solver( Operand<values::Scalar> &s, 
			       Operand<values::Scalar> &t, 
			       Operand<values::Scalar> &a, 
			       Operand<values::Scalar> &v0, 
			       Operand<values::Scalar> &ve );

  //***************
  // test function
  //***************
  int solver_test();
}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "solver_templ.cpp"

#endif
