/*
 * objdesc/move.h
 * 
 * Class to move object (pos,front,up) around in space. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_OBJDESC_MOVE_H_
#define _ANIVISION_OBJDESC_MOVE_H_

#include <hlib/linkedlist.h>

//#include <objdesc/curve.h>
#include <objdesc/frontspec.h>
#include <objdesc/upspec.h>


namespace NUM
{

// An object movement describes the complete object movement. 
// It contains a OCurve for the object position and further info 
// for the front and up vectors. 
// Probably used in a list, hence derived from list base class. 
class OMovement : public LinkedListBase<OMovement>
{
	private:
		// Position curve: 
		OCurve *ocurve;
		
		// Front vector spec: 
		OFrontSpec *ofront;
		
		// Up vector sped: 
		OUpSpec *oup;
		
		// Some flags: 
		int do_normalize_front : 1;   // Normalize front vector? Default: yes. 
		int do_normalize_up : 1;   // Normalize up vector? Default: yes. 
		
	public:
		// Creation time in "setting time ticks". 
		double ctime;
		
		// Set default  pos/front/up values: 
		static void DefaultPos(double *store_here);
		static void DefaultFront(double *store_here);
		static void DefaultUp(double *store_here);
		
	public:  _CPP_OPERATORS
		OMovement();
		~OMovement();
		
		// Get time range for this movement: 
		// This is the (mapped) curve time and hence the 
		// setting time after applying the creation time offset. 
		// Hence, T0() and T1() can be directly compared to 
		// the setting time. 
		double T0() const  {  return(ocurve ? ocurve->T0()+ctime : NAN);  }
		double T1() const  {  return(ocurve ? ocurve->T1()+ctime : NAN);  }
		
		// Convert setting time to movement time: 
		// This is done by subtracting the creation time offset. 
		double CvT(double t) const  {  return(t-ctime);  }
		
		// Get object properties at specified setting time. 
		// The properties are all 3d vectors and stored at the 
		// passed address. 
		// Front and up vectors are normalized (unless forced to do 
		// otherwise). 
		// You should pass the current position returned by GetPos() 
		//   the GetFront() to prevent a second evaluation; use NULL if 
		//   unknown. 
		// You should pass the current position and front vector to 
		//   the GetUp() function for the same reason (can use NULL for 
		//   both if unknown). 
		// Return value: 
		//   0 -> OK
		// -1,+1 -> maybe okay but note that time is below/above range
		//  -2 -> no OCurve set or no curve in OCurve
		int GetPos(double setting_t,double *store_here,EvalContext *) const;
		int GetFront(double setting_t,double *store_here,
			const double *pos_already_known,EvalContext *) const;
		int GetUp(double setting_t,double *store_here,
			const double *pos_already_known,
			const double *front_already_known,EvalContext *) const;
		
		//**** API for user parameters: ****
		
		// Set the position curve. 
		// This curve must have been allocated via operator new and 
		// is then passed to OMovement which will destroy it when no 
		// longer needed. 
		// Return value: 
		//   0 -> OK
		//   1 -> curve pos func was already set (in this case 
		//        the passed curvefunc was deleted to prevent 
		//        mem leaks) 
		int SetPosCurve(OCurve *ocurve);
		
		// Set the front/up spec. 
		// Return value: 
		//   0 -> OK
		//   1 -> front spec was already set (in this case 
		//        the passed front spec was deleted to prevent 
		//        mem leaks) 
		int SetFrontSpec(OFrontSpec *ofront);
		int SetUpSpec(OUpSpec *ofront);
		
		// Pass front/up mode string: 
		//   "normalize" or "!normalize" -> normalize front/up vector?
		// NOTE: Newer settings override older ones. 
		// Return value: 
		//   0 -> OK
		//   2 -> illegal value
		int SetFrontMode(const char *str);
		int SetUpMode(const char *str);
		
		// Create movement, i.e. call creation function on the 
		// position curve etc. Returns error count; diagnostics 
		// are written to user. 
		// current_setting_time is the setting time at creation 
		// and stored in this->ctime. 
		int Create(double current_setting_time,EvalContext *);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_MOVE_H_ */
