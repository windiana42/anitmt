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

#warning "When using message system: remove this line."
using std::cerr;

#warning Remove me: 
int bt_quiet=0;   // be quiet (benchmark test) 

namespace exparse {

std::ostream &cwarn=std::cerr;

namespace  // static namespace
{
struct TokenDesc
{
	OperatorType otype;  // corresponding primary OperatorType or _OTLast 
};

const TokenDesc tdesc[_TLast]=
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
	{ OTDot   },  // TEDot,
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

struct BuiltinValueDesc
{
	const char *name;
	int name_len;  // strlen(name)
	Value val;
};

const BuiltinValueDesc builtin_values[]=
{
	{ "x", 1, Value(Vector(1.0,0.0,0.0)) },
	{ "y", 1, Value(Vector(0.0,1.0,0.0)) },
	{ "z", 1, Value(Vector(0.0,0.0,1.0)) },
	{ NULL, -1, Value() }   // <- LAST ONE
};

}  // end of static namespace

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


// May only be called for Tolken TVxxx
void Parser::_ParseValue(Value *rv,Token valtok)
{
	switch(valtok)
	{
		case TVFlag:   { Flag   tmp; _ParseFlag  (&tmp); *rv=tmp; } break;
		case TVScalar: { Scalar tmp; _ParseScalar(&tmp); *rv=tmp; } break;
		case TVVector: { Vector tmp; _ParseVector(&tmp); *rv=tmp; } break;
		case TVString: { String tmp;  ParseString(&tmp); *rv=tmp; } break;
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

// Returns 1 if found and stores value in *rv. 
int Parser::_ParseBuiltinConstant(Value *rv)
{
	char tmp=yytext[yyleng];
	yytext[yyleng]='\0';
	int found=0;
	for(const BuiltinValueDesc *vd=builtin_values; vd->name; vd++)
	{
		if(vd->name_len!=yyleng)  continue;
		if(strcmp(vd->name,yytext))  continue;
		*rv=vd->val;  // assign value
		found=1;
	}
	yytext[yyleng]=tmp;
	return(found);
}


void Parser::_ParseExpression(LinkedList<OperatorNode> *exp)
{
	Token tok;
	for(;;)
	{
		tok=(Token)NextTok();
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
				// Check for build-in constants. 
				Value tmp;
				if(_ParseBuiltinConstant(&tmp))
				{  // found it
					Value *val=new Value();  *val=tmp;
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
			{  cerr << "POS(" << currpos()->line << "," << currpos()->lpos << ")" << std::endl;  }
				
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


#define Use_strstream 1   /* yy_scan_buffer is sadly only available for non-C++ scanners */

int Parser::Parse(const char *expr,ExpressionTree *tree)
{
	_ResetParser();
	
	#if Use_strstream
		std::istrstream expr_stream(expr);
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
	BasicParserGlue<exparse_FlexLexer>(),
	config()
{
	// BasicParser initialisation: 
	SetConfig(&config);
	tok_WSpace=TWSpace;
	tok_CComment=TCComment;
	tok_CppComment=TCppComment;
	tok_CCommentEnd=TCCommentEnd;
	tok_Unknown=TUnknown;
	
	_ResetParser();
}

Parser::~Parser()
{
	
}


ParserConfig::ParserConfig()
{
}

}  // end of namespace exparse
