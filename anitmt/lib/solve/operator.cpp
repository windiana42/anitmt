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

#include "operator.hpp"

#include <iostream>
#include <assert.h>
#include <math.h>

#include "operand.hpp"
#include "constraint.hpp"

using namespace std;

#warning !!! check whether just_solving-locking is needed in any solvers !!!

namespace solve
{
  //**********************************************
  //**********************************************
  //** Special Operators
  //**********************************************
  //**********************************************

  //***************************************************************************
  // Multi_And_Operator: Logical and with any number of operands
  //***************************************************************************

  bool Multi_And_Operator::initial_result()
  {
    return true;
  }

  bool Multi_And_Operator::calc( const bool &op, const bool &old_res )
  {
    return old_res && op;
  }

  bool Multi_And_Operator::is_op_sufficient( const bool &op )
  {
    return !op;			// one false is sufficient
  }

  Multi_And_Operator::Multi_And_Operator( message::Message_Consultant *c )
    : Basic_Multi_Operand_Operator<bool,bool>(c)
  {}

  //*******************************************************************
  // Is_Solved_Operator: operator to test whether an operand is solved
  //*******************************************************************

  //**********************************
  // Virtual Operand_Listener methods

  // has to check the result of the operand with ID as pointer to operand
  bool Is_Solved_Operator::is_result_ok( const void *, 
					 Solve_Run_Info *info ) throw()
  {
    return result.test_set_value( true, info ); // result is always true
  }

  // tells to use the result calculated by is_result_ok()
  void Is_Solved_Operator::use_result( const void *, 
				       Solve_Run_Info *info ) throw()
  {
    result.use_test_value( info );
  }

  // disconnect operand
  void Is_Solved_Operator::disconnect( const void * )
  { 
    delete this; //!!! no furthur instructions !!!
  }

  Is_Solved_Operator::Is_Solved_Operator( Basic_Operand &op, 
					  message::Message_Consultant *c )
    : result(c)
  {
    op.add_listener( this );
  }

}
