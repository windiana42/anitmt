/*
 * generic.hpp
 * 
 * Header for generic filter driver. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "../../../database.hpp"


class GenericFilterDriverFactory : 
	public TaskDriverFactory
{
	private:
		int verbose;
		
	public:  _CPP_OPERATORS_FF
		GenericFilterDriverFactory(ComponentDataBase *cdb,int *failflag=NULL);
		~GenericFilterDriverFactory();
		
		// Called on program start to set up the GenericFilterDriverFactory; 
		// driver factory registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Called to check and set up some fields in the desc; see 
		// TaskDriver (gdriver.hpp): [overriding virtual]
		int CheckDesc(RF_DescBase *d);
		
		// Create a task driver: 
		TaskDriver *Create(TaskDriverInterface_Local *tdif);
		
		const char *DriverDesc() const;
};


class GenericFilterDriver : 
	public FilterDriver
{
	private:
		
		// Overriding virtual: 
		int ProcessError(ProcessErrorInfo*);
	public:  _CPP_OPERATORS_FF
		GenericFilterDriver(GenericFilterDriverFactory *gf,TaskDriverInterface_Local *tdif,int *failflag=NULL);
		~GenericFilterDriver();
		
		// Overriding virtual: 
		int Execute(const FilterTask *rt,const FilterTaskParams *rtp);
};
