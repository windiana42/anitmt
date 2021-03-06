/*****************************************************************************/
/**   This file offers a reference operator from operand to operand         **/
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

#ifndef __Solve_Reference_Templateimplementation__
#define __Solve_Reference_Templateimplementation__

#include "reference.hpp"

#include <assert.h>

namespace solve
{
  //**************************************************************************
  // Explicite_Reference: operator for assigning referenced expressions to an 
  //			  operand
  //**************************************************************************

  //*** Operand_Listener methods ***
  
  //! has to check the result of the operand with ID as pointer to operand
  template<class T_Operand>
  bool Explicite_Reference<T_Operand>::is_result_ok
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw()
  {
    if( ID == &destination )
    {
      if( !source.is_solved_in_try(info) )
      {
	std::list<Basic_Operand*> bad_ops;
	bad_ops.push_back( &destination );
	
	if( info->problem_handler->may_operand_reject_val( bad_ops,info,
							   &source ) )
	  return false;		// don't accept forein values
	else // am I not allowed to reject...
	  return true;
      }
      else
	// slightly inefficiant
	return source.get_value(info) == destination.get_value(info);
    }

    if( ID == &source )
    {
      assert( source.is_solved_in_try( info ) );
      return destination.test_set_value( source.get_value(info), info );
    }

    assert( false );
    return false;
  }

  //! tells to use the result calculated by is_result_ok()
  template<class T_Operand>
  void Explicite_Reference<T_Operand>::use_result
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw()
  {
    if( ID == &source )
    {
      if( destination.is_solved_in_try( info ) )
	destination.use_test_value( info );
    }
    // if destination is accepted anyway: don't do anything
  }

  //! disconnect operand
  template<class T_Operand>
  void Explicite_Reference<T_Operand>::disconnect( const Basic_Operand *ID )
  {
    if( ID == &source )
    {
      destination.rm_listener( this );
    }
    else
    {
      assert( ID == &destination );
      source.rm_listener( this );
    }

    delete this;
  }

  //*** Constructor ***
  template<class T_Operand>
  Explicite_Reference<T_Operand>::Explicite_Reference
  ( Operand<T_Operand> &dest, Operand<T_Operand> &src )
    : source(src), destination(dest) 
  {
    src.add_listener(this);
    dest.add_listener(this);

    // if src is already solved
    if( src.is_solved() ) 
      dest.set_value( src.get_value() );
  }

  //*** creator function ***

  // explicite references within operand expression trees
  template<class T_Operand>
  inline void explicite_reference( Operand<T_Operand> &dest, 
				   Operand<T_Operand> &src )
  {
    new Explicite_Reference<T_Operand>( dest, src );
  }
}

#endif
