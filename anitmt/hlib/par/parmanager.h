#ifndef _INC_PAR_ParManager_H_
#define _INC_PAR_ParManager_H_ 1

#include "valhdl.h"
#include <string.h>
#include <stdio.h>


namespace par
{

class ParameterSource;

// Central point which knows about all parameters. 
class ParameterManager : public PAR
{
	friend class ParameterSource_CmdLine;
	public:
		// Static global manager: 
		//static ParameterManager *manager;
	private:
		// List of registered ParameterConsumerBases: 
		LinkedList<ParameterConsumerBase> pclist;
		
		// List of registered ParameterSources: 
		LinkedList<ParameterSource> psrclist;
		
		// Top section: 
		Section topsect;
		
		Section *_LookupSection(const char *str,Section *top,const char **ret_str);
		int _TidyUpSection(Section *s);
		inline void _TidyUpSections();
		void _FreeSection(Section *s);
		void _DelParamNotifySources(Section *s,ParamInfo *pi);
		void _ZapParams(Section *s,ParameterConsumerBase *pc=NULL);
		void _ClearSections(ParameterConsumerBase *pc);
		void _ClearSection(Section *s,ParameterConsumerBase *pc=NULL);
		int _CountParams(Section *top);
		
		void _RecursivePrintHelp(Section *top,class SimpleIndentConsoleOutput &sico);
		void _HelpPrintSectionHeader(Section *top,SimpleIndentConsoleOutput &sico);
		void _HelpPrintParamInfo(ParamInfo *pi,SimpleIndentConsoleOutput &sico);
	protected:
		const char *version_string;   // All these strings are not 
		const char *package_name;     // copied and initialized 
		const char *program_name;     // to NULL. 
		const char *add_help_text;
		
		// This version info must be free'd via free() (NOT LFree()). 
		// Note that this function returns NULL on malloc() failure. 
		char *_GenVersionInfo();
	public:  _CPP_OPERATORS
		ParameterManager();
		virtual ~ParameterManager();
		
		// You should call this after having written the parameters and 
		// before actually running the main part of the program. 
		// This calls the CheckParams() of all parameter consumers. 
		// Return value: 
		//   0 -> success
		//  >0 -> errors written; must exit
		int CheckParams();
		
		// Returns the section with the specified name(s) or NULL 
		// if it does not exist (leading `-' in name are skipped). 
		// The name is referring to a section below *top. 
		Section *FindSection(const char *name,Section *top=NULL);
		// Returns the top section: 
		Section *TopSection()  {  return(&topsect);  }
		
		// Find a parameter; the name may contain section names; 
		// leading `-' in name are skipped). 
		// The name is referring to a section below *top. 
		// Returns NULL on failure. 
		ParamInfo *FindParam(const char *name,size_t namelen,Section *top=NULL);
		ParamInfo *FindParam(const char *name,Section *top=NULL)
			{  return(FindParam(name,name ? strlen(name) : 0,top));  }
		
		// Stores the full section name of the passed section in dest 
		// of size len (actually len-1 bytes + '\0'). In case len is too 
		// small, the name is _not_ copied and only the actual length 
		// (without '\0' at the end!) is returned. 
		// The full section name does neither have a leading nor a 
		// trailing `-'. 
		// Return value: length of the complete name. 
		size_t FullSectionName(Section *s,char *dest,size_t len);
		
		// Stores the full parameter name including all sections in dest 
		// of size len (actually len-1 bytes + '\0'). In case len is too 
		// small, the name is _not_ copied (or partly) and the actual 
		// length (without '\0' at the end!) is returned. #
		// The full parameter name does not have a leading `-'. 
		// Return value: length of the name. 
		size_t FullParamName(ParamInfo *pi,char *dest,size_t len);
		
		// These are used primarily by ParameterSource: 
		// Counts the number of parameters in and below *top. 
		int CountParams(Section *top=NULL)
			{  return(_CountParams(top ? top : &topsect));  }
		
		// Description: See ParameterConsumerBase. 
		ParamInfo *AddParam(ParameterConsumerBase *pc,_AddParInfo *api);
		
		// Description: See ParameterConsumerBase. 
		// If the section already exists, the one it returned; 
		// else a new section is allocated. 
		// (Leading `-' skipped.) 
		Section *RegisterSection(ParameterConsumerBase *pc,
			const char *name,const char *helptext=NULL,Section *top=NULL);
		
		// ParameterConsumerBase classes (un)register using these 
		// functions. Used internally. 
		int RegisterParameterConsumerBase(ParameterConsumerBase *ps);
		void UnregisterParameterConsumerBase(ParameterConsumerBase *ps);
		
		// ParameterSource classes (un)register using these functions. 
		// Used internally. 
		int RegisterParameterSource(ParameterSource *psrc);
		void UnregisterParameterSource(ParameterSource *psrc);
		
		// Write a single parameter back to the comsumer 
		// (changing the consumer's value). 
		// Return value: 
		//  0 -> OK (success)
		//  1 -> failed (parameter locked)
		// else -> see ValueHandler::copy().
		int WriteParam(ParamCopy *pc);
		
		// This is for the --help and --version args: 
		// These should return 1. (See srccmd.{h,cc} for details.)
		virtual int PrintHelp(Section *top,FILE *out=stdout);
		// PrintVersion writes: "<program_name> (<package>) version <version>"
		// The package info is left away if it is NULL. 
		virtual int PrintVersion(FILE *out=stderr);
		
		// You should set the program name, package name and version info 
		// Using this function: 
		// The strings are not copied as I expect them to be static anyway. 
		void SetVersionInfo(
			const char *version_string,
			const char *package_name=NULL,   // no package if NULL
			const char *program_name=NULL);  // defaults to prg_name if NULL
		// Using this function you can add an additional help text 
		// appended to the output of the (topsection) -help/--help 
		// option. You usually set bug e-mail address/author/nothing 
		// here. The string is not copied. 
		// Refer to ParameterConsumerBase::AddParam() on help text format. 
		void AdditionalHelpText(const char *str)
			{  add_help_text=str;  }
};

}  // namespace end 

#endif  /* _INC_PAR_ParManager_H_ */
