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

#include "constraint.hpp"

namespace solve
{
  //***************************************************************************
  // Constraint_Checker: assures that operands in expression trees are conform
  //                     to a constraint expression
  //***************************************************************************

  // has to check the result of the operand with ID as pointer to operand
  bool Constraint_Checker::is_result_ok
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw()
  {
    assert( ID == &check );
    assert( check.is_solved_in_try(info) );
    return check.get_value(info) == true; // only true expressions are accepted
  }

  // tells to use the result calculated by is_result_ok()
  void Constraint_Checker::use_result
  ( const Basic_Operand *ID, Solve_Run_Info *info ) throw() {}

  // disconnect operand
  void Constraint_Checker::disconnect( const Basic_Operand *ID )
  {
    assert( ID == &check );
    delete this;
  }

  //***********************
  // Constructor/Destructor

  Constraint_Checker::Constraint_Checker( Operand<values::Flag> &op )
    : check(op)
  {
    op.add_listener(this);
    if( op.is_solved() )
    {
      if( op.get_value() == false )
      {
	op.error() << "value is initially not conform to constraint";
      }
    }
  }
}
