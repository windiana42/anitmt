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

#include "files.hpp"

#include <functional>
#include <algorithm>
#include <strstream>

#include "utils.hpp"

#include <ctype.h>

namespace file{

  /******************/
  /* Error messages */
  /******************/

  File_Error::File_Error( int number, std::string message, 
			  stream::Word_Stream *ws )
    : err::Basic_Pos_Error( number, message ), s(ws){
  }

  void File_Error::print_Position( std::ostream &os){
    s->print_Position(os);
  }

  End_Of_File::End_Of_File() : stream::End_Of_Stream(){
  }

  void error( int number, std::string message, stream::Word_Stream *ws ) 
    throw( File_Error ){
    throw File_Error( number, message,ws );
  }

  /*******************/
  /* Saved positions */
  /*******************/

  Filepos::Filepos( filepos_type fp, off_type l, off_type p )
    : filepos(fp), line(l), pos(p) {}

  // gives wordstream at the saved position
  stream::Word_Stream *File_pos_type::get_Stream(){ 
    file->set_filepos(filepos);
    return file;
  }

  void File_pos_type::print_Position(std::ostream &os){
    os << file->get_filename() << ":" << 
      filepos.line << ":" << filepos.pos << ":";
  }

  R_File *File_pos_type::get_File(){
    return file; 
  }

  File_pos_type::File_pos_type( R_File *f )
    : file(f), filepos(f->get_filepos()) 
  {
  }

  File_pos_type::File_pos_type( R_File *f, Filepos fp )
    : file(f), filepos(fp)
  {
  }

  /*
  void Prec_File_pos_type::set_Pos(){
    filestack.top()->set_filepos( pos->filepos );
    files->set_Open_Files( filestack );
  }

  Prec_File_pos_type::Prec_File_pos_type( R_File_Prec_changecopy *file ){
    files = file;
    pos = file->get_File_Pos();
    filestack = file->get_Open_Files();
  }
  */

  /********************/
  /* Changes in files */
  /********************/

  //*******
  // insert
  void File_Change_insert::change( std::istream *is, std::ostream *os ){
    *os << insert;
  }

  File_Change_insert::File_Change_insert( std::string insertstr ){
    insert = insertstr;
  }


  //*******
  // remove
  void File_Change_remove::change( std::istream *is, std::ostream *os ){
    is->seekg( end );
  }

  // remove till position <e>
  File_Change_remove::File_Change_remove( filepos_type e ){
    end = e;
  }

  // remove till actual position
  File_Change_remove::File_Change_remove( R_File *file ){
    end = file->get_filepos().filepos;
  }

  // remove till actual position
  File_Change_remove::File_Change_remove( R_File_Prec_changecopy *file ){
    end = file->get_File_Pos()->filepos.filepos;
  }

  /*********/
  /* Files */
  /*********/

  void R_File::print_Position(std::ostream &os){ 
    stream::My_Word_Stream::print_Position( os << filename << ":" );
  }

  std::string R_File::get_filename(){
    return filename;
  }

  char R_File::get( char &ch ) throw(End_Of_File){
    if( !file->get(ch) )
      throw End_Of_File();
    return 0;
  }

  // has to be implemented because of interface Word_Stream
  stream::Pos_type *R_File::get_Pos(){
    return new File_pos_type( this );
  }

  Filepos R_File::get_filepos(){
    return Filepos( file->tellg(), line, pos );
  }

  void R_File::set_filepos( Filepos fp ){
    clear_Word_Buffer();
    file->clear();
    file->seekg( fp.filepos );
    pos = fp.pos;
    line = fp.line;
  }

  void R_File::rewind(){
    clear_Word_Buffer();
    file->clear();
    file->seekg( 0 );
    pos = 1;
    line = 1;
  }

  bool R_File::eof(){
    return file->eof();
  }

  R_File::R_File( std::string fname )
    : stream::My_Word_Stream(){
    // the last 10 read words are stored to be ungot
    filename = fname;

    file = new std::ifstream( fname.c_str() );

    if( !(*file) ) 
      error( 100, "could not open \"" + fname + "\"", this );
  }

  R_File::~R_File(){
    delete file;
  }

  /***********************************/
  /* Files that may be copy changed  */
  /***********************************/

  void R_File_changecopy::copy_changed( std::string dest_filename ){
    rewind();
    std::ofstream os( dest_filename.c_str() );

    if( !os ) error( 102, "could not copy to file " + dest_filename, this );

    filepos_type actpos;
    char ch;

    try{
      changes_type::const_iterator i;
      for( i = changes.begin(); i != changes.end(); i++ )
	{
	  actpos = file->tellg();

	  // copy until the next change
	  for(; actpos < (i->first); actpos++ )
	    {
	      ch = get_ch();
	      os.put( ch );
	    }
	  
	  i->second->change( file, &os );
	}
      
      for(;;)
	{
	  ch = get_ch();
	  os.put( ch );
	}
    }
    catch( End_Of_File ) {}
  }

