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

#include "solver.hpp"

namespace anitmt{
  //****************************************
  // Solver: Base Class for Property Solvers
  //****************************************
  Solver::Solver() : n_props_available(0), n_props_connected(-1), try_id(-1) {}
  Solver::~Solver(){}

  // Properties call that if they are distroyed
  void Solver::prop_disconnect(){
    // only connected properties may call this function
    assert( n_props_connected > 0 ); 

    if( --n_props_connected == 0 ) // is the last property disconnecting?
      {
	delete this;		// destroy myself
      }
  }

  //*****************************************************************
  // Accel_Solver: Property Solver for constantly accelerated systems
  //*****************************************************************
  
  Accel_Solver::Accel_Solver( Scalar_Property *_d, Scalar_Property *_t, 
			      Scalar_Property *_a, 
			      Scalar_Property *_v0, Scalar_Property *_ve ) 
    : d(*_d), t(*_t), a(*_a), v0(*_v0), ve(*_ve){
    n_props_connected = 5;
    d.add_Solver( this );
    t.add_Solver( this );
    a.add_Solver( this );
    v0.add_Solver( this );
    ve.add_Solver( this );
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
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Accel_Solver::prop_solution_ok( Property *caller ){
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
	assert( t.s_in_try() );	

	// stretch known ?
	if( v0.s_in_try() )
	  {
	    // acceleration known ?
	    if( a.s_in_try() )
	      {
		values::Scalar res_ve = v0 + a * t;
		values::Scalar res_d  = v0*t + 0.5*a*t*t;

		// verify result for endspeed
		if( !ve.is_this_ok( res_ve, this ) )
		  return false; 

		// verify result for differance
		if( !d.is_this_ok( res_d, this ) )
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
  // Diff_Solver: Solver for a start, end and differance value
  //**********************************************************
  
  Diff_Solver::Diff_Solver( Scalar_Property *_d, Scalar_Property *_s, 
			      Scalar_Property *_e ) 
    : d(*_d), s(*_s), e(*_e){
    n_props_connected = 3;
    d.add_Solver( this );
    s.add_Solver( this );
    e.add_Solver( this );
  }
  
  // Properties call that if they were solved
  void Diff_Solver::prop_was_solved( Property *caller ){
    n_props_available++;
    
    // tell the properties that were calculated in the try before that they
    // may use the result
    if( s_d  && (caller !=  &d) )  d.use_it( this );
    if( s_s  && (caller !=  &s) )  s.use_it( this );
    if( s_e  && (caller !=  &e) )  e.use_it( this );
  }
  
  // Properties call that if they want to validate their results
  // !!! may be self recursive
  bool Diff_Solver::prop_solution_ok( Property *caller ){
    long cur_try_id = caller->get_try_id();
    if( try_id != cur_try_id )	// is this the first call in try
      {
	// reset the check flags that indicate which properties were solved
	s_d=false;s_s=false;s_e=false;
      }
    try_id = cur_try_id;
    
    // differance solved in this try now ?
    if( caller == &d )
      {
	assert( d.s_in_try() );	

	// start value known ?
	if( s.s_in_try() )
	  {
	    // can calculate end value now:
	    values::Scalar res_e = s + d;

	    // verify result for end value
	    if( !e.is_this_ok( res_e, this ) )
	      return false; 

	    s_e = true; // end value solved
	  }	    

	// end value known ?
	if( e.s_in_try() )
	  {
	    // can calculate start value now:
	    values::Scalar res_s = e - d;

	    // verify result for end value
	    if( !s.is_this_ok( res_s, this ) )
	      return false; 

	    s_s = true; // end value solved
	  }	    
      }

    // start value solved in this try now ?
    if( caller == &s )
      {
	assert( s.s_in_try() );	

	// differance value known ?
	if( d.s_in_try() )
	  {
	    // can calculate end value now:
	    values::Scalar res_e = s + d;

	    // verify result for end value
	    if( !e.is_this_ok( res_e, this ) )
	      return false; 

	    s_e = true; // end value solved
	  }	    

	// end value known ?
	if( e.s_in_try() )
	  {
	    // can calculate differance now:
	    values::Scalar res_d = e - s;

	    // verify result for end value
	    if( !d.is_this_ok( res_d, this ) )
	      return false; 

	    s_d = true; // end value solved
	  }	    
      }

    // end value solved in this try now ?
    if( caller == &e )
      {
	assert( e.s_in_try() );	

	// differance value known ?
	if( d.s_in_try() )
	  {
	    // can calculate start value now:
	    values::Scalar res_s = e - d;

	    // verify result for start value
	    if( !s.is_this_ok( res_s, this ) )
	      return false; 

	    s_s = true; // start value solved
	  }	    

	// start value known ?
	if( s.s_in_try() )
	  {
	    // can calculate differance now:
	    values::Scalar res_d = e - s;

	    // verify result for start value
	    if( !d.is_this_ok( res_d, this ) )
	      return false; 

	    s_d = true; // start value solved
	  }	    
      }

    return true;
  }
  
}
