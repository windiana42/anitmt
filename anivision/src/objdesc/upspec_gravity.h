/*
 * objdesc/upspec_gravity.h
 * 
 * Class for the up position of object based upon gravity model. 
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

#ifndef _ANIVISION_OBJDESC_UPSPEC_GRAVITY_H_
#define _ANIVISION_OBJDESC_UPSPEC_GRAVITY_H_

#include <objdesc/upspec.h>


namespace NUM
{

// Base class for up vector spec. 
class OUpSpec_Gravity : public OUpSpec
{
	private:
		// Force up vector, i.e. direction of the gravitational force. 
		// (Actually, the force is anti-parallel to prim_up.) 
		// Default: y. 
		// In case is_center is set, it is the location of the 
		// gravitational center. 
		double force_up[3];
		int is_center : 1;
		int force_up_set : 1;
		
		// Influence of accel vector on up vector; defaults to 0. 
		int use_accel_scale : 1;
		double accel_scale;
		
	public:  _CPP_OPERATORS
		OUpSpec_Gravity();
		~OUpSpec_Gravity();
		
		//**** API for user parameters: ****
		
		// Set force up vector (or center location if is_center is set). 
		// Return value: 
		//   0 -> OK
		//   1 -> already set; nothing done
		//   3 -> already set with different is_center sped
		int SetForce(double *force_up,bool is_center);
		
		// Set influence of acceleration on up vector. 
		//   0 -> OK
		//   1 -> already set; nothing done
		int SetAccelScale(double accel_scale);
		
		// [overrding virtual:]
		int Create(OCurve *ocurve,EvalContext *);
		
		// [overrding virtual:]
		int Eval(OCurve *ocurve,const double *pos_already_known,
			const double *front_vector,
			double t,double *result,EvalContext *) const;
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_UPSPEC_GRAVITY_H_ */
