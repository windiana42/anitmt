/*****************************************************************************/
/**   anitmt-calc main function	                                	    **/
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

#include <fstream>

#include <param/param.hpp>
#include <message/message.hpp>
#include <proptree/proptree.hpp>
#include "anitmt.hpp"

/**/
#include "animation.hpp"
#include "save_filled.hpp"
#include <input/input.hpp>
#include <output/output.hpp>

#include <par/params.hpp>

// input filters
#include <input/adl2/adlparser.hpp>
// output filters
#include <output/oformats.hpp>
/**/
using namespace anitmt;
using namespace message;

enum Message_Sources{ ANITMT_Core };

int main(int argc,char **argv,char **envp)
{
  try
  {
    /*
    Commandline_Parameter_Source param_source(argc,argv,envp);
    Parameter_Manager param_manager(param_source);
    Stream_Message_Handler msg_handler(cerr,cout,cout);

    AniTMT ani( &param_manager, &msg_handler );
    ani.process();
    */
    // This is QnD, I know... but it seems that anitmt.*
    // will someday make all this code here superfluous ?! - OK
    Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
    Message_Manager manager(&msg_handler);
    Message_Consultant default_msg_consultant(&manager, 0);
    Message_Reporter msg(&default_msg_consultant);

    // commandline handler of libpar (params.hpp)
    Command_Line cmd(argc,argv,envp);

    Animation ani("noname", &manager );
    if(!ani.GLOB.param.Parse_Command_Line(&cmd) )
      return -1;
  
    if(!cmd.Check_Unused() )
      return -2;

    // init animation tree
    ani.init();

    stringlist adlfiles = ani.GLOB.param.adl();
    if( adlfiles.empty() )
    {
      msg.error() << "no animation descriptions specified";
      return -3;
    }

    typedef std::list<Input_Interface*> input_type;
    input_type input;

    for(stringlist::iterator i=adlfiles.begin(); i!=adlfiles.end(); i++)
    {
      input.push_back( new ADL_Input( *i, &ani,
				      &default_msg_consultant ) );
    }

    msg.verbose() << "read structure...";

    // for all input interfaces
    for( input_type::iterator i = input.begin(); i != input.end(); i++ )
    {
      (*i)->create_structure(); // let create tree node structure
    }

#warning !!! fixed filename for filled ADL output !!!
    save_filled("unfilled.out", &ani );

    if( msg.get_num_errors() > 0 )
    {
      msg.vindent_set(0);
      msg.verbose() << "After reading structure: giving up due to errors.";
      msg.verbose() << "  errors: " << msg.get_num_errors()
		    << "  warnings: " << msg.get_num_warnings();
      return -6;
    }

    ani.hierarchy_final_init();	// finish structure initialization
    //output->check_components(); // needs filename property (moved below)

    // for all input interfaces
    for( input_type::iterator i = input.begin(); i != input.end(); i++ )
    {
      (*i)->insert_expl_ref(); // insert user references between properties
    }

    if( msg.get_num_errors() > 0 )
    {
      msg.vindent_set(0);
      msg.verbose() << "After inserting user references: giving up due to errors.";
      msg.verbose() << "  errors: " << msg.get_num_errors()
		    << "  warnings: " << msg.get_num_warnings();
      return -6;
    }

    msg.verbose() << "inserting values...";

    // for all input interfaces
    for( input_type::iterator i = input.begin(); i != input.end(); i++ )
    {
      (*i)->insert_values();	// insert concrete values for properties
    }

#warning !!! fixed filename for filled ADL output !!!
    save_filled("expl_filled.out", &ani );

    if( msg.get_num_errors() > 0 )
    {
      msg.vindent_set(0);
      msg.verbose() << "After inserting values: giving up due to errors.";
      msg.verbose() << "  errors: " << msg.get_num_errors()
		    << "  warnings: " << msg.get_num_warnings();
      return -6;
    }

    ani.finish_calculations();

#warning !!! fixed filename for filled ADL output !!!
    save_filled("filled.out", &ani );

    Output_Interface *output;

    //!!! better interface to choose the output filter in the future !!!

    // RAW output    
    output = new Raw_Output( &ani ); 
    output->init();
    output->check_components();
    output->process_results();
    delete output;

    // POV output    
    output = new Pov_Output( &ani ); 
    output->init();
    output->check_components();
    output->process_results();
    delete output;

    if( msg.get_num_errors() > 0 )
    {
      msg.vindent_set(0);
      msg.verbose() << "After processing output: giving up due to errors.";
      msg.verbose() << "  errors: " << msg.get_num_errors()
		    << "  warnings: " << msg.get_num_warnings();
      return -6;
    }

    /**/    
  }
  catch( ... )
  {
    std::cout << "Fatal Error: Exception Caught" << std::endl;
    return -1;
  }
  return 0;
}
