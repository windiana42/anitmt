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

#ifndef __Solve_Operand_Inline_Implementation__
#define __Solve_Operand_Inline_Implementation__

#include "operand.hpp"

#include <algorithm>

namespace solve 
{
  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  // checks wheather an id belongs to curr. test
  bool Solve_Run_Info::is_id_valid( id_type id ) const
  {
    // reverse search for id in valid id's
    valid_test_run_ids_type::const_iterator i = 
      find( valid_test_run_ids.begin(), valid_test_run_ids.end(), id );
    return i != valid_test_run_ids.end();
  }

  // returns current test run ID
  Solve_Run_Info::id_type Solve_Run_Info::get_test_run_id() const
  {
    return test_run_id;
  }

  // adds and returns a new test run ID
  Solve_Run_Info::id_type Solve_Run_Info::new_test_run_id()
  {
    test_run_id = current_default_test_run_id++;
    valid_test_run_ids.push_front( test_run_id );
    return test_run_id;
  }
   
  // adds a test run ID
  void Solve_Run_Info::add_test_run_id( id_type id )
  {
    test_run_id = id;
    valid_test_run_ids.push_front( test_run_id );
  }

  // removes a test run ID
  void Solve_Run_Info::remove_test_run_id( id_type id )
  {
    // remove all ids added after this one
    while( id != valid_test_run_ids.front() )
    {
      valid_test_run_ids.pop_front();
      assert( !valid_test_run_ids.empty() );
    }
    // remove this id
    valid_test_run_ids.pop_front();
    
    // id that was added just before is the new active id now
    assert( !valid_test_run_ids.empty() );
    test_run_id = valid_test_run_ids.front();
  }
  //! get a definitely invalid id
  Solve_Run_Info::id_type Solve_Run_Info::get_inivalid_id() const
  {
    return -1;
  }

  void Solve_Run_Info::set_trial_run( bool trial )
  {
    trial_run = trial;
  }
  bool Solve_Run_Info::is_trial_run()
  {
    return trial_run;
  }

  //**********************************************************
  // Operand: base class for operand values of a certain type
  //**********************************************************

  template<class T>
  void Operand<T>::report_value( Solve_Run_Info *info ) throw()
  {
    // check wether operand is already reporting values...
    if( last_test_run_id != -1 )
    {
      last_test_run_id = -1;	// lock this function against recursion

      listeners_type::iterator i;    
      for( i = listeners.begin(); i != listeners.end(); i++ )
      {
	(*i)->use_result( this, info );
      }
    }
  }

  template<class T>
  bool Operand<T>::test_report_value( Solve_Run_Info *info )
    throw()
  {
    listeners_type::iterator i;    
    for( i = listeners.begin(); i != listeners.end(); i++ )
    {
      if( !(*i)->is_result_ok( this, info ) ) 
      {
	// this operand was involved in this error
	if( !info->is_trial_run() )
	  involved_in_error( get_value(info) ); 
	return false;
      }
    }
    return true;
  }
  
  template<class T>
  bool Operand<T>::is_solved_in_try( const Solve_Run_Info *info ) const
  { 
    return solved || info->is_id_valid( last_test_run_id ); 
  }

  template<class T>
  bool Operand<T>::is_solved() const 
  { 
    return solved; 
  }

  template<class T>
  const T& Operand<T>::get_value() const 
  { 
    assert( is_solved() ); 
    return value; 
  }

  template<class T>
  const T& Operand<T>::operator()() const 
  { 
    assert( is_solved() ); 
    return value; 
  }

  template<class T>
  const T& Operand<T>::get_value( const Solve_Run_Info *info ) const 
  { 
    assert( is_solved_in_try(info) ); 
    return value; 
  }
  
  template<class T>
  bool Operand<T>::set_value( T res, 
			      Solve_Problem_Handler *handler ) throw()
  { 
    Solve_Run_Info info( handler );
    if( test_set_value(res,&info) ) 
    {
      report_value( &info ); 
      solved = true;
      return true;
    }
    return false;
  }

  template<class T>
  bool Operand<T>::test_set_value( T val, Solve_Run_Info *info ) 
    throw()
  {
    if( is_solved_in_try(info) )
    {
      if( value == val )
      {
	return true;
      }
      else
      {
	std::list< Basic_Operand* > bad_ops;
	bad_ops.push_back( this );
	if( info->problem_handler->operand_collision_occured( bad_ops, info,
							      this ) )
	  if( !info->is_trial_run() )
	  {
	    caused_error();		// this operand caused the error
	    error() << "you gave a value which resulted in the value " << val 
		    << " for this operand which collides with " << value;
	  }

	return false;
      }
    }

    value = val;
    last_test_run_id = info->get_test_run_id();
    return test_report_value( info );
  }

  template<class T>
  bool Operand<T>::test_set_value( T res, 
				   Solve_Problem_Handler *handler ) 
    throw()
  { 
    Solve_Run_Info info( handler );
    return test_set_value(res,&info);
  }

  template<class T>
  void Operand<T>::use_test_value( Solve_Run_Info *info ) throw() 
  { 
    assert( is_solved_in_try(info) ); 
    solved = true; 
    report_value( info ); 
  }

}

#endif
