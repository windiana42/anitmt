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

#ifndef __AniTMT_Operand_Inline_Implementation__
#define __AniTMT_Operand_Inline_Implementation__

#include "operand.hpp"

namespace anitmt 
{
  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  // checks wheather an id belongs to curr. test
  bool Solve_Run_Info::is_id_valid( id_type id ) const
  {
    return valid_test_run_ids.find( id ) != valid_test_run_ids.end();
  }

  // returns current test run ID
  Solve_Run_Info::id_type Solve_Run_Info::get_test_run_id() const
  {
    return test_run_id;
  }

  // returns a new test run ID
  Solve_Run_Info::id_type Solve_Run_Info::new_test_run_id()
  {
    test_run_id = current_default_test_run_id++;
    valid_test_run_ids.insert( test_run_id );
    return test_run_id;
  }
   
  // adds a test run ID
  void Solve_Run_Info::add_test_run_id( id_type id )
  {
    test_run_id = id;
    valid_test_run_ids.insert( test_run_id );
  }

  // removes a test run ID
  void Solve_Run_Info::remove_test_run_id( id_type id )
  {
    valid_test_run_ids.erase( valid_test_run_ids.find(id) );
  }
  
  // sets active test run ID
  void Solve_Run_Info::set_test_run_id( id_type id )
  {
    assert( valid_test_run_ids.find(id) != valid_test_run_ids.end() );
    test_run_id = id;
  }

  //**********************************************************
  // Operand: base class for operand values of a certain type
  //**********************************************************

  template<class T>
  void Operand<T>::report_value( const Solve_Run_Info *info ) throw(EX)
  {
    listeners_type::iterator i;    
    for( i = listeners.begin(); i != listeners.end(); i++ )
      {
	(*i)->use_result( this, info );
      }
  }

  template<class T>
  bool Operand<T>::test_report_value( const Solve_Run_Info *info )
    throw(EX)
  {
    listeners_type::iterator i;    
    for( i = listeners.begin(); i != listeners.end(); i++ )
    {
      if( !(*i)->is_result_ok( this, info ) ) return false;
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
  bool Operand<T>::set_value( T res, Solve_Problem_Handler *handler ) throw(EX)
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
  bool Operand<T>::test_set_value( T val, const Solve_Run_Info *info ) 
    throw(EX)
  {
    if( is_solved_in_try(info) )
    {
      return value == val;
    }

    value = val;
    last_test_run_id = info->get_test_run_id();
    return test_report_value( info );
  }

  template<class T>
  bool Operand<T>::test_set_value( T res, Solve_Problem_Handler *handler ) 
    throw(EX)
  { 
    Solve_Run_Info info( handler );
    return test_set_value(res,&info);
  }

  template<class T>
  void Operand<T>::use_test_value( const Solve_Run_Info *info ) throw(EX) 
  { 
    assert( is_solved_in_try(info) ); 
    solved = true; 
    report_value( info ); 
  }

}

#endif
