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

#ifndef _lib_Message_HPP_
#define _lib_Message_HPP_

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
#warning "Replace me with std::stringstream (GCC 3.0)"
#include <strstream>

// strstream message buffer size: 
#define _Message_Buf_Size 1024

namespace message
{
  class GLOB {
  public:
    //! Global object for undefined positions
    static const Null_Position *no_position;
  };

  enum Message_Type
  {  MT_Verbose, MT_Warning, MT_Error, _MT_None  };

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

  //! Position used for messages with a general file position 
  //! (name, line, column)
  class File_Position : public Abstract_Position {
  public:
	void write2stream(std::ostream&, int detail_level) const;
	//! Create a file position
	/*!\param fn - file name
	  \param l - line in file (-1 if unused)
	  \param c - column in file (-1 if not used) */
	// NOTE: We need -1 as `unused' as column may be 0 (``before first 
	//       char of line'') (Wolfgang). 
	File_Position(const std::string fn, const int l=-1, const int c=-1);
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
    //    (could also add pointer to attach higher-level custom objects)
    //! Compare *this to x: 
    bool operator==(const Message_Source_Identifier &x)
    	{  return(origin_id==x.origin_id && id==x.id);  }
  };
  
  //! Message including text, position,...
  struct Message {
    const Abstract_Position *pos;
    std::string message;
    Message_Source_Identifier source_id;
  };

  class Message_Handler {
  public:
    //! This is only an internal type used to avoid changes in the message 
    //! handler API. 
    struct Message {
      Message_Type mtype;
      const Abstract_Position *pos;
      int position_detail;
      std::string message;
      Message_Source_Identifier source;
      bool no_end;  // special flag if the message is terminated by noend. 
      
      Message(Message_Type, const Abstract_Position *, int pos_detail, 
	      const std::string &, const Message_Source_Identifier &,bool);
    };
    
    //! called when a message has to be written. 
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
			 const Abstract_Position *pos, int position_detail, 
			 const std::string &message,
			 const Message_Source_Identifier &source_id,
			 bool no_end );
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
			 const Abstract_Position *pos, int position_detail, 
			 const std::string message, bool no_end );

    Message_Consultant( Message_Manager *manager, 
			Message_Source_Identifier source );
  };

  //**************************************************************
  // Message streams
  enum _NoEnd  {  noend  };
  
  class Message_Stream {
  private:
    bool enabled;		// only one copy of the object is enabled
    
    // data attached to message (for consultant)
    const Abstract_Position *pos;
    int position_detail;
    Message_Consultant *consultant;

    // message itself
    Message_Type mtype;
    //!!! real std::string implementation missing !!!
    //    std::string message;
    //    std::ostringstream msg_stream;
    char message[_Message_Buf_Size];
    std::ostrstream msg_stream;
    
    // If the stream was termianted by noend. 
    bool no_end;
    
    //! Don't assign message streams; use references or copy 
    //! constructor (when using operator<<()). 
    void operator=(const Message_Stream &ms) { }
  public:
    //! Pick up message (two versions: one for const refs; one for 
    //! copied objects)
    template<class T> inline Message_Stream &operator<<(const T &v);
    template<class T> inline Message_Stream &operator<<(T v);
    //! This is used to end messaged using noend. For obvious 
    //! reason, thid returns void. 
    inline void operator<<(_NoEnd v);
    
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! construct only with info data
    Message_Stream(Message_Type mtype,
		   const Abstract_Position *pos,int position_detail,
		   Message_Consultant *consultant,bool enabled); 
    //! copy constructor that disables copy source
    Message_Stream(Message_Stream &src);
    ~Message_Stream();
  };

  //**************************************************************
  // Message Interface for error reporting code

  //! All classes which want to report messages must be derived from 
  //! Message_Reporter. This class is designed so that it is as lean 
  //! as possible (using only one pointer) and does not have virtual 
  //! functions. 
  class Message_Reporter {
    Message_Consultant *consultant;
  public:
    inline bool is_warning();
    inline bool is_verbose(int verbose_level);

    inline Message_Stream error   ( const Abstract_Position *pos, 
				    int position_detail=2 );
    inline Message_Stream warn    ( const Abstract_Position *pos, 
				    int position_detail=2 );
    inline Message_Stream verbose ( int min_verbose_level = 1,
				    const Abstract_Position *pos =GLOB::no_position, 
				    int position_detail = 2 );

    Message_Reporter( Message_Consultant *consultant );
    ~Message_Reporter();
  };

  //**************************************************************
  // Default implementations of Abstract Interfaces
  
  //! This is the default message handler implementation. 
  //! It prints all messages to standard streams. 
  class Stream_Message_Handler : public Message_Handler {
    std::ostream &error_stream, &warning_stream, &verbose_stream;
    Message_Type noend_pending;
    void write_message(Message &msg,bool append_nl=true);
    std::ostream &get_stream(Message_Type mtype);
  public:
    void message( Message &msg );  //!overriding a virtual

    Stream_Message_Handler( std::ostream &error_stream,
			    std::ostream &warning_stream,
			    std::ostream &verbose_stream );
  };
}

#include "message_inline.cpp"

#endif  /* _lib_Message_HPP_ */
