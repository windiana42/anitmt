// Console error handlers for ParameterSource_File. 

#include "srcfile.h"

#include <stdio.h>
#include <errno.h>

namespace par
{

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


int ParameterSource_File::PreprocessorError(
	PreprocessorErrcode ppe,
	const ParamArg::Origin *pos,
	const Section *sect,
	const char *arg)
{
	#warning implement me!
	fprintf(stderr,"Preprocessor error %d (%s:%d) %s %s\n",
		ppe,pos->origin.str(),pos->opos,sect->name,arg);
	
	return(0);
}

}  // namespace end 
