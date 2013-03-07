/*
 * parconsumerovl.h
 * 
 * Contains overloaded version of ParameterConsumer for your convenience. 
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
		// or float/double argument. 
		// See parconsumer.h for meaning of flags. 
		ParamInfo *AddParam(const char *name,const char *helptext,
			int *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_int_handler,flags));  }
		ParamInfo *AddParam(const char *name,const char *helptext,
			unsigned int *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_uint_handler,flags));  }
		ParamInfo *AddParam(const char *name,const char *helptext,
			long *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_long_handler,flags));  }
		ParamInfo *AddParam(const char *name,const char *helptext,
			unsigned long *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_ulong_handler,flags));  }
		ParamInfo *AddParam(const char *name,const char *helptext,
			float *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_float_handler,flags));  }
		ParamInfo *AddParam(const char *name,const char *helptext,
			double *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_double_handler,flags));  }
		
		// Dito for string (RefString) args: 
		ParamInfo *AddParam(const char *name,const char *helptext,
			RefString *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_string_handler,flags));  }
		
		// ...and string list (RefStrList) args: 
		ParamInfo *AddParam(const char *name,const char *helptext,
			RefStrList *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,default_stringlist_handler,flags));  }
		
		// Adds a PTSwitch: 
		ParamInfo *AddParam(const char *name,const char *helptext,
			bool *valptr,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTSwitch,
				helptext,valptr,default_switch_handler,flags));  }
		
		// Adds a PTOption: 
		ParamInfo *AddOpt(const char *name,const char *helptext,
			int *counter,int flags=0)
			{  return(ParameterConsumer::AddParam(name,PTOption,
				helptext,counter,default_option_handler,flags));  }
		
		// Adds an enum: (using an exclusive handler)
		// NOTE: *map is an array of EnumMapEntries; the last entry of this 
		//       array MUST have map[].str set to NULL. 
		typedef EnumValueHandler::MapEntry EnumMapEntry;
		ParamInfo *AddEnum(const char *name,const char *helptext,
			int *valptr,const EnumMapEntry *map,int flags=0)
		{
			EnumValueHandler *vhdl=NEW1<EnumValueHandler>(map);
			if(!vhdl)  return(NULL);
			return(ParameterConsumer::AddParam(name,PTParameter,
				helptext,valptr,vhdl,flags | PExclusiveHdl));
		}
};

}  // namespace end 

#endif  /* _INC_PAR_ParConsumerOverloaded_H_ */
