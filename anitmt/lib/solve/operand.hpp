/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#ifndef __Solve_Operand__
#define __Solve_Operand__

#include <val/val.hpp>
#include <message/message.hpp>

namespace solve 
{
  // classes
  class Basic_Operand;
  template<class T> class Operand;
  template<class T> class Constant;
  class Solve_Run_Info;
}

#include <list>

namespace solve
{
  //*********************************************************
  // Solve_Problem_Handler: handles problems in solve system
  //*********************************************************
  
  class Solve_Problem_Handler 
  {
  public:
    //! should errors be told to user
    virtual bool output_error_messages() = 0;
    // a operand collision occured!
    // ret false: ignore error
    virtual bool operand_collision_occured( std::list< Basic_Operand* > 
					    bad_ops,
					    Solve_Run_Info *info, 
					    message::Message_Reporter *msg ) 
      = 0;

    // Operand signals to reject value 
    // ret false: enforce usage
    virtual bool may_operand_reject_val( std::list< Basic_Operand* > 
					 bad_ops,
					 Solve_Run_Info *info, 
					 message::Message_Reporter *msg )
      = 0;
  };

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  class User_Problem_Handler : public Solve_Problem_Handler 
  {
  public:
    //! should errors be told to user
    virtual bool output_error_messages() { return true; }
    // a operand collision occured!
    // ret false: ignore error
    virtual bool operand_collision_occured( std::list< Basic_Operand* > 
					    bad_ops,
					    Solve_Run_Info *info, 
					    message::Message_Reporter *msg );

    // Operand signals to reject value 
    // ret false: enforce usage
    virtual bool may_operand_reject_val( std::list< Basic_Operand* > 
					 bad_ops,
					 Solve_Run_Info *info, 
					 message::Message_Reporter *msg );

  };

  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  class Solve_Run_Info 
  {
  public:
    Solve_Problem_Handler *problem_handler; // handles some problems
    typedef int id_type;
    bool trial_run;		// is it an unimportant trial run 
  private:
    static id_type current_default_test_run_id;
    
    id_type test_run_id;	// id to mark operands solved in that solve run

    typedef std::list<id_type> valid_test_run_ids_type; 
    /* operands marked with on of these ids are considered as solved in this
      solve run */
    valid_test_run_ids_type valid_test_run_ids;
  public:
    bool is_id_valid( id_type id ) const; 
				// checks wheather an id belongs to curr. test
    id_type get_test_run_id() const; // returns current test run ID
    id_type new_test_run_id();	// adds and returns a new test run ID
    void set_test_run_id( id_type id ); // sets current (valid) run ID
    void add_test_run_id( id_type id ); // adds a test run ID
    void remove_test_run_id( id_type id ); // removes test run ID and 
						  // newer ones
    void set_trial_run( bool trial ); 
    bool is_trial_run();
    id_type get_inivalid_id() const; // get a definitely invalid id

    Solve_Run_Info( Solve_Problem_Handler *handler, int id );
    Solve_Run_Info( Solve_Problem_Handler *handler );
  };


  //***************************************************************************
  // Operand_Listener: Interface for solvers/operators to receive results of 
  //                   Operands
  //***************************************************************************
  class Operand_Listener 
  {
  public:
    // has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw() = 0;
    // tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw() = 0;

    // disconnect operand
    virtual void disconnect( const Basic_Operand *ID ) = 0;
    
    virtual ~Operand_Listener() {}
  };
  
  //**********************************************************
  // Basic_Operand: base class for operand values of any type
  //**********************************************************
  class Basic_Operand
  {
  public:    
    virtual bool is_solved() const = 0;
    virtual bool is_solved_in_try( const Solve_Run_Info *info ) const = 0;
    virtual void use_test_value( Solve_Run_Info *info ) throw() = 0;

    virtual void add_listener( Operand_Listener *listener ) = 0;
    virtual void rm_listener( Operand_Listener *listener ) = 0;
    virtual ~Basic_Operand();
  };

  //**********************************************************
  // Operand: base class for operand values of a certain type
  //**********************************************************
  template<class T> class Operand : public Basic_Operand, 
				    public message::Message_Reporter
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

    void report_value( Solve_Run_Info *info ) 
      throw();
    bool test_report_value( Solve_Run_Info *info ) 
      throw();
  public:	
    bool is_solved() const;
    const T& get_value() const;
    const T& operator()() const;	// same as get_value
    bool set_value( T res, 
			   Solve_Problem_Handler *handler = &default_handler )
      throw();

    //! connects another operand as a solution source
    Operand<T>& assign   ( Operand<T> &src ) throw();
    Operand<T>& assign   ( T src ) throw();
    Operand<T>& operator=( Operand<T> &src ) throw();
    //! connects another operand as a solution source
    Operand( Operand<T> &src ) throw();
    Operand( message::Message_Consultant *consultant );
    virtual ~Operand();

    //**************************************************************
    // virtual functions that may output error messages to the user
    virtual void caused_error() {}
    virtual void involved_in_error( T val ) {}

    //*************************************************
    // for functions in solve system (with correct info)
    bool is_solved_in_try( const Solve_Run_Info *info ) const;
    const T& get_value( const Solve_Run_Info *info ) const;
    bool test_set_value( T val, Solve_Run_Info *info )
      throw();
    bool test_set_value( T val, 
				Solve_Problem_Handler *handler = 
				&default_handler )
      throw();
    void use_test_value( Solve_Run_Info *info )
      throw();

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
    Constant( T val, message::Message_Consultant *msg_consultant );
  };

  template<class T>
  inline Operand<T> &const_op( T val, message::Message_Consultant *consultant )
  { return *(new Constant<T>(val, consultant)); }

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
    virtual bool is_result_ok( const Basic_Operand *ID, Solve_Run_Info *info ) 
      throw();
    // tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();
    // disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

  public:
    Store_Operand_to_Operand( Operand<T> &src, Operand<T> &dest ) throw(); 
  };

}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "operand_templ.cpp"

// include implementation to enable inline functions to be expanded
#ifdef EXTREME_INLINE
#include "operand_inline.cpp"
#endif

#endif

