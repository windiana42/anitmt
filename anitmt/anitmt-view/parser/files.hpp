/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/*****************************************************************************/

#ifndef __files__
#define __files__

#include <string>
#include <fstream>
#include <iostream>
#include <list>
#include <stack>
#include <map>

namespace file{
  class R_File;
  struct Filepos;
  class File_pos_type;
  class File_Change_remove;
  class File_Change_insert;
  class File_Change;
  class R_File_changecopy;
  class R_File_Prec_changecopy;
}

#include "error.hpp"
#include "mystream.hpp"
#include "parser.hpp"

namespace file{

  /******************/
  /* Error messages */
  /******************/

  class File_Error : public err::Basic_Pos_Error{
    stream::Word_Stream *s;

  public:
    void print_Position( std::ostream &os);

    File_Error( int number, std::string message, stream::Word_Stream *ws );
  };

  class End_Of_File : public stream::End_Of_Stream{
  public:
    End_Of_File();
  }; // End of File Exception

  void error( int number, std::string message, stream::Word_Stream *ws ) 
    throw(File_Error);

  /*****************/
  /* Filepositions */
  /*****************/

  typedef std::string::size_type filepos_type;
  typedef std::string::size_type off_type;

  struct Filepos{
    filepos_type filepos;
    off_type line,pos;

    Filepos( filepos_type fp, off_type l, off_type p );
  };

  class File_pos_type : public stream::Pos_type{
    R_File *file;
  public:
    Filepos filepos;

    R_File *get_File();
    stream::Word_Stream *get_Stream(); // get WordStream at the saved position
    void print_Position(std::ostream &os);

    // init with actual position
    File_pos_type( R_File *file );
    File_pos_type( R_File *file, Filepos fp );
  };

  /*
  class Prec_File_pos_type{
    R_File_Prec_changecopy *files; 
    File_pos_type *pos;
    std::stack< R_File_changecopy *> filestack;
  public:
    void set_Pos();		// set position of filesystem 
    Prec_File_pos_type( R_File_Prec_changecopy *file );
  };
  */

  /*********/
  /* Files */
  /*********/

  class R_File : public stream::My_Word_Stream{
  protected:
    std::string filename;

    // gets character
    char get(char &ch) throw(End_Of_File);

    std::ifstream *file;
  public:
    std::string get_filename();

    // prints Position
    void print_Position(std::ostream &os);
    // gets the current position
    stream::Pos_type *get_Pos();

    Filepos get_filepos();
    void set_filepos( Filepos p );

    void rewind();
    bool eof();
    R_File( std::string file );
    virtual ~R_File();
  };

  /********************/
  /* Changes in files */
  /********************/

  class File_Change{
  public:
    virtual void change( std::istream*, std::ostream* ) = 0;
  };

  class File_Change_insert : public File_Change{
    std::string insert;

  public:
    virtual void change( std::istream*, std::ostream* );
    File_Change_insert( std::string insertstr );
    virtual ~File_Change_insert(){}
  };

  class File_Change_remove : public File_Change{
    filepos_type end;

  public:
    virtual void change( std::istream*, std::ostream* );
    File_Change_remove( filepos_type e ); // remove till position <e>
    File_Change_remove( R_File *file ); // remove till actual position
    File_Change_remove( R_File_Prec_changecopy *file ); 
  };

  /*******************************************/
  /* Files that may be changed while copying */
  /*******************************************/

  /*
  class Change_Container{
    File_Change *change;
    filepos_type pos;
  public:
    bool operator< (const Change_Container &ch);
  }
  */

  class R_File_changecopy : public R_File{
    typedef std::multimap< filepos_type, File_Change* > changes_type;
    typedef std::pair    < filepos_type, File_Change* > changes_pair;
    changes_type changes;
  public:
    void copy_changed( std::string dest_filename );

    void insert_change( File_Change *change );
    void insert_change( filepos_type pos, File_Change *change );

    R_File_changecopy( std::string file );
    ~R_File_changecopy();
  };

  /***************************************************************/
  /* Files that are Precompiled and may be changed while copying */
  /***************************************************************/

  enum prectype{ prec_include=1,prec_conditions=2,prec_define=4,prec_all=7 };

  // Precompile file and files that are included
  class R_File_Prec_changecopy : public stream::Word_Stream{
    typedef std::list< R_File_changecopy* > filelist;
    filelist files;
    std::stack< R_File_changecopy *> act_file;
    prectype precompile_type;
    void open( std::string filename );
    void expect( std::string word );

    parser::Parser parser;

    bool ignore_comments;
  public:
    void copy_changed( std::string dest_directory );

    void insert_change( File_Change *change );
    void insert_change( File_pos_type *pos, File_Change *change );

    // gets the next word
    std::string get_Word();
    // puts a word back to the buffer
    void unget_Word();
    // reads until terminater
    std::string get_until(std::string terminator);
    // reads until line end or terminator
    std::string get_Line(std::string terminator = "");
    // prints Position
    void print_Position(std::ostream &os);
    // gets the current position
    stream::Pos_type *get_Pos();

    // gets the current position
    File_pos_type *get_File_Pos();

    std::stack< R_File_changecopy *> get_Open_Files();
    void set_Open_Files(std::stack< R_File_changecopy *> filestack);

    void enable_comments();
    void disable_comments();

    R_File_Prec_changecopy( std::string firstfile, prectype prec_type, 
			    bool ignore_comments = true );
    virtual ~R_File_Prec_changecopy();
  };

}

#endif
