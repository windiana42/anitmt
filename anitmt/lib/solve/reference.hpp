/*****************************************************************************/
/**   This file offers a reference operator from operand to operand         **/
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

#ifndef __Solve_Reference__
#define __Solve_Reference__

#include "operand.hpp"

#include <message/message.hpp>

namespace solve{

  //**************************************************************************
  // Explicite_Reference: operator for assigning referenced expressions to an 
  //			  operand
  //**************************************************************************

  template<class T_Operand>
  class Explicite_Reference
    : public Operand_Listener
  {
    Operand<T_Operand> &source, &destination;

    //*** Operand_Listener methods ***

    //! has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const void *ID, 
			       Solve_Run_Info *info ) throw();
    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const void *ID, Solve_Run_Info *info )
      throw();

    //! disconnect operand
    virtual void disconnect( const void *ID );

  public:
    //*** Constructor ***
    Explicite_Reference( Operand<T_Operand> &dest, Operand<T_Operand> &src );
  };

  //*** creator function ***

  // explicite references within operand expression trees
  template<class T_Operand>
  inline void explicite_reference( Operand<T_Operand> &dest, 
				   Operand<T_Operand> &src );
}

// include implementation to make sure that all specializations of the
// templates are compiled 
#include "reference_templ.cpp"

#endif
