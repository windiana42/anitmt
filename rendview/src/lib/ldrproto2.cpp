/*
 * ldrproto2.cpp
 * LDR protocol constants and functions. 
 *
 * Copyright (c) 2002--2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "prototypes.hpp"  /* NEEDED for config.h / VERSION */
#include "ldrproto.hpp"

#include <hlib/htime.h>
#include <hlib/refstring.h>

#include <ctype.h>

#include <assert.h>
#include <netinet/in.h>

#include "mymath.hpp"

/*#if HAVE_FCNTL_H*/
# include <fcntl.h>
/*#endif*/

#include "sha1hash.hpp"


// Those functions which are also used by the admin shell. 

namespace LDR
{

// resp_buf: buffer of size LDRChallengeRespLength. 
// passwd may be NULL for -none-. 
void LDRComputeCallengeResponse(uchar *challenge,char *resp_buf,
	const char *passwd)
{
	// For empty passwords, the challenge is simply copied and 
	// the remainder is padded with zeros. 
	if(!passwd)
	{
		memcpy(resp_buf,challenge,
			LDRChallengeLength<LDRChallengeRespLength ? 
				LDRChallengeLength : LDRChallengeRespLength);
		if(LDRChallengeRespLength>LDRChallengeLength)
		{  memset(resp_buf+LDRChallengeLength,0,
			LDRChallengeRespLength-LDRChallengeLength);  }
		return;
	}
	
	// For non-empty passwords, a hash is computed: 
	// Feed challenge and password several times into hash...
	size_t pwl=strlen(passwd);
	SHA1Hash hash;
	for(int i=0; i<17; i++)
	{
		hash.Feed((char*)challenge,LDRChallengeLength);
		hash.Feed(passwd,pwl);
	}
	hash.Final();
	assert(hash.HashSize()==LDRChallengeRespLength);
	hash.GetHash(resp_buf);
}

}  // namespace end
