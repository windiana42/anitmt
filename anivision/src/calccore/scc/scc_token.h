/*
 * calccore/scc/scc_token.h
 * 
 * Simple calc core token translator class. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef _ANIVISION_CALCCORE_SCC_SCC_TOKEN_H_
#define _ANIVISION_CALCCORE_SCC_SCC_TOKEN_H_ 1

#include <hlib/refstrhash.h>


namespace CC { namespace S
{

// All the token IDs recognized: 
enum AniDescBlockTokID
{
	// MAKE SURE THESE ARE INDEXED 0..TID_LAST. 
	TID_None=0,
	
	TID_accel_scale,
	TID_addpts,
	TID_constant,
	TID_cspline,
	TID_curve,
	TID_dt,
	TID_fixed,
	TID_force_center,
	TID_force_up,
	TID_front,
	TID_front_mode,
	TID_function,
	TID_gravity,
	TID_interpol,
	TID_interpol_npts,
	TID_join,
	TID_length,
	TID_lspline,
	TID_move,
	TID_none,
	TID_pos,
	TID_pts,
	TID_pts_t,
	TID_pts_vt,
	TID_pts_xt,
	TID_speed,
	TID_t,
	TID_tvals,
	TID_up,
	TID_up_mode,
	
	TID_LAST
};


class SCCTokenTranslator
{
	private:
		RefStringHash<AniDescBlockTokID> tid2str_hash;
		size_t tid2str_hash_lookups;
		
	public:  _CPP_OPERATORS
		SCCTokenTranslator();
		~SCCTokenTranslator();
		
		// Translate AniDescBlockTokID into string representation: 
		static const char *Tok2Str(AniDescBlockTokID tid);
		
		// Lookup "type name" of an ADTNTypedScope: 
		// Returns -1 if not found, AniDescBlockTokID otherwise. 
		// This will actually use a hash for quick lookup. 
		int LookupTypedScopeName(const RefString &tname);
		// Dito for scope entry name (ADTNScopeEntry): 
		// Additionally returns TID_None if ename is a NULL ref. 
		int LookupScopeEntryName(const RefString &ename);
};

}}  // end of namespace CC::S

#endif  /* _ANIVISION_CALCCORE_SCC_SCC_TOKEN_H_ */
