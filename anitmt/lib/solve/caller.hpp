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

#ifndef __Solve_Caller__
#define __Solve_Caller__

namespace solve{
  class Action_Caller;
}

#include <val/val.hpp>
#include "operand.hpp"
#include "solver.hpp"
#include "priority.hpp"

namespace solve{

  //************************************************************************
  // Action_Caller: class that inserts an action when an essential operand
  //                is solved
  //************************************************************************

  class Action_Caller : public Operand_Listener {
    Basic_Operand *trigger;
    Priority_Action *priority_action;
    bool action_placed;

    // has to check the result of the operand with ID as pointer to operand
    virtual bool is_result_ok( const Basic_Operand *ID, 
			       Solve_Run_Info *info ) throw();
    // tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *ID, Solve_Run_Info *info )
      throw();

    // disconnect operand
    virtual void disconnect( const Basic_Operand *ID );

  public:
    Action_Caller( Basic_Operand &op, Priority_Action *act );
    ~Action_Caller();
  };
}
#endif
