/*****************************************************************************/
/**   This file offers an action caller class realized as solver     	    **/
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

#include "caller.hpp"

namespace solve{

  //************************************************************************
  // Action_Caller: class that inserts an action when an essential property
  //                is solved
  //************************************************************************

  // has to check the result of the operand with ID as pointer to operand
  bool Action_Caller::is_result_ok( const void *ID, 
				    Solve_Run_Info *info ) 
    throw()
  {
#ifdef __DEBUG__
    std::cout << "action caller was asked for a solution!" << std::endl;
#endif
    assert( ID == trigger );

    return true;		// always ok!
  }

  // tells to use the result calculated by is_result_ok()
  void Action_Caller::use_result( const void *ID, Solve_Run_Info * )
    throw()
  {
    assert( ID == trigger );
#ifdef __DEBUG__
    std::cout << "place Action!" << std::endl;
#endif
    if( !action_placed )
    {
      priority_action->place_Action();
      action_placed = true;
    }
    //!!! no more function calls here !!! (may be deleted by place_Action)
  }

  // disconnect operand
  void Action_Caller::disconnect( const void *ID ) 
  {
    assert( ID == trigger );
    
    trigger = 0;
    delete this;
    //!!! no more function calls here !!! 
  }

  //**********************
  // Construtor/Destructor

  Action_Caller::Action_Caller( Basic_Operand &op, Priority_Action *act )
    : trigger(&op), priority_action( act ), action_placed(false)
  {
#ifdef __DEBUG__
    std::cout << "Action Caller created!" << std::endl;
#endif
    trigger->add_listener( this );
  }
  
  Action_Caller::~Action_Caller()
  {
    if( trigger )
      trigger->rm_listener( this );
  }
}
