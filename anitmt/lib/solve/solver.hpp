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

#ifndef __Solve_Solver__
#define __Solve_Solver__

#include <val/val.hpp>
#include "operand.hpp"

namespace solve
{

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

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw();
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
  // Basic_Dual_Solution_Solver_for_2_Operands: 
  //                              base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  template< class T_A, class T_B >
  class Basic_Dual_Solution_Solver_for_2_Operands : public Operand_Listener
  {
    Operand<T_A> &a;
    Operand<T_B> &b;

    bool just_solving;           // is set while testing result of solved value

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw();
    // disconnect operand
    void disconnect( const void *ID );

    //*************************************
    // Virtual methods for concrete solvers

    virtual T_A calc_a1( const T_B &b ) = 0;
    virtual T_A calc_a2( const T_B &b ) { assert(0); }
    virtual T_B calc_b1( const T_A &a ) = 0;
    virtual T_B calc_b2( const T_A &a ) { assert(0); }

    virtual bool is_a_ok( const T_A &a, const T_B &b, bool avail_b ) 
    { return true; }
    virtual bool is_b_ok( const T_A &a, const T_B &b, bool avail_a ) 
    { return true; }

    virtual bool is_a1_calcable( const T_B & ) { return true; }
    virtual bool is_a2_calcable( const T_B & ) { return false; }
    virtual bool is_b1_calcable( const T_A & ) { return true; }
    virtual bool is_b2_calcable( const T_A & ) { return false; }

  public:
    Basic_Dual_Solution_Solver_for_2_Operands( Operand<T_A> &a, 
					       Operand<T_B> &b );
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

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw();
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
  // Equal_Solver: solves a = b in any direction
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
  inline void equal_solver( Operand<values::Scalar> &a, 
			    Operand<values::Scalar> &b )
  {
    new Equal_Solver<values::Scalar,values::Scalar>( a, b );
  }
  inline void equal_solver( Operand<values::Vector> &a, 
			    Operand<values::Vector> &b )
  {
    new Equal_Solver<values::Vector,values::Vector>( a, b );
  }
  inline void equal_solver( Operand<values::Matrix> &a, 
			    Operand<values::Matrix> &b )
  {
    new Equal_Solver<values::Matrix,values::Matrix>( a, b );
  }
  inline void equal_solver( Operand<values::String> &a, 
			    Operand<values::String> &b )
  {
    new Equal_Solver<values::String,values::String>( a, b );
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
  inline void sum_solver( Operand<values::Scalar> &a, 
			  Operand<values::Scalar> &b, 
			  Operand<values::Scalar> &c )
  {
    new Sum_Solver<values::Scalar,values::Scalar,values::Scalar>( a, b, c );
  }

  inline void sum_solver( Operand<values::Vector> &a, 
			  Operand<values::Vector> &b, 
			  Operand<values::Vector> &c )
  {
    new Sum_Solver<values::Vector,values::Vector,values::Vector>( a, b, c );
  }

  //***************************************************
  // Product_Solver: solves a = b * c in any direction
  //***************************************************

  template< class T_A, class T_B, class T_C >
  class Product_Solver : public Basic_Solver_for_3_Operands<T_A,T_B,T_C>
  {
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
  inline void product_solver( Operand<values::Scalar> &a, 
			      Operand<values::Scalar> &b, 
			      Operand<values::Scalar> &c )
  {
    new Product_Solver<values::Scalar,values::Scalar,values::Scalar>(a, b, c);
  }

  //************************************************
  // Square_Solver: solves a = b^2 in any direction
  //************************************************

  template< class T_A, class T_B >
  class Square_Solver 
    : public Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>
  {
    virtual T_A calc_a1( const T_B &b );
    virtual T_B calc_b1( const T_A &a );
    virtual T_B calc_b2( const T_A &a );
    virtual bool is_b2_calcable( const T_A & ) { return true; }

    virtual bool is_a_ok( const T_A &a, const T_B &b, bool avail_b );
  public:
    // Square solver a = b
    Square_Solver( Operand<T_A> &a, Operand<T_B> &b )
      : Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>(a,b) {}
  };

  // a = b^2
  inline void square_solver( Operand<values::Scalar> &a, 
			     Operand<values::Scalar> &b )
  {
    new Square_Solver<values::Scalar,values::Scalar>( a, b );
  }

  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************

  void accel_solver( Operand<values::Scalar> &s, 
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
