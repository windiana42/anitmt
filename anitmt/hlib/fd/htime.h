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

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


// timeval and time_t replacement 
class HTime
{
	public:
		enum TimeSpec
		{  usec=0,msec,seconds,minutes,hours,days,_tslast  };
		enum _CurrentTime { Curr };
		enum _NullTime { Null };
		enum _InvalidTime { Invalid };
	private:
		timeval tv;
		static const int64_t conv_fact[];
		static const int64_t round_delta[];
		static const double conv_factD[];
		
		static inline void _Normalize(struct timeval *tv)
		{  register long m=tv->tv_usec/1000000;  if(tv->tv_usec<0)  --m;
			tv->tv_sec+=m;  tv->tv_usec-=1000000*m;  }
		
		void _SetVal(long val,TimeSpec sp,timeval *tv);
		void _SetValL(int64_t val,TimeSpec sp,timeval *tv);
		int64_t _Delta(const HTime *endtime) const;
		inline int64_t _LLConv(const timeval *tv) const
			{  return(int64_t(tv->tv_sec)*int64_t(1000000)+int64_t(tv->tv_usec));  }
		static inline int64_t _RoundAdd(int64_t x,TimeSpec sp)
			{  return(x + (x<0 ? (-round_delta[sp]) : round_delta[sp]));  }
		static inline long _RoundAddMs(long x)
			{  return((x<0) ? (x-500) : (x+500));  }
	public:  _CPP_OPERATORS
		HTime()  { }
		HTime(_CurrentTime)  {  SetCurr();  }
		HTime(_NullTime)  {  tv.tv_sec=0;  tv.tv_usec=0;  }
		HTime(_InvalidTime)  {  tv.tv_usec=-2000000;  }  // see SetInvalid()
		~HTime()  { }
		
		// Copy: 
		HTime(const HTime &h) : tv(h.tv) { }
		HTime &operator=(const HTime &h)  {  tv=h.tv;  return(*this);  }
		
		// Store/set value (SetCurr() for current time; others never needed): 
		HTime(long val,TimeSpec sp=msec)
			{  _SetVal(val,sp,&tv);  }
		int SetCurr()
			{  return(gettimeofday(&tv,NULL));  }
		HTime &operator=(_CurrentTime)  {  SetCurr();  return(*this);  }
		HTime &Set(long val,TimeSpec sp=msec)
			{  _SetVal(val,sp,&tv);  return(*this);  }
		HTime &SetL(int64_t val,TimeSpec sp=msec)
			{  _SetValL(val,sp,&tv);  return(*this);  }
		
		// Get stored time. This is only useful if HTime stores some 
		// elapsed time (e.g. consumed system time) and not a real 
		// date. Only call if !IsInvalid(). 
		// Get() -> get integer value; result truncated at division
		// GetL() -> get 64bit integer value; result truncated at division
		// GetR() -> get integer value; result rounded at division
		// GetD() -> get floating point value 
		long   Get (TimeSpec sp) const
			{  return((sp<_tslast) ? long(_LLConv(&tv)/conv_fact[sp]) : (-1));  }
		long   GetR(TimeSpec sp) const
			{  return((sp<_tslast) ? long(_RoundAdd(_LLConv(&tv),sp)/conv_fact[sp]) : (-1));  }
		int64_t GetL(TimeSpec sp) const
			{  return((sp<_tslast) ? (_LLConv(&tv)/conv_fact[sp]) : (-1));  }
		double GetD(TimeSpec sp) const
			{  return((sp<_tslast) ? (double(_LLConv(&tv))/conv_factD[sp]) : (-1.0));  }
		
		// This should not be used: 
		void SetTimeval(timeval *stv)
			{  tv=*stv;  _Normalize(&tv);  }
		
		// Arithmetics: (val may be <0) [Do not call if IsInvalid().]
		HTime &Add(long val,TimeSpec sp=msec);
		HTime &Sub(long val,TimeSpec sp=msec);
		
		// Calc time differences: (*this = endtime) [Do not call if IsInvalid().]
		HTime operator-(const HTime &start) const;
		HTime &operator-=(const HTime &start);
		
		// Add time differences: [Do not call if IsInvalid().]
		HTime operator+(const HTime &start) const;
		HTime &operator+=(const HTime &start);
		
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
		// BEWARE OF OVERFLOWS. Only call if !IsInvalid(). 
		// Note: Elapsed()  -> result truncated at division
		//       ElapsedL() -> result truncated at division
		//       ElapsedR() -> result rounded at division
		//       ElapsedD() -> floating point division
		long   Elapsed (TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? long(_Delta(endtime)/conv_fact[sp]) : (-1L));  }
		int64_t ElapsedL(TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? (_Delta(endtime)/conv_fact[sp]) : (-1L));  }
		long   ElapsedR(TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? long(_RoundAdd(_Delta(endtime),sp)/conv_fact[sp]) : (-1L));  }
		double ElapsedD(TimeSpec sp,const HTime *endtime=NULL) const
			{  return((sp<_tslast) ? (double(_Delta(endtime))/conv_factD[sp]) : (-1.0));  }
		// Faster for milliseconds: Only call if !IsInvalid(). 
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
		
		// This is special functionality as used e.g. by TimeoutManager. 
		// SetInvalid() sets an invalid time (currently usec=-2000000 
		// which never happens otherwise as usec must always be >0) 
		// and IsInvalid() checks if time is invalid. 
		void SetInvalid()  {  tv.tv_usec=-2000000;  }
		int IsInvalid() const  {  return(tv.tv_usec==-2000000);  }
};

#endif  /* _HLIB_HTime_H_ */
