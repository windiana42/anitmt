/*
 * parmanager.cc
 * 
 * Implementation of central parameter manager. 
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

#include <hlib/prototypes.h>

#include <string.h>

#include "parmanager.h"
#include "parconsumer.h"
#include "parsource.h"
#include "secthdl.h"

#ifndef TESTING
#define TESTING 1
#endif


// Allocation routines to use: 
static inline void *OFree(void *x)
{  if(x)  free(x);  return(NULL);  }
static inline void *OMalloc(size_t s)
{  return(s ? malloc(s) : NULL);  }
static inline void *OReAlloc(void *ptr,size_t s)
{  return(realloc(ptr,s));  }

#ifdef TESTING
#warning TESTING switched on (assert() used). 
#include <assert.h>
#else
#define assert(x)
#endif


namespace par
{

// Static global manager: 
//ParameterManager *ParameterManager::manager=NULL;

static inline int ABS(int x)
{  return((x<0) ? (-x) : x);  }


int ParameterManager::_CountParams(Section *top)
{
	int num=0;
	for(ParamInfo *pi=top->pilist.first(); pi; pi=pi->next)
	{  ++num;  }
	for(Section *s=top->sub.first(); s; s=s->next)
	{  num+=_CountParams(s);  }
	return(num);
}


// You should call this after having written the parameters and 
// before actually running the main part of the program. 
// This calls the (Do)CheckParams() of all parameter consumers. 
// Return value: 
//   0 -> success
//  >0 -> errors written; must exit
int ParameterManager::CheckParams()
{
	int rv=_RecursiveCheckParams(&topsect);
	
	if(!rv)
	{
		// No error: go through the consumers. 
		for(ParameterConsumer *pc=pclist.first(); pc; pc=pc->next)
		{
			if(pc->CheckParams())
			{  ++rv;  }
		}
	}
	
	return(rv);
}


int ParameterManager::_RecursiveCheckParams(Section *sect)
{
	int rv=0;
	
	// For all parameters in the section: 
	for(ParamInfo *pi=sect->pilist.first(); pi; pi=pi->next)
	{  rv+=pi->pc->CheckParam(pi);  }
	
	// For all subsections: 
	for(Section *s=sect->sub.first(); s; s=s->next)
	{  rv+=_RecursiveCheckParams(s);  }
	
	return(rv);
}


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
int ParameterManager::FeedSectionHandlers(
	ParamArg *pa,Section *top_sect,
	const char *bot_nend,Section *bot_sect)
{
	const char *nend=bot_nend;
	const char *nstart=pa->name;
	for(Section *sect=bot_sect; sect; )
	{
		if(sect->sect_hdl)
		{
			// Okay, the section has a section handler. 
			SPHInfo info;
			info.arg=pa;
			info.sect=sect;
			info.nend=nend;
			info.bot_sect=bot_sect;
			info.bot_nend=bot_nend;
			int rv=sect->sect_hdl->parse(&info);
			if(rv<=0)
			{  return(rv);  }
		}
		
		if(sect==top_sect || sect==&topsect)  break;
		if(nend<=nstart)  break;
		
		// Find the name end for one section up: 
		--nend;
		while(*nend=='-' && nend>=nstart)  --nend;
		if(nend<=nstart)  break;
		//while(*nend!='-' && nend>=nstart)  --nend;
		//++nend;
		++nend;
		nend-=strlen(sect->name);
		if(nend<nstart)
		{  assert(0);  break;  }
		
		// And go one section up: 
		sect=sect->up;
		assert(sect);   // checked against topsect above. 
	}
	
	return(1);
}


// This is called by parameter sources which allow special help options 
// (like the command line source). 
// Return value: 
//  0 -> no special option 
//  1 -> pa was a special option and it was handeled 
int ParameterManager::CheckHandleHelpOpts(ParamArg *pa,Section *top_sect)
{
	if(!top_sect)  top_sect=&topsect;
	
	int special=0;
	
	// --version: (only accepted in highmost section)
	if(pa->atype==ParamArg::Option && 
	   top_sect==&topsect)
	{
		if(pa->namelen==7 && 
		   pa->name[0]=='v' && 
		   !strncmp(pa->name,"version",pa->namelen))
		{
			PrintVersion();
			pa->pdone=1;
			++special;
		}
	}
	
	// --help: (section help only prints help for the specified section)
	if(pa->atype==ParamArg::Option && pa->namelen>=4)
	{
		// --sect-sect-help
		if(pa->name>pa->arg.str() &&  // this guarantees that pa->name[-1] is valid, so...
		   !strcmp(&pa->name[pa->namelen-4]-1,"-help"))  // ...namelen-5 is save here. 
		{
			// Must find correct subsection and, if found 
			// call PrintHelp(). 
			const char *end=NULL;
			Section *s=_LookupSection(pa->name,top_sect,&end);
			if(s && end==pa->name+pa->namelen-4)
			{
				PrintHelp(s);
				pa->pdone=1;
				++special;
			}
		}
	}
	
	// Further special help options (only accepted at top level): 
	if(pa->atype==ParamArg::Option && 
	   top_sect==&topsect)
	{
		// Okay, look up in special help lists: 
		for(ParameterConsumer *pc=pclist.first(); pc; pc=pc->next)
		{
			for(SpecialHelpItem *shi=pc->shelp.first(); shi; shi=shi->next)
			{
				if(*shi->optname!=*pa->name)  continue;
				size_t optlen=strlen(shi->optname);
				if(optlen!=pa->namelen)  continue;
				if(strncmp(shi->optname,pa->name,pa->namelen))  continue;
				// Okay, found it. 
				PrintSpecialHelp(pc,shi);
				pa->pdone=1;
				++special;
			}
		}
	}
	
	return(special ? 1 : 0);
}


int ParameterManager::AddSpecialHelp(ParameterConsumer *pc,SpecialHelpItem *shi)
{
	// Check it: 
	if(!shi)  return(0);
	if(!shi->optname || !shi->descr)  return(-2);
	
	// Skip leading `-´ in option name: 
	while(*shi->optname=='-')
	{  ++shi->optname;  }
	if(!(*shi->optname))  return(-2);
	
	SpecialHelpItem *s=NEW<SpecialHelpItem>();
	if(!s)  return(-1);
	
	// simply copy all members...
	*s=*shi;
	
	// ...and put into shelp queue: 
	pc->shelp.append(s);
	
	return(0);
}


// Stores the full section name of the passed section in dest 
// of size len (actually len-1 bytes + '\0'). In case len is too 
// small, the name is _not_ copied and only the actual length 
// (without '\0' at the end!) is returned. 
// The full section name does neither have a leading nor a 
// trailing `-'. 
// Return value: length of the complete name. 
size_t ParameterManager::FullSectionName(Section *s,char *dest,size_t len)
{
	if(!s)  return(0);
	size_t needlen=0;
	for(Section *u=s; u->up; u=u->up)
	{  needlen+=strlen(u->name)+1;  }   /* +1 for the `-' */
	if(needlen)  --needlen;
	if(len)
	{  *dest='\0';  }
	if(len>needlen)  // NOT >= because of '\0' at the end. 
	{
		dest+=needlen;
		*dest='\0';  // important: string termination 
		for(Section *u=s; u->up; u=u->up)
		{
			if(u!=s)  *(--dest)='-';
			size_t cp=strlen(u->name);
			dest-=cp;
			strncpy(dest,u->name,cp);
		}
	}
	return(needlen);
}


