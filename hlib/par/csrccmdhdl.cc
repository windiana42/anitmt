/*
 * csrccmdhdl.cc
 * 
 * Implementation of console error handlers used by ParameterSource_CmdLine. 
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

#define HLIB_IN_HLIB 1
#include "srccmd.h"

namespace par
{

static const char *_three_qm="???";


#define pp_size 3
static const struct pp_info
{
	ParameterSource_CmdLine::PreprocessorErrcode pp;
	const char *str;
} _pp_info[pp_size]=
{
	{ ParameterSource_CmdLine::PreprocessorErrcode(0),        "success" },
	// errors: 
	{ ParameterSource_CmdLine::PPUnknownSection,              "unknown section" },
	{ ParameterSource_CmdLine::PPTooManyEndStatements,        "too many \"-}\" statements" }
};

int ParameterSource_CmdLine::PreprocessorError(
	PreprocessorErrcode ppe,
	const ParamArg::Origin *pos,
	const Section *sect,
	const char *arg)
{
	const char *err_str=_three_qm;
	
	// Lookup error string: 
	for(int i=0; i<pp_size; i++)
	{
		if(_pp_info[i].pp==ppe)
		{  err_str=_pp_info[i].str;  break;  }
	}
	
	size_t slen=manager->FullSectionName(sect,NULL,0);
	char sname[slen+1];
	manager->FullSectionName(sect,sname,slen+1);
	
	(parmanager()->*(ppe<0 ? &ParameterManager::cwarn_printf : &ParameterManager::cerr_printf))(
		"%s: in %s%s%s%s: %s%s%s%s\n",
		prg_name,pos->OriginStr().str(),
		slen ? ", section `" : "",
		slen ? sname : "",
		slen ? "´" : "",
		(ppe<0) ? "warning: " : "",
		err_str,
		arg ? " - " : "",arg ? arg : "");
	
	return(0);
}

}  // namespace end 
