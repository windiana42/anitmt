/*****************************************************************************/
/**   This file offers a constraint checking Operand Listener               **/
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

#ifndef __Solve_Constraint__
#define __Solve_Constraint__

#include "operand.hpp"

namespace solve{

  //***************************************************************************
  // Constraint_Checker: assures that operands in expression trees are conform
  //                     to a constraint expression
  //***************************************************************************

  class Constraint_Checker : public Operand_Listener
  {
    Operand<bool> &check;

    //**********************************
    // Virtual Operand_Listener methods

    // has to check the result of the operand with ID as pointer to operand
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw();
    // disconnect operand
    void disconnect( const void *ID );

  public:
    Constraint_Checker( Operand<bool> &op );
  };

  inline void constraint( Operand<bool> &op )
  {
    new Constraint_Checker( op );
  }

  //***************
  // test function
  //***************
  // this functions are checked in operator_test() in operator.cpp
}
#endif