// Stores the full parameter name including all sections in dest 
// of size len (actually len-1 bytes + '\0'). In case len is too 
// small, the name is _not_ copied (or partly) and the actual 
// length (without '\0' at the end!) is returned. 
// The full parameter name does not have a leading `-'. 
// Return value: length of the name. 
size_t ParameterManager::FullParamName(ParamInfo *pi,char *dest,size_t len,
	int with_synp)
{
	if(!pi)  return(0);
	size_t sectlen=FullSectionName(pi->section,dest,len);
	// Get size for name: synpos[0] is too large by 1 char 
	//                    but that is `-' here. 
	#warning UNTESTED!!! TEST FullParamName(...,with_synp=1)
	size_t namelen=with_synp ? (strlen(pi->name)+1) : ABS(*pi->synpos);
	assert(namelen);
	if(!pi->section->up)  // if the param is in the top section, we 
	{  --namelen;  }      // dont need the leading `-'. 
	size_t needlen=namelen+sectlen;
	if(len>needlen)  // NOT >= because of '\0' at the end. 
	{
		dest+=sectlen;
		if(pi->section->up)  *(dest++)='-';
		strncpy(dest,pi->name,namelen);
		dest[namelen]='\0';
	}
	else if(len)
	{  *dest='\0';  }
	return(needlen);
}


