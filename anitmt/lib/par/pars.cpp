/*
 * pars.cpp
 * 
 * Implementation of routines dealing with command line 
 * options, parameters in files etc. 
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser
 * Bugs, suggestions to wwieser@gmx.de. 
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 * 
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 * 
 * This program is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the 
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.
 * 
 * See the GNU General Public License for details.
 * If you have not received a copy of the GNU General Public License,
 * write to the 
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * Revision History:
 *   Jan 2001   started writing
 *
 */

// DESCRIPTION: ADDING A NEW PARAMETER. 
// 1) Add the parameter to the ``enum Parameter_ID'' (file pars.hpp). 
//    Be sure to add it to the correct section (according to the type). 
// 2) Add a new description of the parameter to the desc_<type> list 
//    in this file. 
// 3) Add a method to access the parameter to class Animation_Parameters. 

#include "pars.hpp"

namespace anitmt
{

namespace PAT
{
	enum Parameter_Argtype
	{
		// Argument is always needed (must be 0)
		// (For stringlist type, this means: add arguments) 
		Needed=0,
		// This is valid for parameters fo type stringlist. 
		// It means (in contrast to Needed above): argument 
		// is needed but the list gets erased before the 
		// new args get added. 
		Replace,
		// If the arg is left away it defaults to true/false
		// These are only valid for parameters of type bool. 
		Def_True,
		Def_False,
		// If the arg is left away, the value is incremented 
		// Tis is only valid for parameters of type int. 
		Increment
	};
}

namespace POT
{
	enum Parameter_Overridetype
	{
		Simple=0,       // simple parameter overriding (e.g. width)
		Needs_Solver    // seeds solver (e.g. [start/end][time/frame], fps
		// NOTE: For Needs_Solver arguments, you have to add code to the 
		//       solver in cmdline.cpp. 
	};
};

const int MaxSynonymes=4;

template<class T> struct Parameter_Description
{
	// Parameter_ID value (see above). 
	PID::Parameter_ID id;
	// names (cmd line arg, config file identifier,...)
	// The first one is the primary one; all others 
	// are synonymes, the LAST MUST BE NULL. 
	const char *names[MaxSynonymes];
	// Default value for the parameter. 
	AParameter<T> defaultval;   // default value (may be not set)
	// Argument type (see the enum above). 
	PAT::Parameter_Argtype argtype;
	// Override type (see the enum above). 
	POT::Parameter_Overridetype ovrtype;
	// Description text. 
	const char *desctxt;
};


// Helper functions against too much ``AParameter<T>()'' in 
// our source: 
template<class T>inline const AParameter<T> Default(const T x)
{  AParameter<T> tmp(x);  return(tmp);  }
template<class T>inline const AParameter<T> Default()
{  AParameter<T> tmp;     return(tmp);  }

// All supported parameters must be added here to these lists. 
// THE LISTS MUST BE KEPT SORTED in the same order as the Parameter_ID. 

static Parameter_Description<bool> desc_bool[PID::_last_bool]=
{
	{ PID::warnings,   {"warnings"}, 
	  Default<bool>(1), PAT::Def_True, POT::Simple, 
	  "Switch on/off warnings" }
};

static Parameter_Description<int> desc_int[PID::_last_int]=
{
	{ PID::width,      {"width","renderWidth"}, 
	  Default<int>(/*unset*/), PAT::Needed, POT::Simple, 
	  "Width of film images (as passed to the render program)" },
	{ PID::height,     {"height","renderHeight"}, 
	  Default<int>(/*unset*/), PAT::Needed, POT::Simple, 
	  "Height of film images (as passed to the render program)" },
	{ PID::startframe, {"startframe"},
	  Default<int>(0),         PAT::Needed, POT::Needs_Solver, 
	  "Frame number of first frame to render" },
	{ PID::endframe,   {"endframe"},
	  Default<int>(),          PAT::Needed, POT::Needs_Solver, 
	  "Frame number of last frame to render" },
	{ PID::frames,     {"frames"},
	  Default<int>(),          PAT::Needed, POT::Needs_Solver, 
	  "Number of frames to render" },
	{ PID::verbose,    {"verbose","verbosity"},
	  Default<int>(0),         PAT::Increment, POT::Simple, 
	  "Verbosity level (0...??)" }
};

static Parameter_Description<double> desc_double[PID::_last_double]=
{
	{ PID::starttime,  {"starttime"},
	  Default<double>(0.0), PAT::Needed, POT::Needs_Solver, 
	  "Time of first frame to render (seconds)" },
	{ PID::endtime,    {"endtime"},
	  Default<double>(),    PAT::Needed, POT::Needs_Solver, 
	  "Time of last frame to render (seconds)" },
	{ PID::duration,   {"duration"},
	  Default<double>(),    PAT::Needed, POT::Needs_Solver, 
	  "Duration of the rendered film (seconds)" },
	{ PID::fps,        {"fps"},
	  Default<double>(),    PAT::Needed, POT::Needs_Solver, 
	  "Number of frames per second" }
};

static Parameter_Description<std::string> desc_string[PID::_last_string]=
{
	{ PID::renderer,   {"renderer"},
	  Default<std::string>(),  PAT::Needed, POT::Simple,
	  "Name of renderer" }
};

static Parameter_Description<stringlist> desc_stringlist[PID::_last_stringlist]=
{
	{ PID::renderargs, {"renderargs"},
	  Default<stringlist>(),   PAT::Needed /*PAT::Replace*/, POT::Simple, 
	  "Arguments to be passed to the renderer. $(width) and $(height) get\n"
	  "        replaced by the real width/height values ($$ -> $)." }
};


PID::Parameter_ID LastID_By_Type(bool*)    {  return(PID::_last_bool);    }
PID::Parameter_ID LastID_By_Type(int*)     {  return(PID::_last_int);     }
PID::Parameter_ID LastID_By_Type(double*)  {  return(PID::_last_double);  }
PID::Parameter_ID LastID_By_Type(std::string*)  {  return(PID::_last_string);  }
PID::Parameter_ID LastID_By_Type(stringlist*)   {  return(PID::_last_stringlist);  }

Parameter_Description<bool> *Desc_By_Type(bool*)     {  return(desc_bool);    }
Parameter_Description<int> *Desc_By_Type(int*)       {  return(desc_int);     }
Parameter_Description<double> *Desc_By_Type(double*) {  return(desc_double);  }
Parameter_Description<std::string> *Desc_By_Type(std::string*) {  return(desc_string);  }
Parameter_Description<stringlist> *Desc_By_Type(stringlist*)   {  return(desc_stringlist);  }


// Set default values (copying them from the parameter description 
// to *this). 
// keepold = true  -> If a parameter does not have a default, the old 
//                    setting is kept. 
// keepold = false -> For parameters which do not have defaults, the 
//                    setting is set to ``not set''. 
template<class T> void Animation_Parameters::Do_Set_Defaults(bool keepold)
{
	T *dummy;  // Pointer prevents an object from being constructed. 
	Parameter_Description<T> *desc=Desc_By_Type(dummy);
	AParameter<T> *par=List_By_Type(dummy);
	for(int i=0; i<LastID_By_Type(dummy); i++)
	{
		if(!keepold || desc[i].defaultval.is_set)
		{  par[i]=desc[i].defaultval;  }
	}
}


// See Copy_From() for a description. 
// NOTE that os may be NULL. 
template<class T> void Animation_Parameters::Do_Copy_From(
	Animation_Parameters &ap,bool keepold /*,ostream *os <-- warning stream*/)
{
	T *dummy;  // Pointer prevents an object from being constructed. 
	AParameter<T> *dest=List_By_Type(dummy);
	AParameter<T> *src=ap.List_By_Type(dummy);
	AParameter<T> *destend=dest+LastID_By_Type(dummy);
	for(; dest<destend; src++,dest++)
	{
		if(!keepold)
		{  dest->is_set=false;  }
		if(src->is_set)
		{
			#if 0
			if(dest->is_set && os)
			{
				// Warn user. 
				#warning need better error reporting; 
				(*os) << "Warning: value gets overriden." << endl;
			}
			#endif
			dest->val=src->val;
			dest->is_set=true;
		}
	}
}


static void _Internal_Argtype_Error(int argtype,const char *name)
{
	cerr << "Internal error: wrong argtype " << argtype << 
		" for parameter " << name << endl <<
		"Please change parameter description in pars.cpp." << endl;
	abort();
}


// Parameter type specific code dealing with different argtypes 
// allowed for this parameter type. 
// The functions are inline'd because they are called only from 
// one position on the source code. 
// Return value: 
//  -2 -> argument needed but missing (nextarg=NULL) (error; don't go on)
//  -1 -> nextarg parsed but parsing failed (don't go on)
//   0 -> success; nextarg unused (need not go on parsing)
//   1 -> no error but nothing done; go on parsing (nextarg!=NULL)
//   2 -> success; nextarg used (need not go on parsing)
// Type: BOOL
inline int Animation_Parameters::Parse_Check_Argtype(bool*,char *nextarg,
	Parameter_Description<bool> *desc,AParameter<bool> *par)
{
	if(desc->argtype==PAT::Needed)
		return(nextarg ? 1 : -2);
	
	if(desc->argtype==PAT::Def_True || 
	   desc->argtype==PAT::Def_False )
	{
		bool newval = (desc->argtype==PAT::Def_True);
		bool took_nextarg=false;
		
		// See if next argument exists and is a valid bool arg. 
		if(nextarg)
		{
			if(Str_To_Value(nextarg,newval,/*silent=*/nextarg_is_next))
				took_nextarg=true;
			else if(!nextarg_is_next)
				return(-1);  // nextarg incorrect
		}
		
		if(par->is_set)
		{
			Warning() << ": Parameter set more than once (previous value: " << 
				par->val << " new value: " << newval << ")" << endl;
		}
		
		par->is_set=true;
		par->val=newval;
		
		return(took_nextarg ? 2 : 0);  // success
	}
	
	_Internal_Argtype_Error(desc->argtype,desc->names[0]);
	return(-2);  // never reached. 
}
// Type: INT
inline int Animation_Parameters::Parse_Check_Argtype(int*,char *nextarg,
	Parameter_Description<int> *desc,AParameter<int> *par)
{
	if(desc->argtype==PAT::Needed)
		return(nextarg ? 1 : -2);
	
	if(desc->argtype==PAT::Increment)
	{
		int newval = par->is_set ? (par->val+1) : 1;
		bool took_nextarg=false;
		
		// See if next argument exists and is a valid int arg. 
		if(nextarg)
		{
			if(Str_To_Value(nextarg,newval,/*silent=*/nextarg_is_next))
				took_nextarg=true;
			else if(!nextarg_is_next)
				return(-1);  // nextarg incorrect
		}
		
		par->is_set=true;
		par->val=newval;
		
		return(took_nextarg ? 2 : 0);  // success
	}
	
	_Internal_Argtype_Error(desc->argtype,desc->names[0]);
	return(-2);  // never reached. 
}
// Type: DOUBLE
inline int Animation_Parameters::Parse_Check_Argtype(double*,char *nextarg,
	Parameter_Description<double> *desc,AParameter<double> *)
{
	if(nextarg)                     return(1);   // do it the normal way
	if(desc->argtype==PAT::Needed)  return(-2);  // argument missing
	
	_Internal_Argtype_Error(desc->argtype,desc->names[0]);
	return(-2);  // never reached. 
}
// Type: STRING
inline int Animation_Parameters::Parse_Check_Argtype(std::string*,char *nextarg,
	Parameter_Description<std::string> *desc,AParameter<std::string> *)
{
	if(nextarg)                     return(1);   // do it the normal way
	if(desc->argtype==PAT::Needed)  return(-2);  // argument missing
	
	_Internal_Argtype_Error(desc->argtype,desc->names[0]);
	return(-2);  // never reached. 
}
// Type: STRINGLIST
inline int Animation_Parameters::Parse_Check_Argtype(stringlist*,char *nextarg,
	Parameter_Description<stringlist> *desc,AParameter<stringlist> *par)
{
	if(desc->argtype!=PAT::Needed && desc->argtype!=PAT::Replace)
	{  _Internal_Argtype_Error(desc->argtype,desc->names[0]);
		return(-2);  }   // <- never reached. 
	
	if(!nextarg)  return(-2);  // argument missing
	if(desc->argtype==PAT::Replace)
	{
		par->val.clear();  // clear list before adding new args
		return(1);  // go on parsing (leads to warning if set for a second time)
	}
	
	// Append the values: 
	bool rv=Str_To_Value(nextarg,par->val);
	if(rv)
	{  par->is_set=true;  return(2);  }
	return(-1);
}


// Return value: 
//   0 -> error: identifier not found 
//   1 -> used arg but not nextarg. 
//   2 -> used arg and nextarg. 
//  -1 -> identifier found but invalid argument (nextarg)
// Be careful: nextarg may be NULL. 
template<class T> int Animation_Parameters::Do_Parse_Setting(
	char *arg,char *nextarg)
{
	T *dummy;  // Pointer prevents an object from being constructed. 
	Parameter_Description<T> *descstart=Desc_By_Type(dummy);
	Parameter_Description<T> *desc=descstart;
	AParameter<T> *parstart=List_By_Type(dummy);
	AParameter<T> *par=parstart;
	AParameter<T> *parend=parstart+LastID_By_Type(dummy);
	for(; par<parend; par++,desc++)
		for(int i=0; i<MaxSynonymes; i++)
		{
			if(!desc->names[i])
			{  break;  }
			if(!strcmp(arg,desc->names[i]))
			{  goto breakfor;  }
		}
	return(0);  // not found
	breakfor:;
	// par and desc set correctly. 
	
	// Handle argtype-specific part: 
	switch(Parse_Check_Argtype(dummy,nextarg,desc,par))
	{
		case  0:  return(1);   // success; nextarg unused
		case  2:  return(2);   // success; nextarg was used
		case -2:  // arg needed but missing
			Error() << ": Required argument missing" << endl;
			return(-1);  // is OK here 
		case -1:  // nextarg parsed and parsing failed
			return(-1);
		case  1:  break;  // go on parsing
		default:  // actually never happens...
			cerr << "Internal error: pars.cpp:" << __LINE__ << endl;
			abort();
	}
	
	T tmpval=par->val;  // needed for stringlist
	bool rv=Str_To_Value(nextarg,tmpval);
	if(rv==false)  // parse error
		return(-1);
	
	if(par->is_set)
	{	Warning() << ": Parameter set more than once." << endl;  }
	par->val=tmpval;
	par->is_set=1;
	
	return(2);
}


template<class T> void Animation_Parameters::Do_Print_Parameters(
	ostream &os,const char *indent,bool print_unset)
{
	T *dummy;  // Pointer prevents an object from being constructed. 
	Parameter_Description<T> *desc=Desc_By_Type(dummy);
	AParameter<T> *par=List_By_Type(dummy);
	AParameter<T> *parend=par+LastID_By_Type(dummy);
	for(; par<parend; par++,desc++)
	{
		if(par->is_set || print_unset)
		{
			if(indent)  os << indent;
			os << desc->names[0] << " = ";
			if(par->is_set)
			{
				String_Value_Converter::Print_Value(os,par->val);
				os << endl;
			}
			else
			{  os << "[unset]" << endl;  }
		}
	}
}


// See Simple_Override(). 
// listtype: true for stringlist
template<class T> int Animation_Parameters::Do_Simple_Override(
	Override_Pars *startop,int nop,ostream &os,bool warnings,
	bool listtype)
{
	int nunset=0;
	T *dummy;  // Pointer prevents an object from being constructed. 
	Parameter_Description<T> *desc=Desc_By_Type(dummy);
	AParameter<T> *thispar=List_By_Type(dummy);
	int endidx=LastID_By_Type(dummy);
	Override_Pars *endop=startop+nop;
	for(int idx=0; idx<endidx; idx++,desc++,thispar++)  // for every parameter
	{
		if(desc->ovrtype!=POT::Simple)  continue;
		// Where the parameter was first and last set (in override order)
		Override_Pars *firstpar=NULL,*lastpar=NULL;
		thispar->is_set=false;
		for(Override_Pars *op=startop; op<endop; op++)
		{
			AParameter<T> *par=(op->ap->List_By_Type(dummy))+idx;
			if(par->is_set)
			{
				thispar->is_set=true;
				if(listtype && desc->argtype==PAT::Needed)
				{  thispar->val+=par->val;  }   // do add...
				else
				{  thispar->val=par->val;   }   // do override...
				
				if(op>startop)   // Do not want warnings if built-in default gets overridden. 
					if(!firstpar)  firstpar=op;
					else           lastpar=op;
			}
		}
		if(!thispar->is_set)
		{
			// parameter not set. 
			os << "Error: Parameter " << desc->names[0] << 
				" not set." << endl;
			++nunset;
		}
		else if(lastpar && warnings)  // parameter overridden (at least once)
		{
			os << "Parameter \"" << desc->names[0] << "\""
				" first set in " << firstpar->type << 
				" gets overridden by settin in " << lastpar->type << 
				"." << endl;
		}
	}
	return(nunset);
}


template<class T> void Animation_Parameters::Do_Print_Help(ostream &os)
{
	T *dummy;  // Pointer prevents an object from being constructed. 
	Parameter_Description<T> *desc=Desc_By_Type(dummy);
	Parameter_Description<T> *descend=desc+LastID_By_Type(dummy);
	for(; desc<descend; desc++)
	{
		os << "    --" << desc->names[0];
		for(int i=1; i<MaxSynonymes; i++)
		{
			if(!desc->names[i])  break;
			os << ",  --" << desc->names[i];
		}
		if(desc->defaultval.is_set)
		{
			os << "  (";
			String_Value_Converter::Print_Value(os,desc->defaultval.val);
			os << ")" << endl;
		}
		else
		{  os /*<< "  (unset)"*/ << endl;  }
		os << "        " << desc->desctxt << endl;
	}
}


// Call this function to copy the built-in defaults 
// to the contents of *this. 
// keepold = true  -> If a parameter does not have a default, the old 
//                    setting is kept. 
// keepold = false -> For parameters which do not have defaults, the 
//                    setting is set to ``not set''. 
void Animation_Parameters::Set_Defaults(bool keepold)
{
	Do_Set_Defaults<bool>(keepold);
	Do_Set_Defaults<int>(keepold);
	Do_Set_Defaults<double>(keepold);
	Do_Set_Defaults<std::string>(keepold);
	Do_Set_Defaults<stringlist>(keepold);
}


// Call this to copy the values of ap into *this. 
// keepold: false: clear all parameters before copying
//          true:  new values (from ap) override ones already 
//                 in *this. 
// warning_stream: where to write warnings when overriding 
//                 parameters. Set this to NULL and no warnings 
//                 will be generated. 
void Animation_Parameters::Copy_From(const Animation_Parameters &_ap,bool keepold)
{
	// Bad hack; needed because we want to access the list via pointers 
	// and this only works if the source is declared non-const even 
	// if it really stays const. 
	Animation_Parameters *ap=(Animation_Parameters *)&_ap;
	
	Do_Copy_From<bool>(*ap,keepold);
	Do_Copy_From<int>(*ap,keepold);
	Do_Copy_From<double>(*ap,keepold);
	Do_Copy_From<std::string>(*ap,keepold);
	Do_Copy_From<stringlist>(*ap,keepold);
}


// Return value: see Do_Parse_Setting(). 
int Animation_Parameters::Parse_Setting(char *arg,char *nextarg)
{
	//cerr << "parsing: <" << arg << "> <" << nextarg << ">" << endl;
	
	int r;
	r=Do_Parse_Setting<bool>(arg,nextarg);         if(r)  return(r);
	r=Do_Parse_Setting<int>(arg,nextarg);          if(r)  return(r);
	r=Do_Parse_Setting<double>(arg,nextarg);       if(r)  return(r);
	r=Do_Parse_Setting<std::string>(arg,nextarg);  if(r)  return(r);
	r=Do_Parse_Setting<stringlist>(arg,nextarg);   if(r)  return(r);
	return(0);
}


// Print the parameters to stream os. 
// Not set parameters are printed only if print_unset is true. 
// indent is written at the beginning of every line (if non-NULL). 
void Animation_Parameters::Print_Parameters(ostream &os,
	const char *indent=NULL,bool print_unset=false)
{
	Do_Print_Parameters<bool>(os,indent,print_unset);
	Do_Print_Parameters<int>(os,indent,print_unset);
	Do_Print_Parameters<double>(os,indent,print_unset);
	Do_Print_Parameters<std::string>(os,indent,print_unset);
	Do_Print_Parameters<stringlist>(os,indent,print_unset);
}


// Takes an array of Override_Pars passed in op (size: nop elements). 
// The values of *this are set to the corresponding values of op[0]...op[nop-1] 
// (in this order) if they are set. 
// vw: switch on/off verbose warnings (telling you which parameter got 
//     overridden). 
// Return values:  0 -> all went file. 
//                >0 -> some values are unset. 
int Animation_Parameters::Simple_Override(Override_Pars *op,int nop,
	ostream &os,bool vw)
{
	int rv=0;
	rv+=Do_Simple_Override<bool>(op,nop,os,vw);
	rv+=Do_Simple_Override<int>(op,nop,os,vw);
	rv+=Do_Simple_Override<double>(op,nop,os,vw);
	rv+=Do_Simple_Override<std::string>(op,nop,os,vw);
	rv+=Do_Simple_Override<stringlist>(op,nop,os,vw,/*lisstype=*/true);
	return(rv);
}


// Print output of the cmd option --help. 
void Animation_Parameters::Print_Help(ostream &os)
{
	os << "Supported animation parameters:" << endl;
	os << "ŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻŻ" << endl;
	os << "Switches (parameters of type bool)" << endl;
	os << "  Possible values: yes,on,true,1 / no,off,false,0" << endl;
	Do_Print_Help<bool>(os);
	os << endl;
	os << "Parameters of integer type" << endl;
	os << "  Specification: C style: prefix `0x\'/`0\' "
		"for hexadecimal/octal values" << endl;
	Do_Print_Help<int>(os);
	os << endl;
	os << "Parameters of floating point type" << endl;
	Do_Print_Help<double>(os);
	os << endl;
	os << "Parameters of string type" << endl;
	os << "  Specification: Two possibilities:" << endl;
	os << "    1. If string does not start with `\"\' after trimming, it is" << endl;
	os << "       read in as encountered after trimming." << endl;
	os << "    2. If string starts with `\"\' after trimming, the text "
		"between the" << endl;
	os << "       first and the second `\"\' is read in. `\\\"\' and `\\\\\' "
		"are replaced by " << endl;
	os << "       `\"\' and `\\\' respectively, all other escapes stay "
		"unchanged." << endl;
	Do_Print_Help<std::string>(os);
	os << endl;
	os << "Parameters of string list type" << endl;
	os << "  Specification: A stringlist is a set of strings separated by "
		"whitespace." << endl;
	Do_Print_Help<stringlist>(os);
	os << endl;
}


Animation_Parameters::Animation_Parameters(const Animation_Parameters &ap) : 
	String_Value_Converter()
{
	include_depth=0;
	// Copy all settings from ap. 
	Copy_From(ap,false);
}

Animation_Parameters::Animation_Parameters() : 
	String_Value_Converter()
{
	// All the arrays set is_set to false by themselves. 
	// So, there is not much to do here...
	include_depth=0;
}

Animation_Parameters::~Animation_Parameters()
{
	// There is currently nothing to clean up. 
}

} /* end of namespace anitmt */
