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

// Used to switch on/off colored output: (Initially 0)
extern int do_colored_output_stdout,do_colored_output_stderr;
void Error(const char *fmt,...);
void Warning(const char *fmt,...);
void Verbose(const char *fmt,...);
void VerboseSpecial(const char *fmt,...);

// Returns number of CPUs; assumes one CPU is detection fails. 
int GetNumberOfCPUs();

// Returns the system load multiplied with 100: 
// Returns -1 on systems where that is not supported. 
int GetLoadValue();

// Check if a file exists and we have the passed permissions: 
int CheckExistFile(RefString *f,int want_read,int want_write=0);

// Check if the first file is newer than the second one 
// (both should exist)
// Return value: 
//  1 -> first is newer
//  0 -> first is not newer
// -1 -> stat failed
// error_prefix is put before error message; use e.g. "Local: ". 
int FirstFileIsNewer(RefString *a,RefString *b,const char *error_prefix);

// Simply delete a file (but not verbosely). 
// error_prefix: is put before error message; use e.g. "Local: ". 
// If may_not_exist is set, no error occurs if the file does not exist. 
// Return value: 0 -> OK; -1 -> error; 1 -> ENOENT && may_not_exist
int DeleteFile(RefString *path,int may_not_exist,const char *error_prefix);

// Rename a file; error is written if that fails. 
// Return val: <0 -> error; 0 -> okay; 1 -> ENOENT && may_not_exist
int RenameFile(RefString *old_name,RefString *new_name,
	int may_not_exist,const char *error_prefix);

#endif  /* _RNDV_LIB_PROTOTYPES_HPP_ */
