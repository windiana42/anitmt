#include <params.hpp>

using namespace anitmt;

int main(int argc,char **argv,char **envp)
{
	Command_Line cmd(argc,argv,envp);
	
	// Central class holding all parameters: 
	Animation_Parameters anipar;
	if(!anipar.Parse_Command_Line(&cmd))
	{  return(1);  }
	
	if(!cmd.Check_Unused())
	{  return(1);  }
	
	// We dump the parameters now (this is a test proggy)
	anipar.Print_Parameters(cerr,0,true);
	
	return(0);
}
