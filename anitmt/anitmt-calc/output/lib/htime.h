/*
 * htime.h
 * 
 * Header containing class HTime, a general time class 
 * (timeval replacement) 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef CALC_ELAPSED_TIME
#  define CALC_ELAPSED_TIME 1
#endif


#ifndef _HLIB_HTime_H_
#define _HLIB_HTime_H_ 1

#if CALC_ELAPSED_TIME

//#include <stdlib.h>
//#include <unistd.h>
#include <sys/time.h>

extern "C"
{ long msec_elapsed(const struct timeval *old,const struct timeval *current); }

#include <string>
extern std::string Throughput_Str(size_t nbytes,double elapsed);

// timeval and time_t replacement 
class HTime
{
	public:
		enum TimeSpec
		{  usec=0,msec,seconds,minutes,hours,days,_tslast  };
	private:
		timeval tv;
		static const long long conv_fact[];
		static const double conv_factD[];
		
		void _SetVal(long val,TimeSpec sp,timeval *tv);
		void _Delta(const HTime *endtime,long long *delta) const;
	public:
		HTime() { }
		~HTime() { }
		
		// Copy: 
		HTime(const HTime &h) : tv(h.tv) { }
		HTime &operator=(const HTime &h)  {  tv=h.tv;  return(*this);  }
		
		// Store/set value (SetCurr() for current time; others never needed): 
		HTime(long val,TimeSpec sp=msec)  // BE SURE val>=0. 
			{  _SetVal(val,sp,&tv);  }
		int SetCurr()
			{  return(gettimeofday(&tv,NULL));  }
		HTime &Set(long val,TimeSpec sp=msec)  // BE SURE val>=0. 
			{  _SetVal(val,sp,&tv);  return(*this);  }
		
		// Arithmetics: (val may be <0) 
		HTime &Add(long val,TimeSpec sp=msec);
		HTime &Sub(long val,TimeSpec sp=msec);
		
		// To compare time values: 
		int operator==(const HTime &h) const
			{  return(tv.tv_sec==h.tv.tv_sec && tv.tv_usec==h.tv.tv_usec);  }
		int operator!=(const HTime &h) const
			{  return(tv.tv_sec!=h.tv.tv_sec || tv.tv_usec!=h.tv.tv_usec);  }
		int operator>(const HTime &h) const
			{  return(tv.tv_sec>h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec>h.tv.tv_usec));  }
		int operator<(const HTime &h) const
			{  return(tv.tv_sec<h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec<h.tv.tv_usec));  }
		int operator>=(const HTime &h) const
			{  return(tv.tv_sec>h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec>=h.tv.tv_usec));  }
		int operator<=(const HTime &h) const
			{  return(tv.tv_sec<h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec<=h.tv.tv_usec));  }
		
		// Time differences: 
		// starttime: *this; endtime: NULL=current 
		// BEWARE OF OVERFLOWS. 
		long   Elapsed (TimeSpec sp,const HTime *endtime=NULL) const;
		double ElapsedD(TimeSpec sp,const HTime *endtime=NULL) const;
		// Faster: 
		long MsecElapsed() const
			{  return(msec_elapsed(&tv,NULL));  }
		long MsecElapsed(const HTime *endtime) const  // endtime is NON-NULL
			{  return(msec_elapsed(&tv,&endtime->tv));  }
};


inline long HTime::Elapsed(TimeSpec sp,const HTime *endtime) const
{
	if(sp>=_tslast)  return(-1L);
	long long tmp;  _Delta(endtime,&tmp);
	return(long((tmp)/conv_fact[sp]));
}

inline double HTime::ElapsedD(TimeSpec sp,const HTime *endtime) const
{
	if(sp>=_tslast)  return(-1L);
	long long tmp;  _Delta(endtime,&tmp);
	return(double(tmp)/conv_factD[sp]);
}

#else  /* CALC_ELAPSED_TIME */

class HTime
{
	public:
		HTime() { }
		~HTime() { }
};

#endif  /* CALC_ELAPSED_TIME */

#endif  /* _HLIB_HTime_H_ */
