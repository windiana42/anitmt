/*
 * pars.hpp
 * 
 * Include file for routines dealing with command line 
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


#ifndef __anitmt_pars_hpp__
#define __anitmt_pars_hpp__

#include <string>
//#include <fstream>
#include <stdio.h>

#include "strlist.hpp"

namespace anitmt
{

// This class is used to pass all command line arguments 
// (and environment vars). 
struct Command_Line
{
	int argc;      // number of arguments in argv 
	char **argv;   // command line args
	int  *used;    // stores if a command line arg was used 
	               // (set to used[i]=strlen(arg[i]) if used)
				   // (value -1 indicates: definitely invalid)
	char **envp;   // environment vars
	bool quitafter;  // quit after parsing args (e.g. --help specified) 
	
	// Call constructor with values passed to main(). 
	Command_Line(int argc,char **argv,char **envp);
	~Command_Line();
	
	// Check for unused arguments. 
	// For every unused arg, an error is written to os. 
	// Return value: true  -> no unused args (okay, go on)
	//               false -> at least one unused arg (error; exit)
	bool Check_Unused(ostream &os=cerr);
};


namespace PID
{
	enum Parameter_ID
	{
		// NOTE: ** YOU MUST KEEP THIS LIST SORTED BY TYPE. **
		//       * The first entry for one argument type must be set to 0 
		//         (e.g. warnings=0) 
		//       * _last_<type> must be the last entry (highest ID)
		
	// Type: BOOL (first entry must be set to 0)
		warnings=0,
		
		_last_bool,  // must be largest bool ID 
	// Type: INT (first entry must be set to 0)
		width=0,
		height,
		startframe,
		endframe,
		frames,
		verbose,
		
		_last_int,  //  must be largest int ID 
	// Type: DOUBLE (first entry must be set to 0)
		starttime=0,
		endtime,
		duration,
		fps,
		
		_last_double,  //  must be largest double ID 
	// Type: STRING (first entry must be set to 0)
		renderer=0,
		
		_last_string,  //  must be largest string ID 
	// Type: STRINGLIST (first entry must be set to 0)
		renderargs=0,
		
		_last_stringlist   //  must be largest stringlist ID 
	};
}


// A single Animation Parameter. 
template<class T> struct AParameter
{
	bool is_set;  // value was specified?
	T val;        // value to be stored
	
	operator T()  {  return(val);  }
	
	AParameter()          : is_set(false)        { }
	AParameter(const T v) : is_set(true),val(v)  { }
	// Copy constructor, assignment operator: 
	AParameter(const AParameter<T> &ap) : 
		is_set(ap.is_set),val(ap.val)  { }
	AParameter &operator=(const AParameter<T> &ap)
		{  is_set=ap.is_set;  val=ap.val;  return(*this);  }
	AParameter &operator=(const T v)
		{  is_set=true;  val=v;  return(*this);  }
};


template<class T> struct Parameter_Description;


class String_Value_Converter
{
	private:
		bool comments_in_line;
		
		// For errors, warnings: 
		ostream *errstream;
		bool cmd_mode;  // command line mode or file mode?
		// for cmd line mode AND file mode: 
		std::string optname;  // --width, --height, ...
		// for file mode: 
		std::string filename;
		int linecnt,linecnt2;
		
		// Print error/warning prefix: 
		ostream &_Prefix(int type);
		bool Value_Omitted();
		
		inline int is_trim(char c);
		bool trim_str(char *str,char *dest);
		bool simple_find_comment(char *str);
	protected:
		ostream &Error()    {  return(_Prefix(1));  }
		ostream &Warning()  {  return(_Prefix(0));  }
		ostream &EStream()  {  return(*errstream);  }
		
		std::string &Get_Filename()  {  return(filename);  }
		std::string &Get_Option()    {  return(optname);  }
	public:
		String_Value_Converter();
		~String_Value_Converter();
		
		// If to allow comments in a line, e.g. 
		// width=400  # use 200 for preview
		// Default: yes (switch off for command line) 
		void Allow_Comments_In_Line(bool sw)
			{  comments_in_line=sw;  }
		
		// Set error reporting and comment parsing mode: 
		void Set_Cmd_Mode()   {  cmd_mode=true;   comments_in_line=false;  }
		void Set_File_Mode()  {  cmd_mode=false;  comments_in_line=true;   }
		
		// Where to write errors (default: cerr)
		void Set_Stream(ostream &os)  {  errstream=&os;  }
		
		// For command line mode: set currently parsed cmd option
		void Set_Option(const char *opt)  {  optname.assign(opt);  }
		void Set_Option(std::string &opt)  {  optname.assign(opt);  }
		
		// For file mode: set currently parsed file and line numbers 
		void Set_File(const char *file)  {  filename.assign(file);  }
		void Set_File(std::string &file)  {  filename.assign(file);  }
		void Set_Line(int l,int l2)  {  linecnt=l;  linecnt2=l2;  }
		// ... and use Set_Option() to set the current parameter. 
		
		// These functions really read in the values: 
		// str is read and the result is stored in val. 
		// str is trimmed if necessary. 
		// Return value: true  -> OK; no error
		//               false -> error in str; val unchanged. 
		bool Str_To_Value(char *str,bool &val,bool silent=false);
		bool Str_To_Value(char *str,int &val,bool silent=false);
		bool Str_To_Value(char *str,double &val);
		bool Str_To_Value(char *str,std::string &val);
		bool Str_To_Value(char *str,stringlist &val);
		
		// Special output functions: Enclosing strings in "", escaping `"' as 
		// well as `\' and writing bool as on/off. 
		static ostream &Print_Value(ostream &os,bool &val);
		static ostream &Print_Value(ostream &os,int &val);
		static ostream &Print_Value(ostream &os,double &val);
		static ostream &Print_Value(ostream &os,std::string &val);
		static ostream &Print_Value(ostream &os,stringlist &val);
};


class Animation_Parameters : String_Value_Converter
{
	private:
		AParameter<bool>        par_bool  [PID::_last_bool];
		AParameter<int>         par_int   [PID::_last_int];
		AParameter<double>      par_double[PID::_last_double];
		AParameter<std::string> par_string[PID::_last_string];
		AParameter<stringlist>  par_stringlist[PID::_last_stringlist];
		
		// Get list by type 
		AParameter<bool>    *List_By_Type(bool*)   { return(par_bool);    }
		AParameter<int>     *List_By_Type(int*)    { return(par_int);     }
		AParameter<double>  *List_By_Type(double*) { return(par_double);  }
		AParameter<std::string> *List_By_Type(std::string*) { return(par_string);      }
		AParameter<stringlist>  *List_By_Type(stringlist*)  { return(par_stringlist);  }
		
		template<class T> void Do_Set_Defaults(bool keepold);
		template<class T> void Do_Copy_From(Animation_Parameters &,bool);
		template<class T> int  Do_Parse_Setting(char *,char *);
		template<class T> void Do_Print_Parameters(ostream &,const char *,bool);
		template<class T> void Do_Print_Help(ostream &os);
		
		int Parse_Check_Argtype(bool*,char *nextarg,
			Parameter_Description<bool> *desc,AParameter<bool> *par);
		int Parse_Check_Argtype(int*,char *nextarg,
			Parameter_Description<int> *desc,AParameter<int> *par);
		int Parse_Check_Argtype(double*,char *nextarg,
			Parameter_Description<double> *desc,AParameter<double> *par);
		int Parse_Check_Argtype(std::string*,char *nextarg,
			Parameter_Description<std::string> *desc,AParameter<std::string> *par);
		int Parse_Check_Argtype(stringlist*,char *nextarg,
			Parameter_Description<stringlist> *desc,AParameter<stringlist> *par);
		
		bool nextarg_is_next;  // commnd line only 
		int Parse_Setting(char *arg,char *nextarg);
		
		int include_depth;
		int Read_One_Line(FILE *is,char *buf,size_t len);
		int Read_In(FILE *is);
		int Do_Read_In(FILE *is);
		
		struct Override_Pars
		{
			const char *type;  // default, config, ini, rif, cmd line 
			char type_short;
			Animation_Parameters *ap;
		};
		int Simple_Override(Override_Pars *array,int npars,
			ostream &os=cerr,bool verbose_warnings=false);
		template<class T> int Do_Simple_Override(Override_Pars *op,
			int nop,ostream &os,bool verbose_warnings,bool listtype=false);
		
		int Solve_TimeFrame_Net(Override_Pars *array,int npars,
			ostream &os,bool warnings);
		void Solve_TimeFrame_Done(/*ugly*/void *tmp);
	public:
		Animation_Parameters();
		~Animation_Parameters();
		Animation_Parameters(const Animation_Parameters &ap);  // copy
		
		// MAIN FUNCTION: 
		// If *this is the principal parameter class, just pass the 
		// command line using the function below. 
		// It will parse the command line, read in files, merge 
		// the parameters and everything needed to get all parameters 
		// set up. 
		// If it returns true, everything is okay. 
		// If it returns false, some errors were issued. Quit now. 
		// All other functions to set parameters are not of interest if 
		// *this is the main parameter class. 
		bool Parse_Command_Line(Command_Line *cmd);
		
		/********* METHODS TO SET PARAMETERS *********/
		
		// There are four methods to set parameters: 
		// 1) using compiled-in defaults 
		// 2) copying the values of another class 
		// 3) using command line arguments 
		// 4) using a file to read the parameters from 
		
		// Call this function to copy the built-in defaults 
		// to the contents of *this. 
		// keepold = true  -> If a parameter does not have a default, 
		//                    the old setting is kept. 
		// keepold = false -> For parameters which do not have defaults, 
		//                    the setting is set to ``not set''. 
		void Set_Defaults(bool keepold);
		
		// Call this to copy the values of ap into *this. 
		// keepold: false: clear all parameters before copying
		//          true:  new values (from ap) override ones already 
		//                 in *this. 
		void Copy_From(const Animation_Parameters &ap,bool keepold);
		
		// Reads in the command line arguments updating 
		// the cmd.used[] array. 
		void Read_Command_Line(Command_Line *cmd);
		
		// Reads in the specified file and takes the parameters from 
		// there. 
		// Return value: 
		//   0 -> success
		//  >0 -> error (1 -> failed to open/read file; 2 -> parse error)
		int Read_File(const char *path);
		
		/********* DIAGNOSIS AND PARAMETER OUTPUT *********/
		
		// Print the parameters to stream os. 
		// Not set parameters are printed only if print_unset is true. 
		// indent is written at the beginning of every line (if non-NULL). 
		void Print_Parameters(ostream &os,const char *indent=NULL,
			bool print_unset=false);
		
		// Print output of the cmd option --help.
		void Print_Help(ostream &os=cerr);
		
		/********* METHODS TO QUERY PARAMETERS *********/
		
		// BOOL parameters: 
		AParameter<bool> warnings()   {  return(par_bool[PID::warnings]);   }
		
		// INTEGER parameters: 
		AParameter<int> width()       {  return(par_int[PID::width]);       }
		AParameter<int> height()      {  return(par_int[PID::height]);      }
		AParameter<int> startframe()  {  return(par_int[PID::startframe]);  }
		AParameter<int> endframe()    {  return(par_int[PID::endframe]);    }
		AParameter<int> frames()      {  return(par_int[PID::frames]);      }
		AParameter<int> verbose()     {  return(par_int[PID::verbose]);     }
		
		// DOUBLE parameters: 
		AParameter<double> starttime()  {  return(par_double[PID::starttime]);  }
		AParameter<double> endtime()    {  return(par_double[PID::endtime]);    }
		AParameter<double> duration()   {  return(par_double[PID::duration]);   }
		AParameter<double> fps()        {  return(par_double[PID::fps]);        }
		
		// STRING parameters: 
		AParameter<std::string> renderer()  {  return(par_string[PID::renderer]);  }
		
		// STRING LIST parameters: 
		AParameter<stringlist> rendeargs()  {  return(par_stringlist[PID::renderargs]);  }
};

// Returns "-" or "--". 
extern const char *Arg_Prefix(const char *arg);

}  /* end of namespace anitmt */

#endif   /* __anitmt_pars_hpp__ */
