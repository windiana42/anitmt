/*
 * cplusplus.h
 * 
 * Special header for C++ classes to keep the executable 
 * small: we do not need the C++ library if every class 
 * provides an own operator delete and an own operator new. 
 * So, just insert the macro _CPP_OPERATORS into your class 
 * definition (must be public). 
 * 
 * See further down for a detailed description and how things work. 
 * 
 * Copyright (c) 1999-2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_cplusplus_h_
#define _HLIB_cplusplus_h_ 1

// Include config. 
#include <hlib/hconfig.h>

#ifdef __cplusplus
  extern "C" {
#endif
extern const char *prg_name;

// CheckMalloc(ptr) simply checks if ptr==NULL. 
// In this case, it aborts (writing an error to stderr) otherwise 
// it returns the pointer. 
extern void *CheckMalloc(void *);

// These are limited versions of malloc(), realloc() and free(). 
// You may set a limit on memory consumption; these routines will 
// fail if you try to exceed that limit. 
// Furthermore: - LMalloc(0) returns NULL
//              - LRealloc(NULL,size)=Malloc(size)
//              - LRealloc(ptr,0)=LFree(ptr)
//              - LFree(ptr) returns NULL and does nothing if ptr=NULL. 
// See misc/limitmalloc.c for a description. 
extern void *LMalloc(size_t size);    // calls malloc() but allows mem limit
extern void *LRealloc(void *,size_t);  // limited realloc()
extern void *LFree(void*);             // limited free() (needed for LMalloc() to work)

// Internal functions used by the NEW*<> templates. 
// _ptr: NULL -> use LMalloc(); else use _ptr. 
// ret: 0 -> success; -1 -> malloc() failed. 
extern int _NewPrepareMemory(size_t size,void *_ptr);
extern void *_NewPrepareApply(size_t size);

// Used to create arrays of elements with a constructor taking only 
// int *failflag. This is internal: see template NEWarray below. 
// Internal array construction/destruction function. 
// You can use the same constructor_func pointer for both. 
// See template _NEWarray_Constructor() for apropriate ones. 
extern void *_NewConstructArray(size_t nelem,size_t elsize,
	void (*constructor_func)(void *ptr,int *failflag));
extern void _NewDestroyArray(void *array,size_t elsize,
	void (*constructor_func)(void *ptr,int *failflag));

// You may pass the class name as string in opt. 
extern void ConstructorFailedExit(const char *opt=NULL);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

/* operator new/delete are never virtual as they are always static. */

// USING _CPP_OPERATORS: 
// To use _CPP_OPERATORS, simply put the macro `_CPP_OPERATORS' in the 
// public part of a class/struct definition. This ensures that we provide 
// our own versions of operator new and delete and thus we do not have 
// to link against some C++ library making the executable much smaller. 
// NOTE: _CPP_OPERATORS' new may return NULL if allocation fails thus 
//       making the constructor crash (SIGSEGV); delete will not fail 
//       on NULL pointers but if the destructor is called, it will crash. 
//   --> If you want to allocate/free a class with operator new and delete, 
//       have a look at _CPP_OPERATORS_FF below. 
#define _CPP_OPERATORS \
	void operator delete(void *_h_ptr) \
	{  LFree(_h_ptr);  }  \
	void *operator new(size_t _h_size) \
	{  return(CheckMalloc(LMalloc(_h_size)));  }

// This is the same as _CPP_OPERATORS but provides operators for 
// new[] and delete[]. Needs default constructor (i.e. without any 
// arguments) to work. Use NEWarray<> template for failsave version. 
#define _CPP_OPERATORS_ARRAY \
	void operator delete[](void *_h_ptr) \
	{  LFree(_h_ptr);  }  \
	void *operator new[](size_t _h_size) \
	{  return(CheckMalloc(LMalloc(_h_size)));  }

