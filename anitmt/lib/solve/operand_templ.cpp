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

namespace solve
{
  //************
  // Exceptions:
  //************
  
  //**************************************************************************
  // Operand: base class for operand values of a certain type
  //**************************************************************************
  // most implementations are in operand_inline.cpp

  template<class T>
  User_Problem_Handler Operand<T>::default_handler;

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
  Operand<T>::Operand<T>( Operand<T> &src ) throw()
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
    if( !op.is_solved() ) return os << "???";

    return os << op.get_value();
  }

  //*****************************************
  // Constant: Operand for holding constants
  //*****************************************

  template<class T>
  Constant<T>::Constant( T val, message::Message_Consultant *msg_consultant )
    : Operand<T>(msg_consultant)
  {
    delete_without_listener = true; // delete me if last listener disconnects

    set_value( val );
  }

  //**************************************************************************
  // Store_Operand_to_Property: reports the value of an operand to a property
  //**************************************************************************
  
  template<class T>
  bool Store_Operand_to_Operand<T>::is_result_ok
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &source );
    return destination.test_set_value( source.get_value(info), info );
  }

  template<class T>
  void Store_Operand_to_Operand<T>::use_result 
  ( const void *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &source );
    if( destination.is_solved_in_try( info ) )
      destination.use_test_value( info );
  }

  template<class T>
  void Store_Operand_to_Operand<T>::disconnect( const void *ID ) 
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
