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


// Used only here. 
struct StaticInfoPerType
{
	const char *_dfile_arg_str;  // "rdfile" / "fdfile"
	const char *_dfile_desc_str;
	const char *_list_rd_arg_str;   // "list-rd" / "list-fd"
	const char *_list_rd_desc_str;
	const char *_info_sect_name_str;   // "ri" / "fi"
	const char *_info_sect_desc_str;
	// sh = special help
	const char *_sh_sect_desc_str;
	const char *_sh_driver_desc_str;
};

// Static const info per type: 
static StaticInfoPerType static_ift[_DTLast]=
{
	// DTRender: 
	{
		"rdfile",
		"path to render description file",
		"list-rd",
		"list available render drivers",
		"ri",
		"Render information (render description)",
		"Replace <name> with the name of the renderer (NOT of the render DRIVER)",
		"driver for the renderer (use --list-rd to see available drivers)"
	},
	// DTFilter:
	{
		"fdfile",
		"path to filter description file",
		"list-fd",
		"list available filter drivers",
		"fi",
		"Filter information (filter description)",
		"Replace <name> with the name of the filter (NOT of the filter DRIVER)",
		"driver for the filter (use --list-fd to see available drivers)"
	}
};



TaskDriverFactory *ComponentDataBase::_FindDriverFactoryByName(
	const char *name,TaskDriverType dtype)
{
	if(dtype<0 || dtype>=_DTLast)
	{  return(NULL);  }
	size_t nlen=strlen(name);
	InfoPerType *dti=&ift[dtype];
	for(TaskDriverFactory *n=dti->drivers.first(); n; n=dti->drivers.next(n))
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


const RF_DescBase *ComponentDataBase::_FindDescByName(const char *name,
	TaskDriverType dtype)
{
	if(!name || dtype<0 || dtype>=_DTLast)  return(NULL);
	
	InfoPerType *dti=&ift[dtype];
	size_t slen=strlen(name);
	for(RF_DescBase *i=dti->desclist.first(); i; i=i->next)
	{
		if(i->name.len()!=slen)  continue;
		if(strcmp(i->name.str(),name))  continue;
		return(i);
	}
	
	return(NULL);
}


const ImageFormat *ComponentDataBase::FindImageFormatByName(const char *name)
{
	if(!name)  return(NULL);
	
	size_t slen=strlen(name);
	for(ImageFormat *i=iflist.first(); i; i=i->next)
	{
		if(strlen(i->name)!=slen)  continue;
		if(strcasecmp(i->name,name))  continue;
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
	
	InfoPerType *dti=&ift[dtype];
	assert(!dti->drivers.prev(drv) && dti->drivers.first()!=drv);
	dti->drivers.append(drv);
	
	return(0);
}


void ComponentDataBase::UnregisterDriverFactory(
	TaskDriverFactory *drv,TaskDriverType dtype)
{
	if(!drv)  return;
	if(dtype<0 || dtype>=_DTLast)  return;
	
	InfoPerType *dti=&ift[dtype];
	
	// Check if queued: 
	if(dti->drivers.first()!=drv && !dti->drivers.prev(drv))  return;
	
	// Delete all render/filter descriptions using this driver: 
	// FIXME: We should not delete the descriptions (or at lease assert()) 
	//        in case the RederDesc in question is still in use. 
	//        Currently this cannot be detected (and will not happen 
	//        as UnregisterDriverFactory() is only called on cleanup). 
	for(RF_DescBase *i=dti->desclist.first(); i; i=i->next)
	{
		if(i->dfactory==drv)
		{  delete dti->desclist.dequeue(i);  }
	}
	
	// Finally dequeue the driver. 
	dti->drivers.dequeue(drv);
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


int ComponentDataBase::RegisterImageFormat(ImageFormat *ifmt)
{
	if(!ifmt)  return(0);
	
	if(!ifmt->name)
	{  return(1);  }
	if(FindImageFormatByName(ifmt->name))
	{  return(1);  }
	
	assert(!iflist.prev(ifmt) && iflist.first()!=ifmt);
	iflist.append(ifmt);
	
	return(0);
}


// Read in the render and filter desc files: 
// Called by main(). 
int ComponentDataBase::ReadInDescFiles(par::ParameterSource_File *file_src)
{
	int fail=0;
	
	for(int dtype=0; dtype<_DTLast; dtype++)
	{
		InfoPerType *dti=&ift[dtype];
		
		// First, we do no longer need (and allow to be modified) 
		// the desc file: 
		DelParam(dti->_descfile_pi);  dti->_descfile_pi=NULL;
		
		if(!dti->descfile.str())  continue;   // no file to read in
		assert(dti->descfile.stype()==0);
		
		Verbose("Reading in %s description file.\n",
			DTypeString((TaskDriverType)dtype));
		
		assert(dti->i_section);
		int rv=file_src->ReadFile(dti->descfile.str(),20,dti->i_section);
		if(rv)
		{  ++fail;  }
	}
	
	return(fail);
}


// Section handler parse routine:
int ComponentDataBase::parse(const Section *s,PAR::SPHInfo *info)
{
	TaskDriverType dt=DTNone;
	for(int i=0; i<_DTLast; i++)
	{
		if(ift[i].i_section==s)
		{  dt=(TaskDriverType)i;  break;  }
	}
	if(dt==DTNone)  return(1);
	
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
		// Okay, there was a `#section xyz� ot sth like that. 
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
	RF_DescBase *d=NULL;
	switch(dt)
	{
		case DTRender:  d=NEW<RenderDesc>();  break;
		case DTFilter:  d=NEW<FilterDesc>();  break;
		default:  assert(0);  break;
	}
	d->name=name;
	if(_RegisterDescParams(dt,d))
	{  delete d;  fail_reg=1;  }
	else  // Queue it: 
	{  ift[dt].desclist.append(d);  }
	
	if(fail_reg)
	{
		Error("failed to register section for %s \"%s\".\n",
			DTypeString(dt),name.str());
		return(-1);
	}
	
	return(0);  // okay, accepted. 
}


// Called for special help options: 
int ComponentDataBase::PrintSpecialHelp(RefStrList *dest,
	const SpecialHelpItem *shi)
{
	if(shi->item_id>=0 && shi->item_id<_DTLast)
	{
		// List render / filter drivers: 
		
		RefString tmp;
		tmp.sprintf(0,"List of available %s drivers:",
			DTypeString((TaskDriverType)shi->item_id));
		dest->append(tmp);
		
		for(const TaskDriverFactory *i=ift[shi->item_id].drivers.first(); i; 
			i=ift[shi->item_id].drivers.next(i))
		{
			tmp.sprintf(0,"\r2+:%s:",i->DriverName());
			dest->append(tmp);
			tmp.sprintf(0,"\r4:  %s",i->DriverDesc());
			dest->append(tmp);
		}
	}
	else if(shi->item_id==-1)
	{
		// List image formats:
		RefString tmp;
		tmp.set("List of knwon image formats:");  dest->append(tmp);
		tmp.set("\r2:name  bpp  ext");  dest->append(tmp);
		
		for(const ImageFormat *i=iflist.first(); i; i=i->next)
		{
			tmp.sprintf(0,"\r2:%-4s  %3d  %-3s",
				i->name,i->bitspp,i->file_extension);
			dest->append(tmp);
		}
	}
	else assert(0);
	
	return(0);
}

// Additional help (called via section handler) 
int ComponentDataBase::PrintSectionHelp(const Section *sect,
	RefStrList * /*dest*/,int when)
{
	for(int dtype=0; dtype<_DTLast; dtype++)
	{
		InfoPerType *dti=&ift[dtype];
		if(sect!=dti->i_section)  continue;
		
		if(when==-1)
		{
			// Register the parameters, so that help text on 
			// them can be written: 
			assert(!dti->_i_help_dummy);
			if(!_RegisterDescParams((TaskDriverType)dtype,NULL))
			{  dti->_i_help_dummy=CurrentSection();  }
		}
		else if(when==+2)
		{
			// Must clean up the parameters for help text again: 
			if(dti->_i_help_dummy)
			{
				RecursiveDeleteParams(dti->_i_help_dummy);
				dti->_i_help_dummy=NULL;
			}
		}
	}
	
	return(0);
}


int ComponentDataBase::CheckParams()
{
	int err_failed=0;
	int warn_failed=0;
	
	Verbose("Final desc setup: ");
	static const char *_vfailed_str="failure]\n";
	
	for(int _dtype=0; _dtype<_DTLast; _dtype++)
	{
		TaskDriverType dtype=(TaskDriverType)_dtype;
		
		if(!err_failed && !warn_failed)
		{  Verbose("[%s: ",DTypeString(dtype));  }
		
		InfoPerType *dti=&ift[dtype];
		
		// Set up dfactory member in RenderDesc / FilterDesc: 
		for(RF_DescBase *i=dti->desclist.first(); i; i=i->next)
		{
			assert(i->name.str());
			
			// If it already has a driver factory: 
			if(i->dfactory)  continue;
			
			const char *dname=i->_drivername.str();
			if(!dname)
			{
				if(!err_failed && !warn_failed)  Verbose(_vfailed_str);
				Error("Failed to specify a %s driver for %s desc \"%s\".\n",
					DTypeString(dtype),DTypeString(dtype),i->name.str());
				++err_failed;
				continue;
			}
			
			TaskDriverFactory *tdf=_FindDriverFactoryByName(dname,dtype);
			if(!tdf)
			{
				if(!err_failed && !warn_failed)  Verbose(_vfailed_str);
				Warning("Unknown %s driver \"%s\" used by %s desc \"%s\" (deleted).\n",
					DTypeString(dtype),dname,DTypeString(dtype),i->name.str());
				++warn_failed;
				continue;
			}
			
			// Set the factory (that's why we're doing this loop after all): 
			i->dfactory=tdf;
		}
		
		// Kill entries without driver factory. 
		// Also remove _drivername as it's no longer needed. 
		int nent=0;
		for(RF_DescBase *_i=dti->desclist.first(); _i; )
		{
			RF_DescBase *i=_i;
			_i=_i->next;
			
			if(!i->dfactory)
			{
				// Kill entry: 
				delete dti->desclist.dequeue(i);
				continue;
			}
			i->_drivername.set(NULL);
			++nent;
		}
		
		if(!err_failed && !warn_failed)
		{  Verbose("%d entries] ",nent);  }
	}
	
	if(!err_failed && !warn_failed)
	{  Verbose("OK\n");  }
	
	return(err_failed ? 1 : 0);
}


// Special case: if rd==NULL -> this is called for help text stuff only. 
int ComponentDataBase::_RegisterDescParams(TaskDriverType dtype,RF_DescBase *d)
{
	assert(!d || d->dtype==dtype);
	
	InfoPerType *dti=&ift[dtype];
	StaticInfoPerType *sdti=&static_ift[dtype];
	
	if(SetSection(d ? d->name.str() : "<name>",
		sdti->_sh_sect_desc_str,
		dti->i_section))
	{  return(-1);  }
	
	add_failed=0;
	
	// Common params: 
	AddParam("driver",
		sdti->_sh_driver_desc_str,
		&d->_drivername,PNoDefault | STExactlyOnce);
	
	// Special (non-common) params: 
	switch(dtype)
	{
		case DTRender:  _RegisterRenderDescParams((RenderDesc *)d);  break;
		case DTFilter:  _RegisterFilterDescParams((FilterDesc *)d);  break;
		default:  assert(0);  break;
	}
	
	if(add_failed)
	{
		RecursiveDeleteParams(CurrentSection());
		return(-1);
	}
	
	return(0);
}


// Special case: if rd==NULL -> this is called for help text stuff only. 
// Failure is indicated by increased add_failed value. 
void ComponentDataBase::_RegisterRenderDescParams(RenderDesc *rd)
{
	AddParam("binpath",
		"path to the binary, either absolute or -ri-searchpath applies",
		rd ? &rd->binpath : NULL,PNoDefault | STAtLeastOnce);
	AddParam("req_args",
		"required additional command line args passed to the renderer",
		rd ? &rd->required_args : NULL);
	AddParam("inc_path",
		"include path passed to renderer (if it supports that)",
		rd ? &rd->include_path : NULL);
}

void ComponentDataBase::_RegisterFilterDescParams(FilterDesc *fd)
{
	AddParam("binpath",
		"path to the binary, either absolute or -di-searchpath applies",
		fd ? &fd->binpath : NULL,PNoDefault | STAtLeastOnce);
	AddParam("req_args",
		"required additional command line args passed to the filter",
		fd ? &fd->required_args : NULL);
}


int ComponentDataBase::_RegisterParams(TaskDriverType dtype)
{
	InfoPerType *dti=&ift[dtype];
	StaticInfoPerType *sdti=&static_ift[dtype];
	
	#warning FIXME: need STExactlyOncePerSource/STMaxOncePerSource to allow for proper overriding
	dti->_descfile_pi=AddParam(sdti->_dfile_arg_str,sdti->_dfile_desc_str,
		&dti->descfile,PNoDefault | STMaxOnce);
	
	// Special help: 
	SpecialHelpItem shi;
	shi.optname=sdti->_list_rd_arg_str;
	shi.descr=sdti->_list_rd_desc_str;
	shi.item_id=dtype;
	AddSpecialHelp(&shi);
	
	
	if(SetSection(sdti->_info_sect_name_str,sdti->_info_sect_desc_str))
	{  return(-1);  }
	dti->i_section=CurrentSection();
	
	// Parameters which are the same for render and filter: 
	AddParam("searchpath","search path for program binary",&dti->searchpath);
	
	// As section parameter handler, the database subscribes at 
	// rd and fd sections. 
	if(SectionParameterHandler::Attach(dti->i_section))
	{  ++add_failed;  }
	
	return(add_failed ? (-1) : 0);
}


int ComponentDataBase::_RegisterParams()
{
	int failed=0;
	
	// First all those in the top section: 
	if(SetSection(NULL))
	{  return(-1);  }
	
	// insert some params for top section...
	
	if(_RegisterParams(DTRender))  ++failed;
	if(_RegisterParams(DTFilter))  ++failed;
	
	// Special option to list image formats: 
	SpecialHelpItem shi;
	static const char *_list_imgfmt_opt_str="list-imgfmt";
	static const char *_list_imgfmt_dest_str="list known image formats";
	shi.optname=_list_imgfmt_opt_str;
	shi.descr=_list_imgfmt_dest_str;
	shi.item_id=-1;
	AddSpecialHelp(&shi);
	
	if(add_failed)  ++failed;
	return(failed);
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
	iflist(failflag)
{
	int failed=0;
	taskman=NULL;
	ift=NULL;
	
	ift=(InfoPerType*)NEWarray<InfoPerType>(_DTLast);
	if(!ift)
	{  ++failed;  }
	
	// Okay, now all that parameter stuff: 
	if(!failed)
	{
		if(_RegisterParams())
		{  ++failed;  }
	}
	
	if(failflag)
	{  *failflag-=failed;  }
	else if(failed)
	{  ConstructorFailedExit("CDB");  }
}

ComponentDataBase::~ComponentDataBase()
{
	//Verbose("Cleanup: ");
	Verbose(".");
	
	if(ift) for(int i=0; i<_DTLast; i++)
	{
		//Verbose("[%s drivers] ",DTypeString((TaskDriverType)i));
		Verbose(".");
		InfoPerType *dti=&ift[i];
		
		// Must kill all the driver factories: 
		while(!dti->drivers.is_empty())
		{  delete dti->drivers.popfirst();  }  // will call UnregisterDriverFactory()
		
		// Now... the rdesclist and fdesclist should be empty here. 
		// That is not the case if there are nodes without a driver. 
		// So clean them up, too...
		while(!dti->desclist.is_empty())
		{  delete dti->desclist.popfirst();  }
	}
	DELarray(ift);
	ift=NULL;
	
	// Must also kill the source factories: 
	//Verbose("[task sources] ");
	Verbose(".");
	while(!tsources.is_empty())
	{  delete tsources.popfirst();  }
	
	// ...and the image formats. 
	//Verbose("[image formats] ");
	Verbose(".");
	while(!iflist.is_empty())
	{  delete iflist.popfirst();  }
	
	//Verbose("OK\n");
}


/******************************************************************************/

ComponentDataBase::InfoPerType::InfoPerType(int *failflag) : 
	drivers(failflag),
	desclist(failflag),
	searchpath(failflag),
	descfile(failflag)
{
	i_section=NULL;
	_descfile_pi=NULL;
	_i_help_dummy=NULL;
}

ComponentDataBase::InfoPerType::~InfoPerType()
{
	
}


/******************************************************************************/

RF_DescBase::RF_DescBase(int *failflag) : 
	name(failflag),
	_drivername(failflag)
{
	dfactory=NULL;
}

RF_DescBase::~RF_DescBase()
{
	dfactory=NULL;  // be sure
}