/*
 * database.cpp
 * 
 * Component (renderers, filters, ...) database implementation. 
 * 
 * Copyright (c) 2001 -- 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "database.hpp"

#include <ctype.h>
#include <assert.h>



TaskDriverFactory *ComponentDataBase::_FindDriverFactoryByName(
	const char *name,TaskDriverType dtype)
{
	if(dtype<0 || dtype>=_DTLast)
	{  return(NULL);  }
	size_t nlen=strlen(name);
	LinkedList<TaskDriverFactory> *list=&drivers[dtype];
	for(TaskDriverFactory *n=list->first(); n; n=list->next(n))
	{
		const char *dn=n->DriverName();
		if(strlen(dn)!=nlen)  continue;
		if(strcmp(dn,name))  continue;
		assert(n->DType()==dtype);
		return(n);
	}
	return(NULL);
}


TaskSourceFactory *ComponentDataBase::_FindSourceFactoryByName(const char *name)
{
	size_t nlen=strlen(name);
	for(TaskSourceFactory *n=tsources.first(); n; n=tsources.next(n))
	{
		const char *dn=n->TaskSourceName();
		if(strlen(dn)!=nlen)  continue;
		if(strcmp(dn,name))  continue;
		return(n);
	}
	return(NULL);
}


const RenderDesc *ComponentDataBase::FindRenderDescByName(const char *name)
{
	if(!name)  return(NULL);
	size_t slen=strlen(name);
	for(RenderDesc *i=rdesclist.first(); i; i=i->next)
	{
		if(i->name.len()!=slen)  continue;
		if(strcmp(i->name.str(),name))  continue;
		return(i);
	}
	return(NULL);
}

const FilterDesc *ComponentDataBase::FindFilterDescByName(const char *name)
{
	if(!name)  return(NULL);
	size_t slen=strlen(name);
	for(FilterDesc *i=fdesclist.first(); i; i=i->next)
	{
		if(i->name.len()!=slen)  continue;
		if(strcmp(i->name.str(),name))  continue;
		return(i);
	}
	return(NULL);
}


int ComponentDataBase::RegisterDriverFactory(
	TaskDriverFactory *drv,TaskDriverType dtype)
{
	if(!drv)  return(0);
	if(dtype<0 || dtype>=_DTLast)
	{  return(-2);  }
	
	if(!drv->DriverName())
	{  return(1);  }
	if(_FindDriverFactoryByName(drv->DriverName(),dtype))
	{  return(1);  }
	
	LinkedList<TaskDriverFactory> *list=&drivers[dtype];
	assert(!list->prev(drv) && list->first()!=drv);
	list->append(drv);
	
	return(0);
}


void ComponentDataBase::UnregisterDriverFactory(
	TaskDriverFactory *drv,TaskDriverType dtype)
{
	if(!drv)  return;
	if(dtype<0 || dtype>=_DTLast)  return;
	
	// Check if queued: 
	LinkedList<TaskDriverFactory> *list=&drivers[dtype];
	if(list->first()!=drv && !list->prev(drv))  return;
	
	// Delete all render/filter descriptions using this driver: 
	// FIXME: We should not delete the descriptions (or at lease assert()) 
	//        in case the RederDesc in question is still in use. 
	//        Currently this cannot be detected (and will not happen 
	//        as UnregisterDriverFactory() is only called on cleanup). 
	switch(dtype)
	{
		case _DTLast:  assert(0);  break;
		case DTNone:  break;
		case DTRender: 
			for(RenderDesc *i=rdesclist.first(); i; i=i->next)
			{
				if(i->dfactory==drv)
				{  delete rdesclist.dequeue(i);  }
			}
			break;
		case DTFilter:
			for(FilterDesc *i=fdesclist.first(); i; i=i->next)
			{
				if(i->dfactory==drv)
				{  delete fdesclist.dequeue(i);  }
			}
			break;
	}
	
	// Finally dequeue the driver. 
	list->dequeue(drv);
}


int ComponentDataBase::RegisterSourceFactory(TaskSourceFactory *tsf)
{
	if(!tsf)  return(0);
	
	if(!tsf->TaskSourceName())
	{  return(1);  }
	if(_FindSourceFactoryByName(tsf->TaskSourceName()))
	{  return(1);  }
	
	// Make sure that it is not queued: 
	assert(!tsources.prev(tsf) && tsources.first()!=tsf);
	tsources.append(tsf);
	
	return(0);
}


void ComponentDataBase::UnregisterSourceFactory(TaskSourceFactory *tsf)
{
	if(!tsf)  return;
	
	// Check if queued: 
	if(tsources.first()!=tsf && !tsources.prev(tsf))  return;
	
	tsources.dequeue(tsf);
}


// Read in the render and filter desc files: 
// Called by main(). 
int ComponentDataBase::ReadInDescFiles(par::ParameterSource_File *file_src)
{
	int fail=0;
	if(_ReadInRenderDescFile(file_src))
	{  ++fail;  }
	if(_ReadInFilterDescFile(file_src))
	{  ++fail;  }
	return(fail);
}


int ComponentDataBase::_ReadInRenderDescFile(par::ParameterSource_File *file_src)
{
	// First, we do no longer need (and allow to be modified) 
	// the rdesc file: 
	DelParam(_rdescfile_pi);  _rdescfile_pi=NULL;
	
	if(!rdescfile.str())
	{  return(0);  }    // no file to read in
	assert(rdescfile.stype()==0);
	
	assert(ri_section);
	int rv=file_src->ReadFile(rdescfile.str(),20,ri_section);
	return(rv);
}

int ComponentDataBase::_ReadInFilterDescFile(par::ParameterSource_File *file_src)
{
	// First, we do no longer need (and allow to be modified) 
	// the fdesc file: 
	DelParam(_fdescfile_pi);  _fdescfile_pi=NULL;
	
	if(!fdescfile.str())
	{  return(0);  }    // no file to read in
	assert(rdescfile.stype()==0);
	
	assert(fi_section);
	int rv=file_src->ReadFile(fdescfile.str(),20,fi_section);
	return(rv);
}


// Section handler parse routine:
int ComponentDataBase::parse(const Section *s,PAR::SPHInfo *info)
{
	TaskDriverType dt;
	     if(s==ri_section)  dt=DTRender;
	else if(s==fi_section)  dt=DTFilter;
	else return(1);
	
	if(s!=info->sect)  return(1);
	
	// Okay, we're getting info about a new render/filter desc, so 
	// quickly add the parameters...
	// But first, to some checking. 
	const char *xname=NULL;
	size_t xnamelen=0;
	if(info->arg)
	{
		// Okay, this is more trickier, we have to find where the 
		// name of the section ends. 
		int illegal=0;
		const char *e=NULL;
		for(const char *c=info->nend; c<info->name_end; c++)
		{
			if(isspace(*c))
			{  ++illegal;  break;  }
			if(*c=='-')
			{
				if(e)
				{  ++illegal;  break;  }
				else
				{  e=c;  }
			}
		}
		xname=info->nend;
		if(!e || illegal)
		{
			Error("in %s: Illegal %s name \"%.*s\".\n",
				info->arg->origin.OriginStr().str(),DTypeString(dt),
				e ? e-xname : 10,xname);
			return(-1);
		}
		xnamelen=e-xname;
	}
	else
	{
		// Okay, there was a `#section xyz´ ot sth like that. 
		int illegal=0;
		for(const char *c=info->nend; c<info->name_end; c++)
		{
			if(*c=='-' || isspace(*c))
			{  ++illegal;  break;  }
		}
		xname=info->nend;
		xnamelen=info->name_end-xname;
		if(info->name_end<=info->nend || illegal)
		{
			#warning fixme: cannot report <SOMEWHERE> as location...
			Error("in <SOMEWHERE - FIXME>: Illegal %s name \"%.*s\".\n",
				DTypeString(dt),
				info->name_end>xname ? info->name_end-xname : 10,xname);
			return(-1);
		}
	}
	// Okay, now we have the new name. 
	Verbose("Adding new %s description \"%.*s\".\n",
		DTypeString(dt),
		int(xnamelen),xname);
	
	RefString name;
	if(name.set0(xname,xnamelen))
	{  return(-1);  }
	
	// Okay, now we need a new description node: 
	int fail_reg=0;
	switch(dt)
	{
		case DTRender:
		{
			RenderDesc *rd=NEW<RenderDesc>();
			if(!rd)  return(-1);
			rd->name=name;
			if(_RegisterRenderDescParams(rd))
			{  delete rd;  fail_reg=1;  break;  }
			// Queue it: 
			rdesclist.append(rd);
		}  break;
		case DTFilter:
		{
			FilterDesc *fd=NEW<FilterDesc>();
			if(!fd)  return(-1);
			fd->name=name;
			if(_RegisterFilterDescParams(fd))
			{  delete fd;  fail_reg=1;  break;  }
			// Queue it: 
			fdesclist.append(fd);
		}  break;
		default:  assert(0);  break;
	}
	
	if(fail_reg)
	{
		Error("failed to register section for render/filter \"%s\".\n",
			name.str());
		return(-1);
	}
	
	return(0);  // okay, accepted. 
}


// Called for special help options: 
int ComponentDataBase::PrintSpecialHelp(RefStrList *dest,
	const SpecialHelpItem *shi)
{
	assert(shi->item_id==DTRender || 
	       shi->item_id==DTFilter );
	
	RefString tmp;
	tmp.sprintf(0,"List of available %s drivers:",
		shi->item_id==DTRender ? "render" : "filter");
	dest->append(tmp);
	
	for(const TaskDriverFactory *i=drivers[shi->item_id].first(); i; 
		i=drivers[shi->item_id].next(i))
	{
		tmp.sprintf(0,"\r2+:%s:",i->DriverName());
		dest->append(tmp);
		tmp.sprintf(0,"\r4:  %s",i->DriverDesc());
		dest->append(tmp);
	}
	
	return(0);
}

// Additional help (called via section handler) 
int ComponentDataBase::PrintSectionHelp(const Section *sect,
	RefStrList * /*dest*/,int when)
{
	if(when==-1)
	{
		// Register the parameters, so that help text on 
		// them can be written: 
		if(sect==ri_section)
		{
			assert(!_ri_help_dummy);
			if(!_RegisterRenderDescParams(NULL))
			{  _ri_help_dummy=CurrentSection();  }
		}
		else if(sect==fi_section)
		{
			assert(!_fi_help_dummy);
			if(!_RegisterFilterDescParams(NULL))
			{  _fi_help_dummy=CurrentSection();  }
		}
	}
	else if(when==+2)
	{
		// Must clean up the parameters for help text again: 
		if(sect==ri_section && _ri_help_dummy)
		{
			RecursiveDeleteParams(_ri_help_dummy);
			_ri_help_dummy=NULL;
		}
		else if(sect==fi_section && _fi_help_dummy)
		{
			RecursiveDeleteParams(_fi_help_dummy);
			_fi_help_dummy=NULL;
		}
	}
	
	return(0);
}


