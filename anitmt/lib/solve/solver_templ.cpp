/*****************************************************************************/
/**   This file offers solver for operands          			    **/
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

#ifndef __Solve_Solver_Templateimplementation__
#define __Solve_Solver_Templateimplementation__

#include "solver.hpp"

#include <math.h>

namespace solve
{

  //*************************************************************************
  // Basic_Solver_for_2_Operands: base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  //**********************************
  // Virtual Operand_Listener methods

  // has to check the result of the operand with ID as pointer to operand
  template< class T_A, class T_B >
  bool Basic_Solver_for_2_Operands<T_A,T_B>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    T_A _a;
    T_B _b;
    bool avail_a = a.is_solved_in_try(info);
    bool avail_b = b.is_solved_in_try(info);
    if( avail_a ) _a = a.get_value(info);
    if( avail_b ) _b = b.get_value(info);

    if( !just_solving )
    {
      just_solving = true;	
      if( ID == &a )
      {
	assert( avail_a );
	if( !is_a_ok( _a, _b, avail_b ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( is_b_calcable( _a ) )
	{
	  if( avail_b )	// is b already availible
	  {
	    if( _b != calc_b(_a) ) // check b and calculated b
	    {
	      just_solving = false;
	      return false;
	    }
	  }
	  else
	  {
	    bool just_solved_b = b.test_set_value( calc_b(_a), info );
	    just_solving = false;	
	    return just_solved_b;
	  }
	}
      }
      if( ID == &b )
      {
	assert( avail_b );
	if( !is_b_ok( _a, _b, avail_a ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( is_a_calcable( _b ) )
	{
	  if( avail_a )	// is a already availible
	  {
	    if( _a != calc_a(_b) ) // check a and calculated a
	    {
	      just_solving = false;
	      return false;
	    }
	  }
	  else
	  {
	    bool just_solved_a = a.test_set_value( calc_a(_b), info );
	    just_solving = false;	
	    return just_solved_a;
	  }
	}
      }
      just_solving = false;	
      return true;
    }
    else
    {
      if( ID == &a ) { return is_a_ok( _a, _b, avail_b ); }
      if( ID == &b ) { return is_b_ok( _a, _b, avail_a ); }
      assert( 0 );		// any operand pointer should equal ID
    }
    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template< class T_A, class T_B >
  void Basic_Solver_for_2_Operands<T_A,T_B>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID==&a || ID==&b );

    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( (ID!=&a) && a.is_solved_in_try( info ) ) a.use_test_value( info ); 
      if( (ID!=&b) && b.is_solved_in_try( info ) ) b.use_test_value( info );
      just_solving = false;
    }
  }
   
  // disconnect operand
  template< class T_A, class T_B >
  void Basic_Solver_for_2_Operands<T_A,T_B>::disconnect( const void *ID )
  {
    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( ID == &a ) 
      {
	b.rm_listener( this );
      }
      else if( ID == &b ) 
      {
	a.rm_listener( this );
      }
      else
	assert(0);

      just_solving = false;	// solving or removing...

      delete this;		// !!! no further commands !!!
    }
  }

  template< class T_A, class T_B >
  Basic_Solver_for_2_Operands<T_A,T_B>::Basic_Solver_for_2_Operands
  ( Operand<T_A> &op_a, Operand<T_B> &op_b )
    : a(op_a), b(op_b), just_solving(false)
  {
    a.add_listener( this );
    b.add_listener( this );
  }

  //*************************************************************************
  // Basic_Dual_Solution_Solver_for_2_Operands: 
  //                              base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  //**********************************
  // Virtual Operand_Listener methods

  // has to check the result of the operand with ID as pointer to operand
  template< class T_A, class T_B >
  bool Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    T_A _a;
    T_B _b;
    bool avail_a = a.is_solved_in_try(info);
    bool avail_b = b.is_solved_in_try(info);
    if( avail_a ) _a = a.get_value(info);
    if( avail_b ) _b = b.get_value(info);

