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
  void Action_Caller::do_when_prop_was_solved( Property *ID ) {
#ifdef __DEBUG__
    std::cout << "place Action!" << std::endl;
#endif
    priority_action->place_Action();
    //!!! no more function calls here !!! (may be deleted by place_Action)
  }

  // Properties call that if they want to validate their results
  bool Action_Caller::check_prop_solution_and_results( Property*, 
					   Solve_Problem_Handler* ) {
    
#ifdef __DEBUG__
    std::cout << "action caller was asked for a solution!" << std::endl;
#endif

    return true;		// always ok!
  }

  // Construtor
  Action_Caller::Action_Caller( Property *cause, Priority_Action *act )
    : priority_action( act ) {
#ifdef __DEBUG__
    std::cout << "Action Caller created!" << std::endl;
#endif
    add_Property( cause );
  }
}
