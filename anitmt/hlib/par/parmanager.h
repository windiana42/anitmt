/*
 * parmanager.h
 * 
 * Contains central parameter manager class. 
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

#ifndef _INC_PAR_ParManager_H_
#define _INC_PAR_ParManager_H_ 1

#include "valhdl.h"
#include <string.h>

class RefStrList;

namespace par
{

class ParameterSource;

// Central point which knows about all parameters. 
class ParameterManager : public PAR
{
	private:
		// List of registered ParameterConsumer: 
		LinkedList<ParameterConsumer> pclist;
		
		// List of registered ParameterSources: 
		LinkedList<ParameterSource> psrclist;
		
		// Top section: 
		Section topsect;
		
		Section *_LookupSection(const char *str,Section *top,const char **ret_str);
		int _TidyUpSection(Section *s);
		inline void _TidyUpSections();
		void _FreeSection(Section *s);
		void _DelParamNotifySources(Section *s,ParamInfo *pi);
		void _ZapParams(Section *s,ParameterConsumer *pc=NULL);
		void _ClearSections(ParameterConsumer *pc);
		void _ClearSection(Section *s,ParameterConsumer *pc=NULL);
		int _CountParams(Section *top);
		inline void _DelParam(Section *s,ParamInfo *pi);  // del and tell par sources
		int _RecursiveCheckParams(Section *sect);
		void _SectionHandlerDetachRecursive(SectionParameterHandler *sph,Section *sect);
		int _SectHdlTraverseMoveUpLogic(int foundstart,
			const char **nend,const char *nstart,Section **sect);
		
		// Console output functions: 
		void _RecursivePrintHelp(Section *top,class SimpleIndentConsoleOutput &sico);
		void _HelpPrintSectionHeader(Section *top,SimpleIndentConsoleOutput &sico);
		void _HelpPrintParamInfo(ParamInfo *pi,SimpleIndentConsoleOutput &sico);
		// Used to call the SectionParameterHandler to format its help text. 
		// when (-1 or +1); see SectionParameterHandler. 
		int _PrintSectionManagerHelp(SectionParameterHandler *sph,Section *sect,
			int when,SimpleIndentConsoleOutput &sico);
	protected:
		const char *version_string;   // All these strings are not 
		const char *package_name;     // copied and initialized 
		const char *program_name;     // to NULL. 
		const char *add_help_text;
		const char *author_str;
		const char *license_str;
		
		// This version info must be free'd via free() (NOT LFree()). 
		// Note that this function returns NULL on malloc() failure. 
		char *_GenVersionInfo();
		
		// Used to format at write the help strings passed through a string 
		// List (see e.g. the special help in ParameterConsumer or the help 
		// text which can be written by the SectionParameterHandler). 
		void _PrintFormatStrListHelp(RefStrList *txtlst,SimpleIndentConsoleOutput &sico);
	public:  _CPP_OPERATORS_FF
		ParameterManager(int *failflag=NULL);
		virtual ~ParameterManager();
		
		// You should call this after having written the parameters and 
		// before actually running the main part of the program. 
		// This calls the CheckParams() of all parameter consumers. 
		// Return value: 
		//   0 -> success
		//  >0 -> errors written; must exit
		int CheckParams();
		
		// Call the section handlers on the specified parameter argument. 
		// bot_sect, bot_nend: till where the parameter could be parsed. 
		//   First, bot_sect's section handler will be queried, then the 
		//   handlers of the sections tree up till top_sect or till the 
		//   start of the parameter name is reached (whichever occurs 
		//   first). 
		// Return value: 
		//   0 -> parameter was accepted by a section handler
		//  <0 -> section handler returned that error value
		//   1 -> parameter was not accepted by any section handler 
		int FeedSectionHandlers(
			ParamArg *pa,Section *top_sect,
			const char *bot_nend,Section *bot_sect);
		
		// Returns the section with the specified name(s) or NULL 
		// if it does not exist (leading `-' in name are skipped). 
		// The name is referring to a section below *top. 
		// tell_section_handler: Normally 0 as the section parameter 
		//   handler does not (have to) know of section lookups. If this 
		//   is set to 1, the section handler gets called if the section 
		//   is not found. This is the case if e.g. in a file parameter 
		//   source a `#section´ statement is found, because subsequent 
		//   params will not have the section name in the parameter name. 
		//   Only set to 1 if you know what you are doing. 
		Section *FindSection(const char *name,Section *top=NULL,
			int tell_section_handler=0);
		// Returns the top section: 
		Section *TopSection()  {  return(&topsect);  }
		
		// Find a parameter; the name may contain section names; 
		// leading `-' in name are skipped). 
		// The name is referring to a section below *top. 
		// Returns NULL on failure. 
		ParamInfo *FindParam(const char *name,size_t namelen,Section *top=NULL)
			{  return(FindParam(name,namelen,top,NULL,NULL));  }
		ParamInfo *FindParam(const char *name,Section *top=NULL)
			{  return(FindParam(name,name ? strlen(name) : 0,top));  }
		// This is the extended version of the function above. 
		// It also stores info in ret_endp and ret_sect which is mainly 
		// intersting in case the patrameter could not be found (return 
		// value NULL): 
		// ret_endp: till where sections could be matched
		// ret_sect: last section that was matched parsing along the 
		//           parameter name until *ret_endp. 
		ParamInfo *FindParam(const char *name,size_t namelen,Section *top,
			const char **ret_endp,Section **ret_sect);
		
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
		// with_syns: include synonymes?
		// Return value: length of the name. 
		size_t FullParamName(ParamInfo *pi,char *dest,size_t len,
			int with_syns=0);
		
		// These are used primarily by ParameterSource: 
		// Counts the number of parameters in and below *top. 
		int CountParams(Section *top=NULL)
			{  return(_CountParams(top ? top : &topsect));  }
		
		// Description: See ParameterConsumer. 
		ParamInfo *AddParam(ParameterConsumer *pc,_AddParInfo *api);
		int DelParam(ParamInfo *pi);
		
		// Description: See ParameterConsumer. 
		// If the section already exists, the one it returned; 
		// else a new section is allocated. 
		// (Leading `-' skipped.) 
		Section *RegisterSection(ParameterConsumer *pc,
			const char *name,const char *helptext=NULL,Section *top=NULL);
		
		// Remove all parameters of the specified parameter consumer 
		// in and below the section *top. 
		// Use only if you really need it
		// Returns number of deleted parameters. 
		void RecursiveDeleteParams(ParameterConsumer *pc,Section *top=NULL);
		
		// ParameterConsumer classes (un)register using these 
		// functions. Used internally. 
		int RegisterParameterConsumer(ParameterConsumer *ps);
		void UnregisterParameterConsumer(ParameterConsumer *ps);
		
		// ParameterSource classes (un)register using these functions. 
		// Used internally. 
		int RegisterParameterSource(ParameterSource *psrc);
		void UnregisterParameterSource(ParameterSource *psrc);
		
		// SectionParameterHandler classes use these: 
		int SectionHandlerAttach(SectionParameterHandler *sph,Section *s);
		void SectionHandlerDetach(SectionParameterHandler *sph,Section *s);
		
		// Write a single parameter back to the comsumer 
		// (changing the consumer's value). 
		// Return value: 
		//  0 -> OK (success)
		//  1 -> failed (parameter locked)
		// else -> see ValueHandler::copy().
		int WriteParam(ParamCopy *pc);
		
		// This is called by parameter sources which allow special help options 
		// (like the command line source). 
		// Return value: 
		//  0 -> no special option 
		//  1 -> pa was a special option and it was handeled 
		int CheckHandleHelpOpts(ParamArg *pa,Section *top_sect);
		
		// See parconsumer.h for detailed info on this. 
		int AddSpecialHelp(ParameterConsumer *pc,SpecialHelpItem *shi);
		
		// This is for the --help and --version args: 
		// Return values (all funcs): currently unused; should be 0. 
		virtual int PrintHelp(Section *top,FILE *out=stdout);
		// PrintVersion writes: "<program_name> (<package>) version <version>"
		// The package info is left away if it is NULL. 
		virtual int PrintVersion(FILE *out=stderr);
		// what: 0 -> license info; 1 -> author info
		virtual int PrintLicense(int what,FILE *out=stderr);
		// Used when a special help option is encountered. 
		virtual int PrintSpecialHelp(ParameterConsumer *pc,
			SpecialHelpItem *shi,FILE *out=stdout);
		
		// You should set the program name, package name and version info 
		// Using this function: 
		// The strings are not copied as I expect them to be static anyway. 
		void SetVersionInfo(
			const char *version_string,
			const char *package_name=NULL,   // no package if NULL
			const char *program_name=NULL);  // defaults to prg_name if NULL
		// Set what is being written as output if --author and --license 
		// are specified; these options are not understood if you 
		// set the corresponding string to NULL. 
		void SetLicenseInfo(
			const char *license_output,
			const char *author_output);
		// Using this function you can add an additional help text 
		// appended to the output of the (topsection) -help/--help 
		// option. You usually set bug e-mail address/author/nothing 
		// here. The string is not copied. 
		// Refer to ParameterConsumer::AddParam() on help text format. 
		void AdditionalHelpText(const char *str)
			{  add_help_text=str;  }
};

}  // namespace end 

#endif  /* _INC_PAR_ParManager_H_ */
