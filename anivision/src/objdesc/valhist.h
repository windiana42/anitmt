/*
 * objdesc/valhist.h
 * 
 * Value history class. This class can store previous values 
 * (each value being an n-dimensional vector) and allows to interpolate 
 * the value at given time in history. Uses efficient B-trees for 
 * storage. 
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

#ifndef _ANIVISION_OBJDESC_VALUEHISTORY_H_
#define _ANIVISION_OBJDESC_VALUEHISTORY_H_

#include <hlib/btree.h>
#include <numerics/num_math.h>


namespace NUM
{

class ValueHistory
{
	public:
		// History interpolation point: 
		struct HIPoint
		{
			double t;
			double x[0];
		}__attribute__((__packed__));
	
	protected:
		// Operator class for HIPoint: 
		struct HLOP_HIPoint
		{
			// Used when comparing values: 
			double t_epsilon;

			// Dimension of the vector space. 
			int dim;

			static inline bool lt(HIPoint const &a,HIPoint const &b)
				{  return(a.t<b.t);   }
			static inline bool le(HIPoint const &a,HIPoint const &b)
				{  return(a.t<=b.t);  }
			
			inline bool eq(HIPoint const &a,HIPoint const &b)
				{  return(fabs(a.t-b.t)<t_epsilon);  }
			inline bool ne(HIPoint const &a,HIPoint const &b)
				{  return(fabs(a.t-b.t)>=t_epsilon);  }

			static inline bool lt(HIPoint const &a,double const &b)
				{  return(a.t<b);   }
			static inline bool le(HIPoint const &a,double const &b)
				{  return(a.t<=b);  }
			static inline bool lt(double const &a,HIPoint const &b)
				{  return(a<b.t);   }
			static inline bool le(double const &a,HIPoint const &b)
				{  return(a<=b.t);  }
			
			inline bool eq(HIPoint const &a,double const &b)
				{  return(fabs(a.t-b)<t_epsilon);  }
			inline bool ne(HIPoint const &a,double const &b)
				{  return(fabs(a.t-b)>=t_epsilon);  }
			inline bool eq(double const &a,HIPoint const &b)
				{  return(fabs(a-b.t)<t_epsilon);  }
			inline bool ne(double const &a,HIPoint const &b)
				{  return(fabs(a-b.t)>=t_epsilon);  }

			inline HIPoint &ass(HIPoint &l,HIPoint const &r)
			{
				l.t=r.t;
				for(int i=0; i<dim; i++)  l.x[i]=r.x[i];
				return(l);
			}

			static const bool pdt=1;
			
			inline size_t size()
				{  return(sizeof(HIPoint)+dim*sizeof(double));  }

			static inline void ini(HIPoint *) __attribute__((__const__)) {}
			inline void ini(HIPoint *p,HIPoint const &a)
				{  ass(*p,a);  }
			static inline void clr(HIPoint *) __attribute__((__const__)) {}
			
			_CPP_OPERATORS
			HLOP_HIPoint(int _dim) : dim(_dim) {}
			~HLOP_HIPoint() {}
		};
		
	private:
		// Dimension of vector space >=1. 
		int dim;
		// This counts the number of points currently in the history: 
		size_t npoints;
		// Max number of points to store: 
		size_t maxpoints;
		// The interpolation points are actually stored here: 
		HLBTree<HIPoint,HLOP_HIPoint> store;
		// Temporary point: 
		HIPoint *tmp_hp;
		
		// Remove a single old point to make place for new points to 
		// be stored. 
		void _RemoveOldPoint();
		
	public:  _CPP_OPERATORS
		// Must pass the dimension of the used vector space as argument. 
		// maxpoints is the max number of history points to store; use 
		//    value 0 to disable the limit. 
		ValueHistory(int dim,size_t maxpoints);
		~ValueHistory();
		
		// Get dimension of used vector space. 
		inline int Dim() const
			{  return(dim);  }
		
		// Get current number of history points: 
		inline size_t NPoints() const
			{  return(npoints);  }
		
		// Get time of oldest/newest point in the history: 
		// Returns NAN if the store is empty. 
		double GetOldestT()
			{  HIPoint *p=store.GetSmallest();  return(p ? p->t : NAN);  }
		double GetNewestT()
			{  HIPoint *p=store.GetLargest();  return(p ? p->t : NAN);  }
		
		// Clear all points: 
		void Clear();
		
		// Add an interpolation point at given time and position (value). 
		// x must de an Dim()-dimensional vector. 
		// Return value: 
		//   1 -> OK, stored and removed old element
		//   0 -> OK, stored
		int Store(double t,double *x);
		
		// Get nearest history points for passed time. 
		// Returns pointer to prev/next point in *p/*n 
		// (NOTE: not safe for operator= because of the member double x[0])
		// Return value: 
		//   0 -> OK, found  (*p!=NULL!=*n)
		//  -1 -> t < GetOldestT()  (*p=NULL)
		//  +1 -> t > GetNewestT()  (*n=NULL)
		//  -2 -> store empty  (*p=*n=NULL)
		//  +2 -> exact match stored in *p, nothing in *n=NULL. 
		int GetHistoryPoints(double t,const HIPoint **p,const HIPoint **n);
		// Same version but returning just the times: 
		// Uses NAN where the the above version uses NULL. 
		int GetHistoryPoints(double t,double &tp,double &tn);
		
		// Perform interpolation. 
		// Get interpolated value at time t. 
		// Return value: 
		//   0 -> OK (interpolation OK)
		//  -1 -> t < GetOldestT() (extrapolation)
		//  +1 -> t > GetNewestT() (extrapolation)
		//  -2 -> store empty (store_here=0)
		//  +2 -> exact match
		int GetPoint(double t,double *store_here);
};

}  // end of namespace NUM

#endif  /* _ANIVISION_OBJDESC_VALUEHISTORY_H_ */
