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
/** Requirements: libval, libmessage, libsolve				    **/
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

#include <val/val.hpp>
#include <message/message.hpp>
#include <solve/operand.hpp>

#include "error.hpp"

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
    std::string name;		// property name
    Prop_Tree_Node *node;	// tree node the property belongs to
    message::Abstact_Position *pos;
				// position where it is defined (by user)

    values::Valtype::Types type;
  public:
    //***********
    // modifiers

    // tells the property where it occurs in user's inputs
    void set_input_position( message::Abstact_Position *pos );

    //******************
    // access functions

    inline std::string			get_name() { return name; }
    inline Prop_Tree_Node*		get_node() { return node; }	
    inline message::Abstract_Position*	get_position() { return pos; }
    inline values::Valtype::Types	get_type() { return type; }

    //************************
    // usage member functions

    // implemented by Type_Property
    virtual std::ostream &write2stream( std::ostream& ) = 0;

    virtual ~Property() {}
  protected:
    // Create Type_Property objects instead!
    Property( std::string name, Prop_Tree_Node *node, 
	      values::Valtype::Types type );
  };

  //***********
  // operators

  std::ostream &operator << ( std::ostream&, Property & );

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************
  template<class T> class Type_Property : public Property, 
					  public solve::Operand<T>
  {
  public:	
    //******************
    // member operators

    //!! shouldn't be convertable to type as baseclass Operand<> has its own 
    //!! operators !! use operator() to convert to type instead !!
    // operator T() const	  // implicite convertion to type (like get()) 
    //   throw( EX_property_not_solved );

    //************************
    // usage member functions

    // connects another operand as a solution source
    solve::Operand<T>& operator=( solve::Operand<T> &src ) throw(EX)
    { return static_cast<solve::Operand<T>&>(*this) = src; }

    virtual std::ostream &write2stream( std::ostream& );

    //*************
    // constructor

    Type_Property( std::string name, Prop_Tree_Node *node, 
		   values::Valtype::Types type );
  };

  //***********
  // operators

  //  template<class T>
  //  std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s );

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  class Scalar_Property : public Type_Property<values::Scalar>
  {
  public:
    //*************
    // constructor

    Scalar_Property( std::string name, Prop_Tree_Node *node );
  };

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  class Vector_Property : public Type_Property<values::Vector>
  {
  public:
    //*************
    // constructor

    Vector_Property( std::string name, Prop_Tree_Node *node );
  };

  //******************************************************
  // Matrix_Property: container for matrix property values
  //******************************************************
  class Matrix_Property : public Type_Property<values::Matrix>
  {
  public:
    //*************
    // constructor

    Matrix_Property( std::string name, Prop_Tree_Node *node );
  };

  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  class String_Property : public Type_Property<values::String>
  {
  public:
    //*************
    // constructor

    String_Property( std::string name, Prop_Tree_Node *node );
  };

  //**************************************************
  // Flag_Property: container for flag property values
  //**************************************************
  class Flag_Property : public Type_Property<values::Flag>
  {
  public:
    //*************
    // constructor

    Flag_Property( std::string name, Prop_Tree_Node *node );
  };

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "property_templ.cpp"

#endif

