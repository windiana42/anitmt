#include <iostream>
#include <message/message.hpp>

#include "adlparser.hpp"

#include <nodes.hpp>
#include <animation.hpp>
#include <error.hpp>		// for old throw exceptions
#include <save_filled.hpp>

int main( int argc, char *argv[] )
{
  std::string infile = "";
  std::string outfile1 = "test_filled_adl.out";
  std::string outfile2 = "final_test_filled_adl.out";

  if( argc > 1 ) 
    infile = argv[1];
  else
  {
    std::cout << "Usage:" << std::endl;
    std::cout << "  testadlparser <infile>" << std::endl;
    return 0;
  }

  if( argc > 2 )
    outfile1 = argv[2];

  if( argc > 3 )
    outfile2 = argv[2];

  anitmt::make_all_nodes_available();

  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );

  anitmt::Animation ani("test", &manager);


  try
  {
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass1 );
    ani.hierarchy_final_init();
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass2 );
    anitmt::save_filled( outfile1, &ani );
    ani.pri_sys.invoke_all_Actions();
    anitmt::save_filled( outfile2, &ani );
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
