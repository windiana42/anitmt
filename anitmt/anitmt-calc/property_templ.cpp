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

  // implicite convertion to contained type
  template<class T>
  Type_Property<T>::operator T() const throw( EX_property_not_solved )
  { 
    if( !is_solved() )
      throw EX_property_not_solved();

    return get_value(); 
  }

  template<class T>
  std::ostream &Type_Property<T>::write2stream( std::ostream& os ) {
    if( !is_solved() )
      return os << "???";
    return os << get_value();
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
