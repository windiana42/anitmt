/*
 * secthdl.h
 * 
 * Header file containing the section parameter handler. 
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

#include "parmanager.h"

#ifndef _INC_PAR_SectionParameterHandler_H_
#define _INC_PAR_SectionParameterHandler_H_ 1

namespace par
{

// A section parameter handler is a handler which is able to deal with all 
// parameters belonging to a section without necessarily knowing about them. 
// It has funcktions called for all parameters (which have names that 
// indicate that it is) in some specified section and you can do your 
// own parameter handling if you like. E.g. it allows you to deal with 
// previously unknown parameters or handle unrecognized parameters 
// differently. 
// A section parameter handler is NOT called for parameters recognized 
// by the parameter system, that is, if an argument is encountered which 
// matches some parameter registered by a parameter consumer, then the 
// parameter handler's parsing function is called but NO section handler. 
// NOTE: There may be max. one section handler per section; if you need 
//       more you have to provide your own sub-handlers using the one 
//       available section handler. 
class SectionParameterHandler : public PAR
{
	friend class ParameterManager;
	private:
		ParameterManager *manager;
	protected:
		// This is called for unknown parameters. 
		// First this is called for the section handler of the bottom most 
		// section (which could be determined by parsing the parameter name) 
		// and then the hierarchy up till the top section or till a section 
		// handler accepts the parameter. 
		// Return value: 
		//   0 -> accept the parameter, parsed it. 
		//   1 -> parameter was not accepted, go one level up. 
		//  -1 -> some sort of error; stop here and do not go one level up
		virtual int parse(SPHInfo *info)
			{  return(1);  }
		
		// This gets called twice if help on the section is to be written. 
		// Refer to ParameterConsumer on how to store the help text in the 
		// passed string list. 
		// Parameters: 
		//   Section *:  which section we're talking about
		//   RefStrList *: store the data to be written here. 
		//   int when: 
		//       -1 -> before the help text for the parameters in the 
		//             section is written (i.e. directly below the section 
		//             header)
		//       +1 -> after the parameters before traversing the subsections
		//       +2 -> after traversing the subesections 
		//  NOTE: Check for `-1´ and `+1´, ... , because more values may 
		//        be implemented later. 
		// Return value: currently ignored; use 0. 
		virtual int PrintSectionHelp(const Section *,RefStrList *,int /*when*/)
			{  return(0);  }
	public:  _CPP_OPERATORS_FF
		SectionParameterHandler(ParameterManager *_manager,int * /*failflag*/=NULL)
			{  manager=_manager;  }
		virtual ~SectionParameterHandler()
			{  DetachAll();  }
		
		// Get pointer to parameter manager: 
		inline ParameterManager *parmanager()
			{  return(manager);  }
		
		// Attach to a section. One section handler can be attached to 
		// several sections but not more than one section handler can 
		// attach to a given section. 
		// Return value: 
		//  0 -> success
		// -1 -> section not found (or NULL)
		// -2 -> there is already a section handler attached to that section
		int Attach(Section *s)
			{  return(parmanager()->SectionHandlerAttach(this,s));  }
		int Attach(const char *section,Section *top=NULL);
		
		// Detach from a section or from all sections. 
		void Detach(Section *s)
			{  if(s) parmanager()->SectionHandlerDetach(this,s);  }
		void DetachAll()
			{  parmanager()->SectionHandlerDetach(this,NULL);  }
};

}  // end of namespace par

#endif  /* _INC_PAR_SectionParameterHandler_H_ */