PAR::ParamInfo *ParameterManager::AddParam(ParameterConsumer *pc,
	_AddParInfo *api)
{
	ParamInfo *pi=NULL;
	
	// NO leading `-' allowed. 
	if(!pc || !api->section || !api->name)  goto done;
	if(!(*api->name))  goto done;
	
	pi=ParamInfo::NewPi(api->name);
	if(pi)
	{
		pi->helptext=api->helptext;
		pi->ptype=api->ptype;
		pi->pc=pc;
		pi->section=api->section;
		pi->vhdl=api->hdl;
		pi->exclusive_vhdl=api->exclusive_hdl;
		pi->valptr=api->valptr;
		pi->is_set=api->has_default ? 1 : 0;
		pi->spectype=api->spectype;
		
		// Check if one of the names in pi->name is already used: 
		int failed=0;
		for(int i=0,sp0=0,sp=0; sp>=0; sp0=sp,i++)
		{
			sp=pi->synpos[i];
			int nlen=ABS(sp)-sp0-1;
			if(nlen<=0)
			{  ++failed;  break;  }
			if(FindParam(&pi->name[sp0],nlen,api->section))
			{  ++failed;  break;  }
		}
		if(failed)
		{  ParamInfo::DelPi(pi);  pi=NULL;  }
		else
		{		
			// Add the parameter to the list...
			api->section->pilist.append(pi);
		}
	}
	
	done:;
	// Must delete value handler if the call to AddParam() is just failing 
	// and the value handler is exclusive: 
	if(!pi && api->exclusive_hdl && api->hdl)
	{  delete api->hdl;  api->hdl=NULL;  }
	
	return(pi);
}


// Tell parameter sources and delete: 
inline void ParameterManager::_DelParam(Section *s,ParamInfo *pi)
{
	_DelParamNotifySources(s,pi);
	s->pilist.dequeue(pi);
	ParamInfo::DelPi(pi);  // delete pi. 
}

int ParameterManager::DelParam(ParamInfo *pi)
{
	if(!pi)  return(0);
	
	_DelParam(pi->section,pi);
	return(0);
}


void ParameterManager::RecursiveDeleteParams(ParameterConsumer *pc,
	Section *top)
{
	if(!pc)  return;
	_ClearSection(top ? top : &topsect,pc);
	_TidyUpSections();
}


PAR::Section *ParameterManager::RegisterSection(ParameterConsumer *pc,
	const char *sect_name,const char *helptext,Section *top=NULL)
{
	if(!pc)  return(NULL);
	
	// Strip leading `-': 
	bool on_top=false;
	if(sect_name)
	{
		const char *sn0=sect_name;
		while(*sect_name=='-')  ++sect_name;
		if(!(*sect_name))
		{
			if(*sn0=='-')  return(NULL);
			on_top=true;
		}
	}
	else
	{  on_top=true;  }
	
	if(!top)  top=&topsect;
	
	if(on_top)
	{  return(top);  }
	
	// !on_top here. 
	
	const char *end;
	Section *s=_LookupSection(sect_name,top,&end);
	if(!(*end))   // section already there 
	{
		if(!s->helptext && helptext)
		{  s->helptext=helptext;  }
		return(s);
	}
	
	// get next section name: (end="secA-secB-secC")
	top=s;
	// must create sub-sections below top (=s). 
	for(;;)
	{
		const char *name=end;
		while(*end && *end!='-')  ++end;
		
		// Need to allocate a subsection of *top with name *name. 
		Section *ns=NEW2<Section>(name,end-name);
		if(!ns)
		{  top=NULL;  break;  }
		
		// Queue the new section: 
		top->sub.append(ns);
		ns->up=top;
		
		top=ns;
		if(!(*end))  break;
		++end;
	}
	
	if(!top)
	{  _TidyUpSections();  return(NULL);  }
	else 
	{  top->helptext=helptext;  }
	
	return(top);
}


PAR::Section *ParameterManager::FindSection(const char *name,Section *top)
{
	if(!top)  top=&topsect;
	if(!name)  return(top);
	if(!*name)  return(top);
	while(*name=='-')  ++name;
	if(!(*name))  return(NULL);  // or return(top) ???
	
	const char *end;
	Section *s=_LookupSection(name,top,&end);
	return((*end) ? NULL : s);  // correct. 
}


