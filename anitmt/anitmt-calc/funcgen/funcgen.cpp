/*****************************************************************************/
/**   functionality generation main function                          	    **/
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

#include <iostream>
#include <message/message.hpp>
#include "afdbase.hpp"
#include "gen_cpp.hpp"

namespace funcgen
{
  int parse_afd( AFD_Root *afd, message::Message_Consultant *c, 
		 std::string filename );
  extern int yydebug;
}


int main(int argn, char *argc[])
{
  funcgen::yydebug = 1;
  if( argn <= 1 )
  {
    std::cout << "usage: funcgen <filename>" << std::endl;
    return -1;
  }
  message::Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
  message::Message_Manager manager(&msg_handler);
  message::Message_Consultant default_msg_consultant(&manager, 0);

  funcgen::Cpp_Code_Generator cpp;	  // C++ Code generator
  funcgen::AFD_Root afd( cpp.get_translator() ); // afd data root
  
  std::cout << "Parsing " << argc[1] << "..." << std::endl;
  funcgen::parse_afd( &afd, &default_msg_consultant, argc[1] );
  std::cout << std::endl;

  //std::cout << "Result: " << std::endl;
  //afd.print();			// debug print data in structure

  funcgen::code_gen_info info("functionality");
  cpp.generate_code( &afd, &info ); // generate C++ Code
}
