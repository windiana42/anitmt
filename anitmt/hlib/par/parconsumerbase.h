#ifndef _INC_PAR_ParConsumerBase_H_
#define _INC_PAR_ParConsumerBase_H_ 1

#include <stddef.h>

#include "parintrnl.h"

namespace par
{

class ParameterManager;

#warning REMOVE ME. 
class GlobalParameterHandler
{
	friend class ParameterParser;
	protected:
		// Gets called before this arg is parsed to get approval. 
		// [CMD line and parameter file]
		// Return value: 
		//  0 -> approved; 
		//  n<0 -> skip n args marking them as unused
		//         (use only for cmd line args)
		//  n>0 -> skip n args marking them as used
		virtual int approve(const ParamArg *)  {  return(0);  }
		
		// Gets called for all args considered files specified on 
		// the cmd line (as naked files, not as parameters). 
		// [CMD line only]
		// Return value: 
		//  0 -> okay; 1 -> unused (++ error counter) 
		virtual int filearg(const char * /*file*/)  {  return(1);  }
		
		// Gets called for all unknwon arg names during parsing. 
		// These might be a global one-letter arg string or 
		// misspelled/illegal arg names. 
		// [CMD line and parameter file]
		// Return value: 
		//    0 -> if arg->used it set, the arg is used; (if 
		//         arg->next->used is set, the next arg is also used...) 
		//         if arg->used is not set: unknown arg; 
		//         increase error counter and write error
		//    1 -> unknown arg; increase error counter; message 
		//         already written 
		virtual int unknown(const ParamArg *)  {  return(0);  }
	public:  _CPP_OPERATORS_FF
		GlobalParameterHandler(int *failflag=NULL);
		virtual ~GlobalParameterHandler();
};



class ParameterConsumerBase : 
	private LinkedListBase<ParameterConsumerBase>, 
	public PAR
{
	friend class ParameterManager;
	friend class LinkedList<ParameterConsumerBase>;
	private:
		Section *curr_section;  // Set by SetSection(). 
	public:  _CPP_OPERATORS_FF
		ParameterConsumerBase(int *failflag=NULL);
		virtual ~ParameterConsumerBase();
		
		// Returns the current section pointer; you probably do not need this. 
		Section *CurrentSection()  {  return(curr_section);  }
		
		// Returns the section with the specified name(s) or NULL if it does 
		// not exist (leading `-' in name are skipped): 
		// The name is referring to a section below *top; if top=NULL, the 
		// topmost section is used. 
		Section *FindSection(const char *name,Section *top=NULL);
		
		// Set the section name. The string is copied (for internal reason). 
		// The section name is e.g. 
		//   "input" (section input) 
		//   "input-warn" (section input, subsection warn)
		// Pass NULL or "" as string if you want the same section as top. 
		// Leading `-' are skipped. 
		// If you set top!=NULL, the specified section name is treated as 
		// subsection of the section top. (Call CurrentSection() to get 
		// the current one.)
		// You may call this function several times; calls to AddParam() 
		// (and its cousins) always refer to the last specified section. 
		// helptext is a non-copied help text string or NULL for none. 
		// The first non-NULL help text string for the section is used, 
		// if a latter SetSection() call supplies a different help text, 
		// it is silently ignored. 
		// Note: The help text is formattes automatocally, so you 
		//       do not have to pass any `\n' and any sort of indention. 
		// Return value: 
		//   0 -> OK
		//  -1 -> failed (malloc() or top!=NULL but not registered) 
		//  -2 -> sect_name NULL or empty 
		// In case of an error, the following parameters get added to the 
		// previous section. 
		int SetSection(const char *sect_name,const char *helptext=NULL,
			Section *top=NULL);
		
		// BE SURE TO CALL SetSection() BEFORE ADDING A PARAMETER. 
		// Add a parameter. This is the basic function; there are 
		// overloaded versions for convenience in derived classes. 
		// name: parameter name (without sections and without leading 
		//       `-'). name is NOT copied. 
		//       A paremeter may have any number of synonyms separated 
		//       by `/', e.g. "w/width/imagewidth". The first one 
		//       specified is the name printed commonly in errors.  
		//   YOU MAY NOT specify the same value pointer/parameter 
		//   more than once. In case one of the specified names is 
		//   already used, AddParam() fails; BUT AddParam() DOES NOT 
		//   CHECK if you use the same value twice, e.g. 
		//   AddParam("w",&width); AddParam("width",&width) 
		//   yields straight into trouble. 
		//  Oh, and don't use nastices like "w//width" or "w/"...
		// ptype: parameter type (see enum ParameterType)
		// helptext: Help text (description) for the option. Set this 
		//           to NULL if there is no help text (option hidden). 
		//           The help text is formatted automatically, so you 
		//           do not have to pass any `\n' and any sort of 
		//           indention (unless, you want to include some sort 
		//           of table; play around and see what is possible...) 
		// valptr: pointer to value; the read-in parameter gets stored 
		//         there. The value should be set up to the default. 
		//         In case there is no default, pass NoDefault(ptype) 
		//         instead of ptype. 
		// hdl: a parameter handler for the argument; you normally need 
		//      only one handler per parameter type (unless, of course, 
		//      you have different parsers for some enum type, etc). 
		// NOTE: 
		//    (Parse routine is referring to the value handler method.)
		//  - PTOption parameters MUST pass an `int*' (or NULL) as valptr. 
		//       This integer will be incremented each time the option 
		//       is encountered and parse() returns PPSSuccess. 
		//       The parse() routine will be called for PTOption with 
		//       ParamArg::value=NULL. 
		//  - PTOptArg: the number of occurances can be queried via 
		//       IsSet() from the returned handler (or you use a custom 
		//       parser). If the PTOptArg parameter is an assignment, the 
		//       ParamArg::value is non-NULL, else it is NULL. 
		//  - PTSwitch: must check ParamArg::name (`no-' prefix) if 
		//       ParamArg::value=NULL.
		//       (You will normally use the internal parser for this.) 
		// Return value: 
		//   NULL -> failed (malloc() or SetSection() not called/failed) 
		//   else -> handle for the parameter 
		// NOTE: Make sure, the value handler knows the parameter type 
		//       (int/string/struct xyz/...)
		ParamInfo *AddParam(
			const char *name,
			ParameterType ptype,
			const char *helptext,   // pass NULL if there is no help text 
			void *valptr,
			ValueHandler *hdl);
		inline ParameterType NoDefault(ParameterType ptype) const
			{  return(ParameterType(ptype | PTNoDefault));  }
		
		#warning should add DelParam();
		
		// Query if a parameter was set (better: how often it 
		// was successfully overridden/added): 
		// 0 -> not set and no default 
		// 1 -> default is set 
		// >1 -> default was overridden 
		int IsSet(const ParamInfo *pi) const  {  return(pi->is_set);  }
		
		// Lock/Unlock a paramter. (A locked parameter cannot be 
		// modified by a ParameterSource.) Parameters default to 
		// unlocked. 
		int IsLocked(const ParamInfo *pi) const  {  return(pi->locked);  }
		void LockParam(ParamInfo *pi,int lock)  {  pi->locked=(lock?1:0);  }
};

}  // namespace end 

#endif  /* _INC_PAR_ParConsumerBase_H_ */
