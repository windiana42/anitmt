#include "parsource.h"
#include "cmdline.h"

#ifndef _INC_PAR_ParSource_CmdLine_H_
#define _INC_PAR_ParSource_CmdLine_H_

namespace par
{

class ParameterSource_CmdLine : public ParameterSource
{
	public:  _CPP_OPERATORS_FF
		ParameterSource_CmdLine(ParameterManager *manager,int *failflag=NULL);
		~ParameterSource_CmdLine();
		
		// This counts the number of special info query options 
		// like --help and --version. If this is non-zero after 
		// parsing, you should exit the program. 
		// (Incremented by Parse()/ReadCmdLine(); never reset to 0.) 
		int n_iquery_opts;
		
		// Reads in the command line. 
		// topsect is the topmost section and defaults to 
		// the root section (when passed NULL). 
		// Returns 0 on success; else errors. 
		int ReadCmdLine(CmdLineArgs *cmd,Section *topsect=NULL);
		
		// Parses in the specified ParamArg. 
		// This function gets called by ReadCmdLine(). 
		// Return value:
		//   0 -> success
		//  -1 -> error 
		//   1 -> pa->pdone set (nothing done)
		//   2 -> this was a special arg (--version, --help); 
		//        exit after processing cmd line. 
		int Parse(ParamArg *pa,Section *topsect=NULL);
};

}  // namespace end 

#endif  /* _INC_PAR_ParSource_CmdLine_H_ */
