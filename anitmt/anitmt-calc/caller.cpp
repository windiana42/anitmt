/*****************************************************************************/
/**   This file offers an action caller class realized as solver     	    **/
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

#include "caller.hpp"

namespace anitmt{

  //************************************************************************
  // Action_Caller: class that inserts an action when an essential property
  //                is solved
  //************************************************************************

  // Properties call that if they were solved
  void Action_Caller::prop_was_solved( Property *ID ) {
    priority_action->place_Action();
    //!!! no more function calls here !!! (may be deleted by place_Action)
  }
  // Properties call that if they want to validate their results
  bool Action_Caller::is_prop_solution_ok( Property*, 
					   Solve_Problem_Handler* ) {

    return true;		// always ok!
  }

  // Construtor
  Action_Caller::Action_Caller( Property *cause, Priority_Action *act )
    : priority_action( act ) {
    add_Property( cause );
  }
}
