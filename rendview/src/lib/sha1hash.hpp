/*
 * sha1hash.hpp 
 * 
 * Originally sha1hash.h in HLib but copied for RendView. 
 * 
 * Header file containing class SHA1Hash, a class for 
 * computing the SHA1 mesage digest / hash algorithm as specified 
 * by NIST/NSA (not the original weaker SHA0 algorithm). 
 * 
 * Copyright (c) 2000--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. (See COPYING.GPL for details.)
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RendView_LIB_SHA1_H_
#define _RendView_LIB_SHA1_H_ 1

#include <hlib/prototypes.h>

class SHA1Hash
{
	private:
		u_int32_t state[5];  // hash status; hash value
		u_int64_t length;    // length of hashed data in BYTES. 
		
		unsigned char tmpbuf[64];  // may be over from last call to Feed()
		int tmp_size;   // number of bytes in tmpbuf 
		
		// msg has a size of 64 bytes. 
		void AtomicHash(const unsigned char *msg,u_int32_t *w);
	public:  _CPP_OPERATORS_FF
		SHA1Hash(int * /*failflag*/=NULL)  {  Reset();  }
		~SHA1Hash()  {  Reset();  }
		
		// Returns how many bytes were already fed (using Feed()) 
		// since the last Reset(). 
		u_int64_t FedBytes()  {  return(length);  }
		
		// How to compute a message digest / hash value: 
		//  1) Reset the hash algorithm using Reset() (this is also 
		//     done by the constructor.) 
		//  2) Give the hash algorithm input. Call Feed() as long 
		//     as you have input. 
		//  3) Do final calculation calling Final(). 
		//  4) Query the hash value using GetHash(). 
		
		// Reset algorithm data; clears current hash value 
		// returned by GetHash(). 
		void Reset();
		
		// Feed input into hash algorithm. 
		// buf does not have to be aligned; len may have any value. 
		// NOTE: Things are faster, if (len%BlockSize())=0. 
		void Feed(const char *buf,size_t len);
		
		// Does final calculation: pads the messge; adds its length, 
		// and computes the last hash stuff. 
		void Final();
		
		// Returns the hash value; make sure the buffer can hold at least 
		// HashSize() bytes. 
		size_t HashSize() const  {  return(20);  }
		void GetHash(char *buf);
};

#endif  /* _RendView_LIB_SHA1_H_ */
