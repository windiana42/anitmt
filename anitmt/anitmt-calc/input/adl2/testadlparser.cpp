#include <iostream>
#include <message/message.hpp>

#include "adlparser.hpp"

#include <nodes.hpp>
#include <animation.hpp>
#include <error.hpp>		// for old throw exceptions
#include <save_filled.hpp>

#include <config.h>

#if(YYDEBUG)
namespace anitmt{
  namespace adlparser{
    extern int yydebug;
  }
}
#endif
int main( int argc, char *argv[] )
{
  std::string infile = "";
  std::string outfile1 = "test_filled_adl.out";
  std::string outfile2 = "final_test_filled_adl.out";

  if( (argc > 1) && (argv[1][0] != '-') ) 
    infile = argv[1];
  else
  {
    std::cout << "Usage:" << std::endl;
    std::cout << "  testadlparser <infile> [<outfile1> [<outfile2> [-d]]]" 
	      << std::endl;
    return 0;
  }

  if( argc > 2 )
    outfile1 = argv[2];

  if( argc > 3 )
    outfile2 = argv[3];

#if(YYDEBUG)
  if( argc > 4 )
    if( !strcmp(argv[4],"-d") )
      anitmt::adlparser::yydebug = 1; // output debugging information
#endif

  anitmt::make_all_nodes_available();

  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );
  message::Message_Reporter msg(&main_consultant);

  anitmt::Animation ani("test", &manager);

  try
  {
    msg.verbose() << "parsing pass 1...";
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass1 );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    msg.verbose() << "finish hierarchy...";
    ani.hierarchy_final_init();
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    msg.verbose() << "parsing pass 2...";
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass2 );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    anitmt::save_filled( outfile1, &ani );
    ani.pri_sys.invoke_all_Actions();
    anitmt::save_filled( outfile2, &ani );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
  } 
  catch( anitmt::EX e ) 
  {
    std::cerr << "Exception: " << e.get_name() << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown Exception caught" << std::endl;
  }
  return 0;
}