int ComponentDataBase::CheckParams()
{
	int failed=0;
	
	#warning FIXME!! !!!!!!!!!!!!!!!! NEED BASE CLASS FOR RenderDesc / FilterDesc !!!!!!!!!!!!!
	
	// Set up dfactory member in RenderDesc and FilterDesc: 
	for(RenderDesc *i=rdesclist.first(); i; i=i->next)
	{
		assert(i->name.str());
		const char *dname=i->_drivername.str();
		TaskDriverFactory *tdf=NULL;
		// If it already has a driver factory: 
		if(i->dfactory)  goto contR;
		if(!dname)
		{  Error("%s: Failed to specify a render driver for render desc \"%s\".\n",
			prg_name,i->name.str());  ++failed;  goto contR;  }
		tdf=_FindDriverFactoryByName(dname,DTRender);
		if(!tdf)
		{  Error("%s: Unknown render driver \"%s\" used by render desc \"%s\".\n",
			prg_name,dname,i->name.str());  ++failed;  goto contR;  }
		i->dfactory=tdf;
		contR:
		i->_drivername.set(NULL);
	}
	
	for(FilterDesc *i=fdesclist.first(); i; i=i->next)
	{
		assert(i->name.str());
		const char *dname=i->_drivername.str();
		TaskDriverFactory *tdf=NULL;
		// If it already has a driver factory: 
		if(i->dfactory)  goto contF;
		if(!dname)
		{  Error("%s: Failed to specify a filter driver for filter desc \"%s\".\n",
			prg_name,i->name.str());  ++failed;  goto contF;  }
		tdf=_FindDriverFactoryByName(dname,DTFilter);
		if(!tdf)
		{  Error("%s: Unknown filter driver \"%s\" used by filter desc \"%s\".\n",
			prg_name,dname,i->name.str());  ++failed;  goto contF;  }
		i->dfactory=tdf;
		contF:
		i->_drivername.set(NULL);
	}
	
	return(failed ? 1 : 0);
}


