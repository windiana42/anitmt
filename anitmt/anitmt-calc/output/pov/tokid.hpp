/*
 * tokid.hpp
 * 
 * Contains enum tokID, token identifiers for POV parser. 
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
 *   Feb 2002   created
 *
 */


#ifndef _Inc_IO_PovTokID_H_
#define _Inc_IO_PovTokID_H_ 1

namespace output_io
{
namespace POV
{

enum tokID
{
	// *** WHEN ADDING TOKENS, REMEMBER TO TAKE ***
	// *** A LOOK AT File_Parser::Find_Tok().   ***
	// General tokens
	tNone=-1,   // keep that
	tString=0,
	tMaskedString,   // \"
	tCOpComment,
	tCClComment,
	tCppComment,
	t_Comment,   // special for tCOpComment and tCppComment
	tNewline,
	tOpBrace,
	tClBrace,
	tAssign,
	tSemicolon,
	tNumbersign,
	tColon,
	
	// POV-Ray tokens
	tpDeclare,
	tpInclude,
	tpFileIdentifier,
	tpAniInsert,  // insert.obj.xx
	tpAniParse,   // parse.obj.xx
	
	// anitmt tokens
	taObject,
	taScalar   // adjust need_delim() if you add identifiers here 
	
};

inline ComponentInterface::IFType toIFType(tokID x)
{
	switch(x)
	{
		case tNone:     return(ComponentInterface::IFNone);
		case taScalar:  return(ComponentInterface::IFScalar);
		case taObject:  return(ComponentInterface::IFObject);
	}
	assert(0);
	return(ComponentInterface::IFNone);
}

inline tokID toTokID(ComponentInterface::IFType x)
{
	switch(x)
	{
		case ComponentInterface::IFNone:    return(tNone);
		case ComponentInterface::IFScalar:  return(taScalar);
		case ComponentInterface::IFObject:  return(taObject);
	}
	assert(0);
	return(tNone);
}



}}  // namespace end 

#endif  /* _Inc_IO_PovTokID_H_ */
