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

#include "property_classes.hpp"

#include <list>
#include <set>
#include <iostream>

#include "val.hpp"
#include "error.hpp"

#include "solver.hpp"
#include "operand_classes.hpp"

namespace anitmt{

  //************
  // Exceptions:
  //************
  
  class EX_property_collision : public EX {
  public:
    EX_property_collision() : EX( "property collision" ) {}
  };

  class EX_solver_is_not_connected : public EX {
  public:
    EX_solver_is_not_connected() : EX( "solver is not connected" ) {}
  };

  //*********************************************************
  // Solve_Problem_Handler: handles problems in solve system
  //*********************************************************
  
  class Solve_Problem_Handler {
  public:
    // a property collision occured!
    // (may throw exceptions!!!)
    virtual void property_collision_occured( std::list< Property* > bad_props )
      throw( EX ) = 0;

    // property signals to reject value 
    // usage may be enforced by returning false
    // (may throw exceptions!!!)
    virtual bool may_property_reject_val( std::list< Property* > bad_props )
      throw( EX ) = 0;
  };

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  class User_Problem_Handler : public Solve_Problem_Handler {
  public:
    // a property collision occured!
    // (throws exceptions)
    virtual void property_collision_occured( std::list<Property*> bad_props )
      throw( EX );

    // property signals to reject value 
    // usage may be enforced by returning false
    virtual bool may_property_reject_val( std::list<Property*> bad_props )
      throw( EX );
  };

  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  class Solve_Run_Info {
    typedef int id_type;

    static id_type current_default_test_run_id;

    std::set<id_type> valid_test_run_ids;
    id_type test_run_id;
  public:
    Solve_Problem_Handler *problem_handler;

    inline bool is_id_valid( id_type id ) const; 
				// checks wheather an id belongs to curr. test
    inline id_type get_test_run_id() const; // returns current test run ID
    inline id_type new_test_run_id();	// adds a new test run ID
    inline void add_test_run_id( id_type id ); // adds a test run ID
    inline void remove_test_run_id( id_type id ); // removes a test run ID
    inline void set_test_run_id( id_type id ); // sets active test run ID

    Solve_Run_Info( Solve_Problem_Handler *handler, int id )
      : problem_handler( handler ), test_run_id( id ) {
      add_test_run_id( id );
    }
    Solve_Run_Info( Solve_Problem_Handler *handler )
      : problem_handler( handler ) {
      new_test_run_id();
    }
  };

  //****************************************
  // Property: container for property values
  //****************************************

  class Property{
    friend class Solver;
    //********************************************************************
    // the following functions should only be accessed by class Solver and
    // derived classes

    void add_Solver( Solver *solver ); // adds a solver for this property
    void disconnect_Solver( Solver *solver )
      throw( EX_solver_is_not_connected ); // removes solver connection

  protected:
    // Solver call this when the previous given value was ok
    // ( see Type_Property::is_this_ok() )
    void use_it( Solver *caller );

    typedef std::list< Solver* > solvers_type;
    solvers_type solvers;	// Solvers that might calculate this property
    bool solved;		// is the property already solved?

    long last_test_run_id;	// to identify the last test solve run for 
				// this property
				// it is used to determine wheather the 
				// property is already solved in test run
  public:
    bool is_solved() const;	// is property solved?

    virtual std::ostream &write2stream( std::ostream& ) = 0;

    Property();
    virtual ~Property();

    //********************************************************************
    // the following functions should only be accessed by class Solver and
    // derived classes

    bool is_solved_in_try( Solve_Run_Info const *info ) const;	
				// is property solved in current try
  };

  std::ostream &operator << ( std::ostream&, Property & );

  //***************************************************************
  // Type_Property: container for property values of a certain type
  //***************************************************************
  template<class T> class Type_Property : public Property{
    T v;			// contained value

    static User_Problem_Handler default_handler;
  public:	
    T get() const;		// returns the value when solved
    T get( Solve_Run_Info const *info ) const; // returns the value in test run
    bool set_if_ok( T v, Solve_Run_Info const *info ); 
				// tries to set the value and returns whether
				// the attempt was successful (true) or not.
				// info includes problem handler and test ID

    bool set_if_ok( T v, 
		    Solve_Problem_Handler *problem_handler=&default_handler ); 
				// tries to set the value and returns whether
				// the attempt was successful (true) or not.
				// problem_handler is for explizite user inputs

    operator T() const;		// implicite convertion to type (like get())

    // allow operator syntax to assign an Operand to a Property
    inline void operator=( Operand<T> &operand ); 

    virtual std::ostream &write2stream( std::ostream& );

    //**********************************************************************
    // the following functions are only used of a graph solution search from
    // the class Solver and derived classes

    // This is called to try if this value might be valid for this property
    // returns true if value is acceptable
    bool is_this_ok( T v, Solver *caller, Solve_Run_Info const *info );
  };

  //  template<class T>
  //  std::ostream &operator<<( std::ostream &os, const Type_Property<T> &s );

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
    operator values::Vector() const;	// implicite convertion to Vector
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

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "property_templ.cpp"

#endif

