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

/*****************************************************************************/
/* ini.cpp : routines for reading INI-Files                                  */
/*****************************************************************************/
/*                                                                           */
/* Date:        6.11.99                                                      */
/* Author:      Martin Trautmann                                             */
/*                                                                           */
/* Remarks:                                                                  */
/*                                                                           */
/*****************************************************************************/

#include "ini.hpp"

/*****************************************************************************/
/* Implementation                                                            */
/*****************************************************************************/

namespace ini{

  /*************/
  /* INI Error */
  /*************/

  INI_Error::INI_Error( int number, std::string message, std::string name, 
			int line )
    : err::Basic_Pos_Error( number, message ),filename(name),linenumber(line){
  }

  void INI_Error::print_Position( std::ostream &os){
    os << filename << ":" << linenumber << ":";
  }

  void ini_error( int number, std::string message, std::string name,int line ) 
    throw( INI_Error ){
    throw INI_Error( number, message, name, line );
  }

  /*********/
  /* clINI */
  /*********/

  void clINI::read_INI_File( std::string filename ){
    std::ifstream ini_file( filename.c_str() );

    if(!ini_file) 
      throw( err::File_Open_Error( 400,"Could not open INI file",filename ) );

    std::string line;

    clSection *act_Section = NULL;
    clOption  *act_Option  = NULL;
    for( int lnum = 1; getline(ini_file,line); ++lnum ){
      std::string exp = line;

      // get the first nonspace character
      std::string::size_type beg;
      for( beg = 0; isspace( exp[beg] ) && (beg < exp.size()) ; ++beg );

      if( beg >= exp.size() ) 	// is there no nonspace character?
	continue;		// try next line

      // remove final '\r' if nessesary
      std::string::size_type it = exp.find_last_of('\r');
      if( it != std::string::npos )
	exp.erase( it );

      // Is exp in square brackets?
      if( exp[beg] == '[' ) 
	{  
	  ++beg;		// name starts after left bracket

	  std::string namebuf = exp.substr(beg);
	  std::string::size_type pos = namebuf.find(']');

	  if( pos != std::string::npos )
	    {
	      std::string secname = namebuf.substr( 0, pos );
	      act_Section = new clSection( secname );
	      add_Section( act_Section );
	      act_Option = NULL;
	    }
	  else
	    {
	      // right bracket expected
	      ini_error( 401, "Right bracket expected", filename, lnum );
	    }
	}
      else
	{
	  if( exp[beg] == ';' )
	    {
	      // comment -> do nothing
	    }
	  else
	    {
	      if( !act_Section )
		{
		  // no section specified!
		  ini_error( 402, "Section expected", filename, lnum );
		}

	      std::string buf = exp.substr(beg);
	      std::string::size_type pos = buf.find('=');
	      
	      if( pos != std::string::npos )
		{
		  std::string optname = buf.substr( 0, pos  );
		  std::string val     = buf.substr( pos + 1 );
		  
		  act_Option = new clOption( optname, val );
		  act_Section->add_Option( act_Option );
		  
		}
	      else
		{
		  // missing '=' in line
		  ini_error( 403, "= expected in line", filename, lnum );
		}
	    }
	}
    }
  }

  void clINI::write_INI_File( std::string filename ){
    std::ofstream ini_file( filename.c_str() );

    if(!ini_file) 
      throw( err::File_Open_Error( 410,"Could not write INI file",filename ) );

    // let the sections write themselves
    if( Sections != 0 )
      Sections->write_all( ini_file );
  }

  clSection *clINI::get_Section( std::string sect_name ){
    if( Sections != 0 )
      return Sections->get_Section( sect_name );

    return 0;
  }

  // insert section in linked list
  void clINI::add_Section( clSection *s){
    s->next = Sections;

    if( Sections != 0 )
      Sections->prev = s;

    Sections = s;
  }


  // remove Section
  void clINI::del_Section( clSection *sec ){
    // relink list
    if( sec->next != 0 )
      sec->next->prev = sec->prev;
  
    if( sec->prev != 0 )
      sec->prev->next = sec->next;
    else
      Sections = sec->next; // new beginning of list
  
    sec->next = 0;
    delete sec;
  }

  // remove Section if available
  char clINI::del_Section( std::string sect_name ){
    clSection *sec = get_Section( sect_name );
  
    if( sec != 0 )
      {
	del_Section( sec );
	return 0;
      }

    return -1;
  }

  clINI::clINI(){
    Sections = 0;
  }

  clINI::~clINI(){
    if( Sections != 0 ) delete Sections;
  }

  /*************/
  /* clSection */
  /*************/

// strip name of the square brackets
  std::string clSection::get_name(){
    return name;
  }

  clSection *clSection::get_Section( std::string sect_name ){
    if( sect_name == name )
      return this;
  
    if( next != 0 )
      return next->get_Section( sect_name );

    return 0;
  }

  clOption *clSection::get_Option( std::string opt_name ){
    if( Options != 0 )
      return Options->get_Option( opt_name );

    return 0;
  }

  std::string clSection::get_Val( std::string opt_name ){
    if( Options != 0 )
      return Options->get_Val( opt_name );

    return "";
  }

  void clSection::add_Option( clOption *o ){
    o->next = Options;
    Options = o;
  }

  void clSection::set_Option( std::string name, std::string val ){
    clOption *o = get_Option( name );

    if( o == 0 )
      {
	add_Option( new clOption( name, val ) );
      }
    else
      {
	o->set_Option( val );
      }
  }

  void clSection::write_all( std::ofstream &file ){
    if( next != 0 )
      next->write_all( file );

    file << '[' << name << ']' << std::endl;
  
    if( Options != 0 )
      Options->write_all( file );

    file << std::endl;
  }

  clSection::clSection( std::string sect_name ) 
    : name( sect_name ), next(0), prev(0), Options(0) {}

  clSection::~clSection(){
    if( Options != 0 ) delete Options;
    if( next    != 0 ) delete next;
  }

  /************/
  /* clOption */
  /************/

  std::string clOption::get_Val(){
    return val;
  }

  std::string clOption::get_Name(){
    return name;
  }

  std::string clOption::get_Val( std::string opt_name ){
    if( opt_name == name )
      return val;
  
    if( next != 0 )
      return next->get_Val( opt_name );

    return ""; // return empty string if option doesn´t exist
  }

  clOption *clOption::get_Option( std::string opt_name ){
    if( opt_name == name )
      return this;
  
    if( next != 0 )
      return next->get_Option( opt_name );

    return 0; 
  }

  void clOption::set_Option( std::string value ){
    val = value;
  }

  void clOption::write_all( std::ofstream &file ){
    if( next != 0 )
      next->write_all( file );

    file << name << '=' << val << std::endl;
  }

  clOption::clOption( std::string opt_name, std::string opt_val ) 
    : name( opt_name ), val( opt_val ), next(0) {}

  clOption::~clOption(){
    if( next != 0 ) delete next;
  }

}
