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

#ifndef __Solve_Event_Solver__
#define __Solve_Event_Solver__

#include <map>
#include <list>

namespace solve 
{
  class Event_Solver;
  class Event;
  class Event_Group;
}

#include "operand.hpp"

namespace solve 
{
  class Event_Solver : public Operand_Listener, 
		       public message::Message_Reporter
  {
  public:
    //!!! solvers without any parameter operands won't be distroyed !!!
    Event_Solver( message::Message_Consultant* );

    /*! tell all operands to whoom any of the events is listening to,
     *  that he should report his value to this solver 
     */
    void connect_operands_to_listener();

  private:
    friend class Event;

    //! enter event into operand listen table
    void event_listens_operand( Event *event, Basic_Operand *op );

    std::map< Basic_Operand*,std::list<Event*> > event_listen_table; 
    std::list<Event*> test_run_ready_events;

    Solve_Run_Info::id_type id; 
    bool in_use_run, in_consider_reset;

    /*! has to check the result of an operand by reporting it to the
     *  right events
     */
    virtual bool is_result_ok( const Basic_Operand *solved_operand, 
			       Solve_Run_Info *info ) throw();

    //! tells to use the result calculated by is_result_ok()
    virtual void use_result( const Basic_Operand *solved_operand, 
			     Solve_Run_Info *info ) throw();
    
    //! disconnect operand
    virtual void disconnect( const Basic_Operand *operand );

  };

  class Event_Group : public message::Message_Reporter
  {
  public:
    Event_Group( message::Message_Consultant* );

    //! adds an event to the group
    void add_event( Event *event );
    //! tell that all events are added now
    void events_added();	

    Operand<values::Flag> data_ok;
  private:
    friend class Event;

    std::list<Event*> events;
    virtual void call_reset() {}

    //! invokes a group reset
    void do_reset( Solve_Run_Info* info );
  public:
    virtual ~Event_Group() {}
  };

  class Event : public message::Message_Reporter
  {
  public:
    Event( Event_Solver *s, Event_Group *g, message::Message_Consultant* );

    /*! add operand to this event; the event will be triggered, when all
     *  added operands are solved 
     */
    void add_operand( Basic_Operand &op );

    Operand<values::Flag> data_ok;
    // for functions in derived event_solver
    inline bool get_retry_at_once() { return retry_at_once; }
    Solve_Run_Info::id_type stored_id;
    std::list< Basic_Operand* > solved_operands; // ... by this event
  private:
    friend class Event_Group;
    friend class Event_Solver;

    enum status_type { no_data, in_test_run, valid_data_in_try, valid_data };
    status_type status;
    Solve_Run_Info::id_type id;	// is test_run still valid

    bool retry_at_once;		// reset may set this when in test run
				// which tells test_run to stop and restart
    int num_ops, num_ops_solved_in_try;
    std::list< Basic_Operand* > operands; 

    std::list<Event*>::iterator me_in_test_run_ready_list;

    virtual bool call_test_run( const Basic_Operand *solved_op, 
				Solve_Run_Info* info,
				Event *event ) = 0;
    virtual void call_reset()  {}
    virtual void call_finish() {}

    Event_Solver *solver;
    Event_Group *group;

    /*! looks whether one of the connected operands might have had an
     *  invalid value 
     */
    bool consider_reset( Solve_Run_Info* info );

    //! does a reset invoked by the group
    void do_reset( Solve_Run_Info* info );

    //! try to rebuild data base when possible
    void consider_recalc_data( Solve_Run_Info* info );

    /*! run test_run if enough operands avail to check whether operand
     * might be ok and to calculate new operand values
     */
    bool is_result_ok( const Basic_Operand *solved_op, Solve_Run_Info* info );

    //! use the results calculated and declare data as valid
    void use_result( const Basic_Operand *solved_operand, 
		     Solve_Run_Info *info );

  public:
    virtual ~Event() {}
  };
}
#endif
