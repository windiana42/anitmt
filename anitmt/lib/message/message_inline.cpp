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

#ifndef _lib_Message_inline_HPP_
#define _lib_Message_inline_HPP_

#include "message.hpp"

namespace message
{
  //**************************************************************
  // Message managing classes

  inline void Message_Manager::message( Message_Type mtype,
					const Abstract_Position *pos, 
					int position_detail, 
					const std::string &message,
					const Message_Source_Identifier &source,
					bool noend )
  {
    Message_Handler::Message msg(mtype,pos,position_detail,message,source,noend);
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

  int Message_Consultant::verbose_indent(int delta)
  {
    vindent+=delta;
    if(vindent<0)  vindent=0;
    return(vindent);
  }
  void Message_Consultant::verbose_indent_set(int val)
  {
    vindent=val;
    if(vindent<0)  vindent=0;
  }

  //**************************************************************
  // Message streams

  template<class T> 
  inline Message_Stream& Message_Stream::operator<<(const T &v)
  {
    if( enabled ) 
      msg_stream << v; 
    return *this; 
  }

  template<class T> inline Message_Stream& Message_Stream::operator<<(T v)
  {
    if( enabled ) 
      msg_stream << v; 
    return *this; 
  }

  inline void Message_Stream::operator<<(_NoEnd)
  {
    no_end=true;
    // Destructor will be called and will write the message. 
  }

  // copy operator that disables the source message
  inline Message_Stream &Message_Stream::operator=( Message_Stream& src ) 
  {
    enabled = src.enabled;
    pos = src.pos;
    position_detail = src.position_detail;
    consultant = src.consultant;
    mtype = src.mtype;
    strcpy( message, src.message );
    no_end = src.no_end;

    src.enabled = false;
    return *this;
  }

  // copies itself to another Message stream and disables itself
  inline void Message_Stream::copy_to( Message_Stream& dest ) 
  {
    dest.enabled = enabled;
    dest.pos = pos;
    dest.position_detail = position_detail;
    dest.consultant = consultant;
    dest.mtype = mtype;
    strcpy( dest.message, message );
    dest.no_end = no_end;

    enabled = false;
  }


  //**************************************************************
  // Message reporting inline code: 

  inline Message_Reporter::Message_Reporter( Message_Consultant *c )
    : consultant(c) {}

  inline Message_Reporter::~Message_Reporter()
  {  consultant=NULL;  }

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

  Message_Stream Message_Reporter::error( const Abstract_Position *pos, 
					  int position_detail )
  {
    Message_Stream ret( MT_Error, pos, position_detail, 
			consultant, true );
    return ret;
  }

  Message_Stream Message_Reporter::warn( const Abstract_Position *pos, 
					 int position_detail )
  {
    Message_Stream ret( MT_Warning, pos, position_detail, 
			consultant, consultant->is_warning() );
    return ret;
  }

  Message_Stream Message_Reporter::verbose ( int min_verbose_level,
					     const Abstract_Position *pos, 
					     int position_detail )
  {
    Message_Stream ret( MT_Verbose, pos, position_detail, 
			consultant, consultant->is_verbose(min_verbose_level) );
    return ret;
  }

  inline int Message_Reporter::vindent(int delta)
  {
    return(consultant->verbose_indent(delta));
  }
  inline void Message_Reporter::vindent_set(int val)
  {
    consultant->verbose_indent_set(val);
  }

  //********************
  // Position classes
  //********************

  // access/modify functions to the position
  inline int File_Position::get_line() 
  {
    return line;
  }
  inline int File_Position::get_column()
  {
    return column;
  }
  inline void File_Position::set_filename( std::string name )
  {
    filename = name;
    line = 1;
    column = 1;
  }
  inline void File_Position::set_pos( int l, int c )
  {
    line = l;
    column = c;
  }
  inline void File_Position::inc_line()
  {
    line++;
    column = 1; 
  }
  inline void File_Position::inc_column( int n )
  {
    column += n;
  }
  inline void File_Position::inc_column()
  {
    column++;
  }
  inline void File_Position::tab_inc_column()
  {
    column = (((column - 1) / tab_len) + 1) * tab_len + 1;
  }
}

#endif  /* _lib_Message_inline_HPP_ */
