/*****************************************************************************/
/**   This file belongs to a parser library and is used by AniTMT	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/*****************************************************************************/

#ifndef __INI__
#define __INI__

#include <string>
#include <fstream>

#include "error.hpp"

namespace ini{
  /*************/
  /* INI Error */
  /*************/

  class INI_Error : public err::Basic_Pos_Error{
    std::string filename;
    int linenumber;

  public:
    void print_Position( std::ostream &os);

    INI_Error( int number, std::string message, std::string name, int line );
  };

  void ini_error( int number, std::string message, std::string name,int line ) 
    throw(INI_Error);

  /*********/
  /* clINI */
  /*********/

  class clINI;
  class clSection;
  class clOption;

  class clINI{
  public:
    clSection *Sections;

    void read_INI_File(std::string DName);
    void write_INI_File(std::string DName);

    clSection *get_Section( std::string name );
    void add_Section( clSection *s );
    void del_Section( clSection *sec );
    char del_Section( std::string secname ); // 0: section doesn't exist

    clINI();
    ~clINI();
  };

  class clSection{
    std::string name;
  public:
    clSection *next; // linked list
    clSection *prev;

    clOption *Options; // linked list of options

    std::string get_name();

    clSection *get_Section( std::string sectName );
    std::string get_Val( std::string name );
    clOption *get_Option( std::string name );
    void add_Option( clOption *o );
    void set_Option( std::string name, std::string val );

    void write_all( std::ofstream &ini_file );

    clSection( std::string sect_name );
    ~clSection();
  };

  class clOption{
    std::string name, val;

  public:
    clOption *next; // Verkettete Liste

    std::string get_Name();
    std::string get_Val();
    std::string get_Val( std::string opt_name );
    clOption *get_Option( std::string opt_name );
    void set_Option( std::string opt_val );

    void write_all( std::ofstream &ini_file );

    clOption( std::string opt_name, std::string opt_val );
    ~clOption();
  };
}
#endif
