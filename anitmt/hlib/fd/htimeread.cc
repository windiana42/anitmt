/*
 * htimeread.cc
 * 
 * Time parsing method(s) for class HTime. 
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

#include <hlib/htime.h>

#include <ctype.h>


//                            ** WARNING: **
// DO NOT TOUCH THIS FUNCTION UNLESS YOU REALLY KNOW WHAT YOU ARE DOING. 
int HTime::ReadTime(const char *str)
{
	// First, set up t with current local time: 
	time_t timep;  ::time(&timep);
	struct tm *_ttmp=::localtime(&timep);
	struct tm t=*_ttmp;
	
	// state: 
	//   0 -> decide
	//   1 -> read date 15.06.[2002]
	//   2 -> read date 2002/06/15
	//   3 -> read time 17:22[:32]
	// done: bit 1,2 -> date
	//       bit   3 -> time
	//       ( so that we can check (done & (1<<state)) )
	short int state=0,done=0;
	int mode=0;  // mode=1 for "+"; mode=-1 for "-". 
	
	const char *s=str;
	int tmp;
	char *end;
	while(isspace(*s))  ++s;
	if(!strncmp(s,"now",3) || !strncmp(s,"NOW",3))
	{
		s+=3;
		while(isspace(*s))  ++s;
		if(*s)  // specified something after "now"
		{
			if(*s=='+')  mode=1;
			else if(*s=='-')  mode=-1;
			else  return(-2);
			// In this case I only allow to add a time, not a date. 
			++s;  done|=6;
			
			// Special cases: "now + DD" and "now + :SS": 
			while(isspace(*s))  ++s;
			// DO NOT modify s below here. 
			short int issec = (*s==':') ? 1 : 0;
			tmp=(int)strtol(s+issec,&end,10);
			while(isspace(*end))  ++end;
			if(!(*end) && tmp>=0 && end>s+issec)
			{
				if(issec)  t.tm_sec+=mode*tmp;
				else       t.tm_mday+=mode*tmp;
				s=end;   // --> *s='\0'
			}
			// fall through to rest
		}
		// If !(*s) we will not enter the while loop and set 
		// current date and time. 
	}
	
	// Prevent any negative values...
	if(strchr(s,'-'))
	{  return(-1);  }
	
	int data[3];
	int di=0;  // data index
	while(*s)
	{
		while(isspace(*s) || (state==0 && *s==','))  ++s;
		if(!(*s))  break;
		
		tmp=(int)strtol(s,&end,10);
		if(s==end)
		{
			//fprintf(stderr,"format error: %s\n",s);
			return(-1);
		}
		s=end;
		
		switch(state)
		{
			case 0:  // decide what to do
				switch(*s)
				{
					case '.':  state=1;  break;
					case '/':  state=2;  break;
					case ':':  state=3;  break;
					default:
						//fprintf(stderr,"format error: %s\n",s);
						return(-1);
				}
				if(state)
				{
					if(done & (1<<state))
					{
						//fprintf(stderr,"specified more than once\n");
						return(-2);
					}
					data[di++]=tmp;
					++s;
				}
				break;
			case 1:  // "15.06.2002" or "15.06."
				if(*s=='.' && di==1)  // month
				{
					data[di++]=tmp;
					++s;
					if(!(*s) || isspace(*s) || *s==',')
					{  goto datedone1;  }
				}
				else if(di==2)  // year
				{
					t.tm_year=tmp-1900;
					datedone1:
					t.tm_mday=data[0];
					t.tm_mon=data[1]-1;
					state=0;  di=0;  done|=6;
				}
				else
				{
					//fprintf(stderr,"format error: %s\n",s);
					return(-1);
				}
				break;
			case 2:  // "2002/06/15"
				if(*s=='/' && di==1)  // month
				{
					data[di++]=tmp;
					++s;
				}
				else if(di==2)   // day
				{
					t.tm_mday=tmp;
					t.tm_mon=data[1]-1;
					t.tm_year=data[0]-1900;
					state=0;  di=0;  done|=6;
				}
				else
				{
					//fprintf(stderr,"format error: %s\n",s);
					return(-1);
				}
				break;
			case 3:  // "17:22:32" or "17:22"
				if((*s==':' || !(*s) || isspace(*s) || *s==',') && di==1)  // minutes
				{
					data[di++]=tmp;
					tmp=0;
					if(*s==':')  ++s;
					if(!(*s) || isspace(*s) || *s==',')  goto  timedone;
				}
				else if(di==2)  // seconds
				{
					data[di++]=tmp;  // yes! [DO NOT REMOVE]
					timedone:
					if(mode==0)
					{
						t.tm_hour=data[0];
						t.tm_min=data[1];
						t.tm_sec=tmp;
					}
					else
					{
						         t.tm_sec+=mode*data[--di];
						if(di) { t.tm_min+=mode*data[--di];
						if(di)   t.tm_hour+=mode*data[--di]; }
					}
					state=0;  di=0;  done|=8;
				}
				else
				{
					//fprintf(stderr,"format error: %s\n",s);
					return(-1);
				}
				break;
		}
	}
	if(di)
	{
		//fprintf(stderr,"format error: di=%d\n",di);
		return(-1);
	}
	
	timep=mktime(&t);
	if(timep<0)
	{
		//fprintf(stderr,"mktime error\n");
		return(-3);
	}
	
	// Store it...
	tv.tv_sec=timep;
	tv.tv_usec=0;
	
	// Check: This may show a time difference becuase LOCAL time 
	//        was read in. 
	//fprintf(stderr,"SUCCESS= %s",ctime(&timep));
	
	return(0);
}


#if 0
int main(int argc,char **arg)
{
	HTime h;
	for(int i=1; i<argc; i++)
	{
		h.ReadTime(arg[i]);
	}
	return(0);
}
#endif