// Special case: if rd==NULL -> this is called for help text stuff only. 
int ComponentDataBase::_RegisterRenderDescParams(RenderDesc *rd)
{
	if(SetSection(rd ? rd->name.str() : "<name>",
		"Replace <name> with the name of the renderer "
		"(NOT of the render DRIVER)",
		ri_section))
	{  return(-1);  }
	
	add_failed=0;
	AddParam("driver",
		"driver for the renderer (use --list-rd to see available drivers)",
		&rd->_drivername,PNoDefault | STExactlyOnce);
	AddParam("binpath",
		"path to the binary, either absolute or -ri-searchpath applies",
		&rd->binpath,PNoDefault | STAtLeastOnce);
	AddParam("req_args",
		"required additional command line args passed to the renderer",
		&rd->required_args);
	AddParam("inc_path",
		"include path passed to renderer (if it supports that)",
		&rd->include_path);
	
	if(add_failed)
	{
		RecursiveDeleteParams(CurrentSection());
		return(-1);
	}
	
	return(0);
}

int ComponentDataBase::_RegisterFilterDescParams(FilterDesc *fd)
{
	if(SetSection(fd ? fd->name.str() : "<name>",
		"Replace <name> with the name of the filter "
		"(NOT of the filter DRIVER)",
		fi_section))
	{  return(-1);  }
	
	add_failed=0;
	AddParam("driver",
		"driver for the filter (use --list-fd to see available drivers)",
		&fd->_drivername,PNoDefault | STExactlyOnce);
	AddParam("binpath",
		"path to the binary, either absolute or -di-searchpath applies",
		&fd->binpath,PNoDefault | STAtLeastOnce);
	AddParam("req_args",
		"required additional command line args passed to the filter",
		&fd->required_args);
	
	if(add_failed)
	{
		RecursiveDeleteParams(CurrentSection());
		return(-1);
	}
	
	return(0);
}


