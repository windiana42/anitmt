/*
 * parse.cpp
 * 
 * Expression parser routines (exparse). 
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

#include "lexer.hpp"

#include "tree.hpp"

#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include <strstream>

#warning Remove me: 
int bt_quiet=0;   // be quiet (benchmark test) 

namespace exparse {

ostream &cwarn=cerr;

struct TokenDesc
{
	OperatorType otype;  // corresponding primary OperatorType or _OTLast 
};

static const TokenDesc tdesc[_TLast]=
{
	/*** General: ***/
	{ _OTLast },  // TIdentifier,
	{ _OTLast },  // TWSpace,
	{ _OTLast },  // TCComment,
	{ _OTLast },  // TCCommentEnd,
	{ _OTLast },  // TCppComment,
	{ _OTLast },  // TUnknown,
	{ _OTLast },  // TEOF,
	/*** Values:***/
	{ OTValue },  // TVScalar,
	{ OTValue },  // TVVector,
	{ OTValue },  // TVString,
	{ OTValue },  // TVFlag,
	/*** For expressions: ***/
	{ OTBrOp  },  // TEOpBr,
	{ OTBrCl  },  // TEClBr,
	{ OTNot   },  // TENot,
	{ OTAdd   },  // TEPlus,
	{ OTSub   },  // TEMinus,
	{ OTPow   },  // TEPow,
	{ OTMul   },  // TEMul,
	{ OTDiv   },  // TEDiv,
	{ OTLE    },  // TELE,
	{ OTGE    },  // TEGE,
	{ OTL     },  // TEL,
	{ OTG     },  // TEG,
	{ OTEq    },  // TEEq,
	{ OTNEq   },  // TENEq,
	{ OTAnd   },  // TEAnd,
	{ OTOr    },  // TEOr,
	{ OTCondQ },  // TECondQ,
	{ OTCondC },  // TECondC,
	{ OTSep   }   // TESep,
};


