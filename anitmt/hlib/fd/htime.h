/*
 * htime.h
 * 
 * Header containing class HTime, a general time class 
 * (timeval replacement) 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_HTime_H_
#define _HLIB_HTime_H_ 1

#include <hlib/prototypes.h>

#include <sys/time.h>

// timeval and time_t replacement 
class HTime
{
	public:
		enum TimeSpec
		{  usec=0,msec,seconds,minutes,hours,days,_tslast  };
		enum _CurrentTime { Curr };
	private:
		timeval tv;
		static const long long conv_fact[];
		static const long long round_delta[];
		static const double conv_factD[];
		
		void _SetVal(long val,TimeSpec sp,timeval *tv);
		long long _Delta(const HTime *endtime) const;
		inline long long _LLConv(const timeval *tv) const
			{  return(((long long)tv->tv_sec)*1000000LL+((long long)tv->tv_usec));  }
		static inline long long _RoundAdd(long long x,TimeSpec sp)
			{  return(x + (x<0LL ? (-round_delta[sp]) : round_delta[sp]));  }
		static inline long _RoundAddMs(long x)
			{  return((x<0L) ? (x-500L) : (x+500L));  }
	public:  _CPP_OPERATORS
		HTime()  { }
		HTime(_CurrentTime)  {  SetCurr();  }
		~HTime()  { }
		
		// Copy: 
		HTime(const HTime &h) : tv(h.tv) { }
		HTime &operator=(const HTime &h)  {  tv=h.tv;  return(*this);  }
		
		// Store/set value (SetCurr() for current time; others never needed): 
		HTime(long val,TimeSpec sp=msec)  // BE SURE val>=0. 
			{  _SetVal(val,sp,&tv);  }
		int SetCurr()
			{  return(gettimeofday(&tv,NULL));  }
		HTime &operator=(_CurrentTime)  {  SetCurr();  return(*this);  }
		HTime &Set(long val,TimeSpec sp=msec)  // BE SURE val>=0. 
			{  _SetVal(val,sp,&tv);  return(*this);  }
		
		// Get stored time. This is only useful if HTime stores some 
		// elapsed time (e.g. consumed system time) and not a real 
		// date. 
		// Get() -> get integer value; result truncated at division
		// GetR() -> get integer value; result rounded at division
		// GetD() -> get floating point value 
		long   Get (TimeSpec sp) const
			{  return((sp<_tslast) ? long(_LLConv(&tv)/conv_fact[sp]) : (-1L));  }
		long   GetR(TimeSpec sp) const
			{  return((sp<_tslast) ? long(_RoundAdd(_LLConv(&tv),sp)/conv_fact[sp]) : (-1L));  }
		double GetD(TimeSpec sp) const
			{  return((sp<_tslast) ? (double(_LLConv(&tv))/conv_factD[sp]) : (-1.0));  }
		
		// This should not be used: 
		void SetTimeval(timeval *stv)
			{  tv=*stv;  }
		
		// Arithmetics: (val may be <0) 
		HTime &Add(long val,TimeSpec sp=msec);
		HTime &Sub(long val,TimeSpec sp=msec);
		
		// Calc time differences: (*this = endtime)
		HTime operator-(const HTime &start);
		HTime &operator-=(const HTime &start);
		
		// To compare time values: 
		inline int operator==(const HTime &h) const
			{  return(tv.tv_sec==h.tv.tv_sec && tv.tv_usec==h.tv.tv_usec);  }
		inline int operator!=(const HTime &h) const
			{  return(tv.tv_sec!=h.tv.tv_sec || tv.tv_usec!=h.tv.tv_usec);  }
		inline int operator>(const HTime &h) const
			{  return(tv.tv_sec>h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec>h.tv.tv_usec));  }
		inline int operator<(const HTime &h) const
			{  return(tv.tv_sec<h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec<h.tv.tv_usec));  }
		inline int operator>=(const HTime &h) const
			{  return(tv.tv_sec>h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec>=h.tv.tv_usec));  }
		inline int operator<=(const HTime &h) const
			{  return(tv.tv_sec<h.tv.tv_sec || 
			         (tv.tv_sec==h.tv.tv_sec && tv.tv_usec<=h.tv.tv_usec));  }
		// True if time value is 0, otherwise false: [not needed]
		//inline bool operator!() const
		//	{  return(!tv.tv_sec && !tv.tv_usec);  }
		
		// Time differences: 
		// starttime: *this; endtime: NULL=current 
		// BEWARE OF OVERFLOWS. 
		// Note: Elapsed()  -> result truncated at division
		//       ElapsedR() -> result rounded at division
		//       ElapsedD() -> floating point division
		long   Elapsed (TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? long(_Delta(endtime)/conv_fact[sp]) : (-1L));  }
		long   ElapsedR(TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? long(_RoundAdd(_Delta(endtime),sp)/conv_fact[sp]) : (-1L));  }
		double ElapsedD(TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? (double(_Delta(endtime))/conv_factD[sp]) : (-1.0));  }
		// Faster for milliseconds: 
		// MsecElapsed()  -> result truncated at division
		// MsecElapsedR() -> result rounded at division
		// NEVER supply NULL as argument endtime here! 
		// There are versions which take no argument for 
		//   endtime = current time
		long MsecElapsed(const HTime *endtime) const  // endtime is NON-NULL
			{  return((endtime->tv.tv_sec  - tv.tv_sec )*1000L + 
			          (endtime->tv.tv_usec - tv.tv_usec)/1000L );  }
		long MsecElapsedR(const HTime *endtime) const  // endtime is NON-NULL
			{  return( (endtime->tv.tv_sec  - tv.tv_sec )*1000L + 
			_RoundAddMs(endtime->tv.tv_usec - tv.tv_usec)/1000L );  }
		long MsecElapsed() const
			{  return(msec_elapsed(&tv,NULL));  }
		long MsecElapsedR() const
			{  return(msec_elapsed_r(&tv,NULL));  }
		
		// Print standard time string for local / universal time: 
		// local: 1 -> local time
		//        0 -> universal (Greenwich) time
		// with_msec: include msec spec
		// Returns static buffer. 
		// (No trailing '\n', of course...)
		const char *PrintTime(int local=1,int with_msec=0);
		
		// This is useful to print elapsed times: 
		// (You may use operator-() to calc elapsed time.) 
		// Returns static buffer. 
		const char *PrintElapsed();
};

#endif  /* _HLIB_HTime_H_ */
