/*****************************************************************************/
/**   concept for event solver system               			    **/
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

#include "event_solver.hpp"

#include "operator.hpp"

namespace solve
{

// *************
// Event Solver
// *************

  //!!! solvers without any parameter operands won't be distroyed !!!
  Event_Solver::Event_Solver( message::Message_Consultant *consultant )
    : message::Message_Reporter(consultant),
      id(0), in_use_run(false), in_consider_reset(false)
  {
  }

  /*! tell all operands to whoom any of the events is listening to,
   *  that he should report his value to this solver */
  void Event_Solver::connect_operands_to_listener()
  { 
    // for all operands 
    std::map< Basic_Operand*,std::list<Event*> >::const_iterator operand;
    for( operand = event_listen_table.begin(); 
	 operand != event_listen_table.end(); 
	 ++operand )
    { 
      operand->first->add_listener(this);
    }      
  }

  //! enter event into operand listen table
  void Event_Solver::event_listens_operand( Event *event, Basic_Operand *op )
  {
    event_listen_table[op].push_back(event);
  }

  /*! has to check the result of an operand by reporting it to the
   *  right events
   */
  bool Event_Solver::is_result_ok( const Basic_Operand *solved_operand, 
				   Solve_Run_Info *info ) throw() 
  {
    in_use_run = false;		// unlock use_result();

    // check whether reset might be nessessary
    if( !info->is_id_valid( id ) )
    {
      assert( in_consider_reset == false );
      in_consider_reset = true;
      // search in test run ready list for events which need a reset
      bool once_more;
      do
      {
	once_more = false;
	std::list<Event*>::const_iterator event;
	for( event = test_run_ready_events.begin();
	     event != test_run_ready_events.end(); ++event )
	{
	  if( (*event)->consider_reset( info ) )
	  {
	    // reset was involked => event list changed => restart
	    once_more = true;
	    break;
	  }
	}
      }while( once_more );
      in_consider_reset = false;
    }
    id = info->get_test_run_id();

    // search in event listen list for events listening to solved operand
    std::map< Basic_Operand*,std::list<Event*> >::const_iterator listen_list;
    listen_list = 
      event_listen_table.find(const_cast<Basic_Operand*>(solved_operand));
    if( listen_list != event_listen_table.end() )
    {
      std::list<Event*>::const_iterator event;
      for( event = listen_list->second.begin(); 
	   event != listen_list->second.end(); ++event )
      {
	// event should consider test_run to test operand
	bool res = (*event)->is_result_ok( solved_operand, info );
	if( res == false ) return false;
      }
    }      
    return true;
  }

  //! tells to use the result calculated by is_result_ok()
  void Event_Solver::use_result( const Basic_Operand *solved_operand, 
			   Solve_Run_Info *info ) throw()
  {
    if( !in_use_run )
    {
      in_use_run = true;
      // check whether reset might be nessessary
      if( !info->is_id_valid( id ) )
      {
	bool once_more;
	do
	{
	  once_more = false;
	  std::list<Event*>::const_iterator event;
	  for( event = test_run_ready_events.begin();
	       event != test_run_ready_events.end(); ++event )
	  {
	    if( (*event)->consider_reset( info ) )
	    {
	      // reset was involked => event list changed => restart
	      once_more = true;
	      break;
	    }
	  }
	}while( once_more );
      }
      
      std::list<Event*>::const_iterator event;
      for( event = test_run_ready_events.begin();
	   event != test_run_ready_events.end(); ++event )
      {
	(*event)->use_result( solved_operand, info );
      }
      test_run_ready_events.clear();
    }
  }
    
  //! disconnect operand
  void Event_Solver::disconnect( const Basic_Operand *operand ) 
  {
    // disconnect this listener from all other operands
    std::map< Basic_Operand*,std::list<Event*> >::const_iterator op;
    for( op = event_listen_table.begin(); op != event_listen_table.end();++op )
    {
      if( op->first != operand )
      {
	op->first->rm_listener( this );
      }
    }
    delete this;		// very dangerous
  }

// *************
// Event Group
// *************

  Event_Group::Event_Group( message::Message_Consultant* consultant )
    : message::Message_Reporter( consultant ), data_ok(consultant) {}