  void R_File_changecopy::insert_change( File_Change *change ){
    changes.insert( changes_pair( offset, change ) );
  }

  void R_File_changecopy::insert_change( filepos_type pos, 
					 File_Change *change ){
    changes.insert( changes_pair( pos, change ) );
  }

  R_File_changecopy::R_File_changecopy( std::string file )
    : R_File( file ){

  }

  R_File_changecopy::~R_File_changecopy(){
    std::for_each( changes.begin(), changes.end(), 
		   utils::delete_maped(changes) );
  }

  /*************************************************/
  /* Precompiled Files that may be copyed changed  */
  /*************************************************/

  void R_File_Prec_changecopy::copy_changed( std::string dest_directory ){
    filelist::iterator i;

    for( i = files.begin(); i != files.end(); i++ )
      {
	(*i)->copy_changed( dest_directory + (*i)->get_filename() );
      }
  }

  void R_File_Prec_changecopy::insert_change( File_Change *change ){
    act_file.top()->insert_change( change );
  }

  void R_File_Prec_changecopy::insert_change( File_pos_type *pos,
					      File_Change *change ){
    dynamic_cast<R_File_changecopy*>(pos->get_File())->
      insert_change( pos->filepos.filepos, change );
  }

  std::string R_File_Prec_changecopy::get_Word(){
    std::string word;

    if( ignore_comments ) 
      act_file.top()->enable_comments();
    else
      act_file.top()->disable_comments();

    while( (word = act_file.top()->get_Word()) == "" )
      {
	if( act_file.size() > 1 ) // one file still remains open
	  act_file.pop();
	else
	  return "";

	if( ignore_comments ) 
	  act_file.top()->enable_comments();
	else
	  act_file.top()->disable_comments();
      }

    if( !(precompile_type) ) return word;

    if( (word == "#include") && (precompile_type | prec_include) )
      {
	std::string incfile = parser.getString().get_string();

	/*
	expect("\"");
	std::string incfile = get_Line("\"");
	if( incfile.size() )
	  if( incfile[ incfile.size() - 1 ] == '\n' )
	    error( 102, "\" after filename expected ", this);
	*/

	try{
	  open( incfile );
	}
	catch(File_Error){
	  // ignore file
	}

	return get_Word();
      }

    return word;
  }

  void R_File_Prec_changecopy::unget_Word(){
    act_file.top()->unget_Word();
  }

  std::string R_File_Prec_changecopy::get_until(std::string terminator){
    std::string str;

    for(;;){
      try{
	str = act_file.top()->get_until(terminator);
	break;
      }
      catch(File_Error){
	if( act_file.size() > 1 )
	  act_file.pop();
	else
	  throw;
      }
    }

    return str;
  }

  std::string R_File_Prec_changecopy::get_Line(std::string terminator){
    return act_file.top()->get_Line(terminator);
  }

  void R_File_Prec_changecopy::print_Position(std::ostream &os){
    act_file.top()->print_Position(os);
  }

  stream::Pos_type *R_File_Prec_changecopy::get_Pos(){
    return act_file.top()->get_Pos();
  }

  File_pos_type *R_File_Prec_changecopy::get_File_Pos(){
    return new File_pos_type( act_file.top() );
  }

  std::stack< R_File_changecopy *> R_File_Prec_changecopy::get_Open_Files(){
    return act_file;
  }

  void R_File_Prec_changecopy::set_Open_Files( std::stack<R_File_changecopy*> 
					       filestack ){
    act_file = filestack;
  }

  
  
  void R_File_Prec_changecopy::expect( std::string word ){
    if( get_Word() != word )
      error( 101, word + " expected", this );
  }

  // special object for filename search
  class chk_filename{
    std::string filename;
  public:
    bool operator ()(R_File_changecopy *f){
      return f->get_filename() == filename;
    }

    chk_filename( std::string file ) : filename (file) {}
  };

  void R_File_Prec_changecopy::open( std::string filename ){
    filelist::iterator i = find_if( files.begin(), files.end(), 
				    chk_filename(filename) );
    R_File_changecopy *f;

    // was there no file with the same filename in files?
    if( i == files.end() )
      {
	f = new R_File_changecopy(filename);
	stream::mk_C_Stream(*f); // prepare file for c++ syntax :)
	if( !ignore_comments ) f->disable_comments();
	
	files.push_back(f);
      }
    else			// there was already a file with the same name
      f = *i;			// use this one

    act_file.push(f);
  }

  void R_File_Prec_changecopy::enable_comments(){
    ignore_comments = true;
  }

  void R_File_Prec_changecopy::disable_comments(){
    ignore_comments = false;
  }

  R_File_Prec_changecopy::R_File_Prec_changecopy( std::string firstfile,
						  prectype prec_type,
						  bool comments)
    : precompile_type( prec_type ), parser(this), ignore_comments(comments){
    open( firstfile );
  }

  R_File_Prec_changecopy::~R_File_Prec_changecopy(){
    // delete all opened files
    for_each( files.begin(), files.end(), utils::delete_ptr( files ) );
  }
}
