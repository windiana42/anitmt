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

#ifndef _lib_Message_HPP_
#define _lib_Message_HPP_

#include <map>

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
#include <sstream>

namespace message
{
  class GLOB {
  public:
    //! Global object for undefined positions
    static Null_Position *no_position;
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
  std::ostream &operator<<( std::ostream&, const Abstract_Position& );

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

    // access/modify functions to the position
    // !assumption!: first line = 1, first column = 1
    inline int get_line();
    inline int get_column();
    inline void set_filename( std::string name );
    inline void set_pos( int line, int column );
    inline void inc_line();
    inline void inc_column( int n );
    inline void inc_column();
    inline void tab_inc_column();

    inline File_Position *duplicate();

    //! Create a file position
    /*!\param fn - file name
      \param l - line in file (-1 if unused)
      \param c - column in file (-1 if not used) */
    // NOTE: We need -1 as `unused' as column may be 0 (``before first 
    //       char of line'') (Wolfgang). 
    File_Position(const std::string filename="<no file>", 
		  const int line=-1, const int column=-1,
		  const int tab_len=8);
  private:
    std::string filename;
    int line, column;		// -1 means unset (init with set_filename)
    int tab_len;
  };

  //! ID type for message source types
  class Message_Source_Identifier {
  public:
    int origin_id;  //! who sent the message (core, ADL parser...)
    int id;         /*! may be used to discriminate between different 
		      ADL parsers, etc. */
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
  //**************************************************************

  //*****************
  // Message_Manager

  //! Central message manager to deliver all messages to one handler
  class Message_Manager {
    Message_Handler *handler;
    std::map<Message_Type,int> num_messages;
  public:
    inline int get_num_messages( Message_Type type );
    inline void clear_num_messages();
    inline void message( Message_Type mtype,
			 const Abstract_Position *pos, int position_detail, 
			 const std::string &message,
			 const Message_Source_Identifier &source_id,
			 bool no_end );
    Message_Manager( Message_Handler *handler );
  };

  //********************
  // Message_Consultant

  /*! Consults message reporters whether messages should be passed to the 
    manager. Therefore it stores some settings. It also attaches a message
    source identifier to the message */
  class Message_Consultant {
    Message_Manager *manager;
    int verbose_level;
    bool warnings;
    Message_Source_Identifier msg_source;
    int vindent;  // number of indentions for verbose messages
  public:
    inline int get_verbose_level();
    inline bool is_verbose(int level);
    inline bool is_warning();
    inline void set_msg_source( Message_Source_Identifier source );
    inline void set_verbose_level( int level );
    inline void set_warnings( bool warn );

    inline int get_num_messages( Message_Type type );
    inline void clear_num_messages();

    void message( Message_Type mtype, 
		  const Abstract_Position *pos, int position_detail, 
		  const std::string &message, bool no_end );

    inline int verbose_indent(int delta);
    inline void verbose_indent_set(int val);

    Message_Consultant( Message_Manager *manager, 
			Message_Source_Identifier source );
  };

  //**************************************************************
  // Message streams
  enum _NoEnd   {  noend  };
  enum _NewLine {  nl  };
  enum _KillMsg {  killmsg  };
  enum _NoInit  {  noinit  };
  
  class Message_Stream {
  private:
    bool enabled;		// only one copy of the object is enabled
    
    // data attached to message (for consultant)
    const Abstract_Position *pos;
    int position_detail;
    Message_Consultant *consultant;

    // message itself
    Message_Type mtype;

    std::ostringstream msg_stream;
    
    // If the stream was termianted by noend. 
    bool no_end;
    
  public:
    //! Pick up message (two versions: one for const refs; one for 
    //! copied objects)
    template<class T> inline Message_Stream &operator<<(const T &v);
    template<class T> inline Message_Stream &operator<<=(T v);
    //! This is used to end messaged using noend. For obvious 
    //! reason, thid returns void. 
    inline void operator<<(_NoEnd v);
    //! This is used to kill a message i.e. preventing from 
    //! being reported. Returns void for obvious reason. 
    inline void operator<<(_KillMsg v);
    //! This is used to insert a newline in a message: 
    inline Message_Stream &operator<<(_NewLine v);
    
    //! implicite convertion to ostream to allow operator access
    //inline operator std::ostream() { return msg_stream; }

    //! copy operator that disables copy source
    Message_Stream &operator=( Message_Stream &ms );
    // copies itself to another Message stream and disables itself
    void copy_to( Message_Stream& dest ); 
    //! uninitialized constuction
    Message_Stream( _NoInit );

    //! construct only with info data
    Message_Stream(Message_Type mtype,
		   const Abstract_Position *pos,int position_detail,
		   Message_Consultant *consultant,bool enabled); 
    //! copy constructor that disables copy source
    Message_Stream(const Message_Stream &src);
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
    Abstract_Position *default_position;
  public:
    inline bool is_warning();
    inline bool is_verbose(int verbose_level);
    inline int verbose_level();

    inline Message_Consultant *get_consultant() const;

    inline Message_Stream error   ( const Abstract_Position *pos = 0,
				    int position_detail=2 );
    inline Message_Stream warn    ( const Abstract_Position *pos = 0, 
				    int position_detail=2 );
    inline Message_Stream verbose ( int min_verbose_level = 1,
				    const Abstract_Position *pos = 0, 
				    int position_detail = 2 );

    inline int get_num_errors();
    inline int get_num_warnings();
    inline int get_num_verboses();
    inline void clear_num_messages();

    //! Functions to change the verbose indent level of this 
    //! consultant: 
    //! delta: number of indention steps to indent more (>0) or less (<0). 
    //! Normally, delta will be +1 or -1. 
    //! Returns new indent size. 
    inline int vindent(int delta);
    //! Set the indention size of a special value; normally used to 
    //! reset it to 0. 
    inline void vindent_set(int vindent=0);

    //! set the default position for messages
    inline void set_msg_default_position( Abstract_Position *pos );

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
    virtual void message( Message &msg );  

    Stream_Message_Handler( std::ostream &error_stream,
			    std::ostream &warning_stream,
			    std::ostream &verbose_stream );
  };
}

#include "message_inline.cpp"

#endif  /* _lib_Message_HPP_ */