  //! adds an event to the group
  void Event_Group::add_event( Event *event )
  {
    events.push_back(event);
  }
  //! tell that all events are added now
  void Event_Group::events_added()
  {
    Multi_And_Operator *m_and;
    m_and = new solve::Multi_And_Operator( get_consultant() );
    data_ok = m_and->get_result();

    std::list<Event*>::iterator i;
    for( i = events.begin(); i != events.end(); ++i )
    {
      m_and->add_operand( is_solved((*i)->data_ok) );
    }
  }

  //! invokes a group reset
  void Event_Group::do_reset( Solve_Run_Info* info )
  {
    std::list<Event*>::const_iterator event;
    // reset data base
    for( event = events.begin(); event != events.end(); ++event )
    {
      (*event)->do_reset( info );
    }
    call_reset();
    // reset events
    for( event = events.begin(); event != events.end(); ++event )
    {
      (*event)->consider_recalc_data( info );
    }
  }


// ******
// Event 
// ******

  Event::Event( Event_Solver *s, Event_Group *g,
		message::Message_Consultant* consultant )
    : message::Message_Reporter( consultant ),
      data_ok(consultant),
      stored_id(-1), id(-1), retry_at_once(false), num_ops(0), 
      num_ops_solved_in_try(0),
      me_in_test_run_ready_list( s->test_run_ready_events.end() ),
      solver(s), group(g)
  {
    group->add_event( this );
  }

  /*! add operand to this event; the event will be triggered, when all
   *  added operands are solved 
   */
  void Event::add_operand( Basic_Operand &op )
  { 
    operands.push_back( &op );
    num_ops++;
    if( op.is_solved() ) num_ops_solved_in_try++;
    solver->event_listens_operand( this, &op );
  }

  /*! looks whether one of the connected operands might have had an
   *  invalid value 
   */
  bool Event::consider_reset( Solve_Run_Info* info )
  {
    assert( status == valid_data_in_try );

    if( info->is_id_valid( id ) ) return false;

    group->do_reset( info );

    return true; // tell caller, that list iterators may have become invalid
  }

  //! does a reset invoked by the group
  void Event::do_reset( Solve_Run_Info* info )
  {
    if( stored_id >= 0 ) 
    {
      assert( info->is_id_valid(stored_id) );
      info->remove_test_run_id( stored_id );
      stored_id = -1;
    }
    switch( status )
    {
    case valid_data:		// recalc will follow
      call_reset();
      break;
    case valid_data_in_try:
      // remove from test run ready list
      solver->test_run_ready_events.erase( me_in_test_run_ready_list );
      me_in_test_run_ready_list = solver->test_run_ready_events.end();
      solved_operands.clear();
      if( !info->is_id_valid(id) )
	status = no_data;	// ID invalid -> no recalc will follow
				// ID   valid -> recalc will follow
      call_reset();
      break;
    case in_test_run:
      retry_at_once = true;	// no recalc (now) 
      call_reset();
      break;
    case no_data: break;	// no (re)calc
    }
  }

  //! try to rebuild data base when possible
  void Event::consider_recalc_data( Solve_Run_Info* info )
  {
    if( (status == valid_data_in_try) || (status == valid_data) )
    {
      assert( (status == valid_data) || info->is_id_valid(id) );
      Solve_Run_Info::id_type save_id = info->get_test_run_id();
      info->set_test_run_id( id ); // use last test run ID again
      bool res = is_result_ok(0,info);	// rerun test_run
      info->set_test_run_id( save_id );
      assert( res == true );
    }
  }