PAR::ParamInfo *ParameterManager::FindParam(const char *name,
	size_t namelen,Section *top,const char **ret_endp,Section **ret_sect)
{
	if(ret_endp)  *ret_endp=NULL;
	if(ret_sect)  *ret_sect=NULL;
	
	if(!name)  return(NULL);
	while(*name=='-')  ++name;
	if(!(*name))  return(NULL);
	
	const char *end;
	Section *s=_LookupSection(name,top ? top : (&topsect),&end);
	if(ret_endp)  *ret_endp=end;
	if(ret_sect)  *ret_sect=s;
	
	ssize_t endlen=name+namelen-end;
	if(endlen<=0)  return(NULL);
	for(ParamInfo *pi=s->pilist.first(); pi; pi=pi->next)
	{
		if(!pi->NameCmp(end,endlen))
		{  return(pi);  }
	}
	
	return(NULL);
}


// Return value: 0 -> okay; 1 -> already registered. 
int ParameterManager::RegisterParameterConsumer(ParameterConsumer *ps)
{
	if(!ps)  return(0);
	
	if(pclist.first()==ps || ps->prev)
	{  return(1);  }   // already registered
	pclist.append(ps);
	
	return(0);
}

void ParameterManager::UnregisterParameterConsumer(ParameterConsumer *ps)
{
	if(!ps)  return;
	// Check if he can be in the list (otherwise RegisterParameterConsumer() 
	// probably failed). 
	if(pclist.first()!=ps && !ps->prev)  return;
	
	#if TESTING
	if(!pclist.find(ps))
	{  fprintf(stderr,"ParameterManager: Consumer %p wants to unregister "
		"but is not registered.\n",ps);  }
	#endif
	
	// We must find and delete all the parameters of this consumer: 
	_ClearSections(ps);
	
	// Then, the consumer must be removed from the consumers list: 
	pclist.dequeue(ps);
	
	// And at last, we have to tidy up...
	_TidyUpSections();
}


int ParameterManager::RegisterParameterSource(ParameterSource *psrc)
{
	if(!psrc)  return(0);
	
	if(psrclist.first()==psrc || psrc->prev)
	{  return(1);  }   // already registered
	psrclist.append(psrc);
	
	return(0);
}

void ParameterManager::UnregisterParameterSource(ParameterSource *psrc)
{
	if(!psrc)  return;
	// Check if he can be in the list (otherwise RegisterParameterSource() 
	// probably failed). 
	if(psrclist.first()!=psrc && !psrc->prev)  return;
	
	#if TESTING
	if(!psrclist.find(psrc))
	{  fprintf(stderr,"ParameterManager: Source %p wants to unregister "
		"but is not registered.\n",psrc);  }
	#endif
	
	// The source must be removed from the source list: 
	psrclist.dequeue(psrc);
}


int ParameterManager::SectionHandlerAttach(SectionParameterHandler *sph,
	Section *s)
{
	if(!sph)  return(0);
	if(!s)  return(-1);
	
	if(!s->sect_hdl)
	{
		// Okay, attach: 
		s->sect_hdl=sph;
		return(0);
	}
	return((s->sect_hdl==sph) ? 0 : -2);
}

void ParameterManager::SectionHandlerDetach(SectionParameterHandler *sph,
	Section *s)
{
	// NOTE: s=NULL means to detach from all sections sph is attached. 
	if(s)
	{
		if(s->sect_hdl==sph)
		{  s->sect_hdl=NULL;  }
		return;
	}
	
	// s=NULL here. 
	_SectionHandlerDetachRecursive(sph,&topsect);
}

void ParameterManager::_SectionHandlerDetachRecursive(
	SectionParameterHandler *sph,Section *sect)
{
	if(sect->sect_hdl==sph)
	{  sect->sect_hdl=NULL;  }
	
	for(Section *s=sect->sub.first(); s; s=s->next)
	{  _SectionHandlerDetachRecursive(sph,s);  }
}


int ParameterManager::WriteParam(ParamCopy *pc)
{
	if(!pc)  return(0);
	
	ParamInfo *pi=pc->info;
	if(pi->locked)  return(1);
	
	int rv=0;
	if(pc->nspec)
	{
		#warning Always use PAR::SOPCopy when writing params?
		rv=pi->vhdl->copy(pi,pi->valptr,pc->copyval,PAR::SOPCopy);
		pi->nspec+=pc->nspec;   // sum up
		++pi->is_set;
	}
	#if TESTING
	else
	{  fprintf(stderr,"ParameterManager: OOps: WriteParam() shall write "
		"param %s with nspec=%d\n",pc->info->name,pc->nspec);  }
	#endif
	
	return(rv);
}


void ParameterManager::SetVersionInfo(
	const char *_version_string,
	const char *_package_name,   // no package if NULL
	const char *_program_name)
{
	version_string=_version_string;
	package_name=_package_name;
	program_name=_program_name;
}


