/*****************************************************************************/
/**   functionality generation main function                          	    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann, Wolfgang Wieser				    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de, wwieser@gmx.de			    **/
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

#include <hlib/parbase.h>


namespace funcgen
{
  int parse_afd( AFD_Root *afd, message::Message_Consultant *c, 
		 std::string filename );
  extern int yydebug;
}



class FuncgenParameters : par::ParameterConsumer_Overloaded
{
  public:
    FuncgenParameters(par::ParameterManager *manager);
    ~FuncgenParameters();
    
    // Overriding virtual: 
    int CheckParams();
    
    int yydebug;   // parser debugging info
    int stdebug;   // structure debugging info
    RefString in_file;
    RefString out_basename;
    RefString namesp;
    RefStrList include_path;
};

FuncgenParameters::FuncgenParameters(par::ParameterManager *manager) : 
    par::ParameterConsumer_Overloaded(manager)
{
  // Set up defaults for the parameters: 
  yydebug=0;
  stdebug=0;
  
  // Then, register the parameters: 
  SetSection(NULL);
  AddOpt("p/yydebug","enable parser debugging output (LOTS of text)",&yydebug);
  AddOpt("s/stdebug","dump structure debug output",&stdebug);
  
  AddParam("i/input","input file",&in_file);
  AddParam("o/output","output basename",&out_basename);
  AddParam("n/namespace","namespace",&namesp);
  AddParam("I","include path (use -I+=path to add a path)",&include_path);
}

int FuncgenParameters::CheckParams()
{
  int missing=0;
  if(!in_file.str())
  {
    std::cerr << "No input file specified. Try " << prg_name << 
    	" --help" << std::endl;
    return(1);
  }
  if(!namesp.str())
  {
    std::cerr << "You did not specify the namespace." << std::endl;
    ++missing;
  }
  if(!out_basename.str() && in_file.str())
  {
    const char *in=in_file.str();
    if(strcmp(in+in_file.len()-5,".afd"))
    {
      std::string tmp(in_file,in_file.len()-4);
      out_basename.set(tmp.c_str());  // copies the c_str. 
    }
    else
    {
      std::cerr << "You did not specify the output file." << std::endl;
      ++missing;
    }
  }
  return(missing);
}

FuncgenParameters::~FuncgenParameters()
{
  // nothing to do...
}



// Holds the name of the program: 
char *prg_name=NULL;

int main(int argc, char *argv[], char *envp[])
{
  prg_name=GetPrgName(argv[0]);
  
  par::ParameterManager parmanager;
  parmanager.SetVersionInfo(
    VERSION,     // version string
    "aniTMT",    // package name
    "funcgen");  // program name
  // Additional help text what funcgen actually does; do NOT put any 
  // newlines in the text. 
  #warning Martin, please fix me (easy)
  parmanager.AdditionalHelpText(
    "funcgen is used to atomatically create ... "
    "Martin, please add some lines about what funcgen actually does");
  
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
  funcgen::yydebug=fgpar.yydebug;
  
  std::string in_file(fgpar.in_file);
  std::string out_basename(fgpar.out_basename);
  std::string namesp(fgpar.namesp); // namespace
  
  #if 0
  std::cout << "Infile: " << in_file << std::endl;
  std::cout << "Outfile: " << out_basename << std::endl;
  std::cout << "Namespace: " << namesp << std::endl;
  #endif
  
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