  /*! run test_run if enough operands avail to check whether operand
   * might be ok and to calculate new operand values
   */
  bool Event::is_result_ok( const Basic_Operand *solved_op, 
			    Solve_Run_Info* info )
  {
    assert( status != in_test_run );

    num_ops_solved_in_try++;
    if( num_ops_solved_in_try >= num_ops )
    {
      assert( operands.size() == (unsigned int) num_ops );

      // calc exact num_ops_solved_in_try
      num_ops_solved_in_try = 0;
      bool unsolved_ops = false;
      std::list< Basic_Operand* >::const_iterator op;
      for( op = operands.begin(); op != operands.end(); ++op )
      {
	if( (*op)->is_solved_in_try(info) ) 
	  num_ops_solved_in_try++;
	else
	  unsolved_ops = true;
      }
      if( unsolved_ops )	// if there are still unsolved operands...
	return true;		// ... return that operand is acceptable

      // set event id 
      id = info->get_test_run_id();

      // run the test_run. If there was a reset (retry_at_once) during
      // test_set_val, the test_run will be broken. Simply run test_run
      // again. So the data is recalculated and the test_run can continue.
      status_type old_status = status;
      status = in_test_run;
      do
      {
	retry_at_once=false;
	bool res = call_test_run( solved_op, info, this );
	if( res == false )
	{
	  // make sure, that reset is done when next operand value is
	  // reported to complex_solver
	  status = valid_data_in_try; // ... but try is not ok

	  solver->test_run_ready_events.push_front(this);
	  assert( me_in_test_run_ready_list == 
		  solver->test_run_ready_events.end() );
	  me_in_test_run_ready_list = solver->test_run_ready_events.begin();
	  return false;
	}
      }while( retry_at_once );
      switch( old_status )
      {
      case no_data:		// got new operand
      case valid_data_in_try:	// is_result_ok was just a recalc
	status = valid_data_in_try;
	solver->test_run_ready_events.push_front(this);
	me_in_test_run_ready_list = solver->test_run_ready_events.begin();
	break;
      case valid_data:		// is_result_ok was just a recalc
	status = valid_data;
	break;
      case in_test_run: assert( false );
      }
    }
    return true;		// return that operand is acceptable
  }

  //! use the results calculated and declare data as valid
  void Event::use_result( const Basic_Operand *solved_operand, 
			  Solve_Run_Info *info )
  {
    assert( status == valid_data_in_try );
    assert( info->is_id_valid(id) );

    std::list< Basic_Operand* >::const_iterator op;
    for( op = solved_operands.begin(); op != solved_operands.end(); ++op )
    {
      (*op)->use_test_value( info );
    }
    call_finish();
    status = valid_data;

    // event ready list will be cleared anyway
    me_in_test_run_ready_list = solver->test_run_ready_events.end();
  }


// ***************************************************************
// generated source could look like this:

  class blabla_Solver;

  class blabla_Solver : public Event_Solver
  {
  public:
    blabla_Solver( Operand<values::Scalar> &__op_s,
		   Operand<values::Scalar> &__op_a,
		   Operand<values::Scalar> &__op_t,		   
		   message::Message_Consultant *consultant );

  private:
    class blabla_Event_Group : public Event_Group
    {
    public:
      typedef void (blabla_Solver::*reset_func_type)();
  
      blabla_Event_Group( blabla_Solver *solver, reset_func_type reset,
			  message::Message_Consultant* );

    private:
      blabla_Solver *blaSolver;

      reset_func_type reset_func;

      virtual void call_reset();
    };

    class blabla_Event : public Event
    {
    public:
      typedef bool (blabla_Solver::*test_run_func_type)
	(const Basic_Operand *, Solve_Run_Info*, Event *);
      typedef void (blabla_Solver::*reset_func_type)();
      typedef void (blabla_Solver::*final_func_type)();

      blabla_Event( blabla_Solver *solver, Event_Group *grp, 
		    test_run_func_type test_run, reset_func_type reset, 
		    final_func_type final,
		    message::Message_Consultant* );

    private:
      blabla_Solver *blaSolver;

      test_run_func_type test_run_func;
      reset_func_type reset_func;
      final_func_type final_func;

      virtual bool call_test_run( const Basic_Operand *solved_op, 
				  Solve_Run_Info* info, Event *event );
      virtual void call_reset();
      virtual void call_final();
    };

    blabla_Event_Group group1; 
    void group1_reset();
    blabla_Event group1_event1;
    bool group1_event1_test_run( const Basic_Operand *solved_op, 
				       Solve_Run_Info* info,
				       Event *event );
    void group1_event1_reset();
    void group1_event1_final();
    blabla_Event_Group group2; 
    void group2_reset();
    blabla_Event group2_event1;
    bool group2_event1_test_run( const Basic_Operand *solved_op, 
				       Solve_Run_Info* info,
				 Event *event );
    void group2_event1_reset();
    void group2_event1_final();
  };


  // *******************
  // blabla_Event_Group
  // *******************
  
  blabla_Solver::blabla_Event_Group::blabla_Event_Group
  ( blabla_Solver *solver, reset_func_type reset, 
    message::Message_Consultant *consultant )
    : Event_Group(consultant), blaSolver(solver), reset_func(reset)
  {}

