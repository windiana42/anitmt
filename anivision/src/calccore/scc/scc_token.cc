/*
 * calccore/scc/scc_token.cc
 * 
 * Simple calc core token translator class. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "scc_token.h"

#include <assert.h>


namespace CC { namespace S
{

struct _TokDesc
{
	AniDescBlockTokID tid;
	const char *str;
	int is_scope_type : 1;
	int is_entry_name : 1;
};

static const _TokDesc tokdesc[TID_LAST]=
{
	{ TID_None,          "[none]",         0, 0 },
	
	{ TID_accel_scale,   "accel_scale",    0, 1 },
	{ TID_addpts,        "addpts",         0, 1 },
	{ TID_constant,      "constant",       1, 0 },
	{ TID_cspline,       "cspline",        1, 0 },
	{ TID_curve,         "curve",          1, 0 },
	{ TID_dt,            "dt",             0, 1 },
	{ TID_fixed,         "fixed",          1, 0 },
	{ TID_force_center,  "force_center",   0, 1 },
	{ TID_force_up,      "force_up",       0, 1 },
	{ TID_front,         "front",          1, 0 },
	{ TID_front_mode,    "front_mode",     0, 1 },
	{ TID_function,      "function",       1, 0 },
	{ TID_gravity,       "gravity",        1, 0 },
	{ TID_interpol,      "interpol",       0, 1 },
	{ TID_interpol_npts, "interpol_npts",  0, 1 },
	{ TID_join,          "join",           1, 0 },
	{ TID_length,        "length",         0, 1 },
	{ TID_lspline,       "lspline",        1, 0 },
	{ TID_move,          "move",           1, 0 },
	{ TID_none,          "none",           1, 0 },
	{ TID_pos,           "pos",            0, 1 },
	{ TID_pts,           "pts",            0, 1 },
	{ TID_pts_t,         "pts_t",          0, 1 },
	{ TID_pts_vt,        "pts_vt",         0, 1 },
	{ TID_pts_xt,        "pts_xt",         0, 1 },
	{ TID_speed,         "speed",          0, 1 },
	{ TID_t,             "t",              0, 1 },
	{ TID_tvals,         "tvals",          0, 1 },
	{ TID_up,            "up",             0, 1 },
	{ TID_up_mode,       "up_mode",        0, 1 },
};


const char *SCCTokenTranslator::Tok2Str(AniDescBlockTokID tid)
{
	if(tid==-1)  return("[unknown]");
	assert(tid>=0 && tid<TID_LAST);
	return(tokdesc[tid].str);
}


int SCCTokenTranslator::LookupTypedScopeName(const RefString &tname)
{
	++tid2str_hash_lookups;
	AniDescBlockTokID tok_id;
	if(!tid2str_hash.lookup(tname,&tok_id,/*lru_requeue=*/0))
	{
		assert(tok_id>=0 && tok_id<TID_LAST);
		if(tokdesc[tok_id].is_scope_type)
		{  return(tok_id);  }
	}
	return(-1);
}


int SCCTokenTranslator::LookupScopeEntryName(const RefString &ename)
{
	if(!ename)
	{  return(TID_None);  }
	
	++tid2str_hash_lookups;
	AniDescBlockTokID tok_id;
	if(!tid2str_hash.lookup(ename,&tok_id,/*lru_requeue=*/0))
	{
		if(tokdesc[tok_id].is_entry_name)
		{  return(tok_id);  }
	}
	
	return(-1);
}


SCCTokenTranslator::SCCTokenTranslator() : 
	tid2str_hash(TID_LAST)
{
	tid2str_hash_lookups=0;
	
	fprintf(stdout,"SCC: Setting up identifier hash: ");
	
	// Note: Beginning loop at 1 is correct; we don't need "[none]"
	//       in the hash. 
	for(int i=1; i<TID_LAST; i++)
	{
		RefString tmp;
		tmp.set(tokdesc[i].str);
		tid2str_hash.store(tmp,(AniDescBlockTokID)i,
				/*lru_requeue=*/0,/*dont_search=*/1);
		
		// The tokdesc array must be in the order of the tokens; 
		// this is being checked here: 
		assert(tokdesc[i].tid==i);
	}
	
	fprintf(stdout,"%d entries\n",tid2str_hash.count());
}

SCCTokenTranslator::~SCCTokenTranslator()
{
	fprintf(stdout,"SCC: Cleaning up identifier hash (%u lookups).\n",
		tid2str_hash_lookups);
}

}}  // end of namespace CC::S
