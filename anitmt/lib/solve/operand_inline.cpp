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

#ifndef __Solve_Operand_Inline_Implementation__
#define __Solve_Operand_Inline_Implementation__

#include "operand.hpp"

#include <algorithm>
#include <assert.h>

#ifdef EXTREME_INLINE
#define _INLINE_ inline
#else
#define _INLINE_ 
#endif


namespace solve 
{
  //**************************************************************
  // Solve_Run_Info: stores information needed during a solve run
  //**************************************************************

  // checks wheather an id belongs to curr. test
  _INLINE_ bool Solve_Run_Info::is_id_valid( id_type id ) const
  {
    // reverse search for id in valid id's
    valid_test_run_ids_type::const_iterator i = 
      find( valid_test_run_ids.begin(), valid_test_run_ids.end(), id );
    return i != valid_test_run_ids.end();
  }

  // returns current test run ID
  _INLINE_ Solve_Run_Info::id_type Solve_Run_Info::get_test_run_id() const
  {
    return test_run_id;
  }

  // adds and returns a new test run ID
  _INLINE_ Solve_Run_Info::id_type Solve_Run_Info::new_test_run_id()
  {
    test_run_id = current_default_test_run_id++;
    valid_test_run_ids.push_front( test_run_id );
    return test_run_id;
  }
   
  // sets current run ID, which has to be valid
  _INLINE_ void Solve_Run_Info::set_test_run_id( id_type id ) 
  {
    assert( is_id_valid( id ) );
    test_run_id = id;
  }

  // adds a test run ID
  _INLINE_ void Solve_Run_Info::add_test_run_id( id_type id )
  {
    test_run_id = id;
    valid_test_run_ids.push_front( test_run_id );
  }

  // removes all test run IDs that are newer than id
  _INLINE_ void Solve_Run_Info::remove_test_run_id( id_type id )
  {
    // remove all ids added after this one
    while( id != valid_test_run_ids.front() )
    {
      valid_test_run_ids.pop_front();
      assert( !valid_test_run_ids.empty() );
    }
    // remove this id
    valid_test_run_ids.pop_front();
    
    // id that was added just before is the new active id now
    assert( !valid_test_run_ids.empty() );
    test_run_id = valid_test_run_ids.front();
  }
  //! get a definitely invalid id
  _INLINE_ Solve_Run_Info::id_type Solve_Run_Info::get_inivalid_id() const
  {
    return -1;
  }

  _INLINE_ void Solve_Run_Info::set_trial_run( bool trial )
  {
    trial_run = trial;
  }
  _INLINE_ bool Solve_Run_Info::is_trial_run()
  {
    return trial_run;
  }
}
#undef _INLINE_

#endif
