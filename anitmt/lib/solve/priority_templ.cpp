/*****************************************************************************/
/**   This file offers a the basic functions for the priority system	    **/
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

#ifndef __Solve_Priority_Templateimplementaion__
#define __Solve_Priority_Templateimplementaion__

#include "priority.hpp"

#include <assert.h>

namespace solve{

  //***************************************************
  // Default_Value<T>: Action that sets a default value
  //***************************************************

  //**********
  // functions

  template<class T>
  void Default_Value<T>::do_it() { 
#ifdef __DEBUG__
    std::cout << "try default!" << std::endl;
#endif

    if( !op.is_solved() )
      {
	// if set succeeds
	if( op.set_value( val, &action_caller_inserter ) )
	  {
#ifdef __DEBUG__
	    std::cout << "default value " << val << " set!" << std::endl;
#endif	    
	    delete this;
	  } 
	// else action caller is already moved by action_caller_inserter
      }
    else
      {
#ifdef __DEBUG__
	std::cout << "default value " << val << " not needed!" << std::endl;
#endif	    
	delete this;
      }
  }
  
  //**************************
  // constructors / destructor

  template<class T>
  Default_Value<T>::Default_Value( Priority_System *sys, 
				   Priority_System::level_type level,
				   Operand<T> &o, T v ) 
    : Priority_Action( sys, level ), action_caller_inserter( this ), 
      op(o), val(v) {

    sys->add_Action( level, this );
  }

  template<class T>
  Default_Value<T>::~Default_Value() {}
  
  //***************************************************************************
  // Push_Connection<T>: establish push connection from one operand to another
  //***************************************************************************

  //**********
  // functions

  template<class T>
  void Push_Connection<T>::do_it() {

#ifdef __DEBUG__
    std::cout << "try push!" << std::endl;
#endif

    if( source.is_solved() )
      {
	if( !destination.is_solved() )
	  {
#ifdef __DEBUG__
	    std::cout << "push source:" << source.get_value() << std::endl;
#endif

	    if( destination.set_value( source.get_value(), 
					&action_caller_inserter ) ) 
	      {
#ifdef __DEBUG__
		std::cout << "push done!" << std::endl;
#endif
		delete this; // if push succeeds:
	      } 
	  }
	else
	  {
#ifdef __DEBUG__
	    std::cout << "push destination already set!" << std::endl;
#endif
	  }
      }
    else
      {
#ifdef __DEBUG__
	std::cerr << "!!Error: push source not ready!" << std::endl;
#endif
      }
  }
  
  //**************************
  // constructors / destructor

  template<class T>
  Push_Connection<T>::Push_Connection( Priority_System *sys, 
				       Priority_System::level_type level,
				       Operand<T> &src, 
				       Operand<T> &dest ) 
    : Priority_Action( sys, level ), action_caller_inserter( this ), 
      source(src), destination(dest) {

    if( src.is_solved() )	// if source is already solved
      {
	sys->add_Action( level, this );
      }
    else
      {
#ifdef __DEBUG__
    std::cout << "insert push caller!" << std::endl;
#endif
	insert_Caller_at_Operand( src );
      }
  }

  template<class T>
  Push_Connection<T>::~Push_Connection() {}

  //***************************************************************************
  // Conditioned_Push<T>: establishes push, when condition is solved and true
  //***************************************************************************

  //**********
  // functions

  template<class T>
  void Condition_Push<T>::do_it() 
  {
    assert( condition.is_solved() );

    if( condition() == true )
    {
      establish_Push_Connection( priority_system, priority_level, 
				 source, destination );
    }
  }
  
  //**************************
  // constructors / destructor

  template<class T>
  Condition_Push<T>::Condition_Push( Priority_System *sys, 
				     Priority_System::level_type level,
				     Operand<T> &src, Operand<T> &dest,
				     Operand<values::Flag> &cond ) 
    : Priority_Action( sys, level ), 
      source(src), destination(dest), condition(cond)
  {
    if( condition.is_solved() )
      do_it();
    else
    {
#ifdef __DEBUG__
      std::cout << "insert condition push caller!" << std::endl;
#endif
      insert_Caller_at_Operand( condition );
    }
  }

  /*
  // force compilation of Specializations:
  void all_priority_specializations() {
    Default_Value<values::Scalar> *force_compilation_of_spec1;
    Default_Value<values::Vector> *force_compilation_of_spec2;
    Default_Value<values::Matrix> *force_compilation_of_spec3;
    Default_Value<values::String> *force_compilation_of_spec4;
    Default_Value<values::Flag>   *force_compilation_of_spec5;

    Push_Connection<values::Scalar> *force_compilation_of_spec6;
    Push_Connection<values::Vector> *force_compilation_of_spec7;
    Push_Connection<values::Matrix> *force_compilation_of_spec8;
    Push_Connection<values::String> *force_compilation_of_spec9;
    Push_Connection<values::Flag>   *force_compilation_of_spec10;
  }
  */
}

#endif
