/*
 * htime.h
 * 
 * Header containing class HTime, a general time class 
 * (timeval replacement) 
 * 
 * Copyright (c) 2001--2004 by Wolfgang Wieser (wwieser@gmx.de) 
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
		enum _MostCurr { MostCurr };
	private:
		// This is updated each time HTime queries the current time 
		// from the sysem. If you need not-so-current time and think 
		// it is not worth the overhead, then use MostCurr. 
		static HTime most_current;
	public:
		// Always use this instead of ::gettimeofday(). 
		static inline int do_gettimeofday(timeval *tv)
			{  int rv=gettimeofday(tv,NULL); most_current.tv=*tv; return(rv);  }
		
		// Re-implementations honoring do_gettimeofday(): 
		static long my_msec_elapsed(const struct timeval *old);
		static long my_msec_elapsed_r(const struct timeval *old);
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
		
		static inline int64_t _LLConv(const timeval *tv)
			{  return(int64_t(tv->tv_sec)*int64_t(1000000)+int64_t(tv->tv_usec));  }
		static inline int64_t _RoundAdd(int64_t x,TimeSpec sp)
			{  return(x + (x<0 ? (-round_delta[sp]) : round_delta[sp]));  }
		static inline long _RoundAddMs(long x)
			{  return((x<0) ? (x-500) : (x+500));  }
		
		int _DoReadTime(const char *str);  // See ReadTime(). 
	public:  _CPP_OPERATORS
		HTime()  {  tv.tv_usec=-2000000;  }  // see SetInvalid()
		HTime(_CurrentTime)  {  SetCurr();  }
		HTime(_NullTime)  {  tv.tv_sec=0;  tv.tv_usec=0;  }
		HTime(_InvalidTime)  {  tv.tv_usec=-2000000;  }  // see SetInvalid()
		HTime(_MostCurr)  {  tv=most_current.tv;  }
		~HTime()  { }
		
		// Copy: 
		HTime(const HTime &h) : tv(h.tv) { }
		HTime &operator=(const HTime &h)  {  tv=h.tv;  return(*this);  }
		
		// This is special functionality as used e.g. by TimeoutManager. 
		// SetInvalid() sets an invalid time (currently usec=-2000000 
		// which never happens otherwise as usec must always be >0) 
		// and IsInvalid() checks if time is invalid. 
		void SetInvalid()  {  tv.tv_usec=-2000000;  }
		int IsInvalid() const  {  return(tv.tv_usec==-2000000);  }
		
		// Store/set value (SetCurr() for current time; others never needed): 
		HTime(long val,TimeSpec sp=msec)
			{  _SetVal(val,sp,&tv);  }
		int SetCurr()
			{  return(do_gettimeofday(&tv));  }
		HTime &operator=(_CurrentTime)  {  SetCurr();  return(*this);  }
		HTime &operator=(_InvalidTime)  {  SetInvalid();  return(*this);  }
		HTime &operator=(_NullTime)     {  tv.tv_sec=0;  tv.tv_usec=0;  return(*this);  }
		HTime &operator=(_MostCurr)     {  tv=most_current.tv;  return(*this);  }
		HTime &Set(long val,TimeSpec sp=msec)
			{  _SetVal(val,sp,&tv);  return(*this);  }
		HTime &SetL(int64_t val,TimeSpec sp=msec)
			{  _SetValL(val,sp,&tv);  return(*this);  }
		
		// Cut off msec/usec part of the time. 
		// Does nothing if time is invalid. 
		void PruneUsec()
			{  if(!IsInvalid())  tv.tv_usec=0;  }
		
		// Get stored time. This is only useful if HTime stores some 
		// elapsed time (e.g. consumed system time) and not a real 
		// date. Only call if !IsInvalid(). 
		// Get() -> get integer value; result truncated at division
		// GetR() -> get integer value; result rounded at division
		// GetL() -> get 64bit integer value; result truncated at division
		// GetLR() -> get 64bit integer value; result rounded at division
		// GetD() -> get floating point value 
		// DO NOT CALL IF TIME IS INVALID. 
		long   Get (TimeSpec sp) const
			{  return((sp<_tslast) ? long(_LLConv(&tv)/conv_fact[sp]) : (-1));  }
		long   GetR(TimeSpec sp) const
			{  return((sp<_tslast) ? long(_RoundAdd(_LLConv(&tv),sp)/conv_fact[sp]) : (-1));  }
		int64_t GetL(TimeSpec sp) const
			{  return((sp<_tslast) ? (_LLConv(&tv)/conv_fact[sp]) : (-1));  }
		int64_t GetLR(TimeSpec sp) const
			{  return((sp<_tslast) ? (_RoundAdd(_LLConv(&tv),sp)/conv_fact[sp]) : (-1));  }
		double GetD(TimeSpec sp) const
			{  return((sp<_tslast) ? (double(_LLConv(&tv))/conv_factD[sp]) : (-1.0));  }
		
		// This should not be used if avoidable: 
		void SetTimeval(timeval *stv)
			{  tv=*stv;  _Normalize(&tv);  }
		void SetTimeT(time_t st)
			{  tv.tv_sec=st;  tv.tv_usec=0;  }
		
		// Arithmetics: (val may be <0) 
		// Don't do anything if time is invalid. 
		HTime &Add(long val,TimeSpec sp=msec);
		HTime &Sub(long val,TimeSpec sp=msec);
		
		// Calc time differences: (*this = endtime) 
		// Result gets invalid if either time is invalid. 
		HTime operator-(const HTime &start) const;
		HTime &operator-=(const HTime &start);
		
		// Add time differences: 
		// Result gets invalid if either time is invalid. 
		HTime operator+(const HTime &start) const;
		HTime &operator+=(const HTime &start);
		
		// Negative time: (Stays Invalid if invalid.) 
		HTime &operator-();
		
		// Some more arithmetics useful for elapsed time: 
		// Divide time by passed factor (e.g. for averaging). 
		// Does nothing if time is invalid. 
		// fact<0 is allowed; fact=0 yields do invalid time. 
		HTime &Div(int fact);
		
		// To compare time values: 
		// NOTE: - The operator== and operator!= always return false 
		//         if at least one time is invalid. 
		//       - The other operators are UNDEFINED if at least 
		//         one time is invalid. 
		inline int operator==(const HTime &h) const
			{  return(!IsInvalid() && !h.IsInvalid() && 
			          tv.tv_sec==h.tv.tv_sec && tv.tv_usec==h.tv.tv_usec);  }
		inline int operator!=(const HTime &h) const
			{  return(IsInvalid() || h.IsInvalid() || 
			          tv.tv_sec!=h.tv.tv_sec || tv.tv_usec!=h.tv.tv_usec);  }
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
		// Special compare functions to compare against HTime::Invalid 
		// or HTime::Null: 
		inline int operator==(_InvalidTime) const
			{  return(IsInvalid());  }
		inline int operator!=(_InvalidTime) const
			{  return(!IsInvalid());  }
		inline int operator==(_NullTime) const
			{  return(!IsInvalid() && !tv.tv_sec && !tv.tv_usec);  }
		inline int operator!=(_NullTime) const
			{  return(!IsInvalid() && (tv.tv_sec || tv.tv_usec));  }
		inline int operator>(_NullTime) const
			{  return(tv.tv_sec>0 || (tv.tv_sec==0 && tv.tv_usec>0));  }
		inline int operator<(_NullTime) const
		//	{  return(tv.tv_sec<0 || (tv.tv_sec==0 && tv.tv_usec<0));  }
			{  return(tv.tv_sec<0);  }   // because tv.tv_usec<0 NEVER 
		inline int operator>=(_NullTime) const
		//	{  return(tv.tv_sec>0 || (tv.tv_sec==0 && tv.tv_usec>=0));  }
			{  return(tv.tv_sec>=0);  }  // because tv.tv_usec>=0 ALWAYS 
		inline int operator<=(_NullTime) const
			{  return(tv.tv_sec<0 || (tv.tv_sec==0 && tv.tv_usec<=0));  }
		
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
			{  return(my_msec_elapsed(&tv));  }
		long MsecElapsedR() const
			{  return(my_msec_elapsed_r(&tv));  }
		
		// Print standard time string for local / universal time: 
		// local: 1 -> local time
		//        0 -> universal (Greenwich) time
		// with_msec: include msec spec
		// Returns static buffer. 
		// (No trailing '\n', of course...)
		const char *PrintTime(int local=1,int with_msec=0) const;
		
		// FIXME: could add a function using strftime(3) 
		//        (time formatting like date(1)). 
		
		// This is useful to print elapsed times: 
		// (You may use operator-() to calc elapsed time.) 
		// Returns static buffer. 
		const char *PrintElapsed(int with_msec=1) const;
		
		// This can be used to read in time from a string. 
		// This is done in the following way: localtime(3) is used to 
		// get the current local time. 
		// In the string you may specify the TIME and DATE in any 
		// order separated by whitespace or comma "," (comma treated 
		// like wspace). 
		// TIME: HH:MM    -> implicitly set seconds to 0
		//       HH:MM:SS -> as usual
		//       examples: 17:23, 5:3, 06:1:02
		// DATE: DD.MM.     -> implicitly set current year. 
		//                     (NOTE the trailing dot ".")
		//       DD.MM.YYYY -> be sure to use the 4 digits for the 
		//                     year and NOT 2-digt abbreviation 
		//       YYYY/MM/DD -> ...if you like that one more
		// You may only specify one TIME and one DATE. 
		// Only specifying the TIME will use the current date. 
		// Only specifying the DATE will use current time at specified 
		//   date. 
		// Not specifying anything or just "now" or "NOW" will use the 
		//   current date and time. 
		// Time offset from current time can be specified with 
		//   now +|- HH:MM:SS   -or-   now +|- MM:SS  -or-  
		//   now +|- DD         -or-   now +|- :SS
		// Return value: 
		//      0 -> success
		//  -1,-2 -> format error
		//     -3 -> mktime failed (time out of range)
		// NOTE: Time can only be specified with second precision; 
		//       usec (sub-second) information is set to 0. 
		// NOTE: Time spec like "Aug 10 2002" and the like will NEVER 
		//       be supported by ReadTime(). 
		//       One may, however, add a ReadTime2() for that case. 
		// Will set HTime::Invalid if return value is not 0. 
		int ReadTime(const char *str)
			{  int rv=_DoReadTime(str);  if(rv) SetInvalid();  return(rv);  }
};

#endif  /* _HLIB_HTime_H_ */
