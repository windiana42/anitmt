/*
 * refstrhash.h 
 * 
 * A hash template for mapping RefStrings to any other type. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _HLIB_RefStringHash_H_
#define _HLIB_RefStringHash_H_ 1

#include "refstring.h"


// This class contains all the code which can be separated 
// from the template RefStringHash. This class is for internal 
// use by RefStringHash, only. 
class _InternalRefStringHash
{
	public:
		struct EntryBase
		{
			EntryBase *next;
			RefString key;
			
			_CPP_OPERATORS_FF
			EntryBase(const RefString &_key,int * /*failflag*/) : 
				key(_key)  { next=NULL; }
			~EntryBase()  {}
		};
	protected:
		int hash_size;      // number of hash heads
		EntryBase **hash;   // array of hash_size - many pointers
		
		// The hash function: 
		int _dohash(const char *s) const;
		int _dohash(const RefString &x) const
			{  return(_dohash(x.str()));  }
		
		// Search for entry. 
		// Must pass hash value in hash_val. 
		// Re-queue at the beginning if lru_requeue is set. 
		EntryBase *_search(const char *key,int hash_val,int lru_requeue);
		// Search for entry and dequeue it (remove it from hash): 
		EntryBase *_search_and_dequeue(const char *key,int hash_val);
		// Implementig RefStringHash::store(): 
		int _store(const RefString &key,const void *value,int lru_requeue,
			int dont_search);
		// Implementing  RefStringHash::remove(): 
		int _remove(const RefString &key,const void *value);
		
		// Virtual functions for entry (de)allocation: 
		// THESE MUST BE OVERRIDDEN BY DERIVED CLASS. 
		virtual EntryBase *_NewEntry(const RefString & /*key*/,
			const void * /*value*/) HL_PureVirt(NULL)
		virtual void _DeleteEntry(EntryBase * /*ent*/) HL_PureVirt(;)
		virtual void _AssignEntry(EntryBase * /*ent*/,
			const void * /*value*/) HL_PureVirt(;)
	public:  _CPP_OPERATORS_FF
		// Construct hash with specified hash size. 
		// hash_size should be >=1. 
		_InternalRefStringHash(int hash_size,int *failflag=NULL);
		// Destructor clears hash: 
		virtual ~_InternalRefStringHash();
		
		// Remove all entries from the hash: 
		void clear();
		
		// Count number of entries: 
		int count() const;
};


// Everything inline for template. 
// Separable code extern in _InternalRefStringHash. 
template<class T>class RefStringHash : public _InternalRefStringHash
{
	public:
		struct Entry : EntryBase
		{
			T value;
			
			Entry(const RefString &_key,const T &v,int *failflag) : 
				EntryBase(_key,failflag),value(v)  {}
			~Entry()  {}
		};
	private:
		// Overriding virtual functions: 
		EntryBase *_NewEntry(const RefString &key,const void *value)
			{  return(NEW2<Entry>(key,*(const T*)value));  }
		void _DeleteEntry(EntryBase *ent)
			{  delete ((Entry*)ent);  }
		void _AssignEntry(EntryBase *ent,const void *value)
			{  ((Entry*)ent)->value=*((const T*)value);  }
		
	public:  _CPP_OPERATORS_FF
		// Construct RefStringHash of specified size. 
		// In case you want to use a prime as hash size -- here 
		// are some: 101, 307, 503, 1009, 5003, 10007, 20011, 
		//           30011, 40009, 65521
		// NOTE THAT RefStringHash CAN ONLY BE USED WITH '\0'-TERMINATED 
		// HASHES, ATM. 
		RefStringHash(int hash_size,int *failflag=NULL) : 
			_InternalRefStringHash(hash_size,failflag)
			{  }
		~RefStringHash()
			{  clear(); /* <-- important. */  }
		
		// Available functions from _InternalRefStringHash: 
		// - clear()
		
		// Store entry in the hash. That means: search for specified 
		// key and store new value under that key. If the key is not 
		// found, allocate a new entry. 
		// If you set dont_search=1, then the key is NOT searched for, 
		// which brings a speed increase BUT ONLY DO THAT IF YOU ARE 
		// SURE THAT THE KEY IS NOT YET PRESENT IN THE HASH. 
		// If you set lru_requeue, the found entry will be requeued at 
		// the head of the hash for faster access "next time". 
		// Return value: 
		//   1 -> OK, existing entry updated
		//   0 -> OK, new entry stored
		//  -1 -> allocation failure
		//  -2 -> key empty
		int store(const RefString &key,const T &value,
			int lru_requeue=1,int dont_search=0)
			{  return(_store(key,&value,lru_requeue,dont_search));  }
		
		// Search entry: 
		// Search for entry with passed key and store value under 
		// passed pointer, if non-NULL. 
		// If you set lru_requeue, the found entry will be requeued at 
		// the head of the hash for faster access "next time". 
		// Return value: 
		//   0 -> okay, found
		//   1 -> value not found
		//  -2 -> key empty
		// (Inline for speed increase.)
		int lookup(const RefString &key,T *value,int lru_requeue=1)
		{
			if(!key || *key.str()=='\0')  return(-2);
			Entry *e=(Entry*)_search(key,_dohash(key),lru_requeue);
			if(!e) return(1);  if(value) *value=e->value;  return(0);
		}
		
		// Remove entry from hash: 
		// The value of the entry is stored in *value if non-NULL. 
		// Return value: 
		//   0 -> okay, removed
		//   1 -> value not found
		//  -2 -> key empty
		int remove(const RefString &key,T *value=NULL)
			{  return(_remove(key,value));  }
			//if(!key || *key.str()=='\0')  return(-2);
			//Entry *e=(Entry*)_search_and_dequeue(key,_dohash(key),0);
			//if(!e) return(1);  if(value) *value=e->value;
			//delete e;  return(0);
		
		// This is a very special function: You can search using a 
		// char* string and get the RefString back if found. Can be 
		// used to re-use RefStrings for better memory use. 
		// Also returns a pointer to the value in the has entry. 
		// You can thus modiy the hash entry value directly. 
		// Return value: 
		//   0 -> okay, found
		//   1 -> value not found
		//  -2 -> key empty
		int LookupRefString(const char *key,RefString *dest,
			T **valptr=NULL,int lru_requeue=1)
		{
			if(!key || *key=='\0')  return(-2);
			Entry *e=(Entry*)_search(key,_dohash(key),lru_requeue);
			if(!e) return(1);  *dest=e->key;  if(valptr) *valptr=&e->value;
			return(0);
		}
};

#endif  /* _HLIB_RefStringHash_H_ */
