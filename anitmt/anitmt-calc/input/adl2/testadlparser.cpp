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
  std::string outfile = "test_filled_adl.out";
  if( argc > 1 ) 
    infile = argv[1];

  if( argc > 2 )
    outfile = argv[2];

  anitmt::make_all_nodes_available();

  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );

  anitmt::Animation ani("test", &manager);

  anitmt::adlparser::adlparser_info info( &main_consultant );

  if( infile == "" )
    info.msg.verbose() << "Enter ADL code:" << std::endl;

  try
  {
    anitmt::adlparser::parse_adl( &ani, &info, infile );
    anitmt::save_filled( outfile, &ani );
  } 
  catch( anitmt::EX e ) 
  {
    info.msg.error() << "Exception: " << e.get_name();
  }
  catch(...)
  {
    info.msg.error() << "Unknown Exception caught";
  }
  return 0;
}
