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

#ifndef __AniTMT_Property__
#define __AniTMT_Property__

namespace anitmt
{
  class Property;
  template<class T> class Type_Property;
  class Scalar_Property;
  class Vector_Property;
  class String_Property;
  class Flag_Property;
}

#include <list>
#include <set>
#include <iostream>

#include "val.hpp"
#include "error.hpp"

#include "operand.hpp"

namespace anitmt
{
  //************
  // Exceptions
  //************

  // may be a position exception later
  class EX_property_not_solved : public EX 
  {
  public:
    EX_property_not_solved() 
      : EX( "property not solved yet" ) {}
  };


  //****************************************
  // Property: container for property values
  //****************************************

  class Property 
  {
    std::string name;
    //Prop_Tree_Node *node;	// tree node the property belongs to
    //Input_Position *pos;

    values::Valtype::Types type;
  protected:
  public:
    virtual std::ostream &write2stream( std::ostream& ) = 0;

    Property( /*std::string name, Prop_Tree_Node *node, Input_Position *pos,
		values::Valtype::Types type*/ );
    virtual ~Property() {}
  };

  std::ostream &operator << ( std::ostream&, Property & );

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************
  template<class T> class Type_Property : public Property, public Operand<T>
  {
  public:	
    operator T() const		// implicite convertion to type (like get())
      throw( EX_property_not_solved );

    // connects another operand as a solution source
    Operand<T>& operator=( Operand<T> &src ) throw(EX)
    { return static_cast<Operand<T>&>(*this) = src; }

    virtual std::ostream &write2stream( std::ostream& );
  };

  //  template<class T>
  //  std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s );

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  class Scalar_Property : public Type_Property<values::Scalar>
  {
  public:
    operator double() const	// implicite convertion to Scalar
      throw( EX_property_not_solved );
  };

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  class Vector_Property : public Type_Property<values::Vector>
  {
  public:
    operator values::Vector() const	// implicite convertion to Vector
      throw( EX_property_not_solved );
  };

  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  class String_Property : public Type_Property<values::String>
  {
  public:
  };

  //**************************************************
  // Flag_Property: container for flag property values
  //**************************************************
  class Flag_Property : public Type_Property<values::Flag>
  {
  public:
  };

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "property_templ.cpp"

#endif

