/*
 * numerics/num_math.h
 * 
 * Numerics library config header. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _WW_NUMERICS_CONFIG_H_
#define _WW_NUMERICS_CONFIG_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef NUMERICS_STANDALONE

#include <hlib/cplusplus.h>
#include <hlib/lmalloc.h>

#else  /* <-- NUMERICS_STANDALONE */

#define _CPP_OPERATORS
#define _CPP_OPERATORS_FF

#include <stdio.h>
#include <stdlib.h>

inline void *LMalloc(size_t s)
{  return(s ? malloc(s) : NULL);  }
inline void *LFree(void *ptr)
{  if(ptr) free(ptr);  return(NULL);  }

#endif  /* NUMERICS_STANDALONE */

#include <assert.h>
// ns_assert is the non-critical assert. 
// This assertion may be left away when the program runs stable. 
// Put this into functions which shall never be called (e.g. non-C++-save 
// constructors or assignment ops). 
#define nc_assert(x)  assert(x)

#endif  /* _WW_NUMERICS_CONFIG_H_ */
