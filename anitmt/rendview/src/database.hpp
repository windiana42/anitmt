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
#include "tdriver/local/tdriver.hpp"

// TaskSourceFactory: 
#include "tsource/tsfactory.hpp"

// TaskDriverInterfaceFactory: 
#include "tdriver/tdfactory.hpp"


class ImageFormat;
struct RenderDesc;
struct FilterDesc;

// Base class for RenderDesc and FilterDesc: 
struct RF_DescBase : LinkedListBase<RF_DescBase>
{
	TaskDriverType dtype;
	
	// Description name (for render desc, e.g. povray3.1g): 
	RefString name;
	
	// driver factory for the renderer 
	// There may be several descriptions pointing to one 
	// driver (e.g. there is a povray driver and a desc for 
	// povray-3.0, povray-3.1 and povray-3.1g). 
	TaskDriverFactory *dfactory;
	
	// This is only used by parameter / argument system and contains a NULL 
	// ref after initialisation. 
	RefString _drivername;
	
	_CPP_OPERATORS_FF
	RF_DescBase(int *failflag=NULL);
	virtual ~RF_DescBase();
};


class ComponentDataBase : 
	par::ParameterConsumer_Overloaded,
	par::SectionParameterHandler
{
	private:
		// Pointer to the task manager (be careful: initially NULL). 
		TaskManager *taskman;
		
		struct InfoPerType
		{
			LinkedList<TaskDriverFactory> drivers;
			LinkedList<RF_DescBase> desclist;
			RefStrList searchpath;
			
			// Parameter stuff: 
			Section *i_section;  // render/filter desc section
			RefString descfile;  // render/filter desc file arg
			ParamInfo *_descfile_pi;  // descfile param info
			Section *_i_help_dummy;   // internal use for special help
			
			_CPP_OPERATORS_FF
			InfoPerType(int *failflag=NULL);
			~InfoPerType();
		} *ift;   // array: [_DTLast]
		
		// List of available task source factories: 
		LinkedList<TaskSourceFactory> tsources;
		
		// List of known image formats: 
		LinkedList<ImageFormat> iflist;
		
		// List of known TaskDriverInterface factories:  
		LinkedList<TaskDriverInterfaceFactory> tdinterfaces;
		
		// Return driver factory with specified type & name 
		// or NULL (exact match). 
		TaskDriverFactory *_FindDriverFactoryByName(
			const char *name,TaskDriverType dtype);
		const RF_DescBase *_FindDescByName(
			const char *name,TaskDriverType _dtype);
		TaskSourceFactory *_FindSourceFactoryByName(const char *name);
		TaskDriverInterfaceFactory *_FindDriverInterfaceFactoryByName(const char *name);
		
		int _RegisterParams();
		int _RegisterParams(TaskDriverType dtype);
		int _RegisterDescParams(TaskDriverType dtype,RF_DescBase *d);
		void _RegisterRenderDescParams(RenderDesc *rd);
		void _RegisterFilterDescParams(FilterDesc *fd);
		
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
		
		// Get render / filter description: 
		const RenderDesc *FindRenderDescByName(const char *name)
			{  return((RenderDesc*)_FindDescByName(name,DTRender));  }
		const FilterDesc *FindFilterDescByName(const char *name)
			{  return((FilterDesc*)_FindDescByName(name,DTFilter));  }
		
		// Return task source factory with specified name or NULL: 
		TaskSourceFactory *FindSourceFactoryByName(const char *name)
			{  return(_FindSourceFactoryByName(name));  }
		
		// Return image format with specified name (not case sensitive): 
		const ImageFormat *FindImageFormatByName(const char *name);
		
		// Return TaskDriverInterface (e.g. local, LDR [server]): 
		TaskDriverInterfaceFactory *FindDriverInterfaceFactoryByName(const char *name)
			{  return(_FindDriverInterfaceFactoryByName(name));  }
		
		// Used by task drivers to get binary search path (one per 
		// task driver type). 
		const RefStrList *GetBinSearchPath(TaskDriverType dtype)
			{  return((dtype<0 || dtype>=_DTLast) ? NULL : &(ift[dtype].searchpath));  }
		
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
		
		// Used by image format init routine to initalize image formats. 
		// There is no unregister function as the formats are deleted by 
		// the destructor of ComponentDataBase. 
		// Return value: as usual 
		int RegisterImageFormat(ImageFormat *ifmt);
		
		// Used by TaskDriverInterfaceFactory just in the way as by 
		// other factories. 
		int RegisterDriverInterfaceFactory(TaskDriverInterfaceFactory *f);
		void UnregisterDriverInterfaceFactory(TaskDriverInterfaceFactory *f);
};

#include "imgfmt/imgfmt.hpp"
#include "tdriver/render/render.hpp"
#include "tdriver/filter/filter.hpp"

#endif  /* _RNDV_DATABASE_HPP_ */