  void blabla_Solver::blabla_Event_Group::call_reset()
  {
    (blaSolver->*reset_func)();
  }


  blabla_Solver::blabla_Event::blabla_Event( blabla_Solver *solver, 
					     Event_Group *grp, 
					     test_run_func_type test_run, 
					     reset_func_type reset, 
					     final_func_type final,
					     message::Message_Consultant
					     *consultant )
    : Event( solver, grp, consultant ), blaSolver(solver), 
      test_run_func(test_run), reset_func(reset), final_func(final)
  {}

  // *************
  // blabla_Event
  // *************

  bool blabla_Solver::blabla_Event::call_test_run( const Basic_Operand*
						   solved_op, 
						   Solve_Run_Info* info, 
						   Event *event )
  {
    return (blaSolver->*test_run_func)(solved_op,info,event);
  }
  void blabla_Solver::blabla_Event::call_reset()
  {
    (blaSolver->*reset_func)();
  }
  void blabla_Solver::blabla_Event::call_final()
  {
    (blaSolver->*final_func)();
  }

  // **************
  // blabla_Solver
  // **************

  void blabla_Solver::group1_reset()
  {
  }

  bool blabla_Solver::group1_event1_test_run
  ( const Basic_Operand *solved_op, Solve_Run_Info* info, Event *event )
  {
    /*
    // <begin>
    bool res;

    // [[ set <OperandName> = <cpp_expression>; ]]
    res = <operand>.test_set_val( <expression> );
    if( (event->retry_at_once) || (res == false) )
    {
      return res;
    }
    // [[ try <OperandName> = <cpp_exression>; ]]
    Solve_Run_Info::test_id save_id = info->get_test_run_id();
    Solve_Run_Info::test_id id = info->new_test_run_id();
    bool was_trial_run = info->is_trial_run();
    info->set_trial_run(true);
    trial_res = <operand>.test_set_val( <cpp_expression>, info );
    info->set_trial_run(was_trial_run);
    if( trial_res == false ) 
    {
      info->remove_test_run_id( id ); // lot's of resets increase id very much
      info->set_test_run_id( save_id );
    }
    if( event->retry_at_once )
    {
      return true;
    }
    // [[ trial_failed ]]
    (trial_res == false)

    // [[ try_reject <OperandName>, ...; ]]  
    std::list<Basic_Operand*> bad_ops;
    bad_ops.push_back( <OperandName1> );
    ...
    res = info->problem_handler->may_operand_reject_val( bad_ops, info, this );
    if( res==true ) return false;

    // [[ is_solved_in_try(<OperandName>) ]]
    (<operand>.is_solved_in_try(info))

    // [[ is_just_solved(<OperandName>) ]]
    (&<operand> == solved_op)

    //??? [[ is_operand_solved_by_this_solver ]] 
    */

    // <end>
    return true;
  }
  void blabla_Solver::group1_event1_reset()
  {
  } 
  void blabla_Solver::group1_event1_final()
  {
  } 

  void blabla_Solver::group2_reset()
  {
  }
  bool blabla_Solver::group2_event1_test_run
  ( const Basic_Operand *solved_op, Solve_Run_Info* info, Event *event )
  {
    return true;
  } 
  void blabla_Solver::group2_event1_reset()
  {
  } 
  void blabla_Solver::group2_event1_final()
  {
  } 

  blabla_Solver::blabla_Solver( Operand<values::Scalar> &__op_s,
				Operand<values::Scalar> &__op_a,
				Operand<values::Scalar> &__op_t,
				message::Message_Consultant *consultant )
    : Event_Solver(consultant),
      group1( this, &blabla_Solver::group1_reset, consultant ),
      group1_event1( this, &group1, 
	      &blabla_Solver::group1_event1_test_run, 
	      &blabla_Solver::group1_event1_reset, 
	      &blabla_Solver::group1_event1_final, consultant ),
      group2( this, &blabla_Solver::group2_reset, consultant ),
      group2_event1( this, &group2, 
	      &blabla_Solver::group2_event1_test_run, 
	      &blabla_Solver::group2_event1_reset, 
	      &blabla_Solver::group2_event1_final, consultant )
  {
    group1_event1.add_operand( __op_s );
    group1_event1.add_operand( __op_a );

    group2_event1.add_operand( __op_t );

    connect_operands_to_listener();
  }

}
