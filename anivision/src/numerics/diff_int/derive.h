/*
 * numerics/diff_int/derive.h
 * 
 * Numerical derivation. 
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

#ifndef _WW_NUMERICS_DERIVE_H_
#define _WW_NUMERICS_DERIVE_H_

#include <numerics/function.h>


namespace NUM  // numerics
{

// Derive f at the position x; use the algorithm as implemented 
// in GSL. abserr returns estimated error if non-NULL. 
double DiffCentral_GSL(Function_R_R &f,double x,double *abserr=NULL);


}  // end of namespace NUM

#endif  /* _WW_NUMERICS_DERIVE_H_ */
