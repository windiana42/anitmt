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

#ifndef __Solve_Priority__
#define __Solve_Priority__

#include <list>
#include <map>

namespace solve {
  class Priority_System;
  class Priority_Action;
  template<class T> class Default_Value;
}

#include <val/val.hpp>
#include "operand.hpp"
#include "caller.hpp"

namespace solve{

  //***************************************************************
  // Priority_System: class that handels a complete priority system
  //***************************************************************

  class Priority_System {
  public:
    typedef double level_type;

  private:
    typedef std::list< Priority_Action* > priority_level_list_type;
    typedef std::map< level_type, priority_level_list_type > priority_tab_type;
    priority_tab_type priority_tab;

    bool invoke_Action();

  public:
    void add_Action( level_type level, Priority_Action *action );
    void invoke_all_Actions();

    ~Priority_System();
  };

  //**************************************************************
  // Priority_Action: interface for Actions in the priority system 
  //**************************************************************

  class Priority_Action {
    typedef std::list< Action_Caller* > callers_type;
    callers_type callers;

    Priority_System *priority_system;
    Priority_System::level_type priority_level;
    

    friend class Action_Caller;
    void place_Action();
    virtual void do_it() = 0;
  protected:
    friend class Action_Caller_Inserter;
    void insert_Caller_at_Operand( Basic_Operand &op );
  public:
    void invoke();		// deletes callers and runs do_it()
    
    virtual ~Priority_Action();

    Priority_Action( Priority_System *sys, Priority_System::level_type level );
  };

  //*******************************************************************
  // Action_Caller_Inserter: inserts action callers in case of Problem
  //*******************************************************************

  class Action_Caller_Inserter : public Solve_Problem_Handler {
    Priority_Action *priority_action;
  public:
    // a operand collision occured!
    // ret false: ignore error
    virtual bool operand_collision_occured( std::list< Basic_Operand* > 
					    bad_ops,
					    Solve_Run_Info *info, 
					    message::Message_Reporter *msg );

    // operand signals to reject value 
    // ret false: enforce usage
    virtual bool may_operand_reject_val( std::list< Basic_Operand* > 
					 bad_ops,
					 Solve_Run_Info *info, 
					 message::Message_Reporter *msg );

    Action_Caller_Inserter( Priority_Action *action );
  };

  //****************************************************
  // Default_Value<T>: Action that sets a default value
  //****************************************************

  template<class T>
  class Default_Value : public Priority_Action {
    Action_Caller_Inserter action_caller_inserter;    

    Operand<T> &op;
    T val;
  public:
    virtual void do_it();

    Default_Value( Priority_System *sys, Priority_System::level_type level,
		   Operand<T> &o, T v );
    virtual ~Default_Value();
  };

  template<class T>
  inline void establish_Default_Value( Priority_System *sys, 
				       Priority_System::level_type level,
				       Operand<T> &p, T v ){
    new Default_Value<T>( sys, level, p, v );
  }

  // specialization for assigning double value to values::Scalar operand
  inline void establish_Default_Value( Priority_System *sys, 
				       Priority_System::level_type level,
				       Operand<values::Scalar> &p, 
				       double v ){
    new Default_Value<values::Scalar>( sys, level, p, values::Scalar(v) );
  }

  //***************************************************************************
  // Push_Connection<T>: establish push connection from one operand to another
  //***************************************************************************

  template<class T>
  class Push_Connection : public Priority_Action {
    Action_Caller_Inserter action_caller_inserter;    

    Operand<T> &source;
    Operand<T> &destination;
  public:
    virtual void do_it();

    Push_Connection( Priority_System *sys, Priority_System::level_type level,
		     Operand<T> &src, Operand<T> &dest );
    virtual ~Push_Connection();
  };

  // establishes push connection between two operands/properties
  template<class T>
  inline void establish_Push_Connection( Priority_System *sys, 
					 Priority_System::level_type level,
					 Operand<T> &src, 
					 Operand<T> &dest ) {
    new Push_Connection<T>( sys, level, src, dest );
  }

  //***************
  // test function
  //***************
  int action_system_test();
}

// force template generation of all used types
#include "priority_templ.cpp"

#endif
