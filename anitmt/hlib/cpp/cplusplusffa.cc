/*
 * cplusplusffa.cc
 * 
 * Array routines for _CPP_OPERATORS_FF macro. 
 * (Reserve memory/apply reservation -- simple.) 
 * 
 * Copyright (c) 2001--2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU Lesser General Public License version 2.1 as published by the 
 * Free Software Foundation. (See COPYING.LGPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */


#include "cplusplus.h"

#include <stdio.h>
#include <stdlib.h>

#ifndef TESTING
#define TESTING 1
#endif


#if TESTING
#warning TESTING switched on. 
#endif

void *_NewConstructArray(size_t nelem,size_t elsize,
	void (*constructor_func)(void *ptr,int *failflag))
{
	if(!nelem)  return(NULL);
	
	// NOTE: This function can (as all the others here) not 
	//       be used on classes with zero size. 
	size_t *mem=(size_t*)LMalloc(nelem*elsize+sizeof(size_t));
	if(!mem)  return(NULL);
	
	// Store the array size: 
	*mem=nelem;
	
	char *array=(char*)(mem+1);  // mem is of type size_t*. 
	int failflag=0;
	for(size_t i=0; i<nelem; i++)
	{
		char *ptr=array+(i*elsize);
		_NewPrepareMemory(elsize,ptr);
		(*constructor_func)(ptr,&failflag);  // serving as constructor 
		if(failflag)
		{
			// Got to destroy them again (ptr already destroyed). 
			if(i) do
			{
				--i;
				ptr=array+(i*elsize);
				(*constructor_func)(ptr,&failflag);  // serving as destructor
			}
			while(i);
			*mem=0;   // overwrite nelem entry with 0
			LFree(mem);
			return(NULL);
		}
	}
	
	return(array);
}


void _NewDestroyArray(void *_array,size_t elsize,
	void (*constructor_func)(void *ptr,int *failflag))
{
	if(!_array)  return;   // null-array...
	
	char *array=(char*)_array;
	size_t *mem=((size_t*)array)-1;
	size_t nelem=*mem;
	*mem=0;  // overwrite element number with 0
	
	#if TESTING
	if(nelem>0xffffff || !nelem)  // arrays of zero size should be NULL pointers!
	{  fprintf(stderr,"Strange array of size %u...\n",nelem);  }
	#endif
	
	size_t i=nelem;
	int failflag=-1;  // tell constructor_func to behave like destructor. 
	if(i) do
	{
		--i;
		char *ptr=array+(i*elsize);
		(*constructor_func)(ptr,&failflag);  // serving as destructor
	}
	while(i);
	
	LFree(mem);
}


#if 0
/**** Small test code: ****/

char *prg_name="atest";

struct Data
{
	_CPP_OPERATORS_FF
	int varA,varB;
	Data(int *ff)  { varA=varB=0; if(!(rand()%10)) --(*ff);  fprintf(stderr,"<C> "); }
	~Data()  {  fprintf(stderr,"<D> "); }
};

struct BLAH
{
	Data *d;
	public:  _CPP_OPERATORS_FF
		BLAH(int *ff=NULL)
		{
			d=NEWff<Data>(ff);
			if(!d)  --(*ff);
			fprintf(stderr,"Constr. <%p> %d (%p)\n",this,*ff,d);
		}
		~BLAH()
		{
			fprintf(stderr,"Destr. <%p> (%p)\n",this,d);
			if(d) delete d;
		}
};


#include <time.h>
#include <hlib/prototypes.h>
int main()
{
	LMallocSetLimit(1000);  // useful value: 100,1000
	
	srandom(time(NULL));
	BLAH *a=NEWarray<BLAH>(16);
	fprintf(stderr,"Array: %p (size %u)\n",a,NArraySize(a));
	
	DELarray(a);
	
	fprintf(stderr,"Alloc: %u; Peak: %u\n",
		LMallocCurrentUsage(),LMallocMaxUsage());
	return(0);
}

#endif