// USING _CPP_OPERATORS_FF: 
// To use _CPP_OPERATORS_FF, simply put the macro `_CPP_OPERATORS_FF' in 
// the public part of a class/struct definition. (See also _CPP_OPERATORS.)
// 
// What is special about _CPP_OPERATORS_FF: `_FF' stands for `fail flag'. 
// >> This provides a simple and efficient solution to two problems: 
//    *  class construction failure 
//         (e.g. constructor wants to aquire a resource but that fails)
//    *  memory allocation failure duing operator new. 
//    More on that below at `how to use it'. 
// NOTE: YOU SHOULD NEVER USE _CPP_OPERATORS_FF WITH CLASSES OF ZERO SIZE. 
// 
// This means that every class using these (de)allocation operators must 
// accept an `int *failflag=NULL' as the last argument to the constructor. 
// (You may provide other constructors, too, but they then cannot be used 
// in combination with _CPP_OPERATORS_FF's operator new and delete.)
// 
// How the failflag works: 
//   failflag is a pointer to an integer initialized to 0. Whenever 
//   something fails inside the constructor, the failflag is decremented. 
//   (NOT incrented!) Note that if the failflag pointer is NULL and 
//   construction fails, ConstructorFailedExit() is called instead of 
//   decreasing failflag. 
//   Of course, failflag must be passed down to all lower level 
//   constructors (base classes, members). 
//   NOTE: The constructor must make sure that the class can properly be 
//         destroyed by the destructor in case something failed in 
//         the constructor. 
//  
// Example: 
//   
//   class TTYCache  /* stupid example.. */
//   {
//       private:
//           SomeResource rsc;
//           char *data;
//       public:  _CPP_OPERATORS_FF
//           TTYCache(int *failflag=NULL) : rsc("/dev/tty",failflag)
//           {
//               int failed=0;           // nothing failed
//               
//               /** construction: on every failure, failed is INcremented **/
//               data=(char*)LMalloc(DATASIZE);
//               if(!data)  {  ++failed;  }  // allocation failed
//               
//               /** end of construction **/
//               if(failflag!=NULL)
//               {  *failflag-=failed;  }  // failflag DEcreased (failed>=0)
//               else if(failed)
//               {  ConstructorFailedExit("TTYCache");  }
//           }
//           ~TTYCache()
//           {
//               // Destructor can deal with case that data allocation 
//               // failed and thus data=NULL. 
//               if(data)
//               {  memset(data,0,DATASIZE);  LFree(data);  }
//           }
//    };
//  
//  Class SomeResource may look pretty much like class TTYCache: 
//  
//    class SomeResource {
//        int fd;
//        public:  _CPP_OPERATORS_FF
//        SomeResourc(const char *path,int *failflag=NULL)  {
//            int failed=0;
//            fd=open(path,O_RDONLY);   /* \___ construction section
//            if(fd<0)  ++failed;       /* /
//            if(failflag!=NULL)  *failflag-=failed;
//            else if(failed)  ConstructorFailedExit("SomeResourc");
//        }
//        ~SomeResourc()  {  // can deal with case that open() failed. 
//            if(fd>=0)  close(fd);
//        }
//    };
// 
// Okay, now how to use it: 
//   Whenever you want to savely allocate a class using _CPP_OPERATORS_FF's 
//   operator new and delete implementations use one of the convenience 
//   templates calles NEW<>, NEWff<>, NEWplus<>, ...
// 
// NEWff<T>(int *failflag): 
//   This template first tries to get memory for the class. 
//   If that fails, NULL is returned (failflag untouched)
//   If memory could be obtained, then (_CPP_OPERATORS_FF's) operator new 
//     is called to place the class inside the allocated piece of memory 
//     and construct it there. failflag is passed down to the constructor. 
//     A pointer to the class is returned. 
//   So, if NEWff returns NULL, allocation failed, if failflag was 
//   decreased, construction failed. 
//   
//   Example: (see example above for class TTYCache)
//     
//     int failflag=0;
//     TTYCache *tty=NEWff<TTYCache>(&failflag);
//     if(tty==NULL) 
//     {  /* oops allocation failed */  }
//     else if(failflag)
//     {  /* oops construction failed */
//        delete tty;  // IMPORTANT: Must free the memory again. 
//     }
//   ...or better...
//     int failflag=0;
//     TTYCache *tty=NEWff<TTYCache>(&failflag);
//     if(failflag)
//     {  delete tty;  tty=NULL;  }
//     if(tty==NULL)
//     {  /* oops: something (allocation/construction) failed */  }
//     
// As you will agree, the latter way is pretty much the thing you need 
// most of the time: First, allocate memory for a class, then construct 
// the class. If construction fails, deallocate it again. 
// In short: Either allocate a properly constructed class or return NULL. 
// This is exactly what NEW<> does: 
// 
// NEW<T>()
//   NEW<> uses NEWff(int *failflag) and immediately destructs and 
//   deallocates the class if construction failed. 
//   Return value: 
//     * NULL if memory allocation or class construction failed. 
//     * A pointer to a newly allocated and successfully constrcuted 
//       class of type T. 
// 
// You often want to specify additional aguments to the constructor. 
// If you use _CPP_OPERATORS_FF, the last argument must always be the 
// int *failflag. You may pass arguments to the constructor using 
// these functions: 
//
// NEW1ff<T>(p0, int *failflag)            new T(p0, failflag)
// NEW2ff<T>(p0,p1, int *failflag)         new T(p0,p1, failflag)
// NEW3ff<T>(p0,p1,p2, int failflag)       new T(p0,p1,p2, failflag)
//   These functions work exactly like NEWff<T>, but they pass 
//   additional arguments (p0,p1,...) to the constructor of T. 
//   (See the right column above.)
//
// NEW1<T>(p0)
// NEW2<T>(p0,p1)
// NEW3<T>(p0,p1,p2)
//   These functions are the analogons to NEW<T> allowing you to pass 
//   additional arguments (p0,p1,...) to the constructor. (Of course, 
//   the constructor must still accept an int *failflag as the last 
//   arg; NEW1<T> internally used NEW1ff<T>, etc...)
//
// Now, you may occationally want to allocate a special data class 
// which has a varaible size, e.g. layout like this: 
// struct data { _CPP_OPERATORS_FF
//    int length;
//    char data[0];
//    data()  {  length=0;  }
//    ~data() { }
// };
//
// In this case, you may use NEWplus: 
//
// NEWplus<T>(size_t extrasize,int *failflag)
//  This works exactly like NEWff<T>(failflag) but it allocates 
//  sizeof(T)+extrasize bytes for the class instead of sizeof(T) 
//  as NEWff<> does. 
//  Note that you must check the failflag yourself and 
//  destruct/deallocate (using operator delete) the class yourself 
//  in case allocation/construction failed. 
//  See NEWff<T>(int *failflag) above for a code example. 
// 
// Oh, and _CPP_OPERATORS_FF also allows you to construct arrays in 
// a safe way using NEWarray. NOTE that you must delete these arrays 
// using DELarray: 
// 
// NEWarray<T>(size_t nelem)
// DELarray<T>(T *ptr)
//   In C++, arrays of classes may only be built from classes which 
//   have a default constructor/constructor taking no arguments. 
//   With _CPP_OPERATORS_FF, it is pretty much the same: You may only 
//   create arrays of classes which have a constuctor taking only 
//   the `int *failflag' argument. 
//   
//   NEWarray will allocate an array of nelem elements of class T. 
//   If the memory allocation or any construction fails, it will 
//   free/destroy the aquired resources and return NULL. Else, a 
//   pointer to the first array elemnt is returned. 
//   
//   More detailed description: First, nelem*sizeof(T)+sizeof(size_t) 
//   bytes of memory are allocated (via LMalloc). 
//   That is memory for the complete array and an array size element. 
//   The array size is saved for DELarray and you may read the array 
//   size at any time using NArraySize(pointer_to_first_elem). 
//   If allocation fails (or nelem==0), NULL is returned. 
//   Now, all the nelem elements are constructed first-to-last using 
//   _CPP_OPERATORS_FF's operator new placing them at memory location 
//   ((char*)mem)+i*sizeof(T). 
//   If a constructor fails, the just constructed class is immediately 
//   destroyed, then all the previous array elements are destroyd in 
//   reverse order (last-to-first), memory is freed again and NULL is 
//   returned. 
//   
//   Return value: (NEWarray<>())
//     * NULL if nelem==0, memory allocation failed or a class 
//       construction failed. 
//     * A pointer to the first element in the array. 
// 
//   To delete an array, you have to use DELarray. DELarray always 
//   returns (T*)NULL. 
//   The function is simple: It destroys all the array elements 
//   last-to-first and the frees the memory chunk used for that array. 
//   
//   NOTE: YOU MUST DELETE AN ARRAY allocated by NEWarray USING 
//         DELarray. USING operator delete IS LETHAL. 
//   NOTE: NEVER TRY TO CREATE ARRAYS WITH ELEMENTS OF ZERO SIZE. 
//
// size_t NArraySize(array)
//   can be used to get the number of elements of the passed array 
//   (pointer to first element). If array=NULL, 0 is returned. 
//
// NOTES AND LIMITATIONS: 
//   * All the described functions shall never be used on objects 
//     of zero size. 
//   * In order to allocate/deallocate memory, all these functions 
//     use LMalloc() and LFree(). See misc/limitmalloc.cc for 
//     details. 
// 
#define _CPP_OPERATORS_FF \
	void operator delete(void *_h_ptr) \
	{  LFree(_h_ptr);  }    \
	void *operator new(size_t _h_size) \
	{  return(_NewPrepareApply(_h_size));  }  /* Calls CheckMalloc if not prepared. */  


