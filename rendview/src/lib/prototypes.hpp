/*
 * prototypes.hpp
 * 
 * Some misc. prototypes...
 * 
 * Copyright (c) 2002--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#ifndef _RNDV_LIB_PROTOTYPES_HPP_
#define _RNDV_LIB_PROTOTYPES_HPP_ 1

#include <hlib/prototypes.h>

// AFTER the HLIB config, include our config. 
// And make sure the version is correct. 
#undef VERSION
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION

#include "config.h"

#include <stdarg.h>


// NOTE: There are two types of assert() calls: 
// assert():      general bug trap 
// hack_assert(): assertion expected to fail because of incomplete 
//                implementation
#define hack_assert(x)  assert(x)

#ifdef NDEBUG
#  error "Forget it. You MUST NOT compile RendView with NDEBUG. It relies on assert()."
#endif


enum RendViewOpMode
{
	RVOM_None=0,
	RVOM_RendView,
	RVOM_LDRClient,
	RVOM_LDRServer
};

// Return opmode enum according to passed binary name. 
// RVOM_None if none matches. 
extern RendViewOpMode GetRendViewOpMode(const char *prg_name);


class RefString;
class HTime;

// We need the logger for error/warning/verbose messages: 
#include <admin/logger.hpp>


struct RVOutputParams
{
	// Used to switch on/off colored output: (Initially 0)
	int enable_color_stdout : 1;
	int enable_color_stderr : 1;
	int : (sizeof(int)*8 - 2);   // <-- Use modulo if more than 16 bits. 
	
	const char *console_red_start,*console_red_end;     // red
	const char *console_Red_start,*console_Red_end;     // bold red
	const char *console_blue_start,*console_blue_end;   // blue
	const char *console_Blue_start,*console_Blue_end;   // bold blue
	
	// Verbose level field. See VERBOSE_xxx for bitmask. 
	u_int32_t vlevel_field;
};

enum
{
	VERBOSE_MiscInfo =  0x00000001,   // misc info like number of CPUs. 
	VERBOSE_TDI =       0x00000002,   // TaskDriver init info (start processing/end)
	VERBOSE_TDR =       0x00000004,   // TaskDriver runtime info (kill/start...)
	VERBOSE_TSI =       0x00000008,   // TaskSource init info (per-frame blocks...)
	VERBOSE_TSP =       0x00000010,   // TaskSource param parse/setup info (skipped xy,...)
	VERBOSE_TSR0 =      0x00000020,   // TaskSource runtime info level 0 (less important)
	VERBOSE_TSR1 =      0x00000040,   // TaskSource runtime info level 1 (more important)
	VERBOSE_TSLR =      0x00000080,   // local task source runtime info (file re-naming...)
	VERBOSE_TSLLR =     0x00000100,   // LDR task source runtime info
	VERBOSE_DBG =       0x01000000,   // DEBUG verbose
	VERBOSE_BasicInit = 0x01000000,   // basic init at startup (managers, ...) (=DEBUG, now)
	VERBOSE_DBGV =      0x02000000,   // DEBUG very verbose (implies ~_DBG)
	_VERBOSE_ALL =      0x7fffffff    // all flags (actually, "more than all")
};


// Global output params: 
extern  RVOutputParams rv_oparams;
// Set them up: 
extern int InitRVOutputParams(int &argc,char **argv,char **envp);
// *errors: error counter
// Return value: 
//  0 -> no output param
//  1 -> arg0 was used
//  2 -> arg0 and arg1 were used
extern int CheckParseOutputParam(const char *arg0,const char *arg1,
	int *errors,int argidx_for_error,int in_cmd_line);
// Used to parse verbose option spec: 
// Global rv_oparams get modified. 
// Return value: 0 -> OK; 1 -> error 
extern int ParseVerbosSpec(const char *str,const char *err_prefix,int in_cmd_line);
// This must be called after the env var processing so that the cmd 
// line values can override those in the env var: 
extern void FixupOverrideVerboseSpec();


#define IsVerbose(vspec) (rv_oparams.vlevel_field & VERBOSE_##vspec)
#define IsVerboseR(vlevel) (rv_oparams.vlevel_field & vlevel)

// Do the checking inline
#define Verbose(vspec,fmt...)  \
	do { \
		if(IsVerbose(vspec)) \
		{  MessageLogger::_Verbose(VERBOSE_##vspec,fmt);  } \
	} while(0)

#define VerboseR(vlevel,fmt...)  \
	do { \
		if(IsVerboseR(vlevel)) \
		{  MessageLogger::_Verbose(vlevel,fmt);  } \
	} while(0)

#define VerboseSpecial(fmt...)\
	do { MessageLogger::_VerboseSpecial(fmt); } while(0)

#define Warning(fmt...)   do { MessageLogger::_Warning(fmt); } while(0)
#define Error(fmt...)     do { MessageLogger::_Error(fmt); } while(0)


// Some string constants: 
extern struct _ConstStrings
{
	const char *allocfail;  // "allocation failure"  :)
} const cstrings;


// Returns number of CPUs; assumes one CPU and warn if detection fails. 
extern int GetNumberOfCPUs();

// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
extern int GetLoadValue();

// Get current working directory: 
// Return value: 
//   0 -> OK
//  -1 -> alloc failure
//  -2 -> getcwd() failed: see errno
//  -3 -> path too long
// String representation is set in *dest unless -1 is retured. 
extern int GetCurrentWorkingDir(RefString *dest);

// Check if a file exists and we have the passed permissions: 
extern int CheckExistFile(RefString *f,int want_read,int want_write=0);

// Get file length and also modification time if non-NULL. 
// Returns -1 if stat fails. 
extern int64_t GetFileLength(const char *path,HTime *mtime);

// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// error_prefix is put before error message; use e.g. "Local: ". 
// save_mtime_a, if non-NULL stores a's mtime (or invalid if !existing). 
extern int FirstFileIsNewer(RefString *a,RefString *b,const char *error_prefix,
	HTime *save_mtime_a=NULL);

// Simply delete a file (but not verbosely). 
// error_prefix: is put before error message; use e.g. "Local: ". 
// If may_not_exist is set, no error occurs if the file does not exist. 
// Return value: 0 -> OK; -1 -> error; 1 -> ENOENT && may_not_exist
extern int DeleteFile(RefString *path,int may_not_exist,
	const char *error_prefix);

// Rename a file; error is written if that fails. 
// Set error_prefix to disable error. 
// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
extern int RenameFile(const RefString *old_name,const RefString *new_name,
	int may_not_exist,const char *error_prefix);
extern int _RenameFile(const char *old_name,const char *new_name,
	int may_not_exist,const char *error_prefix);


// Open input/output file. 
// dir: direction: -1 -> input; +1 -> output 
// Return value: 
//    >=0 -> valid FD
//     -1 -> file or file->str() NULL or dir==0
//     -2 -> open( failed (see errno)
extern int OpenIOFile(RefString *file,int dir);

// Convert passed timeout value from seconds to msec. 
// Writes a warning if the conversion results in integer overflow and 
// disables the timer in this case. 
// *which can be used to pass info which timer it is. 
extern void ConvertTimeout2MSec(long *timeout,const char *which);


// Copy '\0'-terminated string without copying the terminating '\0'. 
inline void strcpy_no0(char *dest,const char *src)
{
	while(*src)
	{  *(dest++)=*(src++);  }
}
// Copy '\0'-terminated string WITH trailing '\0' and return pointer to 
// the char AFTER the terminating '\0' in dest. 
inline char *strcpy_rdst(char *dest,const char *src)
{
	while((*(dest++)=*(src++)));
	return(dest);
}

// Search for chr (which may be '\0') in str buffer of size len. 
// Return pointer to char or NULL. 
inline const char *str_find_chr(const char *str,size_t len,char chr)
{
	for(const char *end=str+len; str<end; str++)
	{
		if(*str==chr)
		{  return(str);  }
	}
	return(NULL);
}

#endif  /* _RNDV_LIB_PROTOTYPES_HPP_ */
