/*****************************************************************************/
/**   Message handling system (errors, warnings, verbose messages)	    **/
/*****************************************************************************/
/**									    **/
/** Authors: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "message.hpp"

#warning "Replace strstream header with a std::stringstream version (GCC 3.0)"
#warning "Check warnings in message_inline.cpp... (about nl)"

#ifdef WOLFGANG
#include <stdio.h>
#endif

namespace message
{
  Null_Position *GLOB::no_position = new Null_Position;

  //**************************************************************
  // Constructors of central message classes

  Message_Manager::Message_Manager( Message_Handler *h )
    : handler(h) {}

  //**************************************************************
  // Message consultant

  Message_Consultant::Message_Consultant( Message_Manager *mgr, 
					  Message_Source_Identifier source )
    : manager(mgr), verbose_level(2), warnings(true), 
      msg_source(source), vindent(0) {}

  void Message_Consultant::message( Message_Type mtype,
				    const Abstract_Position *pos, 
				    int position_detail, 
				    const std::string &message,
				    bool noend )
  {
    // there is no need to check verbose or warnings here as 
    // this is already done when calling the Message_Stream 
    // constructor. Why do things twice?
    std::string i_message;
    if(mtype==MT_Verbose)
      {
	for(int i=0; i<vindent; i++)
	  {  i_message+="  ";  }
      }
    i_message+=message;
    manager->message( mtype, pos, position_detail, i_message, msg_source, 
		      noend );
  }

  //**************************************************************
  // Message streams

  static char *msg_strcpy(char *dest,const char *src,int slen)
  {
     assert(slen<_Message_Buf_Size);
     strncpy(dest,src,slen);
     return(dest+slen);
  }

  Message_Stream::Message_Stream( _NoInit ni ) 
    : enabled(false),
      msg_stream(message,_Message_Buf_Size-1)
  {
    assert( ni == noinit );
  }

  // Message_Stream (base class): 
  Message_Stream::Message_Stream( Message_Type message_type,
				  const Abstract_Position *p, int pos_detail,
				  Message_Consultant *consult, bool _enabled )
    : enabled(_enabled), 
      pos(p), position_detail(pos_detail), consultant(consult),
      mtype(message_type), 
      msg_stream(message,_Message_Buf_Size-1),no_end(false) {}

  // Actually the source is non-const. But for the outside world, const 
  // is just the right thing here.
  Message_Stream::Message_Stream( const Message_Stream& src ) 
    : enabled(src.enabled), 
      pos(src.pos), position_detail(src.position_detail),
      consultant(src.consultant), 
      mtype(src.mtype), 
      #if __GNUC__ < 3  /* there is a bug in gcc-2.95.x STL... */
      #define PCOUNT(src) const_cast<std::ostrstream*>(&src.msg_stream)->pcount()
      msg_stream(msg_strcpy(message,src.message,PCOUNT(src)),
		 _Message_Buf_Size-1-PCOUNT(src)),
      #undef PCOUNT
      #else  // gcc-3 does not have that bug in the STL:
      msg_stream(msg_strcpy(message,src.message,src.msg_stream.pcount()),
		 _Message_Buf_Size-1-src.msg_stream.pcount()),
      #endif
      no_end(src.no_end)
  {
    Message_Stream *ssrc=const_cast<Message_Stream *>(&src);
    ssrc->enabled = false;
  }

  Message_Stream::~Message_Stream()
  {
    msg_stream << '\0';		// add terminator for char[] strings
    if( enabled )
      consultant->message(mtype, pos, position_detail, message, no_end );
  }

  // copy operator that disables the source message
  Message_Stream &Message_Stream::operator=( Message_Stream& src ) 
  {
    enabled = src.enabled;
    pos = src.pos;
    position_detail = src.position_detail;
    consultant = src.consultant;
    mtype = src.mtype;
    // Make sure to terminate: 
    src.message[src.msg_stream.pcount()]='\0';
    msg_stream << src.message;
    #warning If there are no problems with this solution, remove that: [03/2002]
    #if 0 && defined(WOLFGANG)
    // This is likely to do the opposite of what we want: 
    strcpy( message, src.message );
    fprintf(stderr,"message.cpp:%d: Please report me as bug.\n",__LINE__);
    assert(0);
    // Should be something like that:
    //msg_stream=std::strstream(
    //    msg_strcpy(message,src.message,src.msg_stream.pcount()),
    //    _Message_Buf_Size-1-src.msg_stream.pcount());
    #endif
    no_end = src.no_end;

    src.enabled = false;
    return *this;
  }

  // copies itself to another Message stream and disables itself
  using namespace std;
  void Message_Stream::copy_to( Message_Stream& dest ) 
  {
    dest.enabled = enabled;
    dest.pos = pos;
    dest.position_detail = position_detail;
    dest.consultant = consultant;
    dest.mtype = mtype;
    // Make sure to terminate: 
    message[msg_stream.pcount()]='\0';
    dest.msg_stream << message;
    #warning If there are no problems with this solution, remove that: [03/2002]
    #if 0 && defined(WOLFGANG)
    // This is likely to do the opposite of what we want: 
    strcpy( dest.message, message );
    //fprintf(stderr,"message.cpp:%d: Please report me as bug.\n",__LINE__);
    //assert(0);
    // Should be something like that:
    /*const std::strstream tmp(
      msg_strcpy(dest.message,message,msg_stream.pcount()),
      _Message_Buf_Size-1-msg_stream.pcount());*/
    //dest.msg_stream=tmp;
    #endif
    dest.no_end = no_end;

    enabled = false;
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
      {  stream << std::endl;  }
    else  stream.flush();  // streams get flushed by std::endl. 
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

  //********************
  // Position classes
  //********************

  std::ostream &operator<<( std::ostream &os, const Abstract_Position &pos)
  {
    pos.write2stream(os,2);
    return os;
  }

  //****************
  // File Position

  void File_Position::write2stream(std::ostream &os,
				   int detail_level) const {
    os << filename << ':';
    if (line> 0 && detail_level > 0) os << line << ':';
    if (column>=0 && detail_level > 1) os << column << ':';
    os << ' ';
  }


  File_Position::File_Position(const std::string f,
			       const int l, const int c,
			       const int tl) :
    filename(f), line(l), column(c), tab_len(tl) {}
}
