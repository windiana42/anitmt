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
  User_Problem_Handler Type_Property<T>::default_handler;

  // returns the value of the property
  template<class T>
  T Type_Property<T>::get( Solve_Run_Info const *info ) const{
    assert( is_solved_in_try( info ) );
    return v;
  }

  // returns the value of the property
  template<class T>
  T Type_Property<T>::get() const{
    //assert( is_solved() ); //!implicite convertion to double can't pass info!
    return v;
  }

  // tries to set the property and returns whether it was successful (true)
  template<class T>
  bool Type_Property<T>::set_if_ok( T v, Solve_Run_Info const *info ){
    res = is_this_ok( v, 0, info );
    if( res ) use_it(0);

    return res;
  }
  
  // tries to set the property and returns whether it was successful (true)
  template<class T>
  bool Type_Property<T>::set_if_ok( T v, 
				    Solve_Problem_Handler *problem_handler ){
    bool res;

    Solve_Run_Info *info = new Solve_Run_Info( problem_handler );

    res = is_this_ok( v, 0, info );
    if( res ) use_it(0);

    delete info;

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
				     Solve_Run_Info const *info ) {
    
    bool res = true;
    
    // if property is already solved (in current try)
    if( is_solved_in_try( info ) )
      {
	// return whether it is the same result
	if( (v == v_to_try) )
	  return true;

	// report problem
	if( info->problem_handler ) 
	  {
	    std::list< Property* > l; l.push_back( this );
	    info->problem_handler->property_collision_occured( l );
	  }

	return false;
      }

    last_test_run_id = info->get_test_run_id(); 
				// mark this property to be solved in this try
    v = v_to_try;

    // for each solver (*i)
    for( solvers_type::iterator i = solvers.begin(); i != solvers.end(); i++ )
      {
	// skip the solver who gave the solution
	if( (*i) == caller ) continue;

	if( !((*i)->is_prop_solution_ok( this, info )) )
	  {
	    res = false;
	    break;
	  }
      }
    return res;
  }

  template<class T>
  void Type_Property<T>::operator=( Operand<T> &operand ) { 
    assign( *this, operand ); 
  }

  template<class T>
  std::ostream &Type_Property<T>::write2stream( std::ostream& os ) {
    if( !is_solved() )
      return os << "n/a";
    return os << get();
  }

  // obsolete
  //template<class T>
  //std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s ){
  //  return s.write2stream( os );
  //}

  /*
  // force compilation of Specializations:
  void all_property_specializations() {
    Type_Property<values::Scalar> force_compilation_of_spec1;
    Type_Property<values::Vector> force_compilation_of_spec2;
    Type_Property<values::Matrix> force_compilation_of_spec3;
    Type_Property<values::String> force_compilation_of_spec4;
    Type_Property<values::Flag>   force_compilation_of_spec5;
  }
  */
}

#endif
