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

#ifndef _lib_Message_inline_HPP_
#define _lib_Message_inline_HPP_

#include "message.hpp"

namespace message
{
  //**************************************************************
  // Message managing classes
  //**************************************************************

  //*****************
  // Message_Manager

  inline void Message_Manager::message( Message_Type mtype,
					const Abstract_Position *pos, 
					int position_detail, 
					const std::string &message,
					const 
					Message_Source_Identifier &source,
					bool noend )
  {
    num_messages[mtype]++;
    Message_Handler::Message msg( mtype,pos,position_detail,
				  message,source,noend );
    handler->message( msg );
  }

  inline int Message_Manager::get_num_messages( Message_Type type )
  {
    return num_messages[type];
  }

  inline void Message_Manager::clear_num_messages()
  {
    std::map<Message_Type,int>::iterator i;
    for( i = num_messages.begin(); i != num_messages.end(); i++ )
    {
      i->second = 0;
    }
  }

  //********************
  // Message_Consultant

  inline int Message_Consultant::get_num_messages( Message_Type type )
  {
    return manager->get_num_messages( type );
  }

  inline void Message_Consultant::clear_num_messages()
  {
    manager->clear_num_messages();
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
  //**************************************************************

  //****************
  // Message_Stream

  template<class T> 
  inline Message_Stream& Message_Stream::operator<<(const T &v)
  {
    if( enabled ) 
      msg_stream << v; 
    return *this; 
  }

  template<class T> inline Message_Stream& Message_Stream::operator<<=(T v)
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

  inline void Message_Stream::operator<<(_KillMsg)
  {
    enabled=false;
    // Destructor will be called now and message will disappear. 
  }

  inline Message_Stream& Message_Stream::operator<<(_NewLine)
  {
    /*!!! #warning Please fix me: (should be handled more cleverly) !!!*/
    if(enabled)
      msg_stream << "\n";
    return *this;
  }


  //**************************************************************
  // Message reporting inline code: 

  //! set the default position for messages
  inline void Message_Reporter::set_msg_default_position( Abstract_Position 
							  *pos )
  {
    default_position = pos;
  }

  inline Message_Reporter::Message_Reporter( Message_Consultant *c )
    : consultant(c), default_position( GLOB::no_position ) {}

  inline Message_Reporter::~Message_Reporter()
  {  consultant=0;/*why?*/  }

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

  inline int Message_Reporter::verbose_level()
  {
    return consultant->get_verbose_level();
  }

  inline Message_Consultant *Message_Reporter::get_consultant() const
  {
    return consultant;
  }

  Message_Stream Message_Reporter::error( const Abstract_Position *pos, 
					  int position_detail )
  {
    if( pos == 0 ) pos = default_position;
    Message_Stream ret( MT_Error, pos, position_detail, 
			consultant, true );
    return ret;
  }

  Message_Stream Message_Reporter::warn( const Abstract_Position *pos, 
					 int position_detail )
  {
    if( pos == 0 ) pos = default_position;
    Message_Stream ret( MT_Warning, pos, position_detail, 
			consultant, consultant->is_warning() );
    return ret;
  }

  Message_Stream Message_Reporter::verbose ( int min_verbose_level,
					     const Abstract_Position *pos, 
					     int position_detail )
  {
    if( pos == 0 ) pos = default_position;
    Message_Stream ret( MT_Verbose, pos, position_detail, 
			consultant, consultant->is_verbose(min_verbose_level));
    return ret;
  }

  inline int Message_Reporter::get_num_errors()
  {
    return consultant->get_num_messages(MT_Error);
  }

  inline int Message_Reporter::get_num_warnings()
  {
    return consultant->get_num_messages(MT_Warning);
  }

  inline int Message_Reporter::get_num_verboses()
  {
    return consultant->get_num_messages(MT_Verbose);
  }
  
  inline void Message_Reporter::clear_num_messages()
  {
    consultant->clear_num_messages();
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

  inline File_Position *File_Position::duplicate()
  {
    return new File_Position( filename, line, column, tab_len );
  }
}

#endif  /* _lib_Message_inline_HPP_ */
