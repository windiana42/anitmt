/*****************************************************************************/
/**   Message handling system (errors, warnings, verbose messages)	    **/
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

#ifndef __lib_Message_inline__
#define __lib_Message_inline__

#include "message.hpp"

namespace message
{
  //**************************************************************
  // Message managing classes

  inline void Message_Manager::message( Message_Type mtype,
					Abstract_Position *pos, 
					int position_detail, 
					const std::string &message,
					const Message_Source_Identifier &source )
  {
    Message_Handler::Message msg(mtype,pos,position_detail,message,source);
    handler->message( msg );
  }

  int Message_Consultant::get_verbose_level() 
  { 
    return verbose_level; 
  }
  bool Message_Consultant::is_verbose(int level) 
  { 
    return verbose_level >= level; 
  }
  bool Message_Consultant::is_warning() 
  { 
    return warnings; 
  }

  void Message_Consultant::set_msg_source( Message_Source_Identifier src )
  {
    msg_source = src;
  }
  void Message_Consultant::set_verbose_level( int level )
  {
    verbose_level = level;
  }
  void Message_Consultant::set_warnings( bool warn )
  {
    warnings = warn;
  }

  void Message_Consultant::message( Message_Type mtype,
				    Abstract_Position *pos, 
				    int position_detail, 
				    const std::string message )
  {
    // there is no need to check verbose or warnings here as 
    // this is already done when calling the Message_Stream 
    // constructor. Why do things twice?
    manager->message( mtype, pos, position_detail, message, msg_source );
  }

  //**************************************************************
  // Message streams

  template<class T> inline Message_Stream& Message_Stream::operator<<(T v)
  {
    if( enabled ) 
      msg_stream << v; 
    return *this; 
  }

  //**************************************************************
  // Message reporting inline code: 

  inline Message_Reporter::Message_Reporter( Message_Consultant *c )
    : consultant(c) {}

  //**************************************************************
  // Message Interface for error reporting code

  inline bool Message_Reporter::is_warning()
  {
    return consultant->is_warning();
  }

  inline bool Message_Reporter::is_verbose(int verbose_level )
  {
    return consultant->is_verbose( verbose_level );
  }

  Message_Stream Message_Reporter::error( Abstract_Position *pos, 
					  int position_detail )
  {
    Message_Stream ret( MT_Error, pos, position_detail, 
			consultant, true );
    return ret;
  }

  Message_Stream Message_Reporter::warn( Abstract_Position *pos, 
					 int position_detail )
  {
    Message_Stream ret( MT_Warning, pos, position_detail, 
			consultant, consultant->is_warning() );
    return ret;
  }

  Message_Stream Message_Reporter::verbose ( int min_verbose_level,
					     Abstract_Position *pos, 
					     int position_detail )
  {
    Message_Stream ret( MT_Verbose, pos, position_detail, 
			consultant, consultant->is_verbose(min_verbose_level) );
    return ret;
  }

}

#endif
