/*
 * srccmd.h
 * 
 * Header file containing parameter source capable of reading in the 
 * parameters/arguments passed on the command line. 
 * 
 * Copyright (c) 2001 -- 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "parsource.h"
#include "cmdline.h"

#ifndef _INC_PAR_ParSource_CmdLine_H_
#define _INC_PAR_ParSource_CmdLine_H_

namespace par
{

class ParameterSource_CmdLine : public ParameterSource
{
	public:
		enum PreprocessorErrcode
		{
			// errors (>0)
			PPUnknownSection=1,       // "-blah-{" -> section `blah' unknown
			PPTooManyEndStatements    // "-}" occured too often
		};
		
	protected:
		// (Called by the parse function.)
		// This notifies you of cmd line preprocessor errors. 
		// ppe -> tells you which error/warning was encountered; 
		//        NOTE: ppe>0 -> errors; ppe<0 -> warnings
		// pos -> current position; do not free pos->origin but copy 
		//        it if you need it lateron. 
		// sect -> current section pointer (all arg names in files are 
		//        relative to this section). 
		// arg -> this is normally NULL. 
		//        For PPUnknownSection this contains the secion name 
		//          which was not found. 
		// Return value: ignored; return 0. 
		virtual int PreprocessorError(
			PreprocessorErrcode ppe,
			const ParamArg::Origin *pos,
			const Section *sect,
			const char *arg);
		
		// [overriding a virtual:]
		virtual ParamInfo *CheckLookupIsOkay(ParamArg *arg,ParamInfo *pi);
		
	public:  _CPP_OPERATORS_FF
		ParameterSource_CmdLine(ParameterManager *manager,int *failflag=NULL);
		virtual ~ParameterSource_CmdLine();
		
		// This counts the number of special info query options 
		// like --help and --version. If this is non-zero after 
		// parsing, you should exit the program. 
		// (Incremented by Parse()/ReadCmdLine(); never reset to 0.) 
		int n_iquery_opts;
		
		// Reads in the command line. 
		// topsect is the topmost section and defaults to 
		// the root section (when passed NULL). 
		// Returns 0 on success; else errors. 
		int ReadCmdLine(CmdLineArgs *cmd,Section *topsect=NULL)
			{  return(ReadCmdLine(&cmd->args[1],cmd->argc-1,topsect));  }
		// NOTE: pa_array[0] is also parsed (NOT program name). 
		int ReadCmdLine(ParamArg *pa_array,int argc,Section *topsect=NULL);
		
		// Parses in the specified ParamArg. 
		// This function gets called by ReadCmdLine(). 
		// Return value:
		//   0 -> success
		//  -1 -> error 
		//   1 -> pa->pdone set (nothing done)
		//  10 -> this was a special arg (--version, --help); 
		//        exit after processing cmd line. 
		// See also: FindCopyParseParam(). 
		int Parse(ParamArg *pa,Section *topsect=NULL);
};

}  // namespace end 

#endif  /* _INC_PAR_ParSource_CmdLine_H_ */
