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

#include "operand.hpp"

namespace anitmt{

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  // a operand collision occured!
  // (may throw exceptions!!!)
  void User_Problem_Handler::operand_collision_occured
  ( std::list< Basic_Operand* > bad_ops )
    throw(EX)
  {
    throw EX_property_collision();
  }

  // Operand signals to reject value 
  // usage may be enforced by returning false
  // (may throw exceptions!!!)
  bool User_Problem_Handler::may_operand_reject_val
  ( std::list< Basic_Operand* > bad_ops )
    throw(EX)
  {
    return false;
  }  

  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  Solve_Run_Info::id_type Solve_Run_Info::current_default_test_run_id = 1;
}


