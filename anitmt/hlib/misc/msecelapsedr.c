/*
 * msecelapsedr.c 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

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

/* Calc the time between *old and *current in msec. 
 * Current may be NULL (then it is queried from the system). 
 * Result rounded at division (see also msec_elapsed()). 
 */
long msec_elapsed_r(const struct timeval *old,const struct timeval *current)
{
	struct timeval _curr;
	long du;
	if(!current)
	{
		gettimeofday(&_curr,NULL);
		current=&_curr;
	}
	du = (current->tv_usec - old->tv_usec);
	return(
		(current->tv_sec  - old->tv_sec)*1000L + 
		((du<0L) ? (du-500) : (du+500) )/1000L );
}
