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

  inline void Message_Manager::error  ( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier source )
  {
    handler->error( pos, position_detail, message, source );
  }
  inline void Message_Manager::warn   ( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier source )
  {
    handler->warn( pos, position_detail, message, source );
  }

  inline void Message_Manager::verbose( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier source )
  {
    handler->verbose( pos, position_detail, message, source );
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

  void Message_Consultant::error( Abstract_Position *pos, 
				  int position_detail, 
				  std::string message )
  {
    manager->error( pos, position_detail, message, msg_source );
  }

  void Message_Consultant::warn( Abstract_Position *pos, 
				 int position_detail, 
				 std::string message )
  {
    if( is_warning() )
      manager->warn( pos, position_detail, message, msg_source );
  }
   
  void Message_Consultant::verbose( int min_verbose_level,
				    Abstract_Position *pos, 
				    int position_detail, 
				    std::string message )
  {
    if( is_verbose(min_verbose_level) )
      manager->verbose( pos, position_detail, message, msg_source );
  }

  //**************************************************************
  // Message streams

  template<class T> inline Error_Stream& Error_Stream::operator<<(T v)
  {
    msg_stream << v; 
    return *this; 
  } 

  template<class T> inline Warn_Stream& Warn_Stream::operator<<(T v)
  {
    if( enabled )
      msg_stream << v; 
    return *this; 
  } 

  template<class T> inline Verbose_Stream& Verbose_Stream::operator<<(T v)
  {
    if( enabled )
      msg_stream << v; 
    return *this; 
  } 

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

  Error_Stream Message_Reporter::error( Abstract_Position *pos, 
					int position_detail )
  {
    Error_Stream ret( pos, position_detail, consultant );
    return ret;
  }

  Warn_Stream Message_Reporter::warn( Abstract_Position *pos, 
				      int position_detail )
  {
    Warn_Stream ret( pos, position_detail, consultant );
    return ret;
  }

  Verbose_Stream Message_Reporter::verbose ( int min_verbose_level,
					     Abstract_Position *pos, 
					     int position_detail )
  {
    Verbose_Stream ret( min_verbose_level, pos, position_detail, consultant );
    return ret;
  }

}

#endif
