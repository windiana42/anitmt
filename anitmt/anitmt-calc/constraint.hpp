/*****************************************************************************/
/**   This file offers a constraint checking Operand Listener               **/
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

#ifndef __AniTMT_Constraint__
#define __AniTMT_Constraint__

#include "error.hpp"
#include "operand.hpp"

namespace anitmt{

  //************
  // Exceptions:
  //************

  class EX_Constraint_Initially_Failed : public EX 
  {
  public:
    EX_Constraint_Initially_Failed() : EX( "constraint initially failed" ) {}
  };

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
    bool is_result_ok( const void *ID, Solve_Run_Info *info ) throw(EX);
    // tells to use the result calculated by is_result_ok()
    void use_result( const void *ID, Solve_Run_Info *info ) throw(EX);
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
