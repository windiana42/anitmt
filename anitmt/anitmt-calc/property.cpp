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

namespace anitmt{

  //****************************************
  // Property: container for property values
  //****************************************
  long Property::cur_try_id = 0;

  Property::Property() : try_id(-1) {}

  bool Property::s() const { return solved; }
  // returns whether the property is solved in the current try
  bool Property::s_in_try() const {
    if( try_id == cur_try_id )	// did the property get a solution in the
      return true;		// current try? 
    return solved; 
  }
  long Property::get_try_id() const { return cur_try_id; }
  
  // adds a solver for this property
  void Property::add_Solver( Solver *solver ){
    solvers.push_back( solver );
  }

  Property::~Property(){
    // for each solver (*i)
    for( solvers_type::iterator i = solvers.begin(); i != solvers.end(); i++ )
      {
	// solver might be destroyed by this
	(*i)->prop_disconnect();
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
  
  // implicite convertion to vect::vector3
  Vector_Property::operator vect::vector3() const { 
    return get(); 
  }
  
}
