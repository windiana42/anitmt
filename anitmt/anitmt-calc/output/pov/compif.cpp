/*
 * compif.cpp
 * 
 * Component Interface routines for POV output. 
 * 
 * Copyright (c) 2002 by Wolfgang Wieser
 * Bugs to wwieser@gmx.de
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
 *
 */


#include <animation.hpp>
#include "compif.hpp"

namespace output_io
{
namespace POV
{

using namespace anitmt;

std::string ComponentInterface::get_name() const
{
	switch(type)
	{
		case tNone:
			return(std::string(""));
		case taScalar:
			return(((Scalar_Component_Interface*)cif)->get_name());
		case taObject:
			return(((Object_Component_Interface*)cif)->get_name());
	}
	assert(0);
	return(std::string(""));
}

void ComponentInterface::_Assign(const void *src_cif,IFType src_type)
{
	assert(type==tNone);
	type=src_type;
	switch(type)
	{
		case tNone:  break;
		case taScalar:
			cif = new Scalar_Component_Interface(
				*(Scalar_Component_Interface*)src_cif);
			break;
		case taObject:
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
		case tNone:  assert(!cif);  break;
		case taScalar:
			if(cif)
			{  delete ((Scalar_Component_Interface*)cif);  }
			break;
		case taObject:
			if(cif)
			{  delete ((Object_Component_Interface*)cif);  }
			break;
	}
	cif=NULL;
	type=tNone;
}


ComponentInterface::ComponentInterface(const ComponentInterface &src)
{
	type=tNone;
	cif=NULL;
	_Assign(src.cif,src.type);
}

ComponentInterface::ComponentInterface(const Scalar_Component_Interface &src)
{
	type=tNone;
	cif=NULL;
	_Assign(&src,taScalar);
}

ComponentInterface::ComponentInterface(const Object_Component_Interface &src)
{
	type=tNone;
	cif=NULL;
	_Assign(&src,taObject);
}

}  // namespace end 
}  // namespace end   cif=NULL;cif=NULL;
