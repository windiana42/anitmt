/*
 * objdesc/frontspec_speed.h
 * 
 * Class for the front position using speed vector. 
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

#ifndef _ANIVISION_OBJDESC_FRONTSPEC_SPEED_H_
#define _ANIVISION_OBJDESC_FRONTSPEC_SPEED_H_

#include <objdesc/frontspec.h>


namespace NUM
{

// Front spec based upon speed vector. 
class OFrontSpec_Speed : public OFrontSpec
{
	private:
	public:  _CPP_OPERATORS
		OFrontSpec_Speed();
		~OFrontSpec_Speed();
		
		// [overriding virtual:]
		int Create(OCurve *ocurve,EvalContext *);
		
		// [overriding virtuals:]
		virtual int Eval(OCurve *ocurve,const double *pos_already_known,
			double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_FRONTSPEC_SPEED_H_ */
