// Default (console) implementation of ParameterConsumer's 
// (virtual) error/warning handlers. 

#include "parmanager.h"
#include "parconsumer.h"

namespace par
{

void ParameterConsumer::CheckParamError(ParamInfo *pi)
{
	// Okay, if that gets called, we must complain. 
	if(pi->spectype==STNone)  return;   // never happens. 
	
	// Get full parameter name: 
	size_t len=parmanager()->FullParamName(pi,NULL,0);
	char tmp[len+1];
	parmanager()->FullParamName(pi,tmp,len+1);
	
	if(pi->spectype==STAtLeastOnce ||
	   (pi->spectype==STExactlyOnce && !pi->nspec))
	{  fprintf(stderr,"%s: required %s `%s´ not specified.\n",
		prg_name,ParamTypeString(pi->ptype),tmp);  }
	else if(pi->spectype==STExactlyOnce)
	{  fprintf(stderr,"%s: required %s `%s´ must be specified exactly once.\n",
		prg_name,ParamTypeString(pi->ptype),tmp);  }
	else if(pi->spectype==STMaxOnce)
	{  fprintf(stderr,"%s: optional %s `%s´ may not be specified more than "
		"once.\n",prg_name,ParamTypeString(pi->ptype),tmp);  }
	else
	{  fprintf(stderr,"%s: %s `%s´: illegal spectype %d\n",
		prg_name,ParamTypeString(pi->ptype),tmp,pi->spectype);  abort();  }
}

}  // end of namespace par
