// Implementation of (help/version) console/FILE handlers. 

#include <hlib/prototypes.h>

#include <string.h>

#include "parmanager.h"
#include "parconsumer.h"
#include "sindentcout.h"

#include <hlib/refstrlist.h>


#ifndef TESTING
#define TESTING 1
#endif


#ifdef TESTING
#warning TESTING switched on (assert() used). 
#include <assert.h>
#else
#define assert(x)
#endif

namespace par
{

void ParameterManager::_HelpPrintSectionHeader(Section *top,
	SimpleIndentConsoleOutput &sico)
{
	size_t tmplen=FullSectionName(top,NULL,0)+1;
	char tmp[tmplen];
	FullSectionName(top,tmp,tmplen);
	sico("-%s (section): ",tmp);
	int indent=sico.GetIndent();
	if(top->helptext)
	{
		sico.SetIndent(indent+4);
		sico("%s\n",top->helptext);
	}
	sico.SetIndent(indent+2);
	sico("Parameters in section `%s�\n",tmp);
	sico.SetIndent(indent);
}

void ParameterManager::_HelpPrintParamInfo(ParamInfo *pi,
	SimpleIndentConsoleOutput &sico)
{
	sico("-%s (%s): ",pi->name,ParamTypeString(pi->ptype));
	int indent=sico.GetIndent();
	sico.SetIndent(indent+4);
	sico("%s\n",pi->helptext);
	sico.SetIndent(indent);
}

void ParameterManager::_RecursivePrintHelp(Section *top,
	SimpleIndentConsoleOutput &sico)
{
	if(top!=&topsect)
	{  _HelpPrintSectionHeader(top,sico);  }
	
	// I do not use AddIndent() as this will not increase indent 
	// if it gets too large and then output will be back to 
	// indent=0 where it should be >0. 
	int indent=sico.GetIndent();
	// I know, I don't check if sico()'s LMalloc() fails...
	sico.SetIndent(indent+2);
	
	for(ParamInfo *pi=top->pilist.first(); pi; pi=pi->next)
	{
		_HelpPrintParamInfo(pi,sico);
		assert(pi->section==top);   // sanity check...
	}
	for(Section *s=top->sub.first(); s; s=s->next)
	{  _RecursivePrintHelp(s,sico);  }
	
	sico.SetIndent(indent);
}


int ParameterManager::PrintHelp(Section *top,FILE *out)
{
	SimpleIndentConsoleOutput sico;
	sico.SetFile(out);
	
	if(top==&topsect)
	{
		char *version_info=_GenVersionInfo();
		if(version_info)
		{
			sico("** This is %s **\n\n",version_info);
			free(version_info);
		}
		
		sico(
			"Standard query options:\n"
			"  --version: print version string\n"
			"  --help:    print this help\n");
		
		if(!topsect.sub.is_empty())
		{  sico("  --section-help: print help for specified section only\n");  }
		
		// Write out special help options: 
		int header=0;
		for(ParameterConsumer *pc=pclist.first(); pc; pc=pc->next)
		{
			for(SpecialHelpItem *shi=pc->shelp.first(); shi; shi=shi->next)
			{
				if(!header)
				{
					sico("Special query options:\n");
					sico.SetIndent(2);
					header=1;
				}
				
				sico("-%s: %s\n",shi->optname,shi->descr);
			}
		}
		if(header)
		{  sico.SetIndent(0);  }
		
		// Heading for all the other parameters: 
		sico("Available parameters:\n");
	}
	
	_RecursivePrintHelp(top ? top : &topsect,sico);
	
	if(top==&topsect && add_help_text)
	{
		sico.SetIndent(0);
		sico("\n%s\n",add_help_text);
	}
	return(0);
}

int ParameterManager::PrintVersion(FILE *out)
{
	char *version_info=_GenVersionInfo();
	fprintf(out,"%s\n",version_info);  // will write "(null)" if NULL
	if(version_info)  free(version_info);
	return(0);
}


int ParameterManager::PrintSpecialHelp(ParameterConsumer *pc,
	SpecialHelpItem *shi,FILE *out)
{
	SimpleIndentConsoleOutput sico;
	sico.SetFile(out);
	
	if(shi->helptxt)
	{  sico.Puts(shi->helptxt);  }
	else
	{
		// new stuff: 
		RefStrList txtlst;
		pc->PrintSpecialHelp(&txtlst,shi);
		for(const RefStrList::Node *s=txtlst.first(); s; s=s->next)
		{
			if(s->stype()==-1)  continue;   // NULL ref
			assert(s->stype()==0);   // need '\0'-terminated strings
			const char *txt=s->str();
			
			// Read in indent if passed: 
			// Format:
			// "\r"  [mod] indent [colon]
			// indent value is read in as integer; parsing is stopped at 
			// first non-digit; if that is a colon ':', it is als skipped. 
			// mod: '+': add indent 
			//      '-': subtract indent
			//       default: set indet
			// Examples: 
			// "\r4" "\r+2"  "\r-3:"
			if(*txt=='\r')
			{
				++txt;
				
				// Read in mod: 
				int mod=0;
				if(*txt=='+')
				{  mod=+1;  ++txt;  }
				else if(*txt=='-')
				{  mod=-1;  ++txt;  }
				
				// Read in indent: 
				char *end;
				int indent=strtol(txt,&end,10);
				if(end==txt)  indent=0;
				txt=end;
				
				// Skip optional colon: 
				if(*txt==':')
				{  ++txt;  }
				
				if(mod)
				{  sico.AddIndent(indent*mod);  }
				else
				{  sico.SetIndent(indent);  }
			}
			
			if(s!=txtlst.first())
			{  sico.Puts("\n");  }
			sico.Puts(txt);
		}
	}
	sico.SetIndent(0);
	sico.Puts("\n");
	
	return(0);
}


// This version info must be free'd via free() (NOT LFree()). 
// Note that this function returns NULL on malloc() failure. 
char *ParameterManager::_GenVersionInfo()
{
	const char *prgn=(program_name ? program_name : 
		(prg_name ? prg_name : "???"));
	const char *vers=version_string ? version_string : "???";
	int len=strlen(prgn)+strlen(vers)+10+
		(package_name ? (strlen(package_name)+3) : 0);
	char *buf=(char*)malloc(len);
	snprintf(buf,len,"%s%s%s%s version %s",
		prgn,
		package_name ? " (" : "",
		package_name ? package_name : "",
		package_name ? ")" : "",
		vers);
	return(buf);
}

}  // namespace end 
