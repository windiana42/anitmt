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

  //**************************************************************
  // Message streams

  // Message_Stream (base class): 
  Message_Stream::Message_Stream( Message_Type message_type,
				  Abstract_Position *p, int pos_detail,
				  Message_Consultant *consult, bool _enabled )
    : enabled(_enabled), 
      pos(p), position_detail(pos_detail), consultant(consult),
      mtype(message_type), 
      msg_stream(message,_Message_Buf_Size) {}

  Message_Stream::Message_Stream( Message_Stream& src ) 
    : enabled(src.enabled), 
      pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), 
      mtype(src.mtype), 
      message(src.message), msg_stream(message,_Message_Buf_Size)
  {
    src.enabled = false;
  }

  Message_Stream::~Message_Stream()
  {
    msg_stream << '\0';		// add terminator for char[] strings
    if( enabled )
      consultant->message(mtype, pos, position_detail, message );
  }

  //**************************************************************
  // Message handler supporting code: 
  Message_Handler::Message::Message( Message_Type mt, Abstract_Position *p, 
				     int pos_detail, const std::string &msg, 
				     const Message_Source_Identifier &src)
    : mtype(mt), pos(p), position_detail(pos_detail), message(msg),
      source(src) { }

  //**************************************************************
  // Default implementations of Abstract Interfaces

  void Stream_Message_Handler::message( Message &msg )
  {
    std::ostream *stream;
    switch(msg.mtype)
    {
      case MT_Verbose: stream=&verbose_stream;  break;
      case MT_Warning: stream=&warning_stream;  break;
      case MT_Error:   stream=&error_stream;    break;
      default:  assert(0);  stream=&error_stream;  break;
    }
    msg.pos->write2stream(*stream,msg.position_detail);
	 if(msg.mtype==MT_Warning)  *stream << "warning: ";
    else if(msg.mtype==MT_Error)    *stream << "error: ";
    *stream << msg.message << endl;
  }

  Stream_Message_Handler::Stream_Message_Handler( std::ostream &err,
						  std::ostream &warn,
						  std::ostream &verbose )
    : error_stream(err), warning_stream(warn), verbose_stream(verbose)
  {}

  //**************************************************************
  // Position classes
  void File_Position::write2stream(std::ostream &os,
			      int detail_level) const {
	  os << "in file \"" << fn << "\"";
	  if (l && detail_level > 0) os << " line " << l;
	  if (c && detail_level > 1) os << ", column " << c;
	  os << ' ';
  }
  File_Position::File_Position(const std::string _fn,
			       const int _l,
			       const int _c) :
	  fn(_fn), l(_l), c(_c) {}
}
