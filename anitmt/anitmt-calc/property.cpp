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

namespace anitmt{

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  // a property collision occured!
  void User_Problem_Handler::property_collision_occured
  ( std::list<Property*> ) {

    throw EX_property_collision();
  }
  
  // property signals to reject value 
  // usage may be enforced by returning false
  bool User_Problem_Handler::may_property_reject_val
  ( std::list<Property*> ) {
    
    return false;		// user values may not be rejected
  }

  //****************************************
  // Property: container for property values
  //****************************************
  long Property::cur_try_id = 0;

  Property::Property() : try_id(-1) {}

  bool Property::s() const { return solved; }
  // returns whether the property is solved in the current try
  bool Property::is_solved_in_try() const {
    if( try_id == cur_try_id )	// did the property get a solution in the
      return true;		// current try? 
    return solved; 
  }
  long Property::get_try_id() const { return cur_try_id; }
  
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
