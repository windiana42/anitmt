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
    cout << "try default!" << endl;
#endif

    if( prop->set_if_ok( val, &action_caller_inserter ) ) // if set succeeds
      {
#ifdef __DEBUG__
	cout << "default value" << val << " set!" << endl;
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
    cout << "try push!" << endl;
#endif

    if( source->s() )
      {
#ifdef __DEBUG__
	cout << "push source:" << source->get() << endl;
#endif

	if( destination->set_if_ok( source->get(), &action_caller_inserter ) ) 
	  {
#ifdef __DEBUG__
	    cout << "push done!" << endl;
#endif
	    delete this; // if push succeeds:
	  } 
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

    if( dest->s() )		// if source is already solved
      {
	sys->add_Action( level, this );
      }
    else
      {
	insert_Caller_at_Property( src );
      }
  }

  template<class T>
  Push_Connection<T>::~Push_Connection() {}
  
}

#endif
