#ifndef _INC_PAR_CMDLine_H_
#define _INC_PAR_CMDLine_H_ 1

#include "paramarg.h"

namespace par
{

struct CommandLine
{
	_CPP_OPERATORS
	int argc;
	char **argv;
	char **envp;
	
	CommandLine(int _argc,char **_argv,char **_envp)
		{  argc=_argc;  argv=_argv;  envp=_envp;  }
	CommandLine(const CommandLine &cmd)
		{  argc=cmd.argc;  argv=cmd.argv;  envp=cmd.envp;  }
	~CommandLine()  { }
};


class CmdLineArgs : public CommandLine
{
	private:
		// Always use pointers/references. 
		CmdLineArgs(const CmdLineArgs &) : CommandLine(0,NULL,NULL) { }
		void operator=(const CmdLineArgs &)  { }
	public:  _CPP_OPERATORS_FF
		ParamArg *args;  // array of size argc
		
		// Returns program name according to argv[0]. 
		// If argc was 0, "???" is returned. 
		// NOTE: this string is allocated by 
		// ParamArg CmdLineArgs::args[0] and may no longer be 
		// valid if you destroy CmdLineArgs. 
		const char *PrgName() const  {  return(args->name);  }
		
		// Can deal with argc=0. 
		// All args are copied; check failflag for allocation 
		// failure (allocation using LMalloc()). 
		CmdLineArgs(int argc,char **argv,char **envp,
			int *failflag=NULL);
		~CmdLineArgs();
};

} // namespace end 

#endif  /* _INC_PAR_CMDLine_H_ */
