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
	(*i)->disconnect_Solver( this );
      }
  }

  // Properties call that if they are distroyed
  void Solver::disconnect_Property( Property *prop ) 
    throw( EX_Property_Not_Connected ){

    properties_type::iterator i = 
      find( properties.begin(), properties.end(), prop );
    if( i == properties.end() ) 
      throw EX_Property_Not_Connected();

    properties.erase( i );

    if( properties.empty() ) // is the last property disconnecting?
      {
	delete this;		// destroy myself
      }
  }

  void Solver::add_Property( Property *prop ) {
    properties.push_back( prop );
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
  
  // Properties call that if they were solved
  void Accel_Solver::prop_was_solved( Property *caller ){
    n_props_available++;
    
    // tell the properties that were calculated in the try before that they
    // may use the result
    if( s_d  && (caller !=  &d) )  d.use_it( this );
    if( s_t  && (caller !=  &t) )  t.use_it( this );
    if( s_a  && (caller !=  &a) )  a.use_it( this );
    if( s_v0 && (caller != &v0) ) v0.use_it( this );
    if( s_ve && (caller != &ve) ) ve.use_it( this );

    if( s_d && s_t && s_a && s_v0 && s_ve ) 
      delete this;		
    // !!! no more use of object member variables !!!
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Accel_Solver::is_prop_solution_ok( Property *caller, Solve_Problem_Handler *problem_handler ){
    long cur_try_id = caller->get_try_id();
    if( try_id != cur_try_id )	// is this the first call in try
      {
	// reset the check flags that indicate which properties were solved
	s_d=false;s_t=false;s_a=false;s_v0=false;s_ve=false;
      }
    try_id = cur_try_id;
    
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

		s_ve = true; // endspeed solved
		s_d = true; // stretch solved
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
  
  // Properties call that if they were solved
  void Diff_Solver::prop_was_solved( Property *caller ){
    n_props_available++;
    
    // tell the properties that were calculated in the try before that they
    // may use the result
    if( s_d  && (caller !=  &d) )  d.use_it( this );
    if( s_s  && (caller !=  &s) )  s.use_it( this );
    if( s_e  && (caller !=  &e) )  e.use_it( this );

    if( s_d && s_s && s_e ) 
      delete this;		
    // !!! no more use of object member variables !!!
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Diff_Solver::is_prop_solution_ok( Property *caller, Solve_Problem_Handler *problem_handler ){
    long cur_try_id = caller->get_try_id();
    if( try_id != cur_try_id )	// is this the first call in try
      {
	// reset the check flags that indicate which properties were solved
	s_d=false;s_s=false;s_e=false;
      }
    try_id = cur_try_id;
    
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

	    s_e = true; // end value solved
	  }	    

	// end value known ?
	if( e.is_solved_in_try() )
	  {
	    // can calculate start value now:
	    values::Scalar res_s = e - d;

	    // verify result for end value
	    if( !s.is_this_ok( res_s, this, problem_handler ) )
	      return false; 

	    s_s = true; // end value solved
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

	    s_e = true; // end value solved
	  }	    

	// end value known ?
	if( e.is_solved_in_try() )
	  {
	    // can calculate difference now:
	    values::Scalar res_d = e - s;

	    // verify result for difference
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    s_d = true; // difference solved
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

	    s_s = true; // start value solved
	  }	    

	// start value known ?
	if( s.is_solved_in_try() )
	  {
	    // can calculate difference now:
	    values::Scalar res_d = e - s;

	    // verify result for start value
	    if( !d.is_this_ok( res_d, this, problem_handler ) )
	      return false; 

	    s_d = true; // start value solved
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
  
  // Properties call that if they were solved
  void Relation_Solver::prop_was_solved( Property *caller ){
    n_props_available++;
    
    // tell the properties that were calculated in the try before that they
    // may use the result
    if( s_q  && (caller !=  &q) )  q.use_it( this );
    if( s_n  && (caller !=  &n) )  n.use_it( this );
    if( s_d  && (caller !=  &d) )  d.use_it( this );

    if( s_q && s_n && s_d ) 
      delete this;		
    // !!! no more use of object member variables !!!
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Relation_Solver::is_prop_solution_ok( Property *caller, 
					     Solve_Problem_Handler *problem_handler ){
    long cur_try_id = caller->get_try_id();
    if( try_id != cur_try_id )	// is this the first call in try
      {
	// reset the check flags that indicate which properties were solved
	s_q=false;s_n=false;s_d=false;
      }
    try_id = cur_try_id;
    
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

	    s_d = true; // denominator
	  }	    

	// denominator known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate numerator now:
	    values::Scalar res_n = q * d;

	    // verify result for numerator
	    if( !n.is_this_ok( res_n, this, problem_handler ) )
	      return false; 

	    s_n = true; // numerator solved
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

	    s_d = true; // denominator solved
	  }	    

	// denominator known ?
	if( d.is_solved_in_try() )
	  {
	    // can calculate quotient now:
	    values::Scalar res_q = n / d;

	    // verify result for quotient
	    if( !q.is_this_ok( res_q, this, problem_handler ) )
	      return false; 

	    s_q = true; // quotient solved
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

	    s_n = true; // numerator solved
	  }	    

	// numerator known ?
	if( n.is_solved_in_try() )
	  {
	    // can calculate quotient now:
	    values::Scalar res_q = n / d;

	    // verify result for numerator
	    if( !q.is_this_ok( res_q, this, problem_handler ) )
	      return false; 

	    s_q = true; // numerator solved
	  }	    
      }

    return true;
  }
  
}
