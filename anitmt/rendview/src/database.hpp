/*
 * database.hpp
 * 
 * Component (renderers, filters, ...) database header. 
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

#ifndef _RNDV_DATABASE_HPP_
#define _RNDV_DATABASE_HPP_ 1

// Include all the code needed: 
#include <lib/taskmanagement.h>

// Generic TaskDriver stuff: 
#include "gdriver.hpp"

// TaskSourceFactory: 
#include "tsource/tsfactory.hpp"


class ImageFormat;
class RenderDesc;
class FilterDesc;


class ComponentDataBase : 
	par::ParameterConsumer_Overloaded,
	par::SectionParameterHandler
{
	private:
		// Pointer to the task manager (be careful: initially NULL). 
		TaskManager *taskman;
		
		LinkedList<TaskDriverFactory> *drivers;  // array: [_DTLast]
		
		LinkedList<TaskSourceFactory> tsources;
		
		LinkedList<RenderDesc> rdesclist;
		LinkedList<FilterDesc> fdesclist;
		LinkedList<ImageFormat> iflist;
		
		RefStrList *searchpath;  // render/filter search path
		
		// Parameter stuff: 
		Section *ri_section,*fi_section;  // render/filter desc section
		RefString rdescfile,fdescfile;
		ParamInfo *_rdescfile_pi,*_fdescfile_pi;
		
		Section *_ri_help_dummy,*_fi_help_dummy;
		
		// Return driver factory with specified type & name 
		// or NULL (exact match). 
		TaskDriverFactory *_FindDriverFactoryByName(
			const char *name,TaskDriverType dtype);
		TaskSourceFactory *_FindSourceFactoryByName(const char *name);
		
		int _RegisterParams();
		int _RegisterRenderDescParams(RenderDesc *rd);
		int _RegisterFilterDescParams(FilterDesc *fd);
		
		// Used by ReadInDescFiles(): 
		int _ReadInRenderDescFile(par::ParameterSource_File *file_src);
		int _ReadInFilterDescFile(par::ParameterSource_File *file_src);
	protected:
		// Called for special help options: 
		int PrintSpecialHelp(RefStrList *dest,const SpecialHelpItem *shi);
		// Called for special help via seciton handler: 
		int PrintSectionHelp(const Section *sect,RefStrList *dest,int when);
		// Section handler parse routine:
		int parse(const Section *s,SPHInfo *info);
		// Called by parameter manager: 
		int CheckParams();
	public:  _CPP_OPERATORS_FF
		ComponentDataBase(par::ParameterManager *pman,int *failflag=NULL);
		~ComponentDataBase();
		// Used by TaskManager on startup: 
		void _SetTaskManager(TaskManager *tm);
		
		// Get managers: 
		par::ParameterManager *parmanager()
			{  return(ParameterConsumer::parmanager());  }
		TaskManager *taskmanager()
			{  return(taskman);  }
		
		// Called by main to read in the render and filter descriptions: 
		// Return value: 0 -> OK; !=0 -> failure
		int ReadInDescFiles(par::ParameterSource_File *file_src);
		
		// Return driver factory with specified type & name 
		// or NULL (exact match). 
		TaskDriverFactory *FindDriverFactoryByName(const char *name,TaskDriverType _dtype)
			{  return(_FindDriverFactoryByName(name,_dtype));  }
		
		// Return task source factory with specified name or NULL: 
		TaskSourceFactory *FindSourceFactoryByName(const char *name)
			{  return(_FindSourceFactoryByName(name));  }
		
		const RenderDesc *FindRenderDescByName(const char *name);
		const FilterDesc *FindFilterDescByName(const char *name);
		
		// Used by task drivers to get binary search path (one per 
		// task driver type). 
		const RefStrList *GetBinSearchPath(TaskDriverType dtype)
			{  return((dtype<0 || dtype>=_DTLast) ? NULL : &(searchpath[dtype]));  }
		
		// Used by RenderDriver/FilterDriver to (un)register at the 
		// database. 
		// Return value: 
		//  0 -> OK
		//  1 -> driver with that name already registered
		// -1 -> allocation failure 
		// -2 -> illegal TaskDriverType 
		int RegisterDriverFactory(TaskDriverFactory *drv,TaskDriverType tdt);
		void UnregisterDriverFactory(TaskDriverFactory *drv,TaskDriverType tdt);
		
		// Used by TaskSource factories to (un)register at the 
		// database. 
		// Return value: 
		//  0 -> OK
		//  1 -> source with that name already registered
		// -1 -> allocation failure 
		int RegisterSourceFactory(TaskSourceFactory *tsf);
		void UnregisterSourceFactory(TaskSourceFactory *tsf);
};

#include "imgfmt/imgfmt.hpp"
#include "render/render.hpp"
#include "filter/filter.hpp"

#endif  /* _RNDV_DATABASE_HPP_ */
