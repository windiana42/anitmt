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

#include <list>
#include <iostream>

namespace anitmt{
  class Property;

  class Scalar_Property;
  class Vector_Property;
  class String_Property;
  class Flag_Property;
}

#include "val.hpp"
#include "solver.hpp"

namespace anitmt{
  //****************************************
  // Property: container for property values
  //****************************************
  class Property{
  protected:
    typedef std::list< Solver* > solvers_type;
    solvers_type solvers;	// Solvers that might calculate this property
    bool solved;		// is the property already solved?

    static long cur_try_id;	// unique id for a try which is increase
				// after every try
    long try_id;		// to identify the last try for this property
				// try_id == cur_try_id indicates that the
				// property was solved in the try
  public:
    bool s() const;		// is solved?

    Property();
    virtual ~Property();

    //********************************************************************
    // the following functions should only be accessed by class Solver and
    // derived classes

    bool s_in_try() const;	// is property solved in current try
    long get_try_id() const;	// returns the current try id
    void add_Solver( Solver *solver ); // adds a solver for this property
  };

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************
  template<class T> class Type_Property : public Property{
    T v;			// contained value

  public:	
    T get() const;		// returns the value
    bool set_if_ok( T v );	// tries to set the value and returns whether
				// the attempt was successful (true) or not

    operator T() const;		// implicite convertion to type (like get())

    //**********************************************************************
    // the following functions are only used of a graph solution search from
    // the class Solver and derived classes

    // This is called to try if this value might be valid for this property
    // returns true if value is acceptable
    bool is_this_ok( T v, Solver *caller );
    // Solver call this when the given value was ok
    void use_it( Solver *caller );
  };

  template<class T>
  std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s );

  //******************************************************
  // Scalar_Property: container for scalar property values
  //******************************************************
  class Scalar_Property : public Type_Property<values::Scalar>{
  public:
    operator double() const;	// implicite convertion to Scalar
  };

  //******************************************************
  // Vector_Property: container for vector property values
  //******************************************************
  class Vector_Property : public Type_Property<values::Vector>{
  public:
    operator vect::vector3() const;	// implicite convertion to vector3
  };

  //******************************************************
  // String_Property: container for string property values
  //******************************************************
  class String_Property : public Type_Property<values::String>{
  public:
  };

  //**************************************************
  // Flag_Property: container for flag property values
  //**************************************************
  class Flag_Property : public Type_Property<values::Flag>{
  public:
  };

  //**************************************
  // Prop_Pair: Pair of Property and Value
  //**************************************
  class Prop_Pair{
  public:
    virtual bool set_if_ok() = 0;
  };

  //*************************************************************
  // Type_Prop_Pair: Pair of Property and Value of a certain type
  //*************************************************************
  template<class TP, class T>
  class Type_Prop_Pair : public Prop_Pair{
    TP *property;
    T  value;
  public:
    // set the property to the according value
    virtual bool set_if_ok(){ return property->set_if_ok(value); }

    Type_Prop_Pair( TP *prop, T v ) : property(prop), value(v) {}
  };

  // easy access function
  template<class TP, class T>
  Type_Prop_Pair<TP,T> *T_Prop_Pair( TP *prop, T v ){
    return new Type_Prop_Pair<TP,T>( prop, v );
  }

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "property_templ.cpp"

#endif

