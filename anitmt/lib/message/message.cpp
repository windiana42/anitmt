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

#include "message.hpp"

namespace message
{
  Null_Position *GLOB::no_position = new Null_Position;

  //**************************************************************
  // Constructors of central message classes

  Message_Manager::Message_Manager( Message_Handler *h )
    : handler(h) {}

  Message_Consultant::Message_Consultant( Message_Manager *mgr, 
					  Message_Source_Identifier source )
    : manager(mgr), verbose_level(2), warnings(true), msg_source(source) {}

  Message_Reporter::Message_Reporter( Message_Consultant *c )
    : consultant(c) {}

  //**************************************************************
  // Message streams

  // error stream

  Error_Stream::Error_Stream( Abstract_Position *p, int pos_detail,
			      Message_Consultant *consult )
    : enabled(true), 
      pos(p), position_detail(pos_detail), consultant(consult),
      msg_stream(message,300) {}

  Error_Stream::Error_Stream( Error_Stream& src ) 
    : enabled(src.enabled), pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), message(src.message), msg_stream(message,300)
  {
    src.enabled = false;
  }

  Error_Stream::~Error_Stream()
  {
    msg_stream << '\0';		// add terminator for char[] strings
    if( enabled )
      consultant->error( pos, position_detail, message );
  }

  // warning stream

  Warn_Stream::Warn_Stream( Abstract_Position *p, int pos_detail,
			    Message_Consultant *consult )
    : enabled(consult->is_warning()), 
      pos(p), position_detail(pos_detail), consultant(consult),
      msg_stream(message,300) {}

  Warn_Stream::Warn_Stream( Warn_Stream& src ) 
    : enabled(src.enabled), 
      pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), message(src.message), msg_stream(message,300)
  {
    src.enabled = false;
  }

  Warn_Stream::~Warn_Stream()
  {
    msg_stream << '\0';		// add terminator for char[] strings
    if( enabled )
      consultant->warn( pos, position_detail, message );
  }

  // verbose stream

  Verbose_Stream::Verbose_Stream( int verbose_level,
				  Abstract_Position *p, int pos_detail,
				  Message_Consultant *consult )
    : enabled(consult->is_verbose(verbose_level)), 
      min_verbose_level(verbose_level),
      pos(p), position_detail(pos_detail), consultant(consult),
      msg_stream(message,300) {}

  Verbose_Stream::Verbose_Stream( Verbose_Stream& src ) 
    : enabled(src.enabled), min_verbose_level(src.min_verbose_level),
      pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), message(src.message), msg_stream(message,300)
  {
    src.enabled = false;
  }

  Verbose_Stream::~Verbose_Stream()
  {
    if( enabled )
    {
      msg_stream << '\0';	// add terminator for char[] strings
      consultant->verbose( min_verbose_level, pos, position_detail, message );
    }
  }


  //**************************************************************
  // Default implementations of Abstract Interfaces

  void Stream_Message_Handler::error  ( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier )
  {
    pos->write2stream(error_stream,position_detail);
    error_stream << "error: " << message << endl;
  }
  void Stream_Message_Handler::warn   ( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier )
  {
    pos->write2stream(warning_stream,position_detail);
    warning_stream << "warning: " << message << endl;
  }

  void Stream_Message_Handler::verbose( Abstract_Position *pos, 
					int position_detail, 
					std::string message,
					Message_Source_Identifier source )
  {
    pos->write2stream(verbose_stream,position_detail);
    verbose_stream << message << endl;
  }

  Stream_Message_Handler::Stream_Message_Handler( std::ostream &err,
						  std::ostream &warn,
						  std::ostream &verbose )
    : error_stream(err), warning_stream(warn), verbose_stream(verbose)
  {}
}
