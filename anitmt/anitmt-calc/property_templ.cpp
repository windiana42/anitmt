/*****************************************************************************/
/**   This file implements the templates of property.hpp		    **/
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
/** Function: these functions mainly provide the communication between	    **/
/**           solvers and properties of any type                      	    **/
/**           the first ones also offer an interface for other functions    **/
/**           to use the properties                                   	    **/
/*****************************************************************************/

#ifndef __AniTMT_Property_Templateimplementaion__
#define __AniTMT_Property_Templateimplementaion__

#include "property.hpp"

namespace anitmt{

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************

  // static standard user problem handler
  template<class T>
  User_Problem_Handler Type_Property<T>::user_problem_handler;

  // returns the value of the property
  template<class T>
  T Type_Property<T>::get() const{
    assert( is_solved_in_try() );	// make sure this property is solved
    return v;
  }

  // tries to set the property and returns whether it was successful (true)
  template<class T>
  bool Type_Property<T>::set_if_ok( T v, 
				    Solve_Problem_Handler *problem_handler ){
    bool res = is_this_ok( v, 0, problem_handler );
    if( res ) use_it(0);
    cur_try_id++;		// change try_id for next time
    return res;
  }
  
  // implicite convertion to Scalar
  template<class T>
  Type_Property<T>::operator T() const { 
    return get(); 
  }

  // This is called try if this value might be valid for this property
  // returns true if value is acceptable
  // !!! may be recursive
  template<class T>
  bool Type_Property<T>::is_this_ok( T v_to_try, Solver *caller, 
				     Solve_Problem_Handler *problem_handler ) {
    
    bool res = true;
    
    // if property is already solved (in current try)
    if( solved || ( try_id == cur_try_id ) )
      {
	// return whether it is the same result
	if( (v == v_to_try) )
	  return true;

	// report problem
	if( problem_handler ) 
	  {
	    std::list< Property* > l; l.push_back( this );
	    problem_handler->
	      property_collision_occured( l );
	  }

	return false;
      }

    try_id = cur_try_id;	// mark this property to be solved in this try
    v = v_to_try;

    // for each solver (*i)
    for( solvers_type::iterator i = solvers.begin(); i != solvers.end(); i++ )
      {
	// skip the solver who gave the solution
	if( (*i) == caller ) continue;

	if( !((*i)->is_prop_solution_ok( this, problem_handler )) )
	  {
	    res = false;
	    break;
	  }
      }
    return res;
  }

  // Solver call this when the given value was ok
  // !!! may be self recursive
  template<class T>
  void Type_Property<T>::use_it( Solver *caller ){
    // was this property not already
    if( !solved )
      {
	// v already got the value from the try
	assert( try_id == cur_try_id ); // it was hopefully the same try
	// v is accepted as solution now
	solved = true;

	// for each solver (*i)
	for( solvers_type::iterator i=solvers.begin(); i!=solvers.end(); i++ )
	  {
	    if( (*i) == caller ) continue; // avoid recursion back to caller

	    (*i)->prop_was_solved( this );
	  }
      }
  }

  template<class T>
  std::ostream &Type_Property<T>::write2stream( std::ostream& os ) {
    if( !is_solved_in_try() )
      return os << "n/a";
    return os << get();
  }

  // obsolete
  //template<class T>
  //std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s ){
  //  return s.write2stream( os );
  //}

}

#endif
