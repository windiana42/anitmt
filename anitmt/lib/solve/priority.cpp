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
  bool Priority_System::invoke_Action()
  {
    if( priority_tab.empty() ) return false; // no more actions

    // determine action list with lowest priority
    priority_tab_type::iterator i = priority_tab.begin();
    // rename it list
    priority_level_list_type &list = i->second;
    assert( !list.empty() );
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
				    Priority_Action *action )
  {
    priority_tab[ level ].push_back( action );
  }

  void Priority_System::invoke_all_Actions()
  {
    while( invoke_Action() );
  }

  Priority_System::~Priority_System() 
  {
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
  void Priority_Action::place_Action() 
  {
    priority_system->add_Action( priority_level, this );
  }

  // insert Action Caller as solver of Operand
  void Priority_Action::insert_Caller_at_Operand( Basic_Operand &prop ) 
  {
    callers.push_back( new Action_Caller( prop, this ) );
  }

  // deletes callers and runs do_it()
  void Priority_Action::invoke() 
  {
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
  // ret false: ignore error
  bool Action_Caller_Inserter::operand_collision_occured
  ( std::list< Basic_Operand* >, Solve_Run_Info *info, 
    message::Message_Reporter *msg ) 
  {
    // ignore problem
    return false;
  }
  
  // operand signals to reject value 
  // ret false: enforce usage
  bool Action_Caller_Inserter::may_operand_reject_val
  ( std::list< Basic_Operand* > bad_props, Solve_Run_Info *info, 
    message::Message_Reporter *msg )
  {
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

}

