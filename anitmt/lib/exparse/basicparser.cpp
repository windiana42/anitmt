/*
 * basicparser.cpp
 * 
 * Basic parser routines used by several flex-based parsers. 
 * 
 * Copyright (c) 2001 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <stdio.h>
#include "basicparser.hpp"

#warning "When using message system: remove these two lines."
#include <iostream>
using std::cerr;

namespace exparse
{

void BasicParser::SkipCComment()
{
	int ci,lastci=-256;
	int nested=1;
	int nestwarn = (config->allow_nested_comments==1) ? 0 : 1;
	for(;;)
	{
		ci=NextChar();
		if(ci==EOF)
		{
			cerr << "Comment terminated by EOF" << std::endl;
			break;
		}
		if(ci=='/' && lastci=='*')
		{
			if(!(--nested))
			{  break;  }
			lastci=-256;
		}
		else if(config->allow_nested_comments && ci=='*' && lastci=='/')
		{
			if(nested==1 && !nestwarn)
			{
				// REMOVE stdio.h WHEN CLEANED UP MESSAGES!
				fprintf(stderr,"Warning: nested C-style comment\n");
				++nestwarn;
			}
			++nested;
			lastci=-256;
		}
		else
		{  lastci=ci;  }
	}
}


void BasicParser::SkipCppComment()
{
	int ci;
	for(;;)
	{
		ci=NextChar();
		if(ci==EOF)   // C++ -style comment can be terminated by EOF
			break;
		if(ci=='\n')
			break;
	}
}

// Parse any string: 
int BasicParser::ParseString(std::string *rv,bool append)
{
	if(!append)
	{  rv->assign("");  }
	int ci;
	char c;
	int in_esc=0;
	int s_errors=0;
	#warning do it faster by using a string read buf
	for(;;)
	{
		ci=NextChar();
		if(ci==EOF)
		{
			fprintf(stderr,"String terminated by EOF\n");  
			++s_errors;
			break;
		}
		*((unsigned char*)&c)=(unsigned char)ci;
		if(c=='\n')
		{
			fprintf(stderr,"String terminated by newline\n");
			++s_errors;
			break;
		}
		else if(in_esc)
		{
			switch(c)
			{
				case 'n':   c='\n';  break;
				case 'r':   c='\r';  break;
				case 't':   c='\t';  break;
				case 'v':   c='\v';  break;
				case 'a':   c='\a';  break;
				case 'b':   c='\b';  break;
				case 'f':   c='\f';  break;
				case '\\':  c='\\';  break;
				case '\"':  c='\"';  break;
				case '\'':  c='\'';  break;
				default:
					fprintf(stderr,"Warning: unrecognized escape sequence "
						"`\\%c'.\n",c);
					rv->append("\\");
					break;
			}
			rv->append(&c,1);
			in_esc=0;
		}
		else if(c=='\\')
		{  in_esc=1;  }
		else if(c=='\"')
		{  break;  }
		else
		{  rv->append(&c,1);  }
	}
	errors+=s_errors;
	return(s_errors);
}

inline void BasicParser::_UpdatePos(int c,Position *p)
{
	if(c==EOF)  return;
	switch(c)
	{
		case '\t':  p->lpos = (p->lpos/config->tab_char_width+1)*config->tab_char_width;  break;
		case '\n':  p->lpos=0;  ++p->line;  break;
		case '\r':  break;
		case '\f':  /* fallthrough */
		case '\v':  ++p->line;  /* p->lpos=const. */  break;
		default:  ++p->lpos;  break;
	}
}

inline void BasicParser::_UpdatePos(char *str,int len,Position *p)
{
	// This function still has a potential for optimizations... 
	for(unsigned char *c=(unsigned char*)str,*cend=c+len; c<cend; c++)
	{  _UpdatePos(int(*c),p);  }
}

inline void BasicParser::_AddPos(Position *p,Position *delta)
{
	// FIXME: if file handle changes:
	/*if(filehandle_change)
	{
		p->fhandle=delta->fhandle;
		p->line=delta->line;
		p->lpos=delta->lpos;
		delta->line=0;
	} else*/
	if(delta->line)
	{
		p->line+=delta->line;
		p->lpos=delta->lpos;  // `=', NOT `+='
		delta->line=0;
	}
	else
	{  p->lpos+=delta->lpos;  }
	delta->lpos=0;
}


int BasicParser::NextChar()
{
	_AddPos(&p,&delta_p);
	int c=yy_input();
	_UpdatePos(c,&p);
	return(c);
}


int BasicParser::NextTok()
{
	int tok;
	for(;;)
	{
		_AddPos(&p,&delta_p);
		tok=yy_lex();
		currtok=tok;
		if(tok==tok_WSpace)
		{  _UpdatePos(*yy_text,*yy_leng,&p);  }
		else if(tok==tok_CComment)
		{
			_UpdatePos(*yy_text,*yy_leng,&p);
			SkipCComment();
		}
		else if(tok==tok_CppComment)
		{
			_UpdatePos(*yy_text,*yy_leng,&p);
			SkipCppComment();
		}
		else if(tok==tok_CCommentEnd)
		{
			_UpdatePos(*yy_text,*yy_leng,&p);
			cerr << "Stray closing C-style comment" << std::endl;
			++errors;
		}
		else if(tok==tok_Unknown)
		{
			char tmp=(*yy_text)[*yy_leng];
			(*yy_text)[*yy_leng]='\0';
			cerr << "Parse error near unexpected token `" << *yy_text << 
				"'" << std::endl;
			(*yy_text)[*yy_leng]=tmp;
			_UpdatePos(*yy_text,*yy_leng,&p);
			++errors;
		}
		else
		{
			assert(!delta_p.line && !delta_p.lpos);
			_UpdatePos(*yy_text,*yy_leng,&delta_p);
			break;
		}
	}
	return(tok);
}


void BasicParser::ResetParser()
{
	errors=0;
	currtok=-1;
}


BasicParserConfig::BasicParserConfig()
{
	// Set up compiled-in defaults: 
	allow_nested_comments=0;
	tab_char_width=8;
}

static const BasicParserConfig _default_conf;

BasicParser::BasicParser() : 
	#warning FIXME: message consultant is NULL. 
	message::Message_Reporter(NULL)
{
	tok_WSpace=-1;
	tok_CComment=-1;
	tok_CppComment=-1;
	tok_CCommentEnd=-1;
	tok_Unknown=-1;
	
	yy_text=NULL;
	yy_leng=NULL;
	
	ResetParser();
	config=&_default_conf;
	
	p.file="";
	p.line=1;
	p.lpos=0;
	delta_p.file="";
	delta_p.line=0;
	delta_p.lpos=0;
}

BasicParser::~BasicParser()
{
}

}  // namespace end 
