#ifndef _INC_PAR_ParConsumerOverloaded_H_
#define _INC_PAR_ParConsumerOverloaded_H_ 1

#include "parconsumer.h"

namespace par
{

class ParameterConsumer_Overloaded : public ParameterConsumer
{
	public:  _CPP_OPERATORS_FF
		ParameterConsumer_Overloaded(ParameterManager *_manager,int *failflag=NULL) : 
			ParameterConsumer(_manager,failflag)  { }
		virtual ~ParameterConsumer_Overloaded()  { }
		
		// Adds a PTParameter taking an (unsigned) int/long, 
		// or double argument. 
		ParamInfo *AddParam(const char *name,const char *helptext,int *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_int_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,unsigned int *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_uint_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,long *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_long_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,unsigned long *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_ulong_handler));  }
		ParamInfo *AddParam(const char *name,const char *helptext,double *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_double_handler));  }
		
		// Dito for string (RefString) args: 
		ParamInfo *AddParam(const char *name,const char *helptext,RefString *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_string_handler));  }
		
		// ...and string list (RefStrList) args: 
		ParamInfo *AddParam(const char *name,const char *helptext,RefStrList *valptr)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_stringlist_handler));  }
		
		// Adds a PTSwitch: 
		ParamInfo *AddParam(const char *name,const char *helptext,bool *valptr)
			{  return(ParameterConsumer::AddParam(name,PTSwitch,
				helptext,valptr,default_switch_handler));  }
		
		// Adds a PTOption: 
		ParamInfo *AddOpt(const char *name,const char *helptext,int *counter)
			{  return(ParameterConsumer::AddParam(name,PTOption,
				helptext,counter,default_option_handler));  }
		
		// Adds an enum: (using an exclusive handler)
		// NOTE: *map is an array of EnumMapEntries; the last entry of this 
		//       array MUST have map[].str set to NULL. 
		typedef EnumValueHandler::MapEntry EnumMapEntry;
		ParamInfo *AddEnum(const char *name,const char *helptext,int *valptr,
			const EnumMapEntry *map)
		{
			EnumValueHandler *vhdl=NEW1<EnumValueHandler>(map);
			if(!vhdl)  return(NULL);
			return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,vhdl,/*exclusive_hdl=*/1));
		}
};

}  // namespace end 

#endif  /* _INC_PAR_ParConsumerOverloaded_H_ */
