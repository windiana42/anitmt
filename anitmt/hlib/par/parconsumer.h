#ifndef _INC_PAR_ParameterConsumer_H_
#define _INC_PAR_ParameterConsumer_H_ 1

#include <stddef.h>

#include "valhdl.h"


class RefStrList;

namespace par
{

class ParameterManager;


// A ParameterConsumer is a class derived from ParameterConsumer. 
// A ParameterConsumer may register parameters at the parameter system; 
// these parameters can then be set in different ways by different 
// ParameterSources. If you need to know/deal with all parameters which 
// are inside a special section (or you even want to do you own parameter 
// handling for all params below a section, have a look at 
// SectionParameterHandler). 
class ParameterConsumer : 
	private LinkedListBase<ParameterConsumer>, 
	public PAR
{
	friend class ParameterManager;
	friend class LinkedList<ParameterConsumer>;
	private:
		Section *curr_section;  // Set by SetSection(). 
		ParameterManager *manager;
		
		// List of special help oprtions: 
		LinkedList<SpecialHelpItem> shelp;
	protected:
		// Number of times AddParam() / AddSpecialHelp() failed: 
		int add_failed;
		
		// This is called when ParameterManager::CheckParams() is called, 
		// normally that is when all the parameters are set up and before 
		// the program actually does its job. 
		// First, for every parameter, CheckParam(ParamInfo *) is called, 
		// then, if no errors occured, CheckParams() is called. 
		// NOTE that CheckParam(ParamInfo *) in the default implementation 
		//      checks the spectype (STAtLeastOnce, etc.) so you should 
		//      consider calling it in case you override the function. 
		// NOTE: CheckParam(ParamInfo *pi) calls (virtual) CheckParamError() 
		//      in case of an error. 
		// Return value: 
		//   0 -> okay 
		//   1 -> errors written (must stop)
		virtual int CheckParam(ParamInfo *pi);
		virtual int CheckParams()
			{  return(0);  }
		
		// Error handler called by CheckParam(ParamInfo *pi) in case of an 
		// error; default implementation writes error to stderr. 
		virtual void CheckParamError(ParamInfo *pi);
		
		// See AddSpecialHelp() for explanation: 
		// Should print special help as user triggered special help as 
		// desribed by the passed SpecialHelpItem. 
		// Store the text to write in the passed string list. 
		// The items in the string list are written in the order they 
		// appear in the list; each one is terminated with a newline. 
		// DO NOT put newlines into the text unless you want to generate 
		// explici paragraphs (formatting is done automatically). 
		// Each item of the string list can use a different indention 
		// value/depth. To change it, you may put a special magic sequence 
		// at the beginning of any/each string in the list: 
		// The format is: 
		// "\r" [mod] indent [":"]
		// "\r" as the first character indicates that an indent spec is 
		//      following. 
		// mod: "+" -> add indent value
		//      "-" -> subtract indent value
		//      default: set indent value
		// indent: indent value to add/subtract/set in decimal notation. 
		// ":" -> optional colon. The parsing of the indent value is 
		//      stopped at the first non-digit char; that's where the real 
		//      beginning of the message is assumed. If this char is a 
		//      colon, it is also skipped. (Useful e.g. if your message 
		//      begins with a digit.)
		// NOTE: You may need #include <hlib/refstrlist.h>. 
		// Return value: currently unused; use 0. 
		virtual int PrintSpecialHelp(RefStrList *,const SpecialHelpItem *)
			{  return(0);  }
	public:  _CPP_OPERATORS_FF
		ParameterConsumer(ParameterManager *manager,
			int *failflag=NULL);
		virtual ~ParameterConsumer();
		
		// Returns pointer to the parameter manager: 
		ParameterManager *parmanager()
			{  return(manager);  }
		
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
		// Note: The help text is formatted automatocally, so you 
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
		//       by `|', e.g. "w|width|imagewidth". The first one 
		//       specified is the name printed commonly in errors.  
		//   YOU MAY NOT specify the same value pointer/parameter 
		//   more than once. In case one of the specified names is 
		//   already used, AddParam() fails; BUT AddParam() DOES NOT 
		//   CHECK if you use the same value twice, e.g. 
		//   AddParam("w",&width); AddParam("width",&width) 
		//   yields straight into trouble. 
		//  Oh, and don't use nastices like "w||width" or "w|"...
		// ptype: parameter type (see enum ParameterType)
		// helptext: Help text (description) for the option. Set this 
		//           to NULL if there is no help text (option hidden). 
		//           The help text is formatted automatically, so you 
		//           do not have to pass any `\n' and any sort of 
		//           indention (unless, you want to include some sort 
		//           of table; play around and see what is possible...) 
		// valptr: pointer to value; the read-in parameter gets stored 
		//         there. The value should be set up to the default. 
		//         In case there is no default, NoDefault among the 
		//         flags. 
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
		// flags: OR of following flags: 
		//  - PNoDefault -> parameter does not have a default value
		//  - PExclusiveHhl -> handler was allocated using operator new and 
		//          is used exclusively by this param. This means that the 
		//          handler is deleted as soon as the parameter ceases to 
		//          exist; if the parameter allocation failed at all, the 
		//          handler is deleted immediately. 
		//  - One of the spec types: 
		//     STNone         (=0 and can be left away)
		//     STAtLeastOnce  param must be specified at least once
		//     STExactlyOnce  param must be specified exactly once
		//     STMaxOnce      param may not be specified more than once
		//       Note that the fact that the parameter has a defautl does 
		//       not count as `specifying the parameter'. 
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
			ValueHandler *hdl,
			int flags=0);
		
		// Hardly needed but you can use that to delete a parameter: 
		// Return value: 
		//   0 -> success
		//   currently never fails. 
		int DelParam(ParamInfo *pi);
		
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
		
		// Use this funtion to add special help options, like for example 
		// --list-oformats which can be used to list available output 
		// formats. 
		// Note that the advantage here is that a virtual function 
		// (PrintSpecialHelp()) is called if you supply NULL as help text and 
		// thus you can have non-static output (e.g. the output format 
		// drivers register at runtime). 
		// You should fill out all elements of the SpecialHelpItem structure 
		// before calling this function. Note that neiter of the strings 
		// are copied; they are all assumed static (or at least existing as 
		// long as the ParameterConsumer *this exists). [The constructor 
		// sets up the pointers to NULL and the item_id to -1.]
		// SpecialHelpItem members: 
		// optname:  name of the option (e.g. list-oformats) 
		//           NOTE: always absolut (i.e. below global top section)
		// descr:    short description (e.g. "lists available output formats")
		// helptxt:  help text or NULL if the virtual function 
		//           ParameterConsumer::PrintSpecialHelp() shall be called
		// item_id:  arbitrary value assigned by parameter consumer for easy 
		//           recognizion in PrintSpecialHelp(). 
		// Return value: 
		//  0 -> success
		// -1 -> allocation failure
		// -2 -> descr or optname NULL 
		int AddSpecialHelp(SpecialHelpItem *shi);
};

}  // namespace end 

#endif  /* _INC_PAR_ParameterConsumer_H_ */
