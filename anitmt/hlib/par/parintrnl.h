
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

class ParameterConsumerBase;
struct ValueHandler;


struct PAR  // This serves as a namespace and a base class. 
{
	enum ParameterType
	{
		PTParameter=1,  // fps=17, -fps=17, --fps=17, -fps 17, --fps 17
		PTSwitch,       // -[-]check, -[-]no-check, -[-]check=[on|yes|true|off|no|false]
		PTOption,       // -verbose --verbose  (options get counted)
		PTOptPar,       // -[-]ssl[=/usr/local/lib/]
		
		PTTypeMask=0xff,
		
		PTNoDefault=0x100  // parameter has no default value
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
		ParameterConsumerBase *pc;  // whose param is this? 
		Section *section;     // corresponding section 
		const char *name;     // name of the parameter (without sections; NOT 
		                      // copied; containing all synonymns separated by 
		                      // `/' and indexed by synpos[]. 
		int *synpos;          // synonyme name start in name[]; last elem <0. 
		                      // array allocated at (char*)(this+1). 
		const char *helptext; // help text (not copied) or NULL. 
		ParameterType ptype;  // type (see above)
		ValueHandler *vhdl;   // associated value handler 
		int exclusive_vhdl;   // vhdl exclusively allocated for this param; free 
		                      // on destruction if param
		void *valptr;         // pointer to the original var (as specified 
		                      // by the ParameterConsumerBase-derived class) 
		int is_set;           // Value set? AddParam sets this to 1 if there 
		                      // is a default value, else to 0. 
		int locked;           // parameter locked (may not get modified?)
		
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
			ParamInfo &operator=(const ParamInfo &)  {}
	};
	
	struct Section : LinkedListBase<Section>
	{
		const char *name;   // section name (copied)
		const char *helptext;  // or NULL; not copied 
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