    if( !just_solving )
    {
      just_solving = true;
      if( ID == &a )
      {
	assert( avail_a );
	if( !is_a_ok( _a, _b, avail_b ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( is_b1_calcable( _a ) )
	{
	  if( avail_b )	// is b already availible
	  {
	    // check b and calculated b
	    if( _b == calc_b1(_a) )
	    {	
	      just_solving = false;
	      return true;
	    }
	    // check failed; is there a second solution availible?
	    if( !is_b2_calcable( _a ) )
	    {
	      just_solving = false;	
	      return false;
	    }
	    // no problem, perhaps solution 2 is ok...
	  }
	  else
	  {
	    // change test_id for first test run
	    Solve_Run_Info::id_type test1_id = info->new_test_run_id();
	    // start first test run
	    bool just_solved_b = b.test_set_value( calc_b1(_a), info );
	    if( just_solved_b )
	    {
	      just_solving = false;	
	      return true;
	    }
	    else		// undo first test run 
	      info->remove_test_run_id( test1_id );
	  }
	}

	if( is_b2_calcable( _a ) )
	{
	  if( avail_b )	// is b already availible
	  {
	    // check b and calculated b
	    if( _b == calc_b2(_a) )
	    {	
	      just_solving = false;
	      return true;
	    }
	    else		// second check also failed...
	    {
	      just_solving = false;	
	      return false;
	    }
	  }
	  else
	  {
	    // start test run with second solution
	    bool just_solved_b = b.test_set_value( calc_b2(_a), info );
	    just_solving = false;	
	    return just_solved_b;
	  }
	}
	just_solving = false;	
	return true;
      }
      if( ID == &b )
      {
	assert( avail_b );
	if( !is_b_ok( _a, _b, avail_a ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( is_a1_calcable( _b ) )
	{
	  if( avail_a )	// is a already availible
	  {
	    // check a and calculated a
	    if( _a == calc_a1(_b) )
	    {	
	      just_solving = false;
	      return true;
	    }
	    // check failed; is there a second solution availible?
	    if( !is_a2_calcable( _b ) )
	    {
	      just_solving = false;	
	      return false;
	    }
	    // no problem, perhaps solution 2 is ok...
	  }
	  else
	  {
	    // change test_id for first test run
	    Solve_Run_Info::id_type test1_id = info->new_test_run_id();
	    // start first test run
	    bool just_solved_a = a.test_set_value( calc_a1(_b), info );
	    if( just_solved_a )
	    {
	      just_solving = false;	
	      return true;
	    }
	    else		// undo first test run 
	      info->remove_test_run_id( test1_id );
	  }
	}

	if( is_a2_calcable( _b ) )
	{
	  if( avail_a )	// is a already availible
	  {
	    // check a and calculated a
	    if( _a == calc_a2(_b) )
	    {	
	      just_solving = false;
	      return true;
	    }
	    else		// second check also failed...
	    {
	      just_solving = false;	
	      return false;
	    }
	  }
	  else
	  {
	    // start test run with second solution
	    bool just_solved_a = a.test_set_value( calc_a2(_b), info );
	    just_solving = false;	
	    return just_solved_a;
	  }
	}
	just_solving = false;	
	return true;
      }
    }
    else
    {
      if( ID == &a ) { return is_a_ok( _a, _b, avail_b ); }
      if( ID == &b ) { return is_b_ok( _a, _b, avail_a ); }
      assert( 0 );		// any operand pointer should equal ID
    }
    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template< class T_A, class T_B >
  void Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID==&a || ID==&b );

    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( (ID!=&a) && a.is_solved_in_try( info ) ) a.use_test_value( info ); 
      if( (ID!=&b) && b.is_solved_in_try( info ) ) b.use_test_value( info );
      just_solving = false;
    }
  }
   
  // disconnect operand
  template< class T_A, class T_B >
  void Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>::
  disconnect( const void *ID )
  {
    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( ID == &a ) 
      {
	b.rm_listener( this );
      }
      else if( ID == &b ) 
      {
	a.rm_listener( this );
      }
      else
	assert(0);

      just_solving = false;	// solving or removing...

      delete this;		// !!! no further commands !!!
    }
  }

