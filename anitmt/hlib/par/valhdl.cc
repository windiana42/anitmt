#include "parintrnl.h"
#include <string.h>

namespace par  {
namespace internal  {

// Allocation wrappers for ValueHandler::print(): 
char *ValueHandler::stralloc(size_t len)
{  return((char*)(len ? LMalloc(len) : NULL));  }
void ValueHandler::strfree(char *str)
{  LFree(str);  }

// Default ValueHandler implementations: 

void *ValueHandler::alloc(ParamInfo *)
{
	size_t sz=cpsize();
	return(sz ? ((char*)LMalloc(sz)) : NULL);
}

void ValueHandler::free(ParamInfo *,void *val)
{
	LFree(val);
}

int ValueHandler::copy(ParamInfo *,void *dest,void *src,int operation)
{
	if(operation!=SOPCopy)  return(-2);  // not supported 
	size_t sz=cpsize();
	if(!sz)  return(1);
	memcpy(dest,src,sz);
	return(0);
}

size_t ValueHandler::cpsize()
{
	return(0);
}

} }   // end of namespaces par and internal
