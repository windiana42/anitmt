/*
 * objdesc/omatrix.h
 * 
 * Object matrix class describes location and orientation of object 
 * in space. 
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

#ifndef _ANIVISION_OBJDESC_OBJECTMATRIX_H_
#define _ANIVISION_OBJDESC_OBJECTMATRIX_H_

#include <hlib/cplusplus.h>


namespace NUM
{

class ObjectMatrix
{
	public:
		enum _NoInit { NoInit };
		enum _IdentMat { IdentMat };
		enum _ObjPosMat { ObjPosMat, ObjPosMatNrm };
		
		// Defaults for object front, up, right vectors (normally x,y,z): 
		static double obj_front[3];
		static double obj_up[3];
		static double obj_right[3];
	private:
		double m[3][4];    // [r][c]
	public:  _CPP_OPERATORS
		ObjectMatrix(_NoInit) {}
		ObjectMatrix(_IdentMat=IdentMat)
			{  for(short int i=0; i<12; i++) m[0][i]=(i%5 ? 0.0 : 1.0);  }
		ObjectMatrix(const ObjectMatrix &a)
			{  for(short int i=0; i<12; i++) m[0][i]=a.m[0][i];  }
		// Create ObjectMatrix representing object in space which is 
		// at position pos, and has specified front and up vectors. 
		// All the vectors are arrays double[3]. 
		// Use ObjPosMatNrm if the front/up vectors need normalisation, 
		// otherwise use ObjPosMat as first argument. 
		// Can also use SetObjPos() for that task. 
		ObjectMatrix(_ObjPosMat,const double *pos,const double *front,
			const double *up);
		~ObjectMatrix(){}
		
		// Get element: NO RANGE CHECK. [r][c]
		double *operator[](int r)
			{  return(m[r]);  }
		const double *operator[](int r) const
			{  return(m[r]);  }
		
		// This is much like the ObjectMatrix(_ObjPosMat,...) constructor. 
		// Exeption: It does NEVER normalize any passed vectors and 
		//           allows you to pass a "right" direction vector. 
		// Set right=NULL for default front x up (non-normalized but it 
		// will be automatically normalized if front and up are notmalized 
		// and perpendicular). 
		// obj_front, obj_up, obj_right are the front/up/right vectors 
		//    of the object in object coordinates and default to x,y,z. 
		//    NOTE: It is valid to use non-perpendicular values. 
		// Return value: 
		//    0 -> OK
		//    1 -> obj_front, obj_up, obj_right are linarly dependent. 
		int SetObjPos(const double *pos,
			const double *front,const double *up,const double *right,
			const double *obj_front  =ObjectMatrix::obj_front,
			const double *obj_up     =ObjectMatrix::obj_up,
			const double *obj_right  =ObjectMatrix::obj_right);
		
		// Assignment: 
		ObjectMatrix &operator=(const ObjectMatrix &a)
			{ for(short int i=0; i<12; i++) m[0][i]=a.m[0][i]; return(*this); }
		ObjectMatrix &operator=(_IdentMat)
			{ for(short int i=0; i<12; i++) m[0][i]=(i%5 ? 0.0 : 1.0); return(*this); }
		
		// Multiply two matrices...
		friend ObjectMatrix operator*(const ObjectMatrix &a,
			const ObjectMatrix &b);
		
		// Matrix * or / scalar: 
		ObjectMatrix &operator*=(double a)
			{ for(short int i=0; i<12; i++) m[0][i]*=a; return(*this); }
		ObjectMatrix &operator/=(double a)
			{ for(short int i=0; i<12; i++) m[0][i]/=a; return(*this); }
		
		// Add/subtract matrices: 
		ObjectMatrix &operator+=(const ObjectMatrix &a)
			{ for(short int i=0; i<12; i++) m[0][i]+=a.m[0][i]; return(*this); }
		ObjectMatrix &operator-=(const ObjectMatrix &a)
			{ for(short int i=0; i<12; i++) m[0][i]-=a.m[0][i]; return(*this); }
		
		// Query object transformation. 
		// Calculate the vector which results when transforming the 
		// passed object front / up / right / <any object-relative> vector. 
		// This basically calculates matrix * vector without adding the 
		// translation offset. 
		// All arguments 3dim vectors. 
		void CalcTrafo_Rel(double *res,const double *obj_rel_vect) const;
		
		// Calculate complete transformation on vector, 
		// i.e. matrix * vector including translation. 
		void CalcTrafo(double *res,const double *vect) const;
};

// Multiply two matrices: 
extern ObjectMatrix operator*(const ObjectMatrix &a,const ObjectMatrix &b);


}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_OBJECTMATRIX_H_ */
