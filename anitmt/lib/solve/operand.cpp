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

#include "operand.hpp"

namespace solve{

  //********************************************************
  // User_Problem_Handler: handles problems occured by user
  //********************************************************
  
  // a operand collision occured!
  // ret false: ignore error
  bool User_Problem_Handler::operand_collision_occured
  ( std::list< Basic_Operand* > bad_ops, Solve_Run_Info *info, 
    message::Message_Reporter *msg )
  {
    return true;
  }

  // Operand signals to reject value 
  // usage may be enforced by returning false
  bool User_Problem_Handler::may_operand_reject_val
  ( std::list< Basic_Operand* > bad_ops, Solve_Run_Info *info, 
    message::Message_Reporter *msg )
  {
    return false;
  }  

  // **************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  // **************************************************************
  
  Solve_Run_Info::id_type Solve_Run_Info::current_default_test_run_id = 1;

  Solve_Run_Info::Solve_Run_Info( Solve_Problem_Handler *handler, 
				  int id )
    : problem_handler( handler ), trial_run( false ), test_run_id( id ) 
  {
    add_test_run_id( id );
  }

  Solve_Run_Info::Solve_Run_Info( Solve_Problem_Handler *handler )
    : problem_handler( handler ), 
      trial_run( !handler->output_error_messages() ), 
      test_run_id( -2 )
  {
    new_test_run_id();
  }

}


