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

#ifndef __AniTMT_Priority_Templateimplementaion__
#define __AniTMT_Priority_Templateimplementaion__

#include "priority.hpp"

namespace anitmt{

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

    if( prop->set_if_ok( val, &action_caller_inserter ) ) // if set succeeds
      {
#ifdef __DEBUG__
	std::cout << "default value " << val << " set!" << std::endl;
#endif

	delete this;
      } 
  }
  
  //**************************
  // constructors / destructor

  template<class T>
  Default_Value<T>::Default_Value( Priority_System *sys, 
				   Priority_System::level_type level,
				   Type_Property<T> *p, T v ) 
    : Priority_Action( sys, level ), action_caller_inserter( this ), 
      prop(p), val(v) {

    sys->add_Action( level, this );
  }

  template<class T>
  Default_Value<T>::~Default_Value() {}
  
  //***************************************************************************
  // Push_Connection<T>: establish push connection from one property to another
  //***************************************************************************

  //**********
  // functions

  template<class T>
  void Push_Connection<T>::do_it() {

#ifdef __DEBUG__
    std::cout << "try push!" << std::endl;
#endif

    if( source->is_solved() )
      {
#ifdef __DEBUG__
	std::cout << "push source:" << source->get() << std::endl;
#endif

	if( destination->set_if_ok( source->get(), &action_caller_inserter ) ) 
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
	std::err << "!!Error: push source not ready!" << std::endl;
#endif
      }
  }
  
  //**************************
  // constructors / destructor

  template<class T>
  Push_Connection<T>::Push_Connection( Priority_System *sys, 
				       Priority_System::level_type level,
				       Type_Property<T> *src, 
				       Type_Property<T> *dest ) 
    : Priority_Action( sys, level ), action_caller_inserter( this ), 
      source(src), destination(dest) {

    if( dest->is_solved() )	// if source is already solved
      {
	sys->add_Action( level, this );
      }
    else
      {
#ifdef __DEBUG__
    std::cout << "insert push caller!" << std::endl;
#endif
	insert_Caller_at_Property( src );
      }
  }

  template<class T>
  Push_Connection<T>::~Push_Connection() {}

  //****************
  // usage functions

  // establishes push connection to property of foreign tree node
  // ( returnvalue false means: unknown property )
  template<class T>
  bool establish_Push_Connection( Priority_System *sys, 
				  Priority_System::level_type level,
				  Type_Property<T> *src, 
				  Prop_Tree_Node   *dest_node,
				  std::string dest_prop ) {
  
    if( !dest_node ) return false;

#ifdef __DEBUG__
    std::cout << "try to establish push" << std::endl;
#endif
    Type_Property<T> *dest 
      = dynamic_cast< Type_Property<T>* >
      ( dest_node->get_property( dest_prop ) );

    if( !dest ) return false;

    establish_Push_Connection( sys, level, src, dest );

#ifdef __DEBUG__
    std::cout << "push established on level " << level << std::endl;
#endif

    return true;
  }

}

#endif
