/*****************************************************************************/
/**   This file offers a the basic functions for the priority system	    **/
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

#include "priority.hpp"

namespace solve
{
  //***************************************************************
  // Priority_System: class that handels a complete priority system
  //***************************************************************

  // give the action on the lowest priority a try
  bool Priority_System::invoke_Action(){
    // determine action list with lowest priority
    priority_tab_type::iterator i = priority_tab.begin();
    // rename it list
    priority_level_list_type &list = i->second;
    // get the first action in there
    priority_level_list_type::iterator j = list.begin();
    // execute it
    (*j)->invoke();
    // remove it
    list.erase(j);
    // remove priority level if not needed any more
    if( list.empty() ) priority_tab.erase(i);

    return !priority_tab.empty();
  }

  // add priority action on level
  // ( action is deleted from destructor )
  void Priority_System::add_Action( level_type level, 
				    Priority_Action *action ){

    priority_tab[ level ].push_back( action );
  }

  void Priority_System::invoke_all_Actions(){
    while( invoke_Action() );
  }

  Priority_System::~Priority_System() {
    priority_tab_type::iterator i;
    priority_level_list_type::iterator j;

    for( i = priority_tab.begin(); i != priority_tab.end(); i++ )
      {
	for( j = i->second.begin(); j != i->second.end(); j++ )
	  {
	    delete *j;
	  }
      }    
  }

  //**************************************************************
  // Priority_Action: interface for Actions in the priority system 
  //**************************************************************

  // place Action for invokation
  void Priority_Action::place_Action() {
    priority_system->add_Action( priority_level, this );
  }

  // insert Action Caller as solver of Operand
  void Priority_Action::insert_Caller_at_Operand( Basic_Operand &prop ) {
    callers.push_back( new Action_Caller( prop, this ) );
  }

  // deletes callers and runs do_it()
  void Priority_Action::invoke() {
    // remove callers
    /*!!!
    callers_type::iterator i;
    for( i = callers.begin(); i != callers.end(); i++ )
       delete *i;
    callers.clear();
    !!!*/

    // do the action
    do_it();
  }

  //**************************
  // constructors / destructor

  Priority_Action::Priority_Action( Priority_System *sys, 
				    Priority_System::level_type level ) 
    : priority_system( sys ), priority_level( level ) {}

  Priority_Action::~Priority_Action() {}

  //*******************************************************************
  // Action_Caller_Inserter: inserts action callers in case of Problem
  //*******************************************************************

  // a operand collision occured!
  // (throws exceptions)
  void Action_Caller_Inserter::operand_collision_occured
  ( std::list< Basic_Operand* > ) throw(EX)
  {
    // ignore problem
  }
  
  // operand signals to reject value 
  // usage may be enforced by returning false
  bool Action_Caller_Inserter::may_operand_reject_val
  ( std::list< Basic_Operand* > bad_props ) throw(EX) {

    std::list< Basic_Operand* >::iterator i;
    for( i = bad_props.begin(); i != bad_props.end(); i++ )
      {
	priority_action->insert_Caller_at_Operand( **i );
      }

    return true;
  }


  //**************************
  // constructors / destructor

  Action_Caller_Inserter::Action_Caller_Inserter( Priority_Action *action ) 
    : priority_action( action ) {}

  //***************************************************************************
  // Action System test 
  //***************************************************************************

  int action_system_test()
  {
    int errors = 0;

    cout << endl;
    cout << "-------------------------------" << endl;
    cout << "Priority Action System Test..." << endl;
    cout << "-------------------------------" << endl;

    Operand<values::Scalar> s0; // start stretch
    Operand<values::Scalar> se; // end stretch
    Operand<values::Scalar> s ; // differance stretch
    Operand<values::Scalar> t ; // duration
    Operand<values::Scalar> a ; // acceleration
    Operand<values::Scalar> v0; // startspeed
    Operand<values::Scalar> ve; // endspeed

    cout << " beginning" << endl;
    cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;

    accel_solver( s, t, a, v0, ve );
    sum_solver( se, s, s0 );

    Priority_System sys;
    
    cout << " Actions:" << endl;
    cout << "  Level  5:  a= 0.5" << endl;
    cout << "  Level  7:  t=  s0" << endl;
    cout << "  Level 10: s0=   1" << endl;
    establish_Default_Value( &sys,  5, a, values::Scalar(0.5) );
    establish_Default_Value( &sys, 10, s0, values::Scalar(1) );
    establish_Push_Connection( &sys, 7, s0, t ); // push just for fun

    cout << " unset status" << endl;
    cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;
    v0.set_value( 0 );
    cout << " after v0=0" << endl;
    cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;
    sys.invoke_all_Actions();
    cout << " after result of actions" << endl;
    cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;

    return errors;
  }

}

