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

#ifndef __AniTMT_Caller__
#define __AniTMT_Caller__

namespace anitmt{
  class Action_Caller;
}

#include "val.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "priority.hpp"

namespace anitmt{

  //************************************************************************
  // Action_Caller: class that inserts an action when an essential property
  //                is solved
  //************************************************************************

  class Action_Caller : public Solver {
    Priority_Action *priority_action;

    // Properties call that if they were solved
    virtual void do_when_prop_was_solved( Property *ID );
    // calculates results of a solved Property (ID) and returns wheather
    // the solution is ok
    virtual bool check_prop_solution_and_results
    ( Property *ID, Solve_Run_Info const *info );

  public:
    Action_Caller( Property *cause, Priority_Action *act );
  };
}
#endif