// NOTE: DO NOT USE THESE FUNCTIONS ON CLASSES WITH ZERO SIZE. 

// NEWplus: allocate more bytes than sizeof(T). 
template<class T> inline T *NEWplus(size_t extrasize,int *failflag)
{  return(_NewPrepareMemory(sizeof(T)+extrasize,NULL) ? ((T*)NULL) : (new T(failflag)));  }
template<class T,class P0> inline T *NEW1plus(size_t extrasize,P0 p0,int *failflag)
{  return(_NewPrepareMemory(sizeof(T)+extrasize,NULL) ? ((T*)NULL) : (new T(p0,failflag)));  }

// NEWff: user wants to pass failflag. 
template<class T> inline T *NEWff(int *failflag)
{  return(_NewPrepareMemory(sizeof(T),NULL) ? ((T*)NULL) : (new T(failflag)));  }

// NEW: general purpose NEW. 
template<class T> inline T *NEW()
{
	int failflag=0;
	T *ptr=NEWff<T>(&failflag);
	if(ptr && failflag)
	{  delete ptr;  ptr=NULL;  }
	return(ptr);
}


// Additional templates for up to 3 additional args for the constructor: 
// NEW1ff(), NEW2ff(), NEW3ff() and...
template<class T,class P0> inline T *NEW1ff(P0 p0,int *failflag)
{  return(_NewPrepareMemory(sizeof(T),NULL) ? ((T*)NULL) : (new T(p0,failflag)));  }
template<class T,class P0,class P1> inline T *NEW2ff(P0 p0,P1 p1,int *failflag)
{  return(_NewPrepareMemory(sizeof(T),NULL) ? ((T*)NULL) : (new T(p0,p1,failflag)));  }
template<class T,class P0,class P1,class P2> inline T *NEW3ff(P0 p0,P1 p1,P2 p2,int *failflag)
{  return(_NewPrepareMemory(sizeof(T),NULL) ? ((T*)NULL) : (new T(p0,p1,p2,failflag)));  }