// ret_str (may be NULL): returns till where the sections were found; 
// if **ret_str='\0', all sections were found. 
PAR::Section *ParameterManager::_LookupSection(
	const char *str,Section *top,const char **ret_str)
{
	if(!(*str))
	{
		if(ret_str)  *ret_str=str;
		return(top);
	}
	
	const char *nameend=str;
	while(*nameend && *nameend!='-')  ++nameend;
	size_t namelen=nameend-str;
	const char *nextname=nameend;
	if(*nextname=='-')  ++nextname;
	
	for(Section *si=top->sub.first(); si; si=si->next)
	{
		if(strncmp(si->name,str,namelen))  continue;
		if(si->name[namelen])  continue;
		if(*nextname)
		{  return(_LookupSection(nextname,si,ret_str));  }
		if(ret_str)  *ret_str=nextname;
		return(si);
	}
	
	if(ret_str)  *ret_str=str;
	return(top);
}


// Deletes all empty (unnecessary) sections
inline void ParameterManager::_TidyUpSections()
{
	_TidyUpSection(&topsect);
	// topsect cannot be deleted. 
}

int ParameterManager::_TidyUpSection(Section *s)
{
	int needval=0;
	for(Section *_si=s->sub.first(); _si; )
	{
		Section *si=_si;
		_si=_si->next;
		int needed=_TidyUpSection(si);
		if(!needed)
		{  _FreeSection(si);  }
		needval+=needed;
	}
	if(!s->pilist.is_empty())
	{  ++needval;  }
	return(needval);
}

#if TESTING
void ParameterManager::_FreeSection(Section *s)
#else
inline void ParameterManager::_FreeSection(Section *s)
#endif
{
	#if TESTING
	assert(s->pilist.is_empty());
	assert(s->sub.is_empty());
	assert(s->up);   // the top section has up=NULL and may not be 
	                 // deleted (member of ParameterParser). 
	assert(s->up->sub.find(s));
	#endif
	
	// dequeue section: 
	s->up->sub.dequeue(s);
	
	delete s;  // will free the name
}

// Tell the ParameterSources that the parameter pc in section s is deleted. 
inline void ParameterManager::_DelParamNotifySources(Section *s,ParamInfo *pi)
{
	for(ParameterSource *psrc=psrclist.first(); psrc; psrc=psrc->next)
	{  psrc->ParamGetsDeleted(s,pi);  }
}

void ParameterManager::_ZapParams(Section *s,ParameterConsumer *pc)
{
	for(ParamInfo *_pi=s->pilist.first(); _pi; )
	{
		ParamInfo *pi=_pi;
		_pi=_pi->next;
		if(pc && pi->pc!=pc)  continue;
		_DelParam(s,pi);
	}
}


// Clear the sections by removing all parameters added by the 
// passed consumer. If ps==NULL, match for all sources (clean up 
// everything). 
void ParameterManager::_ClearSections(ParameterConsumer *pc)
{
	_ClearSection(&topsect,pc);
	_TidyUpSections();
}

void ParameterManager::_ClearSection(Section *s,ParameterConsumer *pc)
{
	_ZapParams(s,pc);
	
	for(Section *_si=s->sub.first(); _si; )
	{
		Section *si=_si;
		_si=_si->next;
		_ClearSection(si,pc);
	}
}


ParameterManager::ParameterManager(int * /*failflag*/) : 
	pclist(),
	topsect(/*name=*/NULL)
{
	version_string=NULL;
	package_name=NULL;
	program_name=NULL;
	add_help_text=NULL;
	
	// static global manager: 
	//#if TESTING
	//if(manager)
	//{  fprintf(stderr,"ParameterManager: more than one manager\n");  exit(1);  }
	//#endif
	//manager=this;
}

ParameterManager::~ParameterManager()
{
	#if TESTING
	if(!pclist.is_empty())
	{  fprintf(stderr,"ParameterManager::~(): non-unregistered ParameterSources!\n");  }
	#endif
	
	// Clean them up... all...
	_ClearSections(NULL);
	
	#if TESTING
	if(!topsect.pilist.is_empty() || !topsect.sub.is_empty())
	{  fprintf(stderr,"ParameterManager::~(): BUG! incomplete cleanup: %d,%d\n",
		int(topsect.pilist.is_empty()),int(topsect.sub.is_empty()));  }
	#endif
	
	// static global manager: 
	//manager=NULL;
}

}  // namespace end 
