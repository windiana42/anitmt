/*****************************************************************************/
/**   This file offers solver for properties          			    **/
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

#include <assert.h>
#include <algorithm>

#include "solver.hpp"

namespace anitmt{
  //****************************************
  // Solver: Base Class for Property Solvers
  //****************************************
  Solver::Solver() : n_props_available(0), try_id(-1) {}
  Solver::~Solver() {
    properties_type::iterator i;
    for( i = properties.begin(); i != properties.end(); i++ )
      {
	i->first->disconnect_Solver( this );
      }
  }

  // Properties call that if they are distroyed
  void Solver::disconnect_Property( Property *prop ) 
    throw( EX_Property_Not_Connected ){

    // search property 
    properties_type::iterator i;
    for( i = properties.begin(); i != properties.end(); i++ )
      if( i->first == prop ) 
	break;

    if( i == properties.end() ) 
      throw EX_Property_Not_Connected();

    properties.erase( i );

    if( properties.empty() ) // is the last property disconnecting?
      {
	delete this;		// destroy myself
      }
  }

  void Solver::prop_was_solved( Property *caller ){
    n_props_available++;
    
    bool are_all_props_solved = true; // test if all props are solved

    do_when_prop_was_solved( caller );

    // tell the properties that were calculated in the try before that they
    // may use the result as definitive solution
    properties_type::iterator i;
    for( i = properties.begin(); i != properties.end(); i++ )
      {
	
	// was property just solved and it isn't the caller
	if( (i->second == prop_just_solved) && (caller != i->first) ) 
	  {
	    i->second = prop_solved;
	    i->first->use_it( this );
	  }
	else
	  // check if all props are solved
	  if( i->second != prop_solved )
	    are_all_props_solved = false;
      }

    if( are_all_props_solved ) 
      {
#ifdef __DEBUG__
	cout << "warning: untested \"delete this\" of solver!" << endl;
#endif
	delete this;		// dangerous for iterators???
      }
    // !!! no more use of object member variables !!!
  }

  // Properties call that if they want to validate their results
  // (uses virtual function check_prop_solution)
  bool Solver::is_prop_solution_ok
  ( Property *caller, Solve_Problem_Handler *problem_handler ) {
    long cur_try_id = caller->get_try_id();
    if( try_id != cur_try_id )	// is this the first call in try
      {
	// reset the check flags that indicate which properties were solved
	properties_type::iterator i;
	for( i = properties.begin(); i != properties.end(); i++ )
	  i->second = prop_not_solved;

	try_id = cur_try_id;
      }

    
    return check_prop_solution_and_results( caller, problem_handler );
  }

  void Solver::add_Property( Property *prop ) {
    properties[ prop ] = prop_not_solved; 
    prop->add_Solver( this );
  }

  //*****************************************************************
  // Accel_Solver: Property Solver for constantly accelerated systems
  //*****************************************************************
  
