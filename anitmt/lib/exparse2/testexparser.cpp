#include <iostream>
#include <message/message.hpp>

#include "exparser.hpp"

int main()
{
  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );

  exparser::parser_info info( &main_consultant );

  info.msg.verbose() << "Enter scalar expression:" << std::endl;
  solve::Operand<values::Scalar> &res1=exparser::get_scalar_expression( info );
  info.msg.verbose() << "result = " << res1() << std::endl;

  info.msg.verbose() << "Enter scalar expression with identifiers:" 
		     << std::endl;
  solve::Operand<values::Scalar> &res2=exparser::get_scalar_expression( info );
  info.msg.verbose() << "result = " << res2() << std::endl;
  return 0;
}
