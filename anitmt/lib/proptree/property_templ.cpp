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

#ifndef __Functionality_Property_Templateimplementaion__
#define __Functionality_Property_Templateimplementaion__

#include "property.hpp"

namespace functionality
{
  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************

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

  template<class T>
  void Type_Property<T>::caused_error()
  {
    error() << "property " << get_name() << " caused the error";
  }
  template<class T>
  void Type_Property<T>::involved_in_error( T val )
  {
    error() << "setting property " << get_name() << " to " << val 
	    << " was involved in the error";
  }

  //*************
  // constructor

  // return message consultant before implementation of Prop_Tree_Node is known
  message::Message_Consultant *get_consultant_early( Prop_Tree_Node *node );

  template<class T>
  Type_Property<T>::Type_Property( std::string name, Prop_Tree_Node *node, 
				   values::Valtype::Types type )
    : Property( name, node, type ), 
      solve::Operand<T>(get_consultant_early(node))
  {}

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
