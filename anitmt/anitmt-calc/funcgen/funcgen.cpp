/*****************************************************************************/
/**   functionality generation main function                          	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
/**									    **/
/** License: LGPL - free and without any warranty - read COPYING            **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include <iostream>
#include <message/message.hpp>
#include "afdbase.hpp"
#include "gen_cpp.hpp"

#include "funcgen.hpp"

#include <config.h>

namespace funcgen
{
  int parse_afd( AFD_Root *afd, message::Message_Consultant *c, 
		 std::string filename );
#ifdef YYDEBUG
  extern int yydebug;
#endif
}



int main(int argc, char *argv[], char *envp[])
{
  prg_name=GetPrgName(argv[0]);
  
  par::ParameterManager parmanager;
  parmanager.SetVersionInfo(
    VERSION,     // version string
    "aniTMT",    // package name
    "funcgen");  // program name
  // Additional help text what funcgen actually does; do NOT put any 
  // newlines in the text unless you intend to start a new paragraph. 
  parmanager.AdditionalHelpText(
    "funcgen is used to automatically create c++-code from a "
    "functionality description (.afd).\n"
    "It may be used to generate a data tree with calculation facilities."
  );
  
  FuncgenParameters fgpar(&parmanager);
  
  par::CmdLineArgs cmdline(argc,argv,envp);
  par::ParameterSource_CmdLine cmd_src(&parmanager);
  
  if(cmd_src.ReadCmdLine(&cmdline))
  {  exit(1);  }
  
  // Check if info query options were passed; exit in this case: 
  if(cmd_src.n_iquery_opts)
  {  exit(0);  }
  
  int rv=cmd_src.WriteParams();
  if(rv)
  {
    std::cerr << "Error storing command line parameters (" << rv << ")" <<
      std::endl;
    exit(1);
  }
  
  if(parmanager.CheckParams())
  {  exit(1);  }
  
  // Okay, here we go...
#ifdef YYDEBUG
  funcgen::yydebug=fgpar.yydebug;
#endif
  
  #if 0
  std::cout << "Infile: " << fgpar.in_file << std::endl;
  std::cout << "Outfile: " << fgpar.out_basename << std::endl;
  std::cout << "Namespace: " << fgpar.namesp << std::endl;
  #endif
  
  std::string in_file(fgpar.in_file);
  std::string out_basename(fgpar.out_basename);
  std::string namesp(fgpar.namesp); // namespace
  
  
  message::Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
  message::Message_Manager manager(&msg_handler);
  message::Message_Consultant default_msg_consultant(&manager, 0);

  // code generation information
  funcgen::code_gen_info info( namesp, out_basename, 
			      &default_msg_consultant);

  for(const RefStrList::Node *i=fgpar.include_path.first(); i; i=i->next)
  {
    //std::cout << "-I" << i->str() << std::endl;
    info.add_include_path( std::string(i->str()) );
  }
  
  funcgen::Cpp_Code_Generator cpp(&info); // C++ Code generator
  funcgen::AFD_Root afd( cpp.get_translator() ); // afd data root
  
  std::cout << "Parsing \"" << in_file << "\" ..." << std::endl;
  funcgen::parse_afd( &afd, &default_msg_consultant, in_file );
  //std::cout << std::endl;

  if( fgpar.stdebug )
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

  std::cout << std::endl;
  int failed=manager.get_num_messages( message::MT_Error )>0;
  if(!failed)
  {  std::cout << "Code generation (" << out_basename << 
      ") successful." << std::endl;  }

  // Proper return code: 1 -> error; 0 -> success
  return(failed);
}

