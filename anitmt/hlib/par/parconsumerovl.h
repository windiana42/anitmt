#ifndef _INC_PAR_ParConsumerOverloaded_H_
#define _INC_PAR_ParConsumerOverloaded_H_ 1

#include "parconsumerbase.h"

namespace par
{

class ParameterConsumer_Overloaded : public ParameterConsumerBase
{
	public:  _CPP_OPERATORS_FF
		ParameterConsumer_Overloaded(int *failflag=NULL) : 
			ParameterConsumerBase(failflag)  { }
		virtual ~ParameterConsumer_Overloaded()  { }
		
		// Adds a PTParameter taking an (unsigned) int/long, 
		// or double argument. 
		ParamInfo *AddParam(const char *name,const char *helptext,int *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTParameter,
				helptext,valptr,internal::int_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,unsigned int *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTParameter,
				helptext,valptr,internal::uint_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,long *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTParameter,
				helptext,valptr,internal::long_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,unsigned long *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTParameter,
				helptext,valptr,internal::ulong_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,double *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTParameter,
				helptext,valptr,internal::double_handler));  }
		
		// Adds a PTSwitch: 
		ParamInfo *AddParam(const char *name,const char *helptext,bool *valptr)
			{  return(ParameterConsumerBase::AddParam(name,PTSwitch,
				helptext,valptr,internal::switch_handler));  }
		
		// Adds a PTOption: 
		ParamInfo *AddOpt(const char *name,const char *helptext,int *counter)
			{  return(ParameterConsumerBase::AddParam(name,PTOption,
				helptext,counter,internal::option_handler));  }
};

}  // namespace end 

#endif  /* _INC_PAR_ParConsumerOverloaded_H_ */
