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

namespace anitmt
{
  //****************************************
  // Property: container for property values
  //****************************************

  //************************
  // Constructor/Destructor

  Property::Property() {}

  //***********
  // operators

  // operator for output on standard out streams
  std::ostream &operator << ( std::ostream& os, Property &prop ){
    return prop.write2stream( os );
  }

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  
  // implicite convertion to double
  Scalar_Property::operator double() const throw( EX_property_not_solved )
  { 
    if( !is_solved() )
      throw EX_property_not_solved();
    return get_value(); 
  }
  
  // implicite convertion to values::Vector
  Vector_Property::operator values::Vector() const 
    throw( EX_property_not_solved )
  { 
    if( !is_solved() )
      throw EX_property_not_solved();
    return get_value(); 
  }
  
}
