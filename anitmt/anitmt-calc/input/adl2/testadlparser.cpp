#include <iostream>
#include <message/message.hpp>

#include "adlparser.hpp"

#include <nodes.hpp>
#include <animation.hpp>
#include <error.hpp>		// for old throw exceptions
#include <save_filled.hpp>

int main()
{
  anitmt::make_all_nodes_available();

  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );

  anitmt::Animation ani("test", &manager);

  anitmt::adlparser::adlparser_info info( &main_consultant );

  info.msg.verbose() << "Enter ADL code:" << std::endl;

  try
  {
    anitmt::adlparser::parse_adl( &ani, &info );
    anitmt::save_filled( "test_filled_adl.out", &ani );
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
