/*****************************************************************************/
/**   This file offers an input interface                       	    **/
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

// needs to be included here, as anitmt.hpp needs the declaration of 
// Input_Manager
#include "anitmt.hpp"

#ifndef __AniTMT_Input_Interface__
#define __AniTMT_Input_Interface__

namespace anitmt{
  class Input_Manager;
};

#include <param/param.hpp>
#include <message/message.hpp>

#include "animation.hpp"

namespace anitmt
{
  void Input_Manager::init()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->init();
    }
  }
  void Input_Manager::read_structure()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->read_structure();
    }
  }
  void Input_Manager::read_values()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      (*i)->read_values();
    }
  }

  /*! adds a input format
    /param format is deleted automatically by destructor of Input_Manager
  */
  void Input_Manager::add_input_format( Input_Interface *format )
  {
    formats.push_back( format );
  }

  //!!! Quick, dirty implementation of constructor

  Input_Manager::Input_Manager( param::Parameter_Manager *par, 
				message::Message_Consultant *msg,
				Animation *ani )
    : param(par), consultant(msg)
  {
    add_input_format( new ADL_Input(ani) );
  }

  Input_Manager::~Input_Manager()
  {
    for( formats_type::iterator i = formats.begin(); i != formats.end(); i++ )
    {
      delete (*i);
    }
  }
}

#endif

