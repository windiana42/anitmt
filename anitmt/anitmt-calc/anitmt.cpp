#include "proptree.hpp"
#include "animation.hpp"
#include "nodes.hpp"
#include "save_filled.hpp"

#include <adlparser.hpp>
#include <params.hpp>

#include <fstream>

using namespace anitmt;

int main(int argc,char **argv,char **envp)
{
  // commandline handler of libpar (params.hpp)
  Command_Line cmd(argc,argv,envp);

  Animation ani("noname");
  if(!ani.param.Parse_Command_Line(&cmd) )
    return -1;
  
  if(!cmd.Check_Unused() )
    return -2;

  // init animation tree
  make_all_nodes_available();

  stringlist adlfiles = ani.param.adl();
  if( adlfiles.is_empty() )
    {
      cerr << "Error: no animation descriptions specified" << endl;
      return -3;
    }

  adlfiles.rewind();
  do
  { 
    ifstream file( adlfiles.current().c_str() );
    ADLParser p( file, cout, true);
    p.ParseTree( &ani );
  } while( adlfiles.next() );

  ani.pri_sys.invoke_all_Actions();
  save_filled("raw.out", &ani);

  return 0;
}
