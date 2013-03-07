/*
 * btree-old.h 
 * 
 * Complete implementation of B-tree template. 
 * OLD VERSION before using better OP handling with member variable. 
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

#ifndef _HLIB_HLBTree_H_
#define _HLIB_HLBTree_H_ 1

#include <hlib/defop.h>

#ifndef HLIB_DEBUG_BTREE
#  define HLIB_DEBUG_BTREE 0
#endif

#if HLIB_DEBUG_BTREE
// See CheckTree() for this #define: 
#  define HLBT_ASSERT(x)  assert(x)
//#define HLBT_ASSERT(x)  ++nerr
#endif

// Note on the implementation: The "up" pointer in the nodes is functional 
// but not needed anywhere. Hence, it is currently defined out. 
#define HLIB_BTREE_USE_UP 0
#if HLIB_BTREE_USE_UP
#  define HLIB_BTREE_UPQ(x) x
#else
#  define HLIB_BTREE_UPQ(x)
#endif

// Implements standard B-Tree: 
// - Each tree node stores between m and 2*m elements (exception: 
//   the root may store less.) 
// - A node with k keys has k+1 children. 
// - All leaves are on the same level. 
// 
// B-Trees allow operations Store(), Remove(), Find() 
// (and FindNeighbours()/Smallest/Largest) in O(log(n)) runtime. 
// Memory consumption overhead is never worse than 50% (unless the tree 
// is nearly empty, i.e. O(m) elements). 
// 
// WHAT TO USE FOR <T>: 
// This class is probably most useful for T being a simple type 
// (integer, unique pointers to a class or small data-only structures). 
// You may use any POD type or any struct/class which is a "plain data type" 
// (PDT; see defOP::h) and even for "complex data types" as long as you can 
// provide the required operator plugin OP and it has a working default 
// constructor and a working copy constructor (both for temporaries). 
template<class T,class OP=HLDefaultOperators_PDT<T> >class HLBTree
{
	public:
		struct Node
		{
			#if HLIB_BTREE_USE_UP
			Node *up;     // NULL for root node. 
			#endif
			int nelem;    // Number of elements stored. 
			Node **down;  // Children nodes [2*m+1] or NULL if leaf. 
			T elem[0];    // Array of 2*m elements. 
			
			// Lookup (branch) index in this node: range 0..nelem (inclusive). 
			// Does binary search O(log(m)). 
			// If the node is found (exact match), return value is 1 (match), 
			// otherwise it is 0. The index in question is stored in *idx. 
			template<class K>bool _LBI(const K &e,int *idx)
			{
				int a=0,b=nelem-1;
				if(OP::lt(e,elem[0]))  {  *idx=0;  return(0);  }
				if(OP::lt(elem[b],e))  {  *idx=nelem;  return(0);  }
				while(b-a>1)  { int m=(a+b)/2; OP::lt(elem[m],e) ? a=m : b=m; }
				if(OP::eq(e,elem[a]))  {  *idx=a;  return(1);  }
				*idx=b;  return(OP::eq(e,elem[b]));
			}
			
			// Insert element before the passed index. 
			// Must be a leaf node, i.e. down=NULL. Relies on nelem>=1. 
			// idx=0 -> insert as first element; 
			// idx=nelem<2*m -> insert as last element; 
			// May NOT be called if splitting the node will be necessary, 
			// i.e. do not call if nelem==2*m. 
			void _INSLeafNS(int idx,const T &e)
			{
				if(idx==nelem)  {  OP::ini(&elem[nelem++],e);  return;  }
				int i=nelem-1;  OP::ini(&elem[nelem],elem[i]);
				for(; i>idx; i--)  OP::ass(elem[i],elem[i-1]);
				OP::ass(elem[idx],e);  ++nelem;
			}
			
			// Insert element and associated branch pointer before the 
			// passed index. 
			// Must NOT be a leaf node, i.e. down!=NULL. Relies on nelem>=1. 
			// idx=0 -> insert as first element; 
			// idx=nelem<2*m -> insert as last element; 
			// May NOT be called if splitting the node will be necessary, 
			// i.e. do not call if nelem==2*m. 
			void _INS_NS(int idx,const T &e,Node *chld)
			{
				down[nelem+1]=down[nelem];
				if(idx==nelem)
				{  OP::ini(&elem[nelem++],e);  down[idx]=chld;  return;  }
				int i=nelem-1;
				OP::ini(&elem[nelem],elem[i]);  down[nelem]=down[i];
				for(; i>idx; i--)
				{  OP::ass(elem[i],elem[i-1]);  down[i]=down[i-1];  }
				OP::ass(elem[idx],e);  down[idx]=chld;  ++nelem;
			}
			
			// Remove entry with passed index from leaf node. 
			void _DELLeaf(int idx)
			{
				--nelem;
				for(int i=idx; i<nelem; i++)  OP::ass(elem[i],elem[i+1]);
				OP::clr(&elem[nelem]);
			}
			
			// Remove entry with passed index and its _left_ down 
			// pointer from non-leaf node. 
			void _DEL_L(int idx)
			{
				--nelem;
				for(int i=idx; ; i++)
				{
					down[i]=down[i+1];  if(i>=nelem)  break;
					OP::ass(elem[i],elem[i+1]);
				}
				OP::clr(&elem[nelem]);  down[nelem+1]=NULL;
			}
			// Remove entry with passed index and its _right_ down 
			// pointer from non-leaf node. 
			void _DEL_R(int idx)
			{
				--nelem;
				for(int i=idx; i<nelem; )
				{
					OP::ass(elem[i],elem[i+1]);  ++i;
					down[i]=down[i+1];
				}
				OP::clr(&elem[nelem]);  down[nelem+1]=NULL;
			}
			
			_CPP_OPERATORS_FF
			#if HLIB_BTREE_USE_UP
			Node(Node *_up,int * /*failflag*/=NULL) : 
				up(_up),nelem(0),down(NULL) {}
			#else
			Node(int * /*failflag*/=NULL) : nelem(0),down(NULL) {}
			#endif
			~Node()  {  down=(Node**)LFree(down);  }
		};
		
	private:
		// The actual tree data fields: 
		int m;   // Min number of elements per tree node; max number is 2*m. 
		Node *root;   // Tree root. 
		T tmp;   // Temporary element for split/join operations. 
		
		// Some statistics: (Not implemented.)
		//size_t n_elems;   // Total number of elements in the tree. 
		//size_t n_nodes;   // Total number of nodes in the tree. 
		
		// Allocate a new leaf node; parent=NULL for root: 
		// Returns NULL on alloc failure. 
		Node *_AllocLeaf(HLIB_BTREE_UPQ(Node *parent))
		{
			int ff=0;
			#if HLIB_BTREE_USE_UP
			Node *n=NEW1plus<Node>(OP::size()*(m+m),parent,&ff);
			#else
			Node *n=NEWplus<Node>(OP::size()*(m+m),&ff);
			#endif
			if(n && ff)  {  delete n;  n=NULL;  }  return(n);
		}
		// Allocate non-leaf node: 
		Node *_AllocNonLeaf(HLIB_BTREE_UPQ(Node *parent))
		{
			Node *n=_AllocLeaf(HLIB_BTREE_UPQ(parent));  if(!n) return(NULL);
			if(!(n->down=(Node**)LMalloc(sizeof(Node*)*(m+m+1))))
			{  delete n;  return(NULL);  }
			for(int i=0,e=m+m; i<=e; i++)  n->down[i]=NULL;
			return(n);
		}
		// Deallocate a node: 
		// Must already have been removed from the tree and all elements 
		// cleaned up (i.e. n->nelem==0). 
		Node *_FreeNode(Node *n)
			{  delete n;  return(NULL);  }
		
		// Recursively delete tree. 
		void _DeleteTree(Node *tn)
		{
			if(!tn)  return;
			if(tn->down) for(int i=0; i<=tn->nelem; i++)
				_DeleteTree(tn->down[i]);
			for(int i=0; i<tn->nelem; i++)  OP::clr(&tn->elem[i]);
			_FreeNode(tn);
		}
		
		// Find element in subtree. 
		template<class K>T *_Find(const T &key,Node *tn)
		{
			for(int idx; ; tn=tn->down[idx])
			{
				if(tn->_LBI(key,&idx))  return(&tn->elem[idx]);
				if(!tn->down)  break;
			}
			return(NULL);
		}
		
		// Find leaf node with smallest/largest element in subtree. 
		Node *_FindSmallestLeaf(Node *n)
			{  while(n->down) n=n->down[0];  return(n);  }
		Node *_FindLargestLeaf(Node *n)
			{  while(n->down) n=n->down[n->nelem];  return(n);  }
		// Find smallest/largest element in subtree. 
		T *_FindSmallest(Node *n)
			{  return(&_FindSmallestLeaf(n)->elem[0]);  }
		T *_FindLargest(Node *n)
			{  n=_FindLargestLeaf(n);  return(&n->elem[n->nelem-1]);  }
		
		// Find element in subtree. 
		template<class K>T *_FindNeighbours(const K &key,Node *tn,T **p,T **n)
		{
			int idx;
			if(tn->_LBI(key,&idx))
			{
				if(p)  if(tn->down)  *p=_FindLargest(tn->down[idx]);
				       else if(idx>0)  *p=&tn->elem[idx-1];
				if(n)  if(tn->down)  *n=_FindSmallest(tn->down[idx+1]);
				       else if(idx+1<tn->nelem)  *n=&tn->elem[idx+1];
				return(&tn->elem[idx]);
			}
			T *rv = tn->down ? _FindNeighbours(key,tn->down[idx],p,n) : NULL;
			if(p && !*p && idx>0)  *p=&tn->elem[idx-1];
			if(n && !*n && idx<tn->nelem)  *n=&tn->elem[idx];
			return(rv);
		}
		
		// Return value: 
		// -1,0,1 -> see Store(). 
		//  2 -> split node
		int _Store(Node *tn,const T &e,Node **split)
		{
			int idx;
			if(tn->_LBI(e,&idx))  {  OP::ass(tn->elem[idx],e);  return(1);  }
			if(!tn->down)
			{
				if(tn->nelem<m+m)  {  tn->_INSLeafNS(idx,e);  return(0);  }
				// Split leaf node: 
				Node *l=_AllocLeaf(HLIB_BTREE_UPQ(tn->up));
				if(!l)  return(-1);  // (safe)
				int d=0,s=0;
				// left...
				while(d<m)
					OP::ini(&l->elem[d++],s==idx ? (idx=-1,e) : tn->elem[s++]);
				// ...middle...
				if(s==idx)  OP::ini(&tmp,(idx=-1,e));
				else  {  OP::ini(&tmp,tn->elem[s]);
				         if(s>=m) OP::clr(&tn->elem[s]);  ++s;  }
				// ...right
				for(d=0; d<m; )
					if(s==idx)  OP::ass(tn->elem[d++],(idx=-1,e));
					else  {  OP::ass(tn->elem[d++],tn->elem[s]);
					         OP::clr(&tn->elem[s++]);  }
				tn->nelem=m;  l->nelem=m;  *split=l;  return(2);
			}
			Node *isplit;  int rv=_Store(tn->down[idx],e,&isplit);
			if(rv!=2)  return(rv);
			if(tn->nelem<m+m)
			{  tn->_INS_NS(idx,tmp,isplit);  OP::clr(&tmp);  return(0);  }
			// Split non-leaf node: 
			T itmp(tmp);  OP::clr(&tmp);
			Node *l=_AllocNonLeaf(HLIB_BTREE_UPQ(tn->up));
			if(!l)  {  _DeleteTree(isplit);  return(-2);  }
			int d=0,s=0;
			// left...
			while(d<m)
				if(s==idx)  {  OP::ini(&l->elem[d],(idx=-1,itmp));
				               (l->down[d++]=isplit)HLIB_BTREE_UPQ(->up=l);  }
				else  {  OP::ini(&l->elem[d],tn->elem[s]);
				         (l->down[d++]=tn->down[s++])HLIB_BTREE_UPQ(->up=l);  }
			// ...middle...
			if(s==idx) {  (l->down[d]=isplit)HLIB_BTREE_UPQ(->up=l);
			              OP::ini(&tmp,(idx=-1,itmp));  }
			else {  OP::ini(&tmp,tn->elem[s]);  if(s>=m) OP::clr(&tn->elem[s]);
			        (l->down[d]=tn->down[s++])HLIB_BTREE_UPQ(->up=l);  };
			// ...right
			for(d=0; d<m; ) if(s==idx)
			{  OP::ass(tn->elem[d],(idx=-1,itmp));  tn->down[d++]=isplit;  }
			else
			{  OP::ass(tn->elem[d],tn->elem[s]);  OP::clr(&tn->elem[s]);
				tn->down[d++]=tn->down[s];  tn->down[s++]=NULL;  }
			tn->down[d]=tn->down[s];  tn->down[s]=NULL;
			tn->nelem=m;  l->nelem=m;  *split=l;  return(2);
		}
		
		// Handle underflow in tn->down[idx]. 
		// Return value: 
		//   0 -> OK
		//   1 -> underflow in tn. 
		bool _HandleUnderflow(Node *tn,int idx)
		{
			Node *l,*r;
			if(idx<tn->nelem)
			{
				l=tn->down[idx]; r=tn->down[idx+1];
				if(r->nelem>m)  // Transfer from right brother?
				{
					OP::ini(&l->elem[l->nelem++],tn->elem[idx]);
					OP::ass(tn->elem[idx],r->elem[0]);
					if(l->down)
					{  (l->down[l->nelem]=r->down[0])HLIB_BTREE_UPQ(->up=l);
					   r->_DEL_L(0);  }
					else  r->_DELLeaf(0);
					return(0);
				}
			} else {
				// Check left brother (since there is no right one). 
				l=tn->down[idx-1]; r=tn->down[idx];
				if(l->nelem>m)  // Transfer from left brother?
				{
					// For m==1, I'd need an extra case for r->nelem==0 here. 
					if(!r->down)  r->_INSLeafNS(0,tn->elem[idx-1]);
					else  {  r->_INS_NS(0,tn->elem[idx-1],l->down[l->nelem]);
					         HLIB_BTREE_UPQ(r->down[0]->up=r);  }
					OP::ass(tn->elem[idx-1],l->elem[l->nelem-1]);
					if(l->down)  l->_DEL_R(l->nelem-1); 
					else  l->_DELLeaf(l->nelem-1);
					return(0);
				}  --idx;  // <-- important...
			}
			// Merge left and right brother: 
			int d=l->nelem;
			OP::ini(&l->elem[d++],tn->elem[idx]);
			tn->_DEL_R(idx);
			for(int i=0; ; i++)
			{
				if(r->down)  (l->down[d]=r->down[i])HLIB_BTREE_UPQ(->up=l);
				if(i>=r->nelem)  break;
				OP::ini(&l->elem[d++],r->elem[i]);  OP::clr(&r->elem[i]);
			}
			l->nelem=d;  _FreeNode(r);  return(tn->nelem<m);
		}
		
		// Move smallest entry in passed subtree to passed entry. 
		// Return value: 
		//   1 -> underflow
		//   0 -> OK
		bool _MoveSmallest(Node *tn,T *ent)
		{
			if(!tn->down)
			{ OP::ass(*ent,tn->elem[0]); tn->_DELLeaf(0); return(tn->nelem<m); }
			if(_MoveSmallest(tn->down[0],ent))  // <- Underflow in tn->down[0]. 
				return(_HandleUnderflow(tn,0));
			return(0);
		}
		
		// Return value: 
		//   2 -> underflow
		//   1 -> not deleted /not found)
		//   0 -> OK, deleted
		template<class K>int _Remove(Node *tn,const K &key,T *e)
		{
			int idx;
			if(tn->_LBI(key,&idx))
			{	// Found it. 
				if(e)  OP::ass(*e,tn->elem[idx]);
				if(!tn->down)
				{  tn->_DELLeaf(idx);  return(tn->nelem<m ? 2 : 0);  }
				if(_MoveSmallest(tn->down[idx+1],&tn->elem[idx]) && 
					_HandleUnderflow(tn,idx+1))  return(2);
				return(0);
			}
			if(!tn->down)  return(1);
			int rv=_Remove(tn->down[idx],key,e);  if(rv!=2)  return(rv);
			return(_HandleUnderflow(tn,idx) ? 2 : 0);
		}
		
		#if HLIB_DEBUG_BTREE
		// Perform consistency check on tree: 
		int _CheckTree(Node *tn)
		{
			int nerr=0;
			// Make sure element number is right: 
			HLBT_ASSERT(tn->nelem<=m+m);
			HLBT_ASSERT(tn==root || tn->nelem>=m);
			// Make sure elements are sorted in ascenting order: 
			for(int i=1; i<tn->nelem; i++)
				HLBT_ASSERT(OP::lt(tn->elem[i-1],tn->elem[i]));
			if(tn->down) for(int i=0,e=m+m; i<=e; i++)
			{
				Node *d=tn->down[i];
				if(i>tn->nelem)
				{	/*fprintf(stderr,"(%d,%d,%d,%p=%d)",i,tn->nelem,tn->elem[0],d,d ? d->elem[0] : 0);*/
					HLBT_ASSERT(!d);  continue;  }
				HLBT_ASSERT(d);
				/*fprintf(stderr,"(%d,up=%p=%d,shall=%p=%d)",d->elem[0],d->up,d->up->elem[0],tn,tn->elem[0]);*/
				#if HLIB_BTREE_USE_UP
				HLBT_ASSERT(d->up==tn);
				#endif
				if(i>0) HLBT_ASSERT(OP::lt(tn->elem[i-1],d->elem[0]));
				nerr+=_CheckTree(tn->down[i]);
				if(i<tn->nelem)
					HLBT_ASSERT(OP::lt(d->elem[d->nelem-1],tn->elem[i]));
			}
			return(nerr);
		}
		#endif  /* HLIB_DEBUG_BTREE */
		
		// Recursively count number of elements in subtree. 
		size_t _CountElements(Node *tn) const
		{
			size_t c=tn->nelem;
			if(tn->down) for(int i=0; i<=tn->nelem; i++)
				c+=_CountElements(tn->down[i]);
			return(c);
		}
		
		// Recursvely count number of nodes. 
		size_t _CountNodes(Node *tn) const
		{
			size_t c=1;  // <-- this node
			if(tn->down) for(int i=0; i<=tn->nelem; i++)
				c+=_CountNodes(tn->down[i]);
			return(c);
		}
		
	public:  _CPP_OPERATORS_FF
		// m: Min number of number of elements per tree node; max
		//    number is 2*m. Must be at least 2; powers of 2 preferred. 
		HLBTree(int _m=16,int * /*failflag*/=NULL) : m(_m<2 ? 2 : _m),
			root(NULL),tmp() {}
		~HLBTree()  {  _DeleteTree(root); root=NULL;  /*No: clr(&tmp);*/  }
		
		// Clear the whole tree. 
		void Clear()
			{  _DeleteTree(root);  root=NULL;  }
		
		// Is the tree empty? Runtime O(1). 
		bool IsEmpty() const
			{  return(!root);  }
		
		// Count number of stored elements: 
		// Runtime is O(k)  (k = numaber of nodes <= number of elements/m)
		size_t CountElements() const
			{  return(root ? _CountElements(root) : 0);  }
		
		// Count number of tree nodes: 
		size_t CountNodes() const
			{  return(root ? _CountNodes(root) : 0);  }
		
		// Get size of a tree node as used internally to store the data: 
		// Total tree memory consumption is: 
		//   sizeof(HLBtree) + CountNodes()*GetNodeSize()
		size_t GetNodeSize() const
			{  return(sizeof(Node)+OP::size()*(m+m));  }
		
		// Find element in the tree. 
		// Runtime O(log(n))  (n = number of elements in the tree)
		// Returns pointer to the element or NULL. 
		// NOTE: You may change the element as long as you do not 
		//       change its ordering value (which is used by OP::lt()) 
		//       so that the tree stays consistent (=sorted). 
		template<class K>T *Find(const K &key)
			{  return(root ? _Find(key,root) : NULL);  }
		
		// Find the neighbours of passed key. 
		// Runtime O(log(n))  (n = number of elements in the tree)
		// prev,next return the next smaller and larger entries. 
		// If exact match is found, it is returned as return value. 
		// You may use prev/next=NULL if not interested. 
		// In case no prev/next value exists, NULL is returned in 
		// prev/next, respectively. 
		template<class K>T *FindNeighbours(const K &key,T **prev,T **next)
		{
			if(prev) *prev=NULL;  if(next) *next=NULL;  // Important for recursion. 
			return(root ? _FindNeighbours(key,root,prev,next) : NULL);
		}
		
		// Get smallest/largest element in the tree (or NULL): 
		// Runtime is O(depth)  ( depth = O(log(n/m)) ). 
		T *GetSmallest()
			{  return(root ? _FindSmallest(root) : NULL);  }
		T *GetLargest()
			{  return(root ? _FindLargest(root) : NULL);  }
		
		// Store passed elemet in the tree. 
		// Runtime O(log(n))  (n = number of elements in the tree)
		// Return value: 
		//   1 -> not stored but element replaced since it was already 
		//        in the tree
		//   0 -> OK, stored
		//  -1 -> allocation failure (element not inserted)
		//  -2 -> allocation failure during splitting in higher level
		//        whole sub-tree at split location was deleted (BAD!)
		int Store(const T &e)
		{
			if(!root)
			{
				if(!(root=_AllocLeaf(HLIB_BTREE_UPQ(NULL))))  return(-1);
				OP::ini(&root->elem[0],e);  root->nelem=1;  return(0);
			}
			Node *split;  int rv=_Store(root,e,&split);  if(rv!=2) return(rv);
			// Split root. 
			Node *nroot=_AllocNonLeaf(HLIB_BTREE_UPQ(NULL));
			if(!nroot)  {  OP::clr(&tmp);  _DeleteTree(split);  return(-2);  }
			nroot->down[0]=split;  HLIB_BTREE_UPQ(split->up=nroot);
			nroot->down[1]=root;  HLIB_BTREE_UPQ(root->up=nroot);
			OP::ini(&nroot->elem[0],tmp);  OP::clr(&tmp);
			nroot->nelem=1;  root=nroot;  return(0);
		}
		
		// Remove passed element. 
		// Runtime O(log(n))  (n = number of elements in the tree)
		// If e is non-null, store a copy of the found element in *e 
		// (using OP::ass()) before removing it. It is valid to write 
		//   T key_elem=<initialize key>;
		//   btree.Remove(key_elem,&key_elem);
		// Return value: 
		//   0 -> OK, element removed
		//   1 -> element not removed (not in tree)
		template<class K>int Remove(const K &key,T *e=NULL)
		{
			if(!root)  return(1);
			int rv=_Remove(root,key,e);  if(rv!=2)  return(rv);
			if(root->nelem)  return(0);
			if(!root->down)  {  root=_FreeNode(root);  return(0);  }
			Node *oroot=root;  root=root->down[0];  _FreeNode(oroot);  
			HLIB_BTREE_UPQ(root->up=NULL);  return(0);
		}
		
		#if HLIB_DEBUG_BTREE
		// For debugging: Print tree to stream. 
		// Only works for T=int. 
		void DumpTree(FILE *f,Node *tn=NULL,int lvl=0)
		{
			if(!tn && !lvl)  tn=root;  if(!tn)  return;
			for(int i=0; i<tn->nelem; i++)
			{
				if(tn->down) DumpTree(f,tn->down[i],lvl+1);
				for(int j=0; j<lvl; j++)  fprintf(f,"  ");
				fprintf(f,"%d\n",tn->elem[i]);
			}
			if(tn->down) DumpTree(f,tn->down[tn->nelem],lvl+1);
		}
		
		// For debugging: perform tree integrity checks. 
		// Require assert() or returns number of errors, see HLBT_ASSERT. 
		int CheckTree()
		{
			if(!root)  return(0);
			int nerr=0;
			#if HLIB_BTREE_USE_UP
			HLBT_ASSERT(!root->up);
			#endif
			return(nerr+_CheckTree(root));
		}
		#endif  /* HLIB_DEBUG_BTREE */
};

#if HLIB_DEBUG_BTREE
#  undef HLBT_ASSERT
#endif

#endif  /* _HLIB_HLBTree_H_ */
