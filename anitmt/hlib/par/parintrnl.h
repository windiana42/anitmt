
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
		PPSOptAssign,    // assignment of an (counted) _option_ (-opt=value) 
		PPSIllegalAssMode, // specified ParamArg::assmode not supported 
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
		bool has_default;
	};
	
	enum SpecialOp
	{
		SOPCopy=0,  // simply copy values (dest=src)
		SOPAdd,     // add/append values  (dest+=src)
		SOPSub      // subtract values    (dest-=src)
	};
};


// Value handler (pure C++ fans should have a look at 
// the ValueHandlerT template.)
// One value handler is needed for each different value type. 
// So, you will normally need only one ValueHandler for double, 
// one for unsigned long, etc. 
struct ValueHandler : public PAR
{  _CPP_OPERATORS_FF
	ValueHandler(int * /*failflag*/=NULL) { }
	virtual ~ValueHandler() { }
	
	// Allocate and construct a new value returning NULL 
	// if allocation or construction failed: 
	// Default implementation: LMalloc(cpsize()). 
	virtual void *alloc(ParamInfo *);
	
	// Just the opposite of alloc() above; used to properly 
	// destroy and free the passed value: 
	// Default implementation: LFree(). 
	virtual void free(ParamInfo *,void * /*val*/);
	
	// dest and src both point to variables; the function must 
	// modify dest with respect to src and operation. 
	// Operation may have different values, the most basic one is 
	// * SOPCopy which means ``Copy src to dest'' 
	//   (i.e. *(T*)dest=*(T*)src). 
	//   This operation MUST be implemented. 
	//   The copy routine should overwrite the value in dest. 
	// * Other operations may be implemented. 
	// Return value: 
	//   0 -> OK; !=0 -> failed. 
	//  -1 -> allocation failure 
	//  -2 -> operation (!=PAR::SOPCopy) not supported
	// else -> something else failed (your implementation of 
	//         ParameterSource::ParamCopyError() should be 
	//         able to deal with it; use negative values). 
	// Default implementation: 
	//    if(operation!=PAR::SOPCopy)  return(-2);
	//    if(!cpsize())  return(1)  
	//    memcpy(dest,src,cpsize());  return(0). 
	virtual int copy(ParamInfo *,void * /*dest*/,void * /*src*/,
		int /*operation*/ = PAR::SOPCopy);
	
	// In case this handler handles simple values like int/double/etc 
	// which can be savely copied via memcpy(), this returns the size 
	// of the object. If the handler handles values which have to be 
	// constructed instead of copied, cpsize() must return 0 so that 
	// alloc()/copy()/free() (see above) must be used. 
	// Default implementation: return(0). 
	virtual size_t cpsize();
	
	// Parse in the passed argument and store it in *val. 
	// This function has to parse in the argument passed as 
	// ParamArg* and store it under val. val points to some 
	// allocated memory which is large enough to keep the 
	// object of the type this ValueHandler is dealing with. 
	// In case cpsize() returns !=0, the opject referred to 
	// by val is simply allocated via malloc, if it returns 0, 
	// the object is allocated via ValueHandler::alloc() 
	// and thus set up correctly. 
	// The parsing function must take care of the assignment 
	// operator in ParamArg::assmode and return PPSIllegalAssMode 
	// if it does not support that mode. 
	// (assmode='\0' -> `='; assmode='+' -> `+='; etc.) 
	// The parsing function should noch change anything in 
	// the ParamArg (not even pdone). 
	// The ParamInfo* passes the info structure associated with 
	// the passed ParamArg in case the parsing function needs 
	// this info. 
	// Return value: 
	//  <0 -> warning 
	//   0 -> success
	//  >0 -> error 
	// See enum ParParseState for a list of possible error 
	// codes. You may also return custom error codes as longe 
	// as your custom handlers can deal with them. 
	// Default implementation: -pure virtual-
	virtual	ParParseState parse(ParamInfo *,void * /*val*/,
		ParamArg *) HL_PureVirt(PPSIllegalArg)
	
	// Helper for print(): 
	static char *stralloc(size_t len);
	static void strfree(char *str);
	
	// Used to write the value of the passed val to a string. 
	// In case the string representation of the value fits into 
	// dest of size len, it is stored there ('\0'-terminated) 
	// and dest is returned. 
	// In case the string representation is longer than len, 
	// a new one is allocated via ValueHandler::stralloc() and 
	// returned. In case stralloc() fails, NULL is returned. 
	// Default implementation: -pure virtual-
	virtual char *print(ParamInfo *,void * /*val*/,
		char * /*dest*/,size_t /*len*/) HL_PureVirt(NULL)
};


namespace internal
{
	// Used e.g. by ParameterConsumer_Overloaded: 
	// Default value handlers. 
	extern ValueHandler *int_handler;
	extern ValueHandler *uint_handler;
	extern ValueHandler *long_handler;
	extern ValueHandler *ulong_handler;
	extern ValueHandler *double_handler;
	extern ValueHandler *switch_handler;
	extern ValueHandler *option_handler;
	
}  // namespace internal end. 


#ifdef ParNeed_ValueHandlerT

// Description: see struct ValueHandler. 
// Use the capitalized versions instead of the ones inherited 
// by ValueHandler. 
template<class T> class ValueHandlerT : ValueHandler
{
	public:
		ValueHandlerT() { }
		virtual ~ValueHandlerT() { }
		
		virtual T *Alloc(ParamInfo *)
			{  return(new T);  }
		virtual void Free(ParamInfo *,T *val)
			{  if(val)  delete val;  }
		virtual int Copy(ParamInfo *pi,T *dest,T *src,int op)
			{  if(op!=SOPCopy) return(-2); *dest = *src;  return(0);  }
		virtual	ParParseState Parse(ParamInfo *pi,T *val,ParamArg *pa)=0;
		virtual char *Print(ParamInfo *pi,T *val,char *dest,size_t len)=0;
		
	private:
		// All these wrappers...
		virtual void *alloc(ParamInfo *pi)
			{  return(Alloc(pi));  }
		virtual void free(ParamInfo *pi,void *val)
			{  Free(pi,(T*)val);  }
		virtual int copy(ParamInfo *pi,void *dest,void *src,int op)
			{  return(Copy(pi,(T*)dest,(T*)src,op));  }
		virtual	ParParseState parse(ParamInfo *pi,void *val,ParamArg *pa)
			{  return(Parse(pi,(T*)val,pa));  }
		virtual	char *print(ParamInfo *pi,T *val,char *dest,size_t len)
			{  return(Print(pi,(T*)val,dest,len));  }
};

#endif  /* ParNeed_ValueHandlerT */


}  // namespace end 

#endif  /* _INC_PAR_ParInternal_H_ */
