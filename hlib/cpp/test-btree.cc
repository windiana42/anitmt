/*
 * test-btree.cc 
 * 
 * Test HLBtree implementation. Quick test hack, nothing special but it 
 * also demonstrates a possible use of a not purely static operator 
 * class (struct MYOP below). 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define HLIB_DEBUG_BTREE 1

#if HLIB_DEBUG_BTREE
#include <assert.h>
#endif

#include <hlib/btree.h>
#include <hlib/lmalloc.h>

#include <stdio.h>
#include <assert.h>


char *prg_name="test-btree";


template<class T>struct MYOP
{
	int ini_calls;
	int clr_calls;
	
	MYOP()  {  ini_calls=0; clr_calls=0;  }
	~MYOP() {}
	
	static inline bool lt(T const &a,T const &b)  {  return(a<b);   }
	static inline bool le(T const &a,T const &b)  {  return(a<=b);  }
	static inline bool eq(T const &a,T const &b)  {  return(a==b);  }
	static inline bool ne(T const &a,T const &b)  {  return(a!=b);  }
	static inline T &ass(T &l,T const &r)  {  return(l=r);  }
	static const bool pdt=1;
	static inline size_t size() __attribute__((__const__))
		{  return(sizeof(T));  }
	
	inline void ini(T *)  { ++ini_calls; }
	inline void ini(T *p,T const &a)  {  *p=a;  ++ini_calls;  }
	inline void clr(T *)  { ++clr_calls; }
};


int main()
{
	fprintf(stderr,"Testing HLBtree... (size=%u (%u))\n",
		sizeof(HLBTree<int,MYOP<int> >),
		sizeof(MYOP<int>));
	
	int *ini_callsA;
	int *clr_callsA;
	int *ini_callsB;
	int *clr_callsB;
	int *ini_callsC;
	int *clr_callsC;
	int *ini_callsD;
	int *clr_callsD;
	
	do {
		HLBTree<int,MYOP<int> > A(10240);
		HLBTree<int,MYOP<int> > B(16);
		HLBTree<int,MYOP<int> > C(4);
		HLBTree<int,MYOP<int> > D(2);

		ini_callsA=&A.OP.ini_calls;
		clr_callsA=&A.OP.clr_calls;
		ini_callsB=&B.OP.ini_calls;
		clr_callsB=&B.OP.clr_calls;
		ini_callsC=&C.OP.ini_calls;
		clr_callsC=&C.OP.clr_calls;
		ini_callsD=&D.OP.ini_calls;
		clr_callsD=&D.OP.clr_calls;
		
		fprintf(stderr,"  Buildup");
		int nerr=0,i,modval=10000;
		for(i=0; i<5000; i++)
		{
			int val=rand()%modval;
			//fprintf(stderr,"---------<store=%d>----------\n",val);
			int rvA=A.Store(val);
			int rvB=B.Store(val);
			int rvC=C.Store(val);
			int rvD=D.Store(val);
			/*D.DumpTree(stderr);
			fprintf(stderr,"D: nelem=%d; ini/clr=(%d,%d,%d)\n",
				D.CountElements(),ini_callsD,clr_callsD,ini_callsD-clr_callsD);*/
			if(rvA!=rvB || rvA!=rvC || rvA!=rvD)
			{
				fprintf(stderr,"OOPS: store(%d): %d %d %d %d\n",
					val,rvA,rvB,rvC,rvD);
				++nerr;  return(1);
			}
			
			val=rand()%modval;
			//D.DumpTree(stderr);
			//fprintf(stderr,"---------<remove=%d>----------\n",val);
			rvA=A.Remove(val);
			rvB=B.Remove(val);
			rvC=C.Remove(val);
			rvD=D.Remove(val);
			/*D.DumpTree(stderr);
			fprintf(stderr,"D: nelem=%d; ini/clr=(%d,%d,%d)\n",
				D.CountElements(),ini_callsD,clr_callsD,ini_callsD-clr_callsD);*/
			if(rvA!=rvB || rvA!=rvC || rvA!=rvD)
			{
				fprintf(stderr,"OOPS: remove(%d): %d %d %d %d\n",
					val,rvA,rvB,rvC,rvD);
				++nerr;  return(1);
			}
			
			#if HLIB_DEBUG_BTREE
			int nerrA=A.CheckTree();
			int nerrB=B.CheckTree();
			int nerrC=C.CheckTree();
			int nerrD=D.CheckTree();
			if(nerrA || nerrB || nerrC || nerrD)
			{
				fprintf(stderr,"OOPS: check=%d,%d,%d,%d\n",
					nerrA,nerrB,nerrC,nerrD);
				++nerr;  return(1);
			}
			#endif
			
			if(!(i%100))
			{  fprintf(stderr,".");  }
		}
		fprintf(stderr,"%d (%d)\n",D.CountElements(),D.CountNodes());
		
		fprintf(stderr,"  Lookup");
		for(int j=0; j<10000; j++)
		{
			int val=rand()%(modval+100)-50;
			int *pA,*nA,*pB,*nB,*pC,*nC,*pD,*nD;
			int *rvA=A.FindNeighbours(val,&pA,&nA);
			int *rvB=B.FindNeighbours(val,&pB,&nB);
			int *rvC=C.FindNeighbours(val,&pC,&nC);
			int *rvD=D.FindNeighbours(val,&pD,&nD);
			if(!rvA!=!rvB || !rvA!=!rvC || !rvA!=!rvD || 
			   !pA!=!pB || !pA!=!pC || !pA!=!pD || 
			   !nA!=!nB || !nA!=!nC || !nA!=!nD )
			{
				fprintf(stderr,
					"OOPS: neigh=%d,%d,%d,%d; %p,%p,%p,%p; %p,%p,%p,%p *****\n",
					rvA,rvB,rvC,rvD,pA,pB,pC,pD,nA,nB,nC,nD);
				++nerr;  return(1);
			}
			if((pA && (*pA!=*pB || *pA!=*pC || *pA!=*pD)) || 
			   (nA && (*nA!=*nB || *nA!=*nC || *nA!=*nD)) || 
			   (rvA && (*rvA!=*rvB || *rvA!=*rvC || *rvA!=*rvD)) )
			{
				fprintf(stderr,"OOPS: neigh2(%d)=%d/%d/%d, %d/%d/%d, "
					"%d/%d/%d, %d/%d/%d\n",
					val,
					pA ? *pA : 0,rvA ? *rvA : 0,nA ? *nA : 0,
					pB ? *pB : 0,rvB ? *rvB : 0,nB ? *nB : 0,
					pC ? *pC : 0,rvC ? *rvC : 0,nC ? *nC : 0,
					pD ? *pD : 0,rvD ? *rvD : 0,nD ? *nD : 0);
				#if HLIB_DEBUG_BTREE
				D.DumpTree(stdout);
				#endif
				++nerr;  return(1);
			}
			
			if(rvA && *rvA!=val)
			{
				fprintf(stderr,"OOPS: neigh(%d): found=%d,%d,%d,%d\n",
					val,*rvA,*rvB,*rvC,*rvD);
				++nerr;  return(1);
			}
			
			if(rvA)
			{
				int *fA=A.Find(val);
				int *fB=B.Find(val);
				int *fC=C.Find(val);
				int *fD=D.Find(val);
				if(!fA || !fB || !fC || !fD)
				{
					fprintf(stderr,"OOPS: find(%d)=%p,%p,%p,%p\n",
						fA,fB,fC,fD);
					++nerr;  return(1);
				}
				if(*fA!=val || *fB!=val || *fC!=val || *fD!=val)
				{
					fprintf(stderr,"OOPS: find(%d)=%d,%d,%d,%d\n",
						*fA,*fB,*fC,*fD);
					++nerr;  return(1);
				}
			}
			
			if(!(j%500))
			{  fprintf(stderr,".");  }
		}
		fprintf(stderr,"\n");
		
		fprintf(stderr,"  Remove");
		for(;;i++)
		{
			int val=rand()%modval;
			//D.DumpTree(stderr);
			//fprintf(stderr,"---------<remove=%d>----------\n",val);
			int rvA=A.Remove(val);
			int rvB=B.Remove(val);
			int rvC=C.Remove(val);
			int rvD=D.Remove(val);
			/*D.DumpTree(stderr);
			fprintf(stderr,"D: nelem=%d; ini/clr=(%d,%d,%d)\n",
				D.CountElements(),ini_callsD,clr_callsD,ini_callsD-clr_callsD);*/
			if(rvA!=rvB || rvA!=rvC || rvA!=rvD)
			{
				fprintf(stderr,"OOPS: remove(%d): %d %d %d %d\n",
					val,rvA,rvB,rvC,rvD);
				++nerr;  return(1);
			}
			
			#if HLIB_DEBUG_BTREE
			if(!rvA || !(i%100))
			{
				int nerrA=A.CheckTree();
				int nerrB=B.CheckTree();
				int nerrC=C.CheckTree();
				int nerrD=D.CheckTree();
				if(nerrA || nerrB || nerrC || nerrD)
				{
					fprintf(stderr,"OOPS: check=%d,%d,%d,%d\n",
						nerrA,nerrB,nerrC,nerrD);
					++nerr;  return(1);
				}
			}
			#endif
			
			if(!(i%2000))
			{  fprintf(stderr,".");  }
			
			if(A.IsEmpty()) break;
		}
		fprintf(stderr,"%d (%d)\n",D.CountElements(),D.CountNodes());
		
		fprintf(stderr,"  nelem=%d,%d,%d,%d\n  "
			"ini/clr=A(%d,%d,%d),B(%d,%d,%d),C(%d,%d,%d),D(%d,%d,%d)\n",
			A.CountElements(),B.CountElements(),C.CountElements(),
			D.CountElements(),
			*ini_callsA,*clr_callsA,*ini_callsA-*clr_callsA,
			*ini_callsB,*clr_callsB,*ini_callsB-*clr_callsB,
			*ini_callsC,*clr_callsC,*ini_callsC-*clr_callsC,
			*ini_callsD,*clr_callsD,*ini_callsD-*clr_callsD );
	} while(0);
	
	fprintf(stderr,"  A: ini_calls=%d, clr_calls=%d, diff=%d\n",
		*ini_callsA,*clr_callsA,*ini_callsA-*clr_callsA);
	fprintf(stderr,"  B: ini_calls=%d, clr_calls=%d, diff=%d\n",
		*ini_callsB,*clr_callsB,*ini_callsB-*clr_callsB);
	fprintf(stderr,"  C: ini_calls=%d, clr_calls=%d, diff=%d\n",
		*ini_callsC,*clr_callsC,*ini_callsC-*clr_callsC);
	fprintf(stderr,"  D: ini_calls=%d, clr_calls=%d, diff=%d\n",
		*ini_callsD,*clr_callsD,*ini_callsD-*clr_callsD);
	
	if(*ini_callsA-*clr_callsA || 
	   *ini_callsB-*clr_callsB || 
	   *ini_callsC-*clr_callsC || 
	   *ini_callsD-*clr_callsD )
	{  return(1);  }
	
    // Allocation debugging: 
	LMallocUsage lmu;
	LMallocGetUsage(&lmu);
	fprintf(stderr,"%s: %sAlloc: %u bytes in %d chunks; Peak: %u by,%d chks; "
		"(%u/%u/%u)%s\n",
		prg_name,
		lmu.curr_used ? "*** " : "",
		lmu.curr_used,lmu.used_chunks,lmu.max_used,lmu.max_used_chunks,
		lmu.malloc_calls,lmu.realloc_calls,lmu.free_calls,
		lmu.curr_used ? " ***" : "");
	if(lmu.curr_used)  return(1);
	
	fprintf(stderr,"HLBTree test SUCCESSFUL.\n");
	return(0);
}
