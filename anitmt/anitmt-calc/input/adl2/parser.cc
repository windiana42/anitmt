
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_IS_EQUAL	258
#define	TOK_NOT_EQUAL	259
#define	TOK_MORE_EQUAL	260
#define	TOK_LESS_EQUAL	261
#define	TOK_FUNC_SIN	262
#define	TOK_IDENTIFIER	263
#define	TOK_FLAG	264
#define	TOK_SCALAR	265
#define	TOK_VECTOR	266
#define	TOK_MATRIX	267
#define	TOK_STRING	268
#define	TOK_OP_FLAG	269
#define	TOK_OP_SCALAR	270
#define	TOK_OP_VECTOR	271
#define	TOK_OP_MATRIX	272
#define	TOK_OP_STRING	273
#define	TOK_PROP_FLAG	274
#define	TOK_PROP_SCALAR	275
#define	TOK_PROP_VECTOR	276
#define	TOK_PROP_MATRIX	277
#define	TOK_PROP_STRING	278
#define	UMINUS	279
#define	OP_CONVERTION	280

#line 3 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

#include <iostream>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "adlparser.hpp"

#define YYPARSE_PARAM info
#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)

namespace anitmt
{
  namespace adlparser
  {
    // define token type for parser
    #define YYSTYPE Token
    int yylex( Token *lvalp, void *info )
    {
      adlparser_info *i = static_cast<adlparser_info*> (info);
      i->lexer->yylval = lvalp;	// lvalue variable to return token value
      return i->lexer->yylex();
    }

    // redefine error output
#define yyerror( s ) ( static_cast<adlparser_info*>(info)-> \
  msg.error( new message::File_Position( "unknown", \
	       static_cast<adlparser_info*>(info)->lexer->lineno() )) << s, 1 )

#define yyerr ( static_cast<adlparser_info*>(info)-> \
  msg.error( new message::File_Position( "unknown", \
	       static_cast<adlparser_info*>(info)->lexer->lineno() )) )

    /*		       
    int yyerror( char *s )
    {
      cerr << "error: " << s;
      return 0;
    }
    */

    // creates new tree node and makes it the current one
    void change_current_child( void *vptr_info, std::string type, 
			       std::string name="" )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);

      if( name == "" ) 
      {
#warning should create unique name as default node name
	name = "default"; 
      }
      Prop_Tree_Node *node = 
	info->get_current_tree_node()->add_child( type, name );
      if( node == 0 )
      {
	yyerror("couldn't add tree node");
      }
      else
      {
	info->set_new_tree_node( node );
      }
    }

    // changes back to the parent tree node
    inline void change_to_parent( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->tree_node_done();
    }

    // tells the lexer to resolve identifiers as properties
    inline void resolve_properties( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_property;
    }

    // tells the lexer to resolve identifiers as property references
    inline void resolve_references( void *vptr_info )
    {
      adlparser_info *info = static_cast<adlparser_info*>(vptr_info);
      info->id_resolver = &info->res_reference;
    }
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		106
#define	YYFLAG		-32768
#define	YYNTBASE	39

#define YYTRANSLATE(x) ((unsigned)(x) <= 280 ? yytranslate[x] : 69)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    34,
    35,    27,    25,    37,    26,     2,    28,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    31,    36,
     2,    38,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    32,     2,    33,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    29,    30
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    13,    16,    19,    21,    22,
    28,    29,    36,    37,    41,    43,    45,    46,    50,    52,
    54,    55,    59,    61,    63,    64,    68,    70,    72,    73,
    77,    79,    81,    83,    87,    91,    95,    99,   102,   105,
   110,   112,   116,   124,   126,   128,   130,   132,   136,   140,
   144,   148,   151,   153,   157,   165,   167,   169
};

