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
#include <val/val.hpp>

#include "proptree.hpp"

namespace anitmt
{
  //****************************************
  // Property: container for property values
  //****************************************

  //************************
  // Constructor/Destructor

  Property::Property( std::string n, Prop_Tree_Node *nod, 
		      values::Valtype::Types t ) 
    : name(n), node(nod), pos(message::GLOB::no_position), type(t) {}

  //***********
  // operators

  // operator for output on standard out streams
  std::ostream &operator << ( std::ostream& os, Property &prop ){
    return prop.write2stream( os );
  }

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************

  // return message consultant before implementation of Prop_Tree_Node is known
  message::Message_Consultant *get_consultant_early( Prop_Tree_Node *node )
  {
    return node->get_consultant();
  }

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  
  Scalar_Property::Scalar_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Scalar>( name, node, values::Valtype::scalar ) {}

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  
  Vector_Property::Vector_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Vector>( name, node, values::Valtype::vector ) {}
    
  //******************************************************
  // Matrix_Property: container for matrix property values
  //******************************************************
  
  Matrix_Property::Matrix_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Matrix>( name, node, values::Valtype::matrix ) {}
    
  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  
  String_Property::String_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::String>( name, node, values::Valtype::string ) {}
    
  //******************************************************
  // Flag_Property: container for flag property values
  //******************************************************
  
  Flag_Property::Flag_Property( std::string name, Prop_Tree_Node *node )
    : Type_Property<values::Flag>( name, node, values::Valtype::flag ) {}
    
}
