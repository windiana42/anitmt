/*****************************************************************************/
/**   This file offers a container type for properties			    **/
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

#include "property.hpp"

#include <algorithm>
#include "operand.hpp"

namespace anitmt{

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  // a property collision occured!
  void User_Problem_Handler::property_collision_occured
  ( std::list<Property*> ) throw( EX ) {
    
    throw EX_property_collision();
  }
  
  // property signals to reject value 
  // usage may be enforced by returning false
  bool User_Problem_Handler::may_property_reject_val
  ( std::list<Property*> ) throw( EX ) {
    
    return false;		// user values may not be rejected
  }

  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  int Solve_Run_Info::current_default_test_run_id = 1;

  // checks wheather an id belongs to test 
  bool Solve_Run_Info::is_id_valid( Solve_Run_Info::id_type id ) const {
    return valid_test_run_ids.find(id) != valid_test_run_ids.end();
  }

  // returns current test run ID
  Solve_Run_Info::id_type Solve_Run_Info::get_test_run_id() const {
    return test_run_id;
  }

  // adds a new test run ID
  Solve_Run_Info::id_type Solve_Run_Info::new_test_run_id(){
    test_run_id = current_default_test_run_id;
    valid_test_run_ids.insert( test_run_id );
    return test_run_id;
  }

  // adds a test run ID
  void Solve_Run_Info::add_test_run_id( id_type id ){
    valid_test_run_ids.insert(id);
  }

  // removes a test run ID
  void Solve_Run_Info::remove_test_run_id( id_type id ){
    valid_test_run_ids.erase(id);
  }

  // sets active test run ID
  void Solve_Run_Info::set_test_run_id( id_type id ){
    test_run_id = id;
  }

  //****************************************
  // Property: container for property values
  //****************************************

  Property::Property() : last_test_run_id(-1) {}

  bool Property::is_solved() const { return solved; }
  // returns whether the property is solved in the current try
  bool Property::is_solved_in_try( Solve_Run_Info const *info ) const {
    if( info->is_id_valid( last_test_run_id ) )	
				// did the property get a solution in the
      return true;		// current try? 
    return solved; 
  }
  
  // adds a solver for this property
  void Property::add_Solver( Solver *solver ){
    solvers.push_back( solver );
  }
 
  // removes solver connection
  void Property::disconnect_Solver( Solver *solver ) 
    throw( EX_solver_is_not_connected ) {

    solvers_type::iterator i = find( solvers.begin(), solvers.end(), solver );
    if( i == solvers.end() )
      throw EX_solver_is_not_connected();

    solvers.erase( i );
  }

  // Solver call this when the given value was ok
  // !!! may be self recursive
  void Property::use_it( Solver *caller ){
    // was this property not already solved
    if( !solved )
      {
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

  Property::~Property(){
    // for each solver (*i)
    for( solvers_type::iterator i = solvers.begin(); i != solvers.end(); i++ )
      {
	// solver might be destroyed by this
	(*i)->disconnect_Property( this );
      }
  }

  // operator for output on standard out streams
  std::ostream &operator << ( std::ostream& os, Property &prop ){
    return prop.write2stream( os );
  }

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  
  // implicite convertion to double
  Scalar_Property::operator double() const { 
    return get(); 
  }
  
  // implicite convertion to values::Vector
  Vector_Property::operator values::Vector() const { 
    return get(); 
  }
  
}