static const short yyrhs[] = {    -1,
    39,    40,     0,    44,    31,     0,    47,    31,     0,    50,
    31,     0,    53,    31,     0,    56,    31,     0,    41,     0,
     0,     9,    32,    42,    39,    33,     0,     0,     9,     9,
    32,    43,    39,    33,     0,     0,    20,    45,    46,     0,
    59,     0,    64,     0,     0,    21,    48,    49,     0,    60,
     0,    65,     0,     0,    22,    51,    52,     0,    61,     0,
    66,     0,     0,    23,    54,    55,     0,    62,     0,    67,
     0,     0,    24,    57,    58,     0,    63,     0,    68,     0,
    10,     0,    60,    25,    60,     0,    60,    26,    60,     0,
    60,    27,    60,     0,    60,    28,    60,     0,    26,    60,
     0,    25,    60,     0,     8,    34,    60,    35,     0,    11,
     0,    61,    25,    61,     0,    36,    60,    37,    60,    37,
    60,    38,     0,    12,     0,    13,     0,    14,     0,    15,
     0,    65,    25,    65,     0,    65,    26,    65,     0,    65,
    27,    65,     0,    65,    28,    65,     0,    26,    65,     0,
    16,     0,    66,    25,    66,     0,    36,    65,    37,    65,
    37,    65,    38,     0,    17,     0,    18,     0,    19,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   140,   141,   144,   145,   146,   147,   148,   149,   152,   153,
   154,   155,   158,   159,   176,   177,   180,   181,   198,   199,
   202,   203,   220,   221,   224,   225,   242,   243,   246,   247,
   264,   265,   269,   271,   272,   273,   274,   275,   276,   277,
   278,   281,   282,   284,   287,   289,   293,   296,   297,   298,
   299,   300,   301,   304,   305,   309,   312,   314
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_IS_EQUAL","TOK_NOT_EQUAL","TOK_MORE_EQUAL","TOK_LESS_EQUAL","TOK_FUNC_SIN",
"TOK_IDENTIFIER","TOK_FLAG","TOK_SCALAR","TOK_VECTOR","TOK_MATRIX","TOK_STRING",
"TOK_OP_FLAG","TOK_OP_SCALAR","TOK_OP_VECTOR","TOK_OP_MATRIX","TOK_OP_STRING",
"TOK_PROP_FLAG","TOK_PROP_SCALAR","TOK_PROP_VECTOR","TOK_PROP_MATRIX","TOK_PROP_STRING",
"'+'","'-'","'*'","'/'","UMINUS","OP_CONVERTION","';'","'{'","'}'","'('","')'",
"'<'","','","'>'","tree_node_block","statement","child_declaration","@1","@2",
"flag_statement","@3","any_flag_exp","scalar_statement","@4","any_scalar_exp",
"vector_statement","@5","any_vector_exp","matrix_statement","@6","any_matrix_exp",
"string_statement","@7","any_string_exp","flag_exp","scalar_exp","vector_exp",
"matrix_exp","string_exp","op_flag_exp","op_scalar_exp","op_vector_exp","op_matrix_exp",
"op_string_exp", NULL
};
#endif

static const short yyr1[] = {     0,
    39,    39,    40,    40,    40,    40,    40,    40,    42,    41,
    43,    41,    45,    44,    46,    46,    48,    47,    49,    49,
    51,    50,    52,    52,    54,    53,    55,    55,    57,    56,
    58,    58,    59,    60,    60,    60,    60,    60,    60,    60,
    60,    61,    61,    61,    62,    63,    64,    65,    65,    65,
    65,    65,    65,    66,    66,    66,    67,    68
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     2,     2,     2,     1,     0,     5,
     0,     6,     0,     3,     1,     1,     0,     3,     1,     1,
     0,     3,     1,     1,     0,     3,     1,     1,     0,     3,
     1,     1,     1,     3,     3,     3,     3,     2,     2,     4,
     1,     3,     7,     1,     1,     1,     1,     3,     3,     3,
     3,     2,     1,     3,     7,     1,     1,     1
};

static const short yydefact[] = {     1,
     0,     0,    13,    17,    21,    25,    29,     2,     8,     0,
     0,     0,     0,     0,     0,     9,     0,     0,     0,     0,
     0,     3,     4,     5,     6,     7,    11,     1,    33,    47,
    14,    15,    16,     0,    41,    53,     0,     0,    18,    19,
    20,    44,    56,     0,    22,    23,    24,    45,    57,    26,
    27,    28,    46,    58,    30,    31,    32,     1,     0,     0,
     0,    39,    38,    52,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,    10,     0,    34,
    35,    36,    37,     0,    48,    49,    50,    51,     0,     0,
     0,    42,     0,    54,    12,    40,     0,     0,     0,     0,
     0,     0,    43,    55,     0,     0
};

static const short yydefgoto[] = {     1,
     8,     9,    28,    58,    10,    17,    31,    11,    18,    39,
    12,    19,    45,    13,    20,    50,    14,    21,    55,    32,
    63,    46,    51,    56,    33,    64,    47,    52,    57
};

static const short yypact[] = {-32768,
    15,    -7,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -24,
   -17,     2,    13,    34,   -11,-32768,    30,    58,    -8,    28,
    29,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    25,-32768,-32768,    60,    58,-32768,   105,
   109,-32768,-32768,    58,-32768,    22,    42,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    -4,    60,
    60,-32768,-32768,-32768,    60,    60,    60,    60,    16,    16,
    16,    16,    69,    73,    -6,   -14,    40,-32768,    94,    49,
    49,-32768,-32768,    16,    77,    77,-32768,-32768,    60,    16,
    60,-32768,    16,-32768,-32768,-32768,    86,    90,    60,    16,
   -15,    65,-32768,-32768,    70,-32768
};

