/*
 * parintrnl.h
 * 
 * Paramter system interal / shared data types. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


// ### HLIB notes: 
// ### dont use ValueHandlerT (op new...)

#ifndef _INC_PAR_ParInternal_H_
#define _INC_PAR_ParInternal_H_ 1

#include <hlib/prototypes.h>
#include <hlib/linkedlist.h>

#include "cmdline.h"

#include <stddef.h>

namespace par
{

class ParameterConsumer;
struct ValueHandler;
class SectionParameterHandler;


struct PAR  // This serves as a namespace and a base class. 
{
	enum ParameterType
	{
		PTParameter=1,  // fps=17, -fps=17, --fps=17, -fps 17, --fps 17
		PTSwitch,       // -[-]check, -[-]no-check, -[-]check=[on|yes|true|off|no|false]
		PTOption,       // -verbose --verbose  (options get counted)
		PTOptPar        // -[-]ssl[=/usr/local/lib/]
	};
	
	enum
	{
		PNoDefault=   0x010,  // parameter has no default value
		PExclusiveHdl=0x020   // handler exclusively for that parameter
	};
	
	enum PSpecType
	{
		STNone=       0x000,  // (parameter system does not check that)
		STAtLeastOnce=0x100,  // param must be specified at least once
		STExactlyOnce=0x200,  // param must be specified exactly once
		STMaxOnce=    0x300,  // param may not be specified more than once
		_STMask=      0x300
	};
	
	// Returns static data: 
	static const char *ParamTypeString(ParameterType ptype);
	
	enum ParParseState
	{
		PPSGarbageEnd=-32000,  // garbage at the end of the arg
		PPSSuccess=0,   // success; wanrnings <0; errors >0
		PPSValOmitted,   // required value omitted 
		PPSIllegalArg,   // argument badly formatted 
		PPSValOORange,   // value out of range 
		PPSArgTooLong,   // argument too long
		PPSArgUnterminated,  // argument (e.g. string) not terminated
		PPSOptAssign,    // assignment of an (counted) _option_ (-opt=value) 
		PPSIllegalAssMode, // specified ParamArg::assmode not supported 
		PPSAssFailed,    // assignment of value failed 
		PPSUnknown       // arg name unknown; only used internally 
	};

	enum ParParseErrval
	{
		PPENameOmitted,  // sth like `=value'
		PPEUnknownArg    // arg->name unknown 
	};
	
	struct Section;
	
	struct ParamInfo : LinkedListBase<ParamInfo>
	{
		ParameterConsumer *pc;  // whose param is this? 
		Section *section;     // corresponding section 
		const char *name;     // name of the parameter (without sections; NOT 
		                      // copied; containing all synonymns separated by 
		                      // `|' and indexed by synpos[]. 
		int *synpos;          // synonyme name start in name[]; last elem <0. 
		                      // array allocated at (char*)(this+1). 
		const char *helptext; // help text (not copied) or NULL. 
		ParameterType ptype;  // type (see above)
		ValueHandler *vhdl;   // associated value handler 
		int exclusive_vhdl;   // vhdl exclusively allocated for this param; free 
		                      // on destruction if param
		void *valptr;         // pointer to the original var (as specified 
		                      // by the ParameterConsume-derived class) 
		int is_set;           // Value set? AddParam sets this to 1 if there 
		                      // is a default value, else to 0. Incremented each 
		                      // time WriteParam() gets called. 
		PSpecType spectype;   // see above
		int locked;           // parameter locked (may not get modified?)
		int nspec;            // how often the parameter was encountered, 
		                      // summed up over all sources (default does not count)
		
		_CPP_OPERATORS_FF
		
		// Use this to allocate a ParamInfo: 
		// Returns NULL on failure; already reads the passed name 
		// (not copied) and sets up synpos[] table. 
		static ParamInfo *NewPi(const char *name);
		// Use this to free the ParamInfo: 
		static void DelPi(ParamInfo *pi)
			{  if(pi) delete pi;  /* implicitly frees synpos[] */  }
		
		// Compare the passed name of size len with all synonymes 
		// in this->name and return 0 if at least one matches, 
		// else 1. 
		int NameCmp(const char *pname,size_t len);
		
		ParamInfo(int *failflag=NULL);
		~ParamInfo();
		
		private:
			// Never use one of these (assignment & copy constructor): 
			ParamInfo(const ParamInfo &) : LinkedListBase<ParamInfo>() {}
			void operator=(const ParamInfo &)  {}
	};
	
	struct Section : LinkedListBase<Section>
	{
		const char *name;   // section name (copied)
		const char *helptext;  // or NULL; not copied 
		SectionParameterHandler *sect_hdl;  // attached section handler or NULL
		Section *up;   // NULL for topmost section
		
		LinkedList<Section> sub;  // subsections
		LinkedList<ParamInfo> pilist;  // params of this section 
		
		_CPP_OPERATORS_FF
		Section(const char *name,ssize_t namelen=-1,int *failflag=NULL);
		~Section();
	};
	
	struct ParamCopy : LinkedListBase<ParamCopy>
	{
		ParamInfo *info;
		void *copyval;    // copy of the value
		int nspec;    // how often the parameter was encountered in source
		              // (gets summed up when overridden)
		
		// [previous] location of the arg: 
		ParamArg::Origin porigin;
		
		_CPP_OPERATORS_FF
		ParamCopy(int *failflag=NULL);
		~ParamCopy() { }
	};
	
	struct _AddParInfo  // used only indernally to pass params 
	{
		Section *section;
		const char *name;
		const char *helptext;
		ParameterType ptype;
		void *valptr;
		ValueHandler *hdl;
		int exclusive_hdl;  // & allocated via new & must be deleted
		bool has_default;
		PSpecType spectype;
	};
	
	struct SPHInfo  // SectionParameterHandler (parse()) info
	{
		ParamArg *arg;   // the parameter argument we're talking about
		Section *sect;   // section we're currently in (i.e. bot_sect
		                 // or hierarchy above)
		const char *nend;  // where the matched name ends, i.e. where 
		                   // the end of sect is in arg->name 
		Section *bot_sect;  // botton section till where the param 
		                    // could be matched 
		const char *bot_nend; // till where the name could be matched 
		                      // (which lead to end_sect).
		Section *top_sect;  // section which to view top_name relative to
		const char *top_name;  // name making trouble; = arg->name
		const char *name_end;  // pointer to the end of the string in 
		                       // nend/bot_nend/top_name 
	};
	
	struct SpecialHelpItem : LinkedListBase<SpecialHelpItem>
	{
		const char *optname;  // name of the option (e.g. [--]list-oformats)
		const char *descr;    // short description (e.g. "lists available 
		                      // output formats")
		const char *helptxt;  // help text or NULL if the virtual function 
		                      // ParameterConsumer::PrintSpecialHelp() shall 
		                      // be called
		int item_id;          // arbitrary value assigned by parameter 
		                      // consumer for easy recognizion. 
		
		_CPP_OPERATORS_FF
		SpecialHelpItem(int *failflag=NULL);
		~SpecialHelpItem() { }
	};
	
	enum SpecialOp
	{
		SOPCopy=0,  // simply copy values (dest=src)
		SOPAdd,     // add/append values  (dest+=src)
		SOPSub      // subtract values    (dest-=src)
	};
};

}  // namespace end 

#endif  /* _INC_PAR_ParInternal_H_ */
