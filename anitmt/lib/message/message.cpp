/*****************************************************************************/
/**   Message handling system (errors, warnings, verbose messages)	    **/
/*****************************************************************************/
/**									    **/
/** Authors: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "message.hpp"

namespace message
{
  const Null_Position *GLOB::no_position = new Null_Position;

  //**************************************************************
  // Constructors of central message classes

  Message_Manager::Message_Manager( Message_Handler *h )
    : handler(h) {}

  Message_Consultant::Message_Consultant( Message_Manager *mgr, 
					  Message_Source_Identifier source )
    : manager(mgr), verbose_level(2), warnings(true), msg_source(source) {}

  //**************************************************************
  // Message streams

  // Message_Stream (base class): 
  Message_Stream::Message_Stream( Message_Type message_type,
				  const Abstract_Position *p, int pos_detail,
				  Message_Consultant *consult, bool _enabled )
    : enabled(_enabled), 
      pos(p), position_detail(pos_detail), consultant(consult),
      mtype(message_type), 
      msg_stream(message,_Message_Buf_Size),no_end(false) {}

  Message_Stream::Message_Stream( Message_Stream& src ) 
    : enabled(src.enabled), 
      pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), 
      mtype(src.mtype), 
      message(src.message), msg_stream(message,_Message_Buf_Size), 
      no_end(src.no_end)
  {
    src.enabled = false;
  }

  Message_Stream::~Message_Stream()
  {
    msg_stream << '\0';		// add terminator for char[] strings
    if( enabled )
      consultant->message(mtype, pos, position_detail, message, no_end );
  }

  //**************************************************************
  // Message handler supporting code: 
  Message_Handler::Message::Message( Message_Type mt, const Abstract_Position *p, 
				     int pos_detail, const std::string &msg, 
				     const Message_Source_Identifier &src,
				     bool _no_end)
    : mtype(mt), pos(p), position_detail(pos_detail), message(msg),
      source(src), no_end(_no_end) { }

  //**************************************************************
  // Default implementations of Abstract Interfaces

  std::ostream &Stream_Message_Handler::get_stream(Message_Type mtype)
  {
    switch(mtype)
    {
      case MT_Verbose: return(verbose_stream);
      case MT_Warning: return(warning_stream);
      case MT_Error:   return(error_stream);
      case _MT_None:  break;
    }
    assert(0);
    return(error_stream);
  }

  void Stream_Message_Handler::write_message( Message &msg, bool append_nl )
  {
    std::ostream &stream=get_stream(msg.mtype);
    msg.pos->write2stream(stream,msg.position_detail);
	 if(msg.mtype==MT_Warning)  stream << "warning: ";
    /*else if(msg.mtype==MT_Error)    stream << "error: ";*/
    stream << msg.message;
    if(append_nl)
    {  stream << endl;  }
    stream.flush();
  }

  void Stream_Message_Handler::message( Message &msg )
  {
    if(noend_pending!=_MT_None && msg.mtype!=noend_pending)
    {
      get_stream(noend_pending) << std::endl;
      noend_pending=_MT_None;
    }
    write_message(msg,!msg.no_end);
    noend_pending=(msg.no_end) ? msg.mtype : _MT_None;
  }

  Stream_Message_Handler::Stream_Message_Handler( std::ostream &err,
						  std::ostream &warn,
						  std::ostream &verbose )
    : error_stream(err), warning_stream(warn), verbose_stream(verbose), 
      noend_pending(_MT_None)
  {}

  //**************************************************************
  // Position classes
  void File_Position::write2stream(std::ostream &os,
			      int detail_level) const {
	  os << fn << ':';
	  if (l> 0 && detail_level > 0) os << l << ':';
	  if (c>=0 && detail_level > 1) os << c << ':';
	  os << ' ';
  }
  File_Position::File_Position(const std::string _fn,
			       const int _l,
			       const int _c) :
	  fn(_fn), l(_l), c(_c) {}
}