static const short yypgoto[] = {   -27,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -10,     3,-32768,-32768,-32768,   -18,     4,-32768,-32768
};


#define	YYLAST		137


static const short yytable[] = {    41,
    59,    15,    43,    42,     2,    42,    22,    40,    43,    65,
    66,    67,    68,    23,   105,     3,     4,     5,     6,     7,
    27,    93,   103,     2,    16,    74,    62,    44,    78,    91,
    77,    36,    24,    73,     3,     4,     5,     6,     7,    29,
    48,    84,    53,    25,    30,    49,    75,    54,     2,    79,
    85,    86,    87,    88,    80,    81,    82,    83,    60,     3,
     4,     5,     6,     7,    26,    34,    76,    34,    35,   106,
    35,    98,    95,    36,    74,    67,    68,    92,    97,    94,
    73,   102,    37,    38,    37,    61,     0,     0,   101,    69,
    70,    71,    72,    65,    66,    67,    68,    69,    70,    71,
    72,     0,   104,    71,    72,    89,     0,     0,     0,    90,
    65,    66,    67,    68,    69,    70,    71,    72,    65,    66,
    67,    68,    99,     0,     0,     0,   100,     0,    96,    65,
    66,    67,    68,    69,    70,    71,    72
};

static const short yycheck[] = {    18,
    28,     9,    17,    12,     9,    12,    31,    18,    17,    25,
    26,    27,    28,    31,     0,    20,    21,    22,    23,    24,
    32,    36,    38,     9,    32,    44,    37,    36,    33,    36,
    58,    16,    31,    44,    20,    21,    22,    23,    24,    10,
    13,    26,    14,    31,    15,    18,    25,    19,     9,    60,
    69,    70,    71,    72,    65,    66,    67,    68,    34,    20,
    21,    22,    23,    24,    31,     8,    25,     8,    11,     0,
    11,    90,    33,    16,    93,    27,    28,    75,    89,    76,
    91,   100,    25,    26,    25,    26,    -1,    -1,    99,    25,
    26,    27,    28,    25,    26,    27,    28,    25,    26,    27,
    28,    -1,    38,    27,    28,    37,    -1,    -1,    -1,    37,
    25,    26,    27,    28,    25,    26,    27,    28,    25,    26,
    27,    28,    37,    -1,    -1,    -1,    37,    -1,    35,    25,
    26,    27,    28,    25,    26,    27,    28
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYPARSE_RETURN_TYPE
#define YYPARSE_RETURN_TYPE int
#endif


#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 222 "/usr/share/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
YYPARSE_RETURN_TYPE
yyparse (void *);
#else
YYPARSE_RETURN_TYPE
yyparse (void);
#endif
#endif

YYPARSE_RETURN_TYPE
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
#ifndef YYSTACK_USE_ALLOCA
  int yyfree_stacks = 0;
#endif

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
#ifndef YYSTACK_USE_ALLOCA
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
#endif	    
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 9:
#line 152 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-1].identifier()); ;
    break;}
case 10:
#line 153 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 11:
#line 154 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-2].identifier(),yyvsp[-1].identifier());;
    break;}
case 12:
#line 155 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 13:
#line 158 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ resolve_references(info); ;
    break;}
case 14:
#line 159 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_FLAG )
   {
     if( !yyvsp[-2].prop_flag().set_value( yyvsp[0].tok().flag() ) )
     {
       yyerr << "error while setting property " << yyvsp[-2].prop_flag().get_name() << " to "
	     << yyvsp[0].tok().flag();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_FLAG );
     solve::explicite_reference( yyvsp[-2].prop_flag(), yyvsp[0].tok().op_flag() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 15:
#line 176 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.flag() = yyvsp[0].flag(); yyval.tok() = res;;
    break;}
case 16:
#line 177 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_flag(yyvsp[0].op_flag()); yyval.tok() = res;;
    break;}
case 17:
#line 180 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ resolve_references(info); ;
    break;}
case 18:
#line 181 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_SCALAR )
   {
     if( !yyvsp[-2].prop_scalar().set_value( yyvsp[0].tok().scalar() ) )
     {
       yyerr << "error while setting property " << yyvsp[-2].prop_scalar().get_name() << " to "
	     << yyvsp[0].tok().scalar();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_SCALAR );
     solve::explicite_reference( yyvsp[-2].prop_scalar(), yyvsp[0].tok().op_scalar() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 19:
#line 198 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.scalar() = yyvsp[0].scalar(); yyval.tok() = res;;
    break;}
case 20:
#line 199 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_scalar(yyvsp[0].op_scalar()); yyval.tok() = res;;
    break;}
