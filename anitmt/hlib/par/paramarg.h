/*
 * paramarg.h
 * 
 * Basic parameter argument as read in from parameter sources. 
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

#ifndef _INC_PAR_ParamArg_H_
#define _INC_PAR_ParamArg_H_ 1

#include <stddef.h>
#include <hlib/refstring.h>

namespace par
{

// Represents cmd line args and file parameters
class ParamArg
{
	public:
		enum Type
		{
			Unknown=0,
			Filename,
			Assignment,
			Option
		};
		enum OriginType 
		{
			_FromNowhere=0,   // Special value; used internally 
			FromFile,
			FromCmdLine
		};
		
		static Type Classify(const char *cmd_arg);
	private:
		// Internal stuff: 
		void _Cleanup();
		void _Assign(const ParamArg &src);
	public:  _CPP_OPERATORS_FF
		// Valid for all args/pars: 
		RefString arg;    // complete arg string
		const char *name;   // points to mem inside arg after leading `-'
		size_t namelen;     // length of name in *name. 
		const char *value;  // points to next char after `=' or NULL 
		char assmode;       // '\0', '+' or '-'. 
		int pdone;         // 1 -> already done (parsed in...) 
		                   // 0 -> not done; -1 -> error 
		Type atype;        // File parameters always have Assignment type. 
		
		struct Origin
		{  _CPP_OPERATORS_FF
			// Origin type (where the arg comes from): 
			OriginType otype;
			RefString origin;  // file name; NULL for cmd line arg 
			int opos;          // file: line number; cmd line: arg number 
			
			// Convert origin spec to string: 
			RefString OriginStr() const;
			
			// (may fail)
			Origin(OriginType _otype,const char *_origin,int _opos,
				int *failflag=NULL) : origin(_origin,failflag) 
				{  otype=_otype;  opos=_opos;  }
			// (will never fail)
			Origin(OriginType _otype,RefString &_origin,int _opos,
				int * /*failflag*/=NULL) : origin(_origin)
				{  otype=_otype;  opos=_opos;  }
			// (will never fail)
			Origin(const Origin &src) : origin(src.origin)
				{  otype=src.otype;  opos=src.opos;  }
			~Origin()  { }
		} origin;
		
		// Only set for cmd line args (otherwise NULL): 
		ParamArg *next;   // initially NULL
		
	
	// Copy it: 
	void Assign(const ParamArg &src)
		{  _Cleanup();  _Assign(src);  }
	
	// Copy it: 
	ParamArg(const ParamArg &src) : arg(src.arg), origin(src.origin)
		{  _Assign(src);  }
	// Construct a cmd line arg (cmd_arg copied; 
	// arg number passed in argnum): [May fail: RefString alloc for arg]
	ParamArg(const char *cmd_arg,int argnum,int *failflag=NULL);
	// Construct a file par (par always copied; file just 
	// referenced): [May fail: RefString alloc for par]
	ParamArg(const char *par,const RefString &file,int line,int *failflag=NULL);
	// NULL constructor to be used to build arrays; use with care: 
	ParamArg(int *failflag=NULL);  // never fails
	// Destructor...
	~ParamArg();
};

}  // namespace end 

#endif  /* _INC_PAR_ParamArg_H_ */
