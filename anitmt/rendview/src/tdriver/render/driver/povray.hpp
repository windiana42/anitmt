/*
 * povray.hpp
 * 
 * Header for POVRay renderer driver. 
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


#include "../../../database.hpp"


class POVRayDriverFactory : 
	public TaskDriverFactory
{
	friend class POVRayDriver;
	private:
		int verbose;
	
	public:  _CPP_OPERATORS_FF
		POVRayDriverFactory(ComponentDataBase *cdb,int *failflag=NULL);
		~POVRayDriverFactory();
		
		// Called on program start to set up the POVRayDriverFactory; 
		// POVRayDriverFactory registers at ComponentDataBase. 
		// Return value: 0 -> OK; >0 -> error. 
		static int init(ComponentDataBase *cdb);
		
		// Called to check and set up some fields in the desc; see 
		// TaskDriver (gdriver.hpp): [overriding virtual]
		int CheckDesc(RF_DescBase *d);
		
		// Create a task driver: 
		TaskDriver *Create(TaskDriverInterface_Local *tdif);
		
		// Overriding virtual: 
		const char *DriverDesc() const;
};


class POVRayDriver : 
	public RenderDriver
{
	private:
		// Parameter/Settings pointer: settings only allocated once 
		// and can be modified by command line, etc. 
		// They can be accessed via (POVRayDriverFactory *)f;
		// (*f being member of TaskDriver). 
		
		POVRayDriverFactory *settings()
			{  return((POVRayDriverFactory *)f);  }
		
		int _FillOutputFormat(char *dest,int len,const ImageFormat *fmt);
	protected:
		// Overriding virtual; gets called by TaskDriver on an error 
		// or to notify driver of things (for verbose messages): 
		// Return value: currently ignored; use 0. 
		int ProcessError(ProcessErrorInfo *pei);
	public:  _CPP_OPERATORS_FF
		POVRayDriver(POVRayDriverFactory *pf,TaskDriverInterface_Local *tdif,int *failflag=NULL);
		~POVRayDriver();
		
		// Overriding virtual: 
		int Execute(const RenderTask *rt,const RenderTaskParams *rtp);
};