// ...NEW1(), NEW2(), NEW3() 
template<class T,class P0> inline T *NEW1(P0 p0)
{
	int failflag=0;  T *ptr=NEW1ff<T>(p0,&failflag);
	if(ptr && failflag)  {  delete ptr;  ptr=NULL;  }
	return(ptr);
}
template<class T,class P0,class P1> inline T *NEW2(P0 p0,P1 p1)
{
	int failflag=0;  T *ptr=NEW2ff<T>(p0,p1,&failflag);
	if(ptr && failflag)  {  delete ptr;  ptr=NULL;  }
	return(ptr);
}
template<class T,class P0,class P1,class P2> inline T *NEW3(P0 p0,P1 p1,P2 p2)
{
	int failflag=0;  T *ptr=NEW3ff<T>(p0,p1,p2,&failflag);
	if(ptr && failflag)  {  delete ptr;  ptr=NULL;  }
	return(ptr);
}

// Well, this is not really needed but helps in deleting. 
// Does nothing in ptr=NULL and sets ptr=NULL. 
template<class T> inline void DELETE(T* &ptr)
{  if(ptr)  {  delete ptr;  ptr=NULL;  }  }

// Internal function to be passed as function pointer to 
// (even more internal) function _NewConstructArray(). 
// This server as a constructor AND a destructor. 
template<class T> inline void _NEWarray_Constructor(void *ptr,int *failflag)
{
	// DO NOT TOUCH THAT (leave the two if's). 
	if(!(*failflag))  new T(failflag);  // new returns ptr here
	if(*failflag) ((T*)ptr)->~T();  /* `delete (T*)ptr' would call LFree(obj) */
}

// Returns the size of an array allocated by NEWarray<> or 0 if array=NULL. 
inline size_t NArraySize(void *array)
{  return(array ? (*(((size_t*)array)-1)) : 0);  }

// NEWarray: like NEW, but for arrays with nelem elements. 
// Each elem is created via call to operator new (_NewPrepareApply() 
// returns the correct address so that there is no gap in the array). 
// NOTE: THIS WILL BREAK IF YOU DO NOT INCLUDE _CPP_OPERATORS_FF INTO 
//       THE CLASS DEFINITION. 
template<class T> inline T *NEWarray(size_t nelem)
{  return((T*)_NewConstructArray(nelem,sizeof(T),&_NEWarray_Constructor<T>));  }

// DELarray: used to destroy arrays created by NEWarray. 
template<class T> inline T *DELarray(T *array)
{  _NewDestroyArray(array,sizeof(T),&_NEWarray_Constructor<T>);  return((T*)NULL);  }


// some reminders: 
//#define malloc(x)     Use_LMalloc_instead_of_malloc(x) 
//#define realloc(x,s)  Use_LRealloc_instead_of_realloc(x,s) 
//#define free(x)       Use_LFree_instead_of_free(x) 
//#define Free(x)       Use_LFree_instead_of_Free(x) 


/******************************************************************************/
// NOW, the issue about pure virtual functions: 
// You may 
//#define HL_PureVirt(x) =0;
// to see if the derived classes provide all functions they should. 
// This, however, will increase binary size as all sorts of stuff seems 
// to get linked in. 
// So, you may use: 
// (This will return x if the (former pure) virual function gets called.) 
//#define HL_PureVirt(x)  {  return x;  }
//NOTE: THIS IS NOW IN hconfig.h AND TURNED ON/OFF BY HLIB_SIZE_OPT


#endif /* _HLIB_cplusplus_h_ */