void Parser::_SkipCComment()
{
	int ci,lastci=-256;
	int nested=1;
	int nestwarn = (config.allow_nested_comments==1) ? 0 : 1;
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
		else if(config.allow_nested_comments && ci=='*' && lastci=='/')
		{
			if(nested==1 && !nestwarn)
			{
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


void Parser::_SkipCppComment()
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


// Valid flex-reported scalars only: 
void Parser::_ParseScalar(Scalar *rv)
{
	char *end;
	char tmp=yytext[yyleng];
	yytext[yyleng]='\0';
	errno=0;
	*rv=strtod(yytext,&end);
	if(errno==ERANGE)
	{  cwarn << "argument is out of scalar range" << std::endl;  }
	assert(!(*end));   // make sure whole string could be read
	yytext[yyleng]=tmp;
}

// Valid, simple flex-reported vectors <scalar,scalar,scalar> 
// only: 
void Parser::_ParseVector(Vector *rv)
{
	char tmp=yytext[yyleng];
	yytext[yyleng]='\0';
	char *txt=yytext;
	assert(*txt=='<');  ++txt;
	Scalar v[3];
	for(int i=0; i<3; i++)
	{
		while(isspace(*txt))  ++txt;
		char *end;
		errno=0;
		v[i]=strtod(txt,&end);  txt=end;
		if(errno==ERANGE)
		{  cwarn << "argument is out of scalar range" << std::endl;  }
		while(isspace(*txt))  ++txt;
		assert(*txt==((i==2) ? '>' : ','));
		if(*txt)  ++txt;   // skip `,' or `>'
	}
	yytext[yyleng]=tmp;
	rv->operator()(0,v[0]);
	rv->operator()(1,v[1]);
	rv->operator()(2,v[2]);
}

// Checks for flag and returns 1 in case it is one. 
int Parser::_ParseFlag(Flag *rv)
{
	char *txt=yytext;
	char tmp=txt[yyleng];
	txt[yyleng]='\0';
	int rtv=0;
	if(!strcmp(txt,"true") || 
	   !strcmp(txt,"yes") ||
	   !strcmp(txt,"on") )
	{  *rv=true;  rtv=1;  }
	else 
	if(!strcmp(txt,"false") || 
	   !strcmp(txt,"off") ||
	   !strcmp(txt,"no") )
	{  *rv=false;  rtv=1;  }
	txt[yyleng]=tmp;
	return(rtv);
}

// Parse any string: 
void Parser::_ParseString(String *rv)
{
	rv->assign("");
	int ci;
	char c;
	int in_esc=0;
	#warning do it faster by using a string read buf
	for(;;)
	{
		ci=NextChar();
		if(ci==EOF)
		{  fprintf(stderr,"String terminated by EOF\n");  
			break;  }  // FIXME: return(EndOfFile);
		*((unsigned char*)&c)=(unsigned char)ci;
		if(c=='\n')
		{  fprintf(stderr,"String terminated by newline\n");
			break;  }
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
}


// May only be called for Tolken TVxxx
void Parser::_ParseValue(Value *rv,Token valtok)
{
	switch(valtok)
	{
		case TVFlag:   { Flag   tmp; _ParseFlag  (&tmp); *rv=tmp; } break;
		case TVScalar: { Scalar tmp; _ParseScalar(&tmp); *rv=tmp; } break;
		case TVVector: { Vector tmp; _ParseVector(&tmp); *rv=tmp; } break;
		case TVString: { String tmp; _ParseString(&tmp); *rv=tmp; } break;
		default:  assert(0);  abort();
	}
}


const FunctionDesc *Parser::_ParseFunctionName()
{
	char tmp=yytext[yyleng];
	yytext[yyleng]='\0';
	const FunctionDesc *fdesc=LookupFunction(yytext);  // binary search 
	yytext[yyleng]=tmp;
	return(fdesc);
}


inline void Parser::_UpdatePos(int c,Position *p)
{
	if(c==EOF)  return;
	switch(c)
	{
		case '\t':  p->lpos = (p->lpos/config.tab_char_width+1)*config.tab_char_width;  break;
		case '\n':  p->lpos=0;  ++p->line;  break;
		case '\r':  break;
		case '\f':  /* fallthrough */
		case '\v':  ++p->line;  /* p->lpos=const. */  break;
		default:  ++p->lpos;  break;
	}
}

inline void Parser::_UpdatePos(char *str,int len,Position *p)
{
	// This function still has a potential for optimizations... 
	for(unsigned char *c=(unsigned char*)str,*cend=c+len; c<cend; c++)
	{  _UpdatePos(int(*c),p);  }
}

inline void Parser::_AddPos(Position *p,Position *delta)
{
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

// Always use this instead of yyinput(). 
int Parser::NextChar()
{
	_AddPos(&p,&delta_p);
	int c=exparse_FlexLexer::yyinput();
	_UpdatePos(c,&p);
	return(c);
}

// Make sure nobody calls it: 
int Parser::yyinput()
{  assert(0);  return(EOF);  }

// Always use this instead of yylex(). 
Token Parser::NextTok()
{
	Token tok;
	for(;;)
	{
		_AddPos(&p,&delta_p);
		tok=(Token)yylex();
		currtok=tok;
		switch(int(tok))
		{
			case TWSpace:
				_UpdatePos(yytext,yyleng,&p);
				break;
			case TCComment:
				_UpdatePos(yytext,yyleng,&p);
				_SkipCComment();
				break;
			case TCppComment:
				_UpdatePos(yytext,yyleng,&p);
				_SkipCppComment();
				break;
			case TCCommentEnd:
				_UpdatePos(yytext,yyleng,&p);
				cerr << "Stray closing C-style comment" << std::endl;
				++errors;
				break;
			case TUnknown:
			{
				char tmp=yytext[yyleng];
				yytext[yyleng]='\0';
				cerr << "Parse error near unexpected token `" << yytext << 
					"'" << std::endl;
				yytext[yyleng]=tmp;
				_UpdatePos(yytext,yyleng,&p);
				++errors;
			}	break;
			default:
				assert(!delta_p.line && !delta_p.lpos);
				_UpdatePos(yytext,yyleng,&delta_p);
				return(tok);
		}
	}
	/* UNREACHED */
	return(tok);
}


void Parser::_ParseExpression(LinkedList<OperatorNode> *exp)
{
	Token tok;
	for(;;)
	{
		tok=NextTok();
		#warning this is for testing only (must treat EOF differently) 
		if(tok==TEOF)  break;
		Operator *op=NULL,*op2=NULL;
		
		const TokenDesc *td=&tdesc[tok];
		if(td->otype!=_OTLast)
		{
			// Okay, we have the usual operator type. 
			if(td->otype==OTValue)
			{
				Value *val=new Value();
				_ParseValue(val,tok);
				op=new Operator(val,/*attach=*/true);
			}
			// Parser ambiguities (<> -> vector or lesser/greater and 
			// +- -> unary or binary) are treated lateron in 
			// ExpressionTree::Create(). 
			else
			{
				op=new Operator(td->otype);
			}
		}
		if(tok==TIdentifier)
		{
			int understood=0;
			// if(!understood)
			{
				const FunctionDesc *fdesc=_ParseFunctionName();
				if(fdesc)
				{  // it is a function 
					op=new Operator(fdesc);
					++understood;
				}
			}
			if(!understood)
			{
				Flag tmpf;
				if(_ParseFlag(&tmpf))
				{  // it is a flag
					Value *val=new Value();  *val=tmpf;
					op=new Operator(val,/*attach=*/true);
					++understood;
				}
			}
			if(!understood)
			{
				char tmp=yytext[yyleng];
				yytext[yyleng]='\0';
				
			#warning REMOVE ME: debugging only: 
			if(!strcmp(yytext,"pos"))
			{  cerr << "POS(" << p.line << "," << p.lpos << ")" << std::endl;  }
				
				cerr << "Unrecognized identifier `" << yytext << "'." << std::endl;
				yytext[yyleng]=tmp;
				++errors;
			}
		}
		
		while(op)
		{
			OperatorNode *on=new OperatorNode(op);
			exp->append(on);
			//fprintf(stderr,"<%s,%d> ",on->op->desc()->name,on->op->type());
			op=op2;  op2=NULL;
		}
		
		// Dump pos: 
		//cerr << "*POS(" << p.line << "," << p.lpos << ")*" << std::endl;
	}
}


#define Use_strstream 1   /* sadly only available for non-C++ scanners */

int Parser::Parse(const char *expr,ExpressionTree *tree)
{
	_ResetParser();
	
	#if Use_strstream
		istrstream expr_stream(expr);
		yyrestart(&expr_stream);
	#else
		size_t expr_len=strlen(expr);
		char *expr_buf = new char[expr_len+2];
		strcpy(expr_buf,expr);
		// Flex wants double-NUL-terminated strings: 
		expr_buf[expr_len++]='\0';
		expr_buf[expr_len++]='\0';
		
		YY_BUFFER_STATE ebuf=yy_scan_buffer(expr_buf,expr_len);
		assert(ebuf);
	#endif
	
	LinkedList<OperatorNode> exp;
	_ParseExpression(&exp);   // increments errors
	errors=-errors;  // parse errors -> retval <0
	
	tree->Clear();
	
	if(!errors)  // no parse errors
	{
		if(!exp.is_empty())
		{
			errors=tree->Create(&exp,/*parser_ambig=*/true,/*free=*/true);
			assert(errors>=0);
		}
	}
	
	#if !Use_strstream
		yy_delete_buffer(ebuf);
	#endif
	return(errors);
}


void Parser::_ResetParser()
{
	errors=0;
	currtok=_TLast;
}


Parser::Parser() : 
	config()
{
	_ResetParser();
	
	p.file="";
	p.line=1;
	p.lpos=0;
	delta_p.file="";
	delta_p.line=0;
	delta_p.lpos=0;
}

Parser::~Parser()
{
	
}


ParserConfig::ParserConfig()
{
	allow_nested_comments=1;
	tab_char_width=8;
}

}  // end of namespace exparse
