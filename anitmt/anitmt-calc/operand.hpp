/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
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

#ifndef __AniTMT_Operand__
#define __AniTMT_Operand__

#include "val.hpp"

namespace anitmt 
{
  // classes
  class Basic_Operand;
  template<class T> class Operand;
  template<class T> class Constant;
}

#include "error.hpp"
#include <list>
#include <set>

namespace anitmt
{
  //************
  // Exceptions:
  //************

  class EX_property_collision : public EX 
  {
  public:
    EX_property_collision() : EX( "property collision" ) {}
  };

  class EX_solver_is_not_connected : public EX 
  {
  public:
    EX_solver_is_not_connected() : EX( "solver is not connected" ) {}
  };

  //*********************************************************
  // Solve_Problem_Handler: handles problems in solve system
  //*********************************************************
  
  class Solve_Problem_Handler 
  {
  public:
    // a operand collision occured!
    // (may throw exceptions!!!)
    virtual void operand_collision_occured( std::list< Basic_Operand* > 
					    bad_ops )
      throw(EX) = 0;

    // Operand signals to reject value 
    // usage may be enforced by returning false
    // (may throw exceptions!!!)
    virtual bool may_operand_reject_val( std::list< Basic_Operand* > 
					 bad_ops )
      throw(EX) = 0;
  };

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  class User_Problem_Handler : public Solve_Problem_Handler 
  {
  public:
    // a operand collision occured!
    // (may throw exceptions!!!)
    virtual void operand_collision_occured( std::list< Basic_Operand* > 
					    bad_ops )
      throw(EX);

    // Operand signals to reject value 
    // usage may be enforced by returning false
    // (may throw exceptions!!!)
    virtual bool may_operand_reject_val( std::list< Basic_Operand* > 
					 bad_ops )
      throw(EX);
  };

  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  class Solve_Run_Info 
  {
  public:
    Solve_Problem_Handler *problem_handler;
  private:
    typedef int id_type;

    static id_type current_default_test_run_id;

    std::set<id_type> valid_test_run_ids;
    id_type test_run_id;
  public:
    inline bool is_id_valid( id_type id ) const; 
				// checks wheather an id belongs to curr. test
    inline id_type get_test_run_id() const; // returns current test run ID
    inline id_type new_test_run_id();	// returns a new test run ID
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


  //***************************************************************************
  // Operand_Listener: Interface for solvers/operators to receive results of 
  //                   Operands
  //***************************************************************************
  class Operand_Listener 
  {
  public:
    // has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, 
			       const Solve_Run_Info *info ) throw(EX) = 0;
    // tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, const Solve_Run_Info *info )
      throw(EX) = 0;

    // disconnect operand
    virtual void disconnect( const void *ID ) = 0;
    
    virtual ~Operand_Listener() {}
  };
  
  //**********************************************************
  // Basic_Operand: base class for operand values of any type
  //**********************************************************
  class Basic_Operand
  {
  public:    
    virtual void add_listener( Operand_Listener *listener ) = 0;
    virtual void rm_listener( Operand_Listener *listener ) = 0;
    virtual ~Basic_Operand() {}
  };

  //**********************************************************
  // Operand: base class for operand values of a certain type
  //**********************************************************
  template<class T> class Operand : public Basic_Operand
  {
    static User_Problem_Handler default_handler;
  protected:
    T value;			// to store the value
    bool solved;

    int last_test_run_id;	// is used to determine wheather value stores
				// a valid solution in the current solve run

    bool delete_without_listener;

    typedef std::list<Operand_Listener*> listeners_type;
    listeners_type listeners; 

    inline void report_value( const Solve_Run_Info *info ) 
      throw(EX);
    inline bool test_report_value( const Solve_Run_Info *info ) 
      throw(EX);
  public:	
    inline bool is_solved() const;
    inline const T& get_value() const;
    inline bool set_value( T res, 
			   Solve_Problem_Handler *handler = &default_handler )
      throw(EX);

    // connects another operand as a solution source
    Operand<T>& operator=( Operand<T> &src ) throw(EX);

    Operand();
    virtual ~Operand();

    //*************************************************
    // for functions in solve system (with correct info)
    inline bool is_solved_in_try( const Solve_Run_Info *info ) const;
    inline const T& get_value( const Solve_Run_Info *info ) const;
    inline bool test_set_value( T val, const Solve_Run_Info *info )
      throw(EX);
    inline bool test_set_value( T val, 
				Solve_Problem_Handler *handler = 
				&default_handler )
      throw(EX);
    inline void use_test_value( const Solve_Run_Info *info )
      throw(EX);

    void add_listener( Operand_Listener *listener );
    void rm_listener( Operand_Listener *listener );
  };

  //***********
  // operators

  template<class T>
  std::ostream &operator <<( std::ostream &os, const Operand<T> &op );

  //*****************************************
  // Constant: Operand for holding constants
  //*****************************************

  template<class T>
  class Constant : public Operand<T> 
  {
  public:
    Constant( T val );
  };

  //**********************************************************************
  // Store_Operand_to_Operand: stores the value of one operand to another
  //**********************************************************************

  template<class T> 
  class Store_Operand_to_Operand
    : public Operand_Listener 
  {
    Operand<T> &source;
    Operand<T> &destination;

    //** Operand Listener Methods **

    // has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, const Solve_Run_Info *info ) 
      throw(EX);
    // tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, const Solve_Run_Info *info )
      throw(EX);
    // disconnect operand
    virtual void disconnect( const void *ID );

  public:
    Store_Operand_to_Operand( Operand<T> &src, Operand<T> &dest ); 
  };

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operand_templ.cpp"

// include implementation to enable inline functions to be expanded
#include "operand_inline.cpp"

#endif

