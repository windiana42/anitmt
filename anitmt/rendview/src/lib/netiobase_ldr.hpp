/*
 * netiobase_ldr.hpp
 * 
 * Basic network IO for LDR client and sever). 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _RNDV_LIB_NETWORKIOBASE_LDR_HPP_
#define _RNDV_LIB_NETWORKIOBASE_LDR_HPP_

#include <lib/netiobase.hpp>
#include <lib/ldrproto.hpp>


class NetworkIOBase_LDR : 
	public NetworkIOBase
{
	protected:
		
		struct RespBuf
		{
			size_t alloc_len;
			char *data;
			// Note: while the RespBuf is in use, this is !=Cmd_NoCommand; 
			// You may only modify the buffer if that is Cmd_NoCommand. 
			LDR::LDRCommand content;
		};
		
		// Send and receive buffers (used for different things): 
		RespBuf recv_buf;
		RespBuf send_buf;
		
		// Return value: 1 -> alloc failure 
		int _ResizeRespBuf(RespBuf *buf,size_t newlen);
		
		// Active commands being currently done by FDCopy in and out: 
		LDR::LDRCommand in_active_cmd;
		LDR::LDRCommand out_active_cmd;
		
		// Set up >FDCopy out< for sending passed RespBuf. 
		// NOTE: hdr->length, hdr->command MUST BE PASSED IN HOST ORDER 
		//       and are translated into network order. 
		// Start sending passed buffer using FDCopyBase etc. 
		// Also sets out_active_cmd. 
		// Retval: 
		//   0 -> OK
		//  -1 -> (alloc) failure
		int _FDCopyStartSendBuf(LDR::LDRHeader *hdr);
		
		// Read in command body; i.e. allocate ldr->length bytes, 
		// copy LDRHeader and start reading the missing number of 
		// bytes. Has protection against too long packets. 
		// NOTE: LDR::LDRHeader MUST BE IN HOST ORDER. 
		// Also sets in_active_cmd. 
		// Return value: 
		//    0 -> OK
		//   -1 -> alloc failure
		//   -2 -> LDRHeader contains illegal size
		int _StartReadingCommandBody(RespBuf *dest,LDR::LDRHeader *hdr);
		
	public:
		NetworkIOBase_LDR(int *failflag);
		~NetworkIOBase_LDR();
};

#endif  /* _RNDV_LIB_NETWORKIOBASE_LDR_HPP_ */