static const char *_str_list_rd_opt="list-rd";
static const char *_str_list_rd_dsc="list available render drivers";
static const char *_str_list_fd_opt="list-fd";
static const char *_str_list_fd_dsc="list available filter drivers";


int ComponentDataBase::_RegisterParams()
{
	// First all those in the top section: 
	if(SetSection(NULL))
	{  return(-1);  }
	
	#warning FIXME: need STExactlyOncePerSource/STMaxOncePerSource to allow for proper overriding
	_rdescfile_pi=AddParam("rdfile","path to render description file",
		&rdescfile,PNoDefault | STMaxOnce);
	_fdescfile_pi=AddParam("fdfile","path to filter description file",
		&fdescfile,PNoDefault | STMaxOnce);
	
	// Special help: 
	SpecialHelpItem shi;
	shi.optname=_str_list_rd_opt;
	shi.descr=_str_list_rd_dsc;
	shi.item_id=DTRender;
	AddSpecialHelp(&shi);
	shi.optname=_str_list_fd_opt;
	shi.descr=_str_list_fd_dsc;
	shi.item_id=DTFilter;
	AddSpecialHelp(&shi);
	
	
	// Then the render stuff: 
	if(SetSection("ri",   // `render info'
		"Render information (render description)"))
	{  return(-1);  }
	ri_section=CurrentSection();
	
	AddParam("searchpath","search path for render binary",
		&searchpath[DTRender]);
	
	
	// Then the filter stuff: 
	if(SetSection("fi",   // `filter info'
		"Filter information (filter description)"))
	{  return(-1);  }
	fi_section=CurrentSection();
	
	AddParam("searchpath","search path for filter binary",
		&searchpath[DTFilter]);
	
	
	return(add_failed ? (-1) : 0);
}


void ComponentDataBase::_SetTaskManager(TaskManager *tm)
{
	assert(!taskman);
	taskman=tm;
}

ComponentDataBase::ComponentDataBase(par::ParameterManager *pman,int *failflag) : 
	par::ParameterConsumer_Overloaded(pman,failflag),
	par::SectionParameterHandler(pman,failflag),
	tsources(failflag),
	rdesclist(failflag),
	fdesclist(failflag),
	iflist(failflag),
	// param stuff: 
	rdescfile(failflag),fdescfile(failflag)
{
	int failed=0;
	taskman=NULL;
	searchpath=NULL;
	drivers=NULL;
	
	ri_section=fi_section=NULL;
	_ri_help_dummy=_fi_help_dummy=NULL;
	_rdescfile_pi=_fdescfile_pi=NULL;
	
	drivers=(LinkedList<TaskDriverFactory>*)
		NEWarray< LinkedList<TaskDriverFactory> >(_DTLast);
	if(!drivers)
	{  ++failed;  }
	
	searchpath=(RefStrList*)NEWarray<RefStrList>(_DTLast);
	if(!searchpath)
	{  ++failed;  }
	
	// Okay, now all that parameter stuff: 
	if(_RegisterParams())
	{  ++failed;  }
	
	// As section parameter handler, the database subscribes at 
	// rd and fd sections. 
	if(SectionParameterHandler::Attach(ri_section))
	{  ++failed;  }
	if(SectionParameterHandler::Attach(fi_section))
	{  ++failed;  }
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("CDB");  }
}

ComponentDataBase::~ComponentDataBase()
{
	// Must kill all the driver factories: 
	if(drivers)
	{
		for(int i=0; i<_DTLast; i++)
		{
			while(!drivers[i].is_empty())
			{  delete drivers[i].popfirst();  }  // will call UnregisterDriverFactory()
		}
		DELarray(drivers);
	}
	drivers=NULL;
	
	// Must also kill the source factories: 
	while(!tsources.is_empty())
	{  delete tsources.popfirst();  }
	
	// Also tidy up the search path: 
	DELarray(searchpath);
	searchpath=NULL;
	
	// Now... the rdesclist and fdesclist should be empty here. 
	// That is not the case if there are nodes without a driver. 
	// So clean them up, too...
	while(!rdesclist.is_empty())
	{  delete rdesclist.popfirst();  }
	while(!fdesclist.is_empty())
	{  delete fdesclist.popfirst();  }
}