case 21:
#line 202 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ resolve_references(info); ;
    break;}
case 22:
#line 203 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_VECTOR )
   {
     if( !yyvsp[-2].prop_vector().set_value( yyvsp[0].tok().vector() ) )
     {
       yyerr << "error while setting property " << yyvsp[-2].prop_vector().get_name() << " to "
	     << yyvsp[0].tok().vector();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_VECTOR );
     solve::explicite_reference( yyvsp[-2].prop_vector(), yyvsp[0].tok().op_vector() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 23:
#line 220 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.vector() = yyvsp[0].vector(); yyval.tok() = res;;
    break;}
case 24:
#line 221 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_vector(yyvsp[0].op_vector()); yyval.tok() = res;;
    break;}
case 25:
#line 224 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ resolve_references(info); ;
    break;}
case 26:
#line 225 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_MATRIX )
   {
     if( !yyvsp[-2].prop_matrix().set_value( yyvsp[0].tok().matrix() ) )
     {
       yyerr << "error while setting property " << yyvsp[-2].prop_matrix().get_name() << " to "
	     << yyvsp[0].tok().matrix();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_MATRIX );
     solve::explicite_reference( yyvsp[-2].prop_matrix(), yyvsp[0].tok().op_matrix() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 27:
#line 242 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.matrix() = yyvsp[0].matrix(); yyval.tok() = res;;
    break;}
case 28:
#line 243 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_matrix(yyvsp[0].op_matrix()); yyval.tok() = res;;
    break;}
case 29:
#line 246 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ resolve_references(info); ;
    break;}
case 30:
#line 247 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_STRING )
   {
     if( !yyvsp[-2].prop_string().set_value( yyvsp[0].tok().string() ) )
     {
       yyerr << "error while setting property " << yyvsp[-2].prop_string().get_name() << " to "
	     << yyvsp[0].tok().string();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_STRING );
     solve::explicite_reference( yyvsp[-2].prop_string(), yyvsp[0].tok().op_string() ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 31:
#line 264 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.string() = yyvsp[0].string(); yyval.tok() = res;;
    break;}
case 32:
#line 265 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_string(yyvsp[0].op_string()); yyval.tok() = res;;
    break;}
case 33:
#line 269 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.flag() = yyvsp[0].flag();;
    break;}
case 34:
#line 271 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar();;
    break;}
case 35:
#line 272 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar();;
    break;}
case 36:
#line 273 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar();;
    break;}
case 37:
#line 274 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar();;
    break;}
case 38:
#line 275 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = -yyvsp[0].scalar();;
    break;}
case 39:
#line 276 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[0].scalar();;
    break;}
case 40:
#line 277 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = sin(yyvsp[-1].scalar());;
    break;}
case 41:
#line 278 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[0].scalar();;
    break;}
case 42:
#line 281 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector();;
    break;}
case 43:
#line 283 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 44:
#line 284 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.vector() = yyvsp[0].vector();;
    break;}
case 45:
#line 287 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.matrix() = yyvsp[0].matrix();;
    break;}
case 46:
#line 289 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.string() = yyvsp[0].string();;
    break;}
case 47:
#line 293 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_flag() = yyvsp[0].op_flag();;
    break;}
case 48:
#line 296 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() + yyvsp[0].op_scalar();;
    break;}
case 49:
#line 297 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() - yyvsp[0].op_scalar();;
    break;}
case 50:
#line 298 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() * yyvsp[0].op_scalar();;
    break;}
case 51:
#line 299 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = yyvsp[-2].op_scalar() / yyvsp[0].op_scalar();;
    break;}
case 52:
#line 300 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = -yyvsp[0].op_scalar();;
    break;}
case 53:
#line 301 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_scalar() = yyvsp[0].op_scalar();;
    break;}
case 54:
#line 304 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_vector() = yyvsp[-2].op_vector() + yyvsp[0].op_vector();;
    break;}
case 55:
#line 306 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	  yyerror("vector creation from operands not supported yet!");
	  assert(0); ;
    break;}
case 56:
#line 309 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_vector() = yyvsp[0].op_vector();;
    break;}
case 57:
#line 312 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_matrix() = yyvsp[0].op_matrix();;
    break;}
case 58:
#line 314 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_string() = yyvsp[0].op_string();;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 554 "/usr/share/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
#ifndef YYSTACK_USE_ALLOCA
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
#endif    
  return 1;
}
#line 317 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

    int parse_adl( Prop_Tree_Node *node, adlparser_info *info )
    {
      info->set_new_tree_node( node );
      info->id_resolver = &info->res_property;
      return yyparse( static_cast<void*>(info) );
    }
  }
}
