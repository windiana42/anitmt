/*
 * srcenv.h
 * 
 * Header file containing parameter source capable of reading in the 
 * parameters/arguments passed on the environment. 
 * 
 * Copyright (c) 2003 -- 2004 by Wolfgang Wieser (wwieser@gmx.de) 
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

#ifndef _INC_PAR_ParSource_Environ_H_
#define _INC_PAR_ParSource_Environ_H_

namespace par
{

class ParameterSource_Environ : public ParameterSource
{
	public:  _CPP_OPERATORS_FF
		ParameterSource_Environ(ParameterManager *manager,int *failflag=NULL);
		~ParameterSource_Environ();
		
		// Reads in the environment. 
		// topsect is the topmost section and defaults to 
		// the root section (when passed NULL). 
		// Returns 0 on success; else errors. 
		int ReadEnviron(CmdLineArgs *cmd,Section *topsect=NULL)
			{  return(ReadEnviron(cmd->envp,topsect));  }
		int ReadEnviron(char **envp,Section *topsect=NULL);
		
		// Parses in the specified ParamArg. 
		// This function gets called by ReadEnviron(). 
		// Return value:
		//   0 -> success
		//  -1 -> error 
		//   1 -> pa->pdone set (nothing done)
		//   2 -> okay; parsed using CopyAndParseParam()
		//  -4 -> CopyAndParseParam() returned NULL, 
		//  10 -> we do not recognize this env var. 
		// See also: 
		int Parse(ParamArg *pa,Section *topsect=NULL);
};

}  // namespace end 

#endif  /* _INC_PAR_ParSource_Environ_H_ */
