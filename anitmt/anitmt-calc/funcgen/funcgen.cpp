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
  if( argn <= 4 )
  {
    std::cout << "usage: funcgen <in_filename> <out_basename> <namespace> "
	      << "<include_path> [debug] [verbose] " << std::endl;
    return -1;
  }
  if( argn >= 6 )
  {
    funcgen::yydebug = 1;
  }

  std::string in_file  = argc[1];
  std::string out_basename = argc[2];
  std::string namesp  = argc[3]; // namespace
  std::string include_path = argc[4]; // path where included files are 

  message::Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
  message::Message_Manager manager(&msg_handler);
  message::Message_Consultant default_msg_consultant(&manager, 0);

  // code generation information
  funcgen::code_gen_info info( namesp, out_basename, 
			      &default_msg_consultant);
  info.add_include_path( include_path );

  funcgen::Cpp_Code_Generator cpp(&info); // C++ Code generator
  funcgen::AFD_Root afd( cpp.get_translator() ); // afd data root
  
  std::cout << "Parsing... " << in_file << "..." << std::endl;
  funcgen::parse_afd( &afd, &default_msg_consultant, in_file );
  std::cout << std::endl;

  if( argn >= 7 )
  {
    std::cout << "Result: " << std::endl;
    afd.print();			// debug print data in structure
  }
  std::cout << std::endl;

  std::cout << manager.get_num_messages( message::MT_Error )
	    << " Errors" << std::endl;
  std::cout << manager.get_num_messages( message::MT_Warning )
	    << " Warnings" << std::endl;
  std::cout << std::endl;

  std::cout << "Generating Code... " << std::endl;
  cpp.generate_code( &afd ); // generate C++ Code

  std::cout << manager.get_num_messages( message::MT_Error )
	    << " Errors" << std::endl;
  std::cout << manager.get_num_messages( message::MT_Warning )
	    << " Warnings" << std::endl;

}
