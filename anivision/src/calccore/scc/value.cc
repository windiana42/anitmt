/*
 * calccore/scc/value.cc
 * 
 * Simple calc core value main file. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "ccif_simple.h"

namespace CC { namespace S
{


CCValue::CCValue(CCSetting *_setting,int _dimension) : 
	CCIF_Value(_setting,_dimension),
	LinkedListBase<CCValue>()
{
	Setting()->RegisterValue(this);
}

CCValue::~CCValue()
{
	Setting()->UnregisterValue(this);
}

}}  // end of namespace CC::S
