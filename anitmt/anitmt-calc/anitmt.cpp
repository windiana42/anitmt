/*****************************************************************************/
/**   AniTMT main function                                        	    **/
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

#include "proptree.hpp"
#include "animation.hpp"
#include "nodes.hpp"
#include "save_filled.hpp"
#include "input.hpp"

#include <adlparser.hpp>
#include <params.hpp>

#include <fstream>

using namespace anitmt;

int main(int argc,char **argv,char **envp)
{
  // commandline handler of libpar (params.hpp)
  Command_Line cmd(argc,argv,envp);

  Animation ani("noname");
  if(!ani.param.Parse_Command_Line(&cmd) )
    return -1;
  
  if(!cmd.Check_Unused() )
    return -2;

  // init animation tree
  make_all_nodes_available();

  stringlist adlfiles = ani.param.adl();
  if( adlfiles.is_empty() )
    {
      cerr << "Error: no animation descriptions specified" << endl;
      return -3;
    }

  typedef std::list<Input_Interface*> input_type;
  input_type input;

  adlfiles.rewind();
  do
  { 
    input.push_back( new ADL_Input( adlfiles.current() ) );

    /*
    ifstream file( adlfiles.current().c_str() );
    ADLParser p( file, cout, true);
    p.ParseTree( &ani );
    */
  } while( adlfiles.next() );

  // for all input interfaces
  for( input_type::iterator i = input.begin(); i != input.end(); i++ )
  {
    (*i)->set_animation( &ani ); // set animation
    (*i)->create_structure();	// let create tree node structure
  }

  ani.hierarchy_final_init();	// finish structure initialization

  // for all input interfaces
  for( input_type::iterator i = input.begin(); i != input.end(); i++ )
  {
    (*i)->insert_expl_ref();	// insert user references between properties
  }

  // for all input interfaces
  for( input_type::iterator i = input.begin(); i != input.end(); i++ )
  {
    (*i)->insert_values();	// insert concrete values for properties
  }

  ani.pri_sys.invoke_all_Actions();

  save_filled("filled.out", &ani);

  return 0;
}
