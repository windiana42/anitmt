// Console error handlers for ParameterSource_File. 

#include "srcfile.h"

#include <stdio.h>
#include <errno.h>

namespace par
{

extern const char *_three_qm;   // "???"


int ParameterSource_File::FailedFileOp(const char *path,int line,int op)
{
	if(op==0)
	{  fprintf(stderr,"%s: failed to open \"%s\": %s\n",
		prg_name,path,strerror(errno));  }
	else
	{
		fprintf(stderr,"%s: while reading in %s:%d: %s\n",
		prg_name,path,line,
			(op==2) ? strerror(errno) : 
			/* op=-1,1,3: */ "memory allocation failed");
	}
	return(0);
}


#define pp_size 11
static const struct pp_info
{
	ParameterSource_File::PreprocessorErrcode pp;
	const char *str;
} _pp_info[pp_size]=
{
	{ ParameterSource_File::PreprocessorErrcode(0),        "success" },
	// errors: 
	{ ParameterSource_File::PPArgOmitted,                  "required argument omitted" },
	{ ParameterSource_File::PPIncludeNestedTooOften,       "include nested too deeply" },
	{ ParameterSource_File::PPUnknownSection,              "unknown section" },
	{ ParameterSource_File::PPTooManyEndStatements,        "too many #end statements" },
	{ ParameterSource_File::PPUnterminatedString,          "unterminated string" },
	// warnings: 
	{ ParameterSource_File::PPWarningIgnoringInclude,      "ignoring #incluse directive" },
	{ ParameterSource_File::PPWarningIgnoringGarbageAtEol, "ignoring garbage at end of line" }
};

int ParameterSource_File::PreprocessorError(
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
	
	fprintf(stderr,"%s: in %s%s%s: %s%s%s%s\n",
		prg_name,pos->OriginStr().str(),
		sect->name ? ", section " : "",
		sect->name ? sect->name : "",
		(ppe<0) ? "warning: " : "",
		err_str,
		arg ? " - " : "",arg ? arg : "");
	
	return(0);
}

}  // namespace end 
