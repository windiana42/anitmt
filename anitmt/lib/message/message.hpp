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
  // Reporter (n)-->(1) Consultant (n)-->(1) Manager (1)-->(1) Handler
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

  enum Message_Type
  {  MT_Verbose, MT_Warning, MT_Error  };

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

  //! Position used for messages with a general file position (name, line, column)
  class File_Position : public Abstract_Position {
  public:
	void write2stream(std::ostream&, int detail_level) const;
	//! Create a file position
	/*\param fn - file name
	  \param l - line in file (0 if unused)
	  \param c - column in file (0 if not used) */
	File_Position(const std::string fn, const int l=0, const int c=0);
  private:
	  const std::string fn;
	  const int l, c;
  };

  //! ID type for message source types
  class Message_Source_Identifier {
  public:
    int origin_id;  //! who sent the message (core, ADL parser...)
    int id;         //! may be used to discriminate between different ADL parsers, etc.
    Message_Source_Identifier( int origin ) : origin_id(origin),id(-1) {}
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
    struct Message {
      Message_Type mtype;
      Abstract_Position *pos;
      int position_detail;
      std::string message;
      Message_Source_Identifier source;
      
      Message(Message_Type, Abstract_Position *, int pos_detail, 
	      const std::string &, const Message_Source_Identifier &);
    };
    
    //! called when a message is to be written. 
    virtual void message( Message &msg )=0;
    virtual ~Message_Handler() {}
  };

  //**************************************************************
  // Message managing classes

  //! Central message manager to deliver all messages to one handler
  class Message_Manager {
    Message_Handler *handler;
  public:
    inline void message( Message_Type mtype,
			 Abstract_Position *pos, int position_detail, 
			 const std::string &message,
			 const Message_Source_Identifier &source_id );
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

    inline void message( Message_Type mtype, 
			 Abstract_Position *pos, int position_detail, 
			 const std::string message );

    Message_Consultant( Message_Manager *manager, 
			Message_Source_Identifier source );
  };

  //**************************************************************
  // Message streams
  class Message_Stream {
  private:
    bool enabled;		// only one copy of the object is enabled
    
    // data attached to message (for consultant)
    Abstract_Position *pos;
    int position_detail;
    Message_Consultant *consultant;

    // message itself
    Message_Type mtype;
    //!!! real std::string implementation missing !!!
    //    std::string message;
    //    std::ostringstream msg_stream;
    char message[_Message_Buf_Size];
    std::ostrstream msg_stream;
  public:
    //! pick up message
    template<class T> inline Message_Stream &operator<<(T v); 
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! construct only with info data
    Message_Stream(Message_Type mtype,
		   Abstract_Position *pos,int position_detail,
		   Message_Consultant *consultant,bool enabled); 
    //! copy constructor that disables copy source
    Message_Stream(Message_Stream &src);
    ~Message_Stream();
  };

  //**************************************************************
  // Message Interface for error reporting code

  class Message_Reporter {
    Message_Consultant *consultant;
  public:
    inline bool is_warning();
    inline bool is_verbose(int verbose_level );

    inline Message_Stream error   ( Abstract_Position *pos, 
				    int position_detail=2 );
    inline Message_Stream warn    ( Abstract_Position *pos, 
				    int position_detail=2 );
    inline Message_Stream verbose ( int min_verbose_level = 1,
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
    void message( Message &msg );  //!overriding a virtual

    Stream_Message_Handler( std::ostream &error_stream,
			    std::ostream &warning_stream,
			    std::ostream &verbose_stream );
  };
}

#include "message_inline.cpp"

#endif