  template< class T_A, class T_B >
  Basic_Dual_Solution_Solver_for_2_Operands<T_A,T_B>::
  Basic_Dual_Solution_Solver_for_2_Operands
  ( Operand<T_A> &op_a, Operand<T_B> &op_b )
    : a(op_a), b(op_b), just_solving(false)
  {
    a.add_listener( this );
    b.add_listener( this );
  }

  //*************************************************************************
  // Basic_Solver_for_3_Operands: base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  //**********************************
  // Virtual Operand_Listener methods

  // has to check the result of the operand with ID as pointer to operand
  template< class T_A, class T_B, class T_C >
  bool Basic_Solver_for_3_Operands<T_A,T_B,T_C>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    T_A _a;
    T_B _b;
    T_C _c;
    bool avail_a = a.is_solved_in_try(info);
    bool avail_b = b.is_solved_in_try(info);
    bool avail_c = c.is_solved_in_try(info);
    if( avail_a ) _a = a.get_value(info);
    if( avail_b ) _b = b.get_value(info);
    if( avail_c ) _c = c.get_value(info);

    if( !just_solving )
    {
      just_solving = true;	
      if( ID == &a )
      {
	assert( avail_a );
	if( !is_a_ok( _a, _b, _c, avail_b, avail_c ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( avail_b )
	{
	  if( is_c_calcable( _a, _b ) )
	  {
	    if( avail_c )	// are b and c already availible
	    {
	      if( _c != calc_c(_a,_b) ) // check c and calculated c
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      _c = calc_c(_a,_b);
	      bool just_solved_c = c.test_set_value( _c, info );
	      just_solving = false;	
	      return just_solved_c;
	    }
	  }
	}

	if( avail_c )
	{
	  if( is_b_calcable( _a, _c ) )
	  {
	    if( avail_b )	// are b and c already availible
	    {
	      if( _b != calc_b(_a,_c) ) // check b and calculated b
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      bool just_solved_b = b.test_set_value( calc_b(_a,_c), info );
	      just_solving = false;	
	      return just_solved_b;
	    }
	  }
	}

	if( avail_b && avail_c ) 
	{
	  just_solving = false;
	  return true;
	}

	if( is_b_calcable_from_a( _a ) )
	{
	  bool just_solved_b = b.test_set_value( calc_b_from_a(_a), info );
	  if(!just_solved_b)	// if b rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  }
	}
	if( is_c_calcable_from_a( _a ) )
	{
	  bool just_solved_c = c.test_set_value( calc_c_from_a(_a), info );
	  if(!just_solved_c)	// if c rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
      }
      if( ID == &b ) {
	assert( avail_b );
	if( !is_b_ok( _a,_b,_c,avail_a,avail_c ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( avail_a )
	{
	  if( is_c_calcable( _a, _b ) )
	  {
	    if( avail_c )
	    {
	      if( _c != calc_c(_a,_b) ) // check c and calculated c
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      bool just_solved_c = c.test_set_value( calc_c(_a,_b), info );
	      just_solving = false;	
	      return just_solved_c;
	    }
	  }
	}

	if( avail_c )
	{
	  if( is_a_calcable( _b, _c ) )
	  {
	    if( avail_a )
	    {
	      if( _a != calc_a(_b,_c) ) // check a and calculated a
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      bool just_solved_a = a.test_set_value( calc_a(_b,_c), info );
	      just_solving = false;	
	      return just_solved_a;
	    }
	  }
	}

	if( avail_a && avail_c ) 
	{
	  just_solving = false;
	  return true;
	}


	if( is_a_calcable_from_b( _b ) )
	{
	  bool just_solved_a = a.test_set_value( calc_a_from_b(_b), info );
	  if(!just_solved_a)	// if a rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
	if( is_c_calcable_from_b( _b ) )
	{
	  bool just_solved_c = c.test_set_value( calc_c_from_b(_b), info );
	  if(!just_solved_c)	// if c rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
      }
      if( ID == &c ) {
	assert( avail_c );
	if( !is_c_ok( _a,_b,_c,avail_a,avail_b ) )
	{ 
	  just_solving = false;	
	  return false; 
	}
	
	if( avail_a )
	{
	  if( is_b_calcable( _a, _c ) )
	  {
	    if( avail_b )
	    {
	      if( _b != calc_b(_a,_c) ) // check b and calculated b
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      bool just_solved_b = b.test_set_value( calc_b(_a,_c), info );
	      just_solving = false;	
	      return just_solved_b;
	    }
	  }
	}

	if( avail_b )
	{
	  if( is_a_calcable( _b, _c ) )
	  {
	    if( avail_a )
	    {
	      if( _a != calc_a(_b,_c) ) // check a and calculated a
	      {
		just_solving = false;
		return false;
	      }
	    }
	    else
	    {
	      bool just_solved_a = a.test_set_value( calc_a(_b,_c), info );
	      just_solving = false;	
	      return just_solved_a;
	    }
	  }
	}

	if( avail_a && avail_b ) 
	{
	  just_solving = false;
	  return true;
	}

	if( is_a_calcable_from_c( _c ) )
	{
	  bool just_solved_a = a.test_set_value( calc_a_from_c(_c), info );
	  if(!just_solved_a)	// if a rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
	if( is_b_calcable_from_c( _c ) )
	{
	  bool just_solved_b = b.test_set_value( calc_b_from_c(_c), info );
	  if(!just_solved_b)	// if b rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
      }
      just_solving = false;	
      return true;
    }
    else
    {
      if( ID == &a ) { return is_a_ok( _a, _b, _c, avail_b, avail_c ); }
      if( ID == &b ) { return is_b_ok( _a, _b, _c, avail_a, avail_c ); }
      if( ID == &c ) { return is_c_ok( _a, _b, _c, avail_a, avail_b ); }
      assert( 0 );		// any operand pointer should equal ID
    }
    return true;
  }

  // tells to use the result calculated by is_result_ok()
  template< class T_A, class T_B, class T_C >
  void Basic_Solver_for_3_Operands<T_A,T_B,T_C>::use_result
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID==&a || ID==&b || ID==&c );

    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( (ID!=&a) && a.is_solved_in_try( info ) ) a.use_test_value( info ); 
      if( (ID!=&b) && b.is_solved_in_try( info ) ) b.use_test_value( info );
      if( (ID!=&c) && c.is_solved_in_try( info ) ) c.use_test_value( info ); 
      just_solving = false;
    }
  }
   
  // disconnect operand
  template< class T_A, class T_B, class T_C >
  void Basic_Solver_for_3_Operands<T_A,T_B,T_C>::disconnect( const void *ID )
  {
    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( ID == &a ) 
      {
	b.rm_listener( this );
	c.rm_listener( this );
      }
      else if( ID == &b ) 
      {
	a.rm_listener( this );
	c.rm_listener( this );
      }
      else if( ID == &c ) 
      {
	a.rm_listener( this );
	b.rm_listener( this );
      }
      just_solving = false;	// solving or removing...

      delete this;		// !!! no further commands !!!
    }
  }

  template< class T_A, class T_B, class T_C >
  Basic_Solver_for_3_Operands<T_A,T_B,T_C>::Basic_Solver_for_3_Operands
  ( Operand<T_A> &op_a, Operand<T_B> &op_b, Operand<T_C> &op_c )
    : a(op_a), b(op_b), c(op_c), just_solving(false)
  {
    a.add_listener( this );
    b.add_listener( this );
    c.add_listener( this );
  }

  //************************************************
  // Sum_Solver: solves a = b + c in any direction
  //************************************************

  template< class T_A, class T_B, class T_C >
  T_A Sum_Solver<T_A,T_B,T_C>::calc_a( const T_B &b, const T_C &c )
  {
    return b + c;
  }
  template< class T_A, class T_B, class T_C >
  T_B Sum_Solver<T_A,T_B,T_C>::calc_b( const T_A &a, const T_C &c )
  {
    return a - c;
  }
  template< class T_A, class T_B, class T_C >
  T_C Sum_Solver<T_A,T_B,T_C>::calc_c( const T_A &a, const T_B &b )
  {
    return a - b;
  }

  template< class T_A, class T_B, class T_C >
  Sum_Solver<T_A,T_B,T_C>::Sum_Solver
  ( Operand<T_A> &a, Operand<T_B> &b, Operand<T_C> &c )
  : Basic_Solver_for_3_Operands<T_A,T_B,T_C>( a, b, c ) {}

  //***************************************************
  // Product_Solver: solves a = b * c in any direction
  //***************************************************

  template< class T_A, class T_B, class T_C >
  T_A Product_Solver<T_A,T_B,T_C>::calc_a( const T_B &b, const T_C &c )
  {
    return b * c;
  }
  template< class T_A, class T_B, class T_C >
  T_B Product_Solver<T_A,T_B,T_C>::calc_b( const T_A &a, const T_C &c )
  {
    assert( c != 0 );
    return a / c;
  }
  template< class T_A, class T_B, class T_C >
  T_C Product_Solver<T_A,T_B,T_C>::calc_c( const T_A &a, const T_B &b )
  {
    assert( b != 0 );
    return a / b;
  }

  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_a_ok
  ( const T_A &a, const T_B &b, const T_C &c, bool avail_b, bool avail_c )
  {
    if( a != 0 )
    {
      if( avail_b ) if( b == 0 ) return false;
      if( avail_c ) if( c == 0 ) return false;
    }
    else // a == 0
    {
      if( avail_b && avail_c ) if( b != 0 && c != 0 ) return false;
    }
    return true;
  }
  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_b_ok
  ( const T_A &a, const T_B &b, const T_C &c, bool avail_a, bool avail_c )
  {
    if( avail_a )
    {
      if( a != 0 )
      {
	if( b == 0 ) return false;
      }
      else // a == 0
      {
	if( avail_c ) if( c != 0 && b != 0 ) return false;
      }
    }
    return true;
  }
  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_c_ok
  ( const T_A &a, const T_B &b, const T_C &c, bool avail_a, bool avail_b )
  {
    if( avail_a )
    {
      if( a != 0 )
      {
	if( c == 0 ) return false;
      }
      else // a == 0
      {
	if( avail_b ) if( b != 0 && c != 0 ) return false;
      }
    }
    return true;
  }

  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_b_calcable( const T_A &, const T_C &c )
  { 
    return c != 0; 
  }
  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_c_calcable( const T_A &, const T_B &b )
  { 
    return b != 0; 
  }
  
  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_a_calcable_from_b( const T_B &b )
  {
    return b == 0;
  }
  template< class T_A, class T_B, class T_C >
  bool Product_Solver<T_A,T_B,T_C>::is_a_calcable_from_c( const T_C &c )
  {
    return c == 0;
  }
  
  template< class T_A, class T_B, class T_C >
  T_A Product_Solver<T_A,T_B,T_C>::calc_a_from_b( const T_B &b )
  {
    assert( b == 0 );
    return 0;
  }
  template< class T_A, class T_B, class T_C >
  T_A Product_Solver<T_A,T_B,T_C>::calc_a_from_c( const T_C &c )
  {
    assert( c == 0 );
    return 0;
  }

  template< class T_A, class T_B, class T_C >
  Product_Solver<T_A,T_B,T_C>::Product_Solver
  ( Operand<T_A> &a, Operand<T_B> &b, Operand<T_C> &c )
  : Basic_Solver_for_3_Operands<T_A,T_B,T_C>( a, b, c ) {}

  //************************************************
  // Square_Solver: solves a = b^2 in any direction
  //************************************************

  template< class T_A, class T_B >
  T_A Square_Solver<T_A,T_B>::calc_a1( const T_B &b )
  {
    return b*b;
  }

  template< class T_A, class T_B >
  T_B Square_Solver<T_A,T_B>::calc_b1( const T_A &a )
  {
    return sqrt( a );
  }

  template< class T_A, class T_B >
  T_B Square_Solver<T_A,T_B>::calc_b2( const T_A &a )
  {
    return -sqrt( a );
  }

  template< class T_A, class T_B >
  bool Square_Solver<T_A,T_B>::is_a_ok
  ( const T_A &a, const T_B &/*b*/, bool /*avail_b*/ )
  {
    return a >= 0;		// negative square root impossible
  }
}

#endif
