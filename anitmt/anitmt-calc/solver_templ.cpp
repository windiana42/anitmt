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

#ifndef __AniTMT_Solver_Templateimplementation__
#define __AniTMT_Solver_Templateimplementation__

#include "solver.hpp"

namespace anitmt{

  //*************************************************************************
  // Basic_Solver_for_3_Operands: base class for solvers that can solve each
  //                              of the three operands from the other two
  //*************************************************************************

  //**********************************
  // Virtual Operand_Listener methods

  // has to check the result of the operand with ID as pointer to operand
  template< class T_A, class T_B >
  bool Basic_Solver_for_2_Operands<T_A,T_B>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
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
	    just_solved_b = b.test_set_value( calc_b(_a), info );
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
	    just_solved_a = a.test_set_value( calc_a(_b), info );
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
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID==&a || ID==&b );

    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( just_solved_a ){ a.use_test_value( info ); just_solved_a=false; }
      if( just_solved_b ){ b.use_test_value( info ); just_solved_b=false; }
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
    : a(op_a), b(op_b), just_solving(false), 
      just_solved_a(false), just_solved_b(false)
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
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
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
	      just_solved_c = c.test_set_value( _c, info );
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
	      just_solved_b = b.test_set_value( calc_b(_a,_c), info );
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
	  just_solved_b = b.test_set_value( calc_b_from_a(_a), info );
	  if(!just_solved_b)	// if b rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  }
	}
	if( is_c_calcable_from_a( _a ) )
	{
	  just_solved_c = c.test_set_value( calc_c_from_a(_a), info );
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
	      just_solved_c = c.test_set_value( calc_c(_a,_b), info );
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
	      just_solved_a = a.test_set_value( calc_a(_b,_c), info );
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
	  just_solved_a = a.test_set_value( calc_a_from_b(_b), info );
	  if(!just_solved_a)	// if a rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
	if( is_c_calcable_from_b( _b ) )
	{
	  just_solved_c = c.test_set_value( calc_c_from_b(_b), info );
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
	      just_solved_b = b.test_set_value( calc_b(_a,_c), info );
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
	      just_solved_a = a.test_set_value( calc_a(_b,_c), info );
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
	  just_solved_a = a.test_set_value( calc_a_from_c(_c), info );
	  if(!just_solved_a)	// if a rejected 
	  { 
	    just_solving = false;	
	    return false;  
	  } 
	}
	if( is_b_calcable_from_c( _c ) )
	{
	  just_solved_b = b.test_set_value( calc_b_from_c(_c), info );
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
  ( const void *ID, Solve_Run_Info *info ) throw(EX)
  {
    assert( ID==&a || ID==&b || ID==&c );

    if( !just_solving )
    {
      just_solving = true;	// solving or removing...
      if( just_solved_a ){ a.use_test_value( info ); just_solved_a=false; }
      if( just_solved_b ){ b.use_test_value( info ); just_solved_b=false; }
      if( just_solved_c ){ c.use_test_value( info ); just_solved_c=false; }
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
    : a(op_a), b(op_b), c(op_c), just_solving(false), 
      just_solved_a(false), just_solved_b(false), just_solved_c(false)
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

}

#endif
