/*
 * prototypes.hpp
 * 
 * Some misc. prototypes...
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


class RefString;

struct RVOutputParams
{
	// Used to switch on/off colored output: (Initially 0)
	int enable_color_stdout : 1;
	int enable_color_stderr : 1;
	int : (sizeof(int)*8 - 2);   // <-- Use modulo if more than 16 bits. 
	
	// Verbose level field. See VERBOSE_xxx for bitmask. 
	u_int32_t vlevel_field;
};

enum
{
	VERBOSE_BasicInit = 0x00000001,   // basic init at startup (managers, ...)
	VERBOSE_MiscInfo =  0x00000002,   // misc info like number of CPUs. 
	VERBOSE_TDI =       0x00000004,   // TaskDriver init info (start processing/end)
	VERBOSE_TDR =       0x00000008,   // TaskDriver runtime info (kill/start...)
	VERBOSE_TSI =       0x00000010,   // TaskSource init info (per-frame blocks...)
	VERBOSE_TSP =       0x00000020,   // TaskSource param parse/setup info (skipped xy,...)
	VERBOSE_TSR0 =      0x00000040,   // TaskSource runtime info level 0 (less important)
	VERBOSE_TSR1 =      0x000000b0,   // TaskSource runtime info level 1 (more important)
	VERBOSE_TSLR =      0x00000100,   // local task source runtime info (file re-naming...)
	VERBOSE_TSLLR =     0x00000200,   // LDR task source runtime info
	VERBOSE_0 =         0x10000000
};


// Global output params: 
extern  RVOutputParams rv_oparams;
// Set them up: 
extern void InitRVOutputParams(int &argc,char **argv,char **envp);

extern void Error(const char *fmt,...)    __attribute__ ((__format__ (__printf__, 1, 2)));
extern void Warning(const char *fmt,...)  __attribute__ ((__format__ (__printf__, 1, 2)));
extern void _Verbose(const char *fmt,...) __attribute__ ((__format__ (__printf__, 1, 2)));
extern void VerboseSpecial(const char *fmt,...) __attribute__ ((__format__ (__printf__, 1, 2)));

// Do the checking inline
#define Verbose(vspec,fmt...)  \
	do { \
		if(rv_oparams.vlevel_field & VERBOSE_##vspec) \
		{  _Verbose(fmt);  } \
	} while(0)

// Returns number of CPUs; assumes one CPU and warn if detection fails. 
extern int GetNumberOfCPUs();

// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
extern int GetLoadValue();

// Check if a file exists and we have the passed permissions: 
extern int CheckExistFile(RefString *f,int want_read,int want_write=0);

// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// error_prefix is put before error message; use e.g. "Local: ". 
extern int FirstFileIsNewer(RefString *a,RefString *b,const char *error_prefix);

// Simply delete a file (but not verbosely). 
// error_prefix: is put before error message; use e.g. "Local: ". 
// If may_not_exist is set, no error occurs if the file does not exist. 
// Return value: 0 -> OK; -1 -> error; 1 -> ENOENT && may_not_exist
extern int DeleteFile(RefString *path,int may_not_exist,
	const char *error_prefix);

// Rename a file; error is written if that fails. 
// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
extern int RenameFile(RefString *old_name,RefString *new_name,
	int may_not_exist,const char *error_prefix);

// Open input/output file. 
// dir: direction: -1 -> input; +1 -> output 
// Return value: 
//    >=0 -> valid FD
//     -1 -> file or file->str() NULL or dir==0
//     -2 -> open( failed (see errno)
extern int OpenIOFile(RefString *file,int dir);

#endif  /* _RNDV_LIB_PROTOTYPES_HPP_ */