  Accel_Solver::Accel_Solver( Scalar_Property *_d, Scalar_Property *_t, 
			      Scalar_Property *_a, 
			      Scalar_Property *_v0, Scalar_Property *_ve ) 
    : d(*_d), t(*_t), a(*_a), v0(*_v0), ve(*_ve){
    add_Property( _d );
    add_Property( _t );
    add_Property( _a );
    add_Property( _v0 );
    add_Property( _ve );
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Accel_Solver::check_prop_solution_and_results
  ( Property *caller, Solve_Problem_Handler *problem_handler ){

    // duration solved in this try now ?
    if( caller == &t )
      {
	assert( t.is_solved_in_try() );	

	// stretch known ?
	if( v0.is_solved_in_try() )
	  {
	    // acceleration known ?
	    if( a.is_solved_in_try() )
	      {
		values::Scalar res_ve = v0 + a * t;
		values::Scalar res_d  = v0*t + 0.5*a*t*t;

		// verify result for endspeed
		if( !ve.is_this_ok( res_ve, this, problem_handler ) )
		  return false; 

		// verify result for difference
		if( !d.is_this_ok( res_d, this, problem_handler ) )
		  return false; 

		properties[ &ve ] = prop_just_solved; // endspeed solved
		properties[ &d ] = prop_just_solved; // stretch solved
	      }	    
	    //...
	  }	    
	//...
      }

    //...

    return true;
  }

  //**********************************************************
  // Diff_Solver: Solver for a start, end and difference value
  //**********************************************************
  
  Diff_Solver::Diff_Solver( Scalar_Property *_d, Scalar_Property *_s, 
			      Scalar_Property *_e ) 
    : d(*_d), s(*_s), e(*_e){
    add_Property( _d );
    add_Property( _s );
    add_Property( _e );
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Diff_Solver::check_prop_solution_and_results
  ( Property *caller, Solve_Problem_Handler *problem_handler ){

    // difference solved in this try now ?
    if( caller == &d )
      {
	assert( d.is_solved_in_try() );	

	// start value known ?
	if( s.is_solved_in_try() )
	  {
	    // can calculate end value now:
	    values::Scalar res_e = s + d;

	    // verify result for end value
	    if( !e.is_this_ok( res_e, this, problem_handler ) )
	      return false; 

	    properties[ &e ] = prop_just_solved; // end value solved
	  }	    

	// end value known ?
	if( e.is_solved_in_try() )
	  {
	    // can calculate start value now:
	    values::Scalar res_s = e - d;

	    // verify result for end value
	    if( !s.is_this_ok( res_s, this, problem_handler ) )
	      return false; 

	    properties[ &s ] = prop_just_solved; // end value solved
	  }	    
      }

    // start value solved in this try now ?
    if( caller == &s )
      {
	assert( s.is_solved_in_try() );	

	// difference value known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate end value now:
	    values::Scalar res_e = s + d;

	    // verify result for end value
	    if( !e.is_this_ok( res_e, this, problem_handler ) )
	      return false; 

	    properties[ &e ] = prop_just_solved; // end value solved
	  }	    

	// end value known ?
	if( e.is_solved_in_try() )
	  {
	    // can calculate difference now:
	    values::Scalar res_d = e - s;

	    // verify result for difference
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    properties[ &d ] = prop_just_solved; // difference solved
	  }	    
      }

    // end value solved in this try now ?
    if( caller == &e )
      {
	assert( e.is_solved_in_try() );	

	// difference value known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate start value now:
	    values::Scalar res_s = e - d;

	    // verify result for start value
	    if( !s.is_this_ok( res_s, this, problem_handler ) )
	      return false; 

	    properties[ &s ] = prop_just_solved; // start value solved
	  }	    

	// start value known ?
	if( s.is_solved_in_try() )
	  {
	    // can calculate difference now:
	    values::Scalar res_d = e - s;

	    // verify result for start value
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    properties[ &d ] = prop_just_solved; // start value solved
	  }	    
      }

    return true;
  }
  

  //*******************************************************************
  // Relation_Solver: solver for a relation and the according to values
  //*******************************************************************
  
  Relation_Solver::Relation_Solver( Scalar_Property *_q, Scalar_Property *_n, 
			      Scalar_Property *_d ) 
    : q(*_q), n(*_n), d(*_d){
    add_Property( _q );
    add_Property( _n );
    add_Property( _d );
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Relation_Solver::check_prop_solution_and_results
  ( Property *caller, Solve_Problem_Handler *problem_handler ){

    // relation solved in this try now ?
    if( caller == &q )
      {
	assert( q.is_solved_in_try() );	

	// assure quotient doesn't equal zero
	if( q == 0 ) 
	  return false;		// avoid division by zero

	// numerator known ?
	if( n.is_solved_in_try() )
	  {
	    // can calculate denominator
	    values::Scalar res_d = n / q;

	    // verify result for denominator
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    properties[ &d ] = prop_just_solved; // denominator
	  }	    

	// denominator known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate numerator now:
	    values::Scalar res_n = q * d;

	    // verify result for numerator
	    if( !n.is_this_ok( res_n, this, problem_handler ) )
	      return false; 

	    properties[ &n ] = prop_just_solved; // numerator solved
	  }	    
      }

    // numerator solved in this try now ?
    if( caller == &n )
      {
	assert( n.is_solved_in_try() );	

	// quotient known ?
	if( q.is_solved_in_try() )
	  {
	    // can calculate denominator now:
	    values::Scalar res_d = n / q;

	    // verify result for denominator
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    properties[ &d ] = prop_just_solved; // denominator solved
	  }	    

	// denominator known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate quotient now:
	    values::Scalar res_q = n / d;

	    // verify result for quotient
	    if( !q.is_this_ok( res_q, this, problem_handler ) )
	      return false; 

	    properties[ &q ] = prop_just_solved; // quotient solved
	  }	    
      }

    // denominator solved in this try now ?
    if( caller == &d )
      {
	assert( d.is_solved_in_try() );	

	// quotient value known ?
	if( q.is_solved_in_try() )
	  {
	    // can calculate numerator now:
	    values::Scalar res_n = q * d;

	    // verify result for numerator
	    if( !n.is_this_ok( res_n, this, problem_handler ) )
	      return false; 

	    properties[ &n ] = prop_just_solved; // numerator solved
	  }	    

	// numerator known ?
	if( n.is_solved_in_try() )
	  {
	    // can calculate quotient now:
	    values::Scalar res_q = n / d;

	    // verify result for numerator
	    if( !q.is_this_ok( res_q, this, problem_handler ) )
	      return false; 

	    properties[ &q ] = prop_just_solved; // numerator solved
	  }	    
      }

    return true;
  }
  
}
