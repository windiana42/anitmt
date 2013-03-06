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

#ifndef __Solve_Operand_Templateimlementation__
#define __Solve_Operand_Templateimlementation__

#include "operand.hpp"

#include <algorithm>
#include <assert.h>

namespace solve
{
  //************
  // Exceptions:
  //************

  template<class T>
  User_Problem_Handler Operand<T>::default_handler;
  
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
  void Operand<T>::add_listener( Operand_Listener *listener ) 
  {
    listeners.push_back( listener );
  }

  template<class T>
  void Operand<T>::rm_listener( Operand_Listener *listener ) 
  {
    listeners_type::iterator i = find( listeners.begin(), listeners.end(), 
				       listener );
    assert( i != listeners.end() );
    listeners.erase( i );

    if( listeners.empty() && delete_without_listener )
      delete this;		// !!! No further commands !!!
  }

  template<class T>
  Operand<T>& Operand<T>::assign   ( T src ) throw()
  {
    set_value( src );
    return *this;
  }
  //! connects another operand as a solution source
  template<class T>
  Operand<T>& Operand<T>::assign   ( Operand<T> &src ) throw()
  {
    new Store_Operand_to_Operand<T>( src, *this );
    return *this;
  }
  template<class T>
  Operand<T>& Operand<T>::operator=( Operand<T> &src ) throw()
  {
    new Store_Operand_to_Operand<T>( src, *this );
    return *this;
  }

  //**************************
  // constructors/destructors

  //! connects another operand as a solution source
  template<class T>
  Operand<T>::Operand( Operand<T> &src ) throw()
    : message::Message_Reporter(src.get_consultant()),
      solved(false), last_test_run_id(-1), delete_without_listener(false)
      
  {
    new Store_Operand_to_Operand<T>( src, *this );
  }

  template<class T>
  Operand<T>::Operand( message::Message_Consultant *msg_consultant ) 
    : message::Message_Reporter(msg_consultant),
      solved(false), last_test_run_id(-1), delete_without_listener(false)      
  {
  }

  template<class T>
  Operand<T>::~Operand() 
  {
    listeners_type::iterator i;    
    for( i = listeners.begin(); i != listeners.end(); i++ )
    {
      (*i)->disconnect( this );
    }
  }

  //***********
  // operators

  template<class T>
  std::ostream &operator <<( std::ostream &os, const Operand<T> &op )
  {
    if( !op.is_solved() ) return os << "<not yet solved>";

    return os << op.get_value();
  }

  //*****************************************
  // Constant: Operand for holding constants
  //*****************************************

  template<class T>
  Constant<T>::Constant( T val, message::Message_Consultant *msg_consultant )
    : Operand<T>(msg_consultant)
  {
    Operand<T>::delete_without_listener = true; // delete me if last listener disconnects

    set_value( val );
  }

  //**************************************************************************
  // Store_Operand_to_Property: reports the value of an operand to a property
  //**************************************************************************
  
  template<class T>
  bool Store_Operand_to_Operand<T>::is_result_ok
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &source );
    return destination.test_set_value( source.get_value(info), info );
  }

  template<class T>
  void Store_Operand_to_Operand<T>::use_result 
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &source );
    if( destination.is_solved_in_try( info ) )
      destination.use_test_value( info );
  }

  template<class T>
  void Store_Operand_to_Operand<T>::disconnect( const Basic_Operand *ID ) 
  {
    delete this;		// !!! no further commands !!!
  }

  template<class T>
  Store_Operand_to_Operand<T>::Store_Operand_to_Operand
  ( Operand<T> &src, Operand<T> &dest ) throw()
    : source(src), destination(dest) 
  {
    // register to source Operand 
    src.add_listener( this );

    if( src.is_solved() )
    {
      dest.set_value( src.get_value() );
    }
  }
}

#endif
