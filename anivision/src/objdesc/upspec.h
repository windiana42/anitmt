/*
 * objdesc/upspec.h
 * 
 * Class for the up position of object. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_OBJDESC_UPSPEC_H_
#define _ANIVISION_OBJDESC_UPSPEC_H_

#include <objdesc/curve.h>


namespace NUM
{

// Base class for up vector spec. 
class OUpSpec
{
	public:  _CPP_OPERATORS
		OUpSpec()
			{  }
		virtual ~OUpSpec()
			{  }
		
		// Create up spec. Must pass already-created associated 
		// curve which is needed for the time range. 
		// The curve itslef is not internally stored and must be passed 
		// again in the eval function. 
		// Return value: 
		//   0 -> OK
		//   1 -> error (reported) 
		virtual int Create(OCurve *ocurve,EvalContext *);
		
		// Evaluate function. 
		// Returned vector is NOT normalized. 
		// Must passe associated object curve (OCurve) and should pass 
		// current position in pos_already_known (use NULL if unknown; 
		// ocurve->Eval() will then be called if needed). 
		// In contrast, the current front vector MUST be passed. 
		// Return value: 
		//   0 -> OK
		// -1,+1 -> maybe okay but note that time is below/above range
		//  -2 -> no OCurve set or no curve in OCurve [or failing assertion instead...]
		virtual int Eval(OCurve *ocurve,const double *pos_already_known,
			const double *front_vector,
			double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_UPSPEC_H_ */
