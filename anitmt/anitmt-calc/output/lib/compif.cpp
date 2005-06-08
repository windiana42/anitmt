/*
 * compif.cpp
 * 
 * Component Interface routines for POV output. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser
 * Bugs to > wwieser -a- gmx -*- de <
 * 
 * This is a part of the aniTMT animation project. 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * Revision History:
 *   Feb 2002   started writing
 *   Mar 2002   moved from pov/ to lib/ ; ITFype now independent type
 * 
 */


#include <animation.hpp>
#include "compif.hpp"

#include <assert.h>

namespace output_io
{

using namespace anitmt;

std::string ComponentInterface::get_name() const
{
	switch(type)
	{
		case IFNone:
			return(std::string(""));
		case IFScalar:
			return(((Scalar_Component_Interface*)cif)->get_name());
		case IFObject:
			return(((Object_Component_Interface*)cif)->get_name());
	}
	assert(0);
	return(std::string(""));
}

void ComponentInterface::_Assign(const void *src_cif,IFType src_type)
{
	assert(type==IFNone);
	type=src_type;
	switch(type)
	{
		case IFNone:  break;
		case IFScalar:
			cif = new Scalar_Component_Interface(
				*(Scalar_Component_Interface*)src_cif);
			break;
		case IFObject:
			cif = new Object_Component_Interface(
				*(Object_Component_Interface*)src_cif);
			break;
		default:  assert(0);  break;
	}
}

void ComponentInterface::Clear()
{
	switch(type)
	{
		case IFNone:  assert(!cif);  break;
		case IFScalar:
			if(cif)
			{  delete ((Scalar_Component_Interface*)cif);  }
			break;
		case IFObject:
			if(cif)
			{  delete ((Object_Component_Interface*)cif);  }
			break;
	}
	cif=NULL;
	type=IFNone;
}


ComponentInterface::ComponentInterface(const ComponentInterface &src)
{
	type=IFNone;
	cif=NULL;
	_Assign(src.cif,src.type);
}

ComponentInterface::ComponentInterface(const Scalar_Component_Interface &src)
{
	type=IFNone;
	cif=NULL;
	_Assign(&src,IFScalar);
}

ComponentInterface::ComponentInterface(const Object_Component_Interface &src)
{
	type=IFNone;
	cif=NULL;
	_Assign(&src,IFObject);
}

}  // namespace end
