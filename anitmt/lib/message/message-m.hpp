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

#ifndef __lib_Message__
#define __lib_Message__

namespace message
{
  class Abstract_Position;
  class Null_Position;

  // Data flow:
  // Message_Reporter (n)-->(1) Message_Consultant (n)-->(1) Message_Manager
  // (1)-->(1) Message_Handler
  class Message_Handler;    
  class Message_Manager;
  class Message_Consultant;
  class Message_Reporter;

}

#include <string>
#include <iostream>
//!!! should be replaced by stringstream
#include <strstream>

// strstream message buffer size: 
#define _Message_Buf_Size 1024

namespace message
{
  class GLOB {
  public:
    //! Global object for undefined positions
    static Null_Position *no_position;
  };

  //**************************************************************
  // Abstract Interfaces
  
  //! Base class of all message positions
  class Abstract_Position {
  public:
    virtual void write2stream( std::ostream&, int detail_level ) const = 0;
    virtual ~Abstract_Position() {}
  };

  //! Position used for messages without position
  class Null_Position : public Abstract_Position {
  public:
    void write2stream( std::ostream&, int detail_level ) const {}
  };

  //! ID type for message source types
  class Message_Source_Identifier {
  public:
    int id;
    Message_Source_Identifier( int identifier ) : id(identifier) {}
    //!!! could be implemented with hierarchical id system !!!
  };
  //! Message including text, position,...
  struct Message {
    Abstract_Position *pos;
    std::string message;
    Message_Source_Identifier source_id;
  };

  class Message_Handler {
  public:
    virtual void error  ( Abstract_Position *pos, int position_detail, 
			  std::string message,
			  Message_Source_Identifier source_id ) = 0;
    virtual void warn   ( Abstract_Position *pos, int position_detail, 
			  std::string message,
			  Message_Source_Identifier source_id ) = 0;
    virtual void verbose( Abstract_Position *pos, int position_detail, 
			  std::string message,
			  Message_Source_Identifier source_id ) = 0;
    virtual ~Message_Handler() {}
  };

  //**************************************************************
  // Message managing classes

  //! Central message manager to deliver all messages to one handler
  class Message_Manager {
    Message_Handler *handler;
  public:
    inline void error  ( Abstract_Position *pos, int position_detail, 
			 std::string message,
			 Message_Source_Identifier source_id );
    inline void warn   ( Abstract_Position *pos, int position_detail, 
			 std::string message,
			 Message_Source_Identifier source_id );
    inline void verbose( Abstract_Position *pos, int position_detail, 
			 std::string message,
			 Message_Source_Identifier source_id );
    Message_Manager( Message_Handler *handler );
  };

  /*! Consults message reporters whether messages should be passed to the 
    manager. Therefore it stores some settings. It also attaches a message
    source identifier to the message */
  class Message_Consultant {
    Message_Manager *manager;
    int verbose_level;
    bool warnings;
    Message_Source_Identifier msg_source;
  public:
    inline int get_verbose_level();
    inline bool is_verbose(int level);
    inline bool is_warning();
    inline void set_msg_source( Message_Source_Identifier source );
    inline void set_verbose_level( int level );
    inline void set_warnings( bool warn );

    inline void error  ( Abstract_Position *pos, int position_detail, 
			 std::string message );
    inline void warn   ( Abstract_Position *pos, int position_detail, 
			 std::string message );
    inline void verbose( int min_verbose_level,
			 Abstract_Position *pos, int position_detail, 
			 std::string message );

    Message_Consultant( Message_Manager *manager, 
			Message_Source_Identifier source );
  };

  //**************************************************************
  // Message streams
  class Message_Stream {
  public:
    bool enabled;		// only one copy of the object is enabled
    
    // data attached to message (for consultant)
    Abstract_Position *pos;
    int position_detail;
    Message_Consultant *consultant;

    // message itself
    //!!! real std::string implementation missing !!!
    //    std::string message;
    //    std::ostringstream msg_stream;
    char message[_Message_Buf_Size];
    std::ostrstream msg_stream;

    //! construct only with info data
    Message_Stream(Abstract_Position *pos,int position_detail,
		  Message_Consultant *consultant,bool enabled); 
    //! copy constructor that disables copy source
    Message_Stream(Message_Stream &src);
    ~Message_Stream();
  };

  // error stream

  class Error_Stream : protected Message_Stream {
  public:
    //! pick up message
    template<class T> inline Error_Stream &operator<<(T v); 
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! construct only with info data
    Error_Stream( Abstract_Position *pos, int position_detail,
		  Message_Consultant *consultant ); 
    //! copy constructor that disables copy source
    Error_Stream( Error_Stream& src );
    //! send message to consultant
    ~Error_Stream();		
  private:
    //! disallow copy operator
    void operator=( Error_Stream& ){}
  };

  // warn stream

  class Warn_Stream : protected Message_Stream {
  public:
    //! pick up message
    template<class T> inline Warn_Stream &operator<<(T v); 
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! construct only with info data
    Warn_Stream( Abstract_Position *pos, int position_detail,
		 Message_Consultant *consultant ); 
    //! copy constructor that disables copy source
    Warn_Stream( Warn_Stream& src );
    //! send message to consultant
    ~Warn_Stream();		
  private:
    //! disallow copy operator
    void operator=( Warn_Stream& ){}
  };

  // verbose stream

  class Verbose_Stream : protected Message_Stream {
    int min_verbose_level;
  public:
    //! pick up message
    template<class T> inline Verbose_Stream &operator<<(T v); 
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! construct only with info data
    Verbose_Stream( int min_verbose_level,
		    Abstract_Position *pos, int position_detail,
		    Message_Consultant *consultant ); 
    //! copy constructor that disables copy source
    Verbose_Stream( Verbose_Stream& src );
    //! send message to consultant
    ~Verbose_Stream();		
  private:
    //! disallow copy operator
    void operator=( Verbose_Stream& ){}
  };

  //**************************************************************
  // Message Interface for error reporting code

  class Message_Reporter {
    Message_Consultant *consultant;
  public:
    inline bool is_warning();
    inline bool is_verbose(int verbose_level );

    inline Error_Stream   error   ( Abstract_Position *pos, 
				    int position_detail=2 );
    inline Warn_Stream    warn    ( Abstract_Position *pos, 
				    int position_detail=2 );
    inline Verbose_Stream verbose ( int min_verbose_level = 1,
				    Abstract_Position *pos =GLOB::no_position, 
				    int position_detail = 2 );

    Message_Reporter( Message_Consultant *consultant );
  };

  //**************************************************************
  // Default implementations of Abstract Interfaces
  
  //! Prints all messages on standard streams
  class Stream_Message_Handler : public Message_Handler {
    std::ostream &error_stream, &warning_stream, &verbose_stream;
  public:
    void error  ( Abstract_Position *pos, int position_detail, 
		  std::string message,
		  Message_Source_Identifier source_id );
    void warn   ( Abstract_Position *pos, int position_detail, 
		  std::string message,
		  Message_Source_Identifier source_id );
    void verbose( Abstract_Position *pos, int position_detail, 
		  std::string message,
		  Message_Source_Identifier source_id );
    
    Stream_Message_Handler( std::ostream &error_stream,
			    std::ostream &warning_stream,
			    std::ostream &verbose_stream );
  };
}

#include "message_inline.cpp"

#endif
