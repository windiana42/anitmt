
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_ERROR	258
#define	TOK_IS_EQUAL	259
#define	TOK_NOT_EQUAL	260
#define	TOK_MORE_EQUAL	261
#define	TOK_LESS_EQUAL	262
#define	TOK_FUNC_SIN	263
#define	TOK_FUNC_SQRT	264
#define	TOK_IDENTIFIER	265
#define	TOK_FLAG	266
#define	TOK_SCALAR	267
#define	TOK_VECTOR	268
#define	TOK_MATRIX	269
#define	TOK_STRING	270
#define	TOK_OP_FLAG	271
#define	TOK_OP_SCALAR	272
#define	TOK_OP_VECTOR	273
#define	TOK_OP_MATRIX	274
#define	TOK_OP_STRING	275
#define	TOK_PROP_FLAG	276
#define	TOK_PROP_SCALAR	277
#define	TOK_PROP_VECTOR	278
#define	TOK_PROP_MATRIX	279
#define	TOK_PROP_STRING	280
#define	UMINUS	281
#define	OP_CONVERTION	282

#line 15 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

#include <iostream>
#include <string>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "adlparser.hpp"

#include "parser_functions.hpp"

#define YYPARSE_PARAM info
#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)

// open namespaces
namespace anitmt
{
  namespace adlparser
  {

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		128
#define	YYFLAG		-32768
#define	YYNTBASE	41

#define YYTRANSLATE(x) ((unsigned)(x) <= 282 ? yytranslate[x] : 71)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    36,
    37,    29,    27,    39,    28,     2,    30,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    33,    38,
     2,    40,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    34,     2,    35,     2,     2,     2,     2,     2,
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
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    31,    32
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    13,    16,    19,    21,    22,
    28,    29,    36,    37,    41,    43,    45,    46,    50,    52,
    54,    55,    59,    61,    63,    64,    68,    70,    72,    73,
    77,    79,    81,    83,    87,    91,    95,    99,   102,   105,
   109,   114,   119,   121,   125,   133,   135,   137,   139,   141,
   145,   149,   153,   157,   160,   163,   167,   172,   174,   176,
   178,   182,   190,   192,   194
};

static const short yyrhs[] = {    -1,
    41,    42,     0,    46,    33,     0,    49,    33,     0,    52,
    33,     0,    55,    33,     0,    58,    33,     0,    43,     0,
     0,    11,    34,    44,    41,    35,     0,     0,    11,    11,
    34,    45,    41,    35,     0,     0,    22,    47,    48,     0,
    61,     0,    66,     0,     0,    23,    50,    51,     0,    62,
     0,    67,     0,     0,    24,    53,    54,     0,    63,     0,
    68,     0,     0,    25,    56,    57,     0,    64,     0,    69,
     0,     0,    26,    59,    60,     0,    65,     0,    70,     0,
    12,     0,    62,    27,    62,     0,    62,    28,    62,     0,
    62,    29,    62,     0,    62,    30,    62,     0,    28,    62,
     0,    27,    62,     0,    36,    62,    37,     0,     9,    36,
    62,    37,     0,    10,    36,    62,    37,     0,    13,     0,
    63,    27,    63,     0,    38,    62,    39,    62,    39,    62,
    40,     0,    14,     0,    15,     0,    16,     0,    17,     0,
    67,    27,    67,     0,    67,    28,    67,     0,    67,    29,
    67,     0,    67,    30,    67,     0,    28,    67,     0,    27,
    67,     0,    36,    67,    37,     0,    10,    36,    67,    37,
     0,    62,     0,    23,     0,    18,     0,    68,    27,    68,
     0,    38,    67,    39,    67,    39,    67,    40,     0,    19,
     0,    20,     0,    21,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    88,    89,    92,    93,    94,    95,    96,    97,   100,   101,
   102,   104,   107,   110,   127,   128,   131,   134,   152,   153,
   156,   159,   177,   178,   181,   184,   202,   203,   206,   209,
   227,   228,   232,   234,   235,   236,   237,   238,   239,   240,
   241,   242,   243,   246,   247,   249,   252,   254,   258,   261,
   262,   263,   264,   265,   266,   267,   268,   270,   272,   273,
   276,   277,   281,   284,   286
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_ERROR","TOK_IS_EQUAL","TOK_NOT_EQUAL","TOK_MORE_EQUAL","TOK_LESS_EQUAL",
"TOK_FUNC_SIN","TOK_FUNC_SQRT","TOK_IDENTIFIER","TOK_FLAG","TOK_SCALAR","TOK_VECTOR",
"TOK_MATRIX","TOK_STRING","TOK_OP_FLAG","TOK_OP_SCALAR","TOK_OP_VECTOR","TOK_OP_MATRIX",
"TOK_OP_STRING","TOK_PROP_FLAG","TOK_PROP_SCALAR","TOK_PROP_VECTOR","TOK_PROP_MATRIX",
"TOK_PROP_STRING","'+'","'-'","'*'","'/'","UMINUS","OP_CONVERTION","';'","'{'",
"'}'","'('","')'","'<'","','","'>'","tree_node_block","statement","child_declaration",
"@1","@2","flag_statement","@3","any_flag_exp","scalar_statement","@4","any_scalar_exp",
"vector_statement","@5","any_vector_exp","matrix_statement","@6","any_matrix_exp",
"string_statement","@7","any_string_exp","flag_exp","scalar_exp","vector_exp",
"matrix_exp","string_exp","op_flag_exp","op_scalar_exp","op_vector_exp","op_matrix_exp",
"op_string_exp", NULL
};
#endif

static const short yyr1[] = {     0,
    41,    41,    42,    42,    42,    42,    42,    42,    44,    43,
    45,    43,    47,    46,    48,    48,    50,    49,    51,    51,
    53,    52,    54,    54,    56,    55,    57,    57,    59,    58,
    60,    60,    61,    62,    62,    62,    62,    62,    62,    62,
    62,    62,    62,    63,    63,    63,    64,    65,    66,    67,
    67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
    68,    68,    68,    69,    70
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     2,     2,     2,     1,     0,     5,
     0,     6,     0,     3,     1,     1,     0,     3,     1,     1,
     0,     3,     1,     1,     0,     3,     1,     1,     0,     3,
     1,     1,     1,     3,     3,     3,     3,     2,     2,     3,
     4,     4,     1,     3,     7,     1,     1,     1,     1,     3,
     3,     3,     3,     2,     2,     3,     4,     1,     1,     1,
     3,     7,     1,     1,     1
};

static const short yydefact[] = {     1,
     0,     0,    13,    17,    21,    25,    29,     2,     8,     0,
     0,     0,     0,     0,     0,     9,     0,     0,     0,     0,
     0,     3,     4,     5,     6,     7,    11,     1,    33,    49,
    14,    15,    16,     0,     0,    43,    60,    59,     0,     0,
     0,    18,    58,    20,    46,    63,     0,    22,    23,    24,
    47,    64,    26,    27,    28,    48,    65,    30,    31,    32,
     1,     0,     0,     0,    39,    55,    38,    54,    58,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    58,     0,
     0,     0,     0,    10,     0,     0,     0,     0,     0,    58,
     0,    40,    56,    34,    35,    36,    37,    58,    50,    51,
    52,    53,     0,     0,     0,    44,     0,    61,    12,     0,
    39,    38,     0,    41,    42,    57,     0,     0,     0,     0,
     0,     0,     0,     0,    45,    62,     0,     0
};

static const short yydefgoto[] = {     1,
     8,     9,    28,    61,    10,    17,    31,    11,    18,    42,
    12,    19,    48,    13,    20,    53,    14,    21,    58,    32,
    98,    49,    54,    59,    33,    80,    50,    55,    60
};

static const short yypact[] = {-32768,
    16,    -6,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   -30,
   -22,   -20,   -15,     2,     9,-32768,    -8,    -3,    -2,    -1,
    28,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    -4,    14,-32768,-32768,-32768,    -3,    -3,
    -3,-32768,    12,   141,-32768,-32768,    -3,-32768,    25,    46,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    45,    38,    -3,-32768,-32768,-32768,-32768,    18,    55,
    38,    38,    38,    38,    -3,    -3,    -3,    -3,    36,    60,
   -12,    53,    83,-32768,    43,    38,    38,    38,   119,    44,
   125,-32768,-32768,    67,    67,-32768,-32768,-32768,    72,    72,
-32768,-32768,    38,    -3,    38,-32768,    -3,-32768,-32768,    38,
-32768,-32768,   130,-32768,-32768,-32768,    99,   106,   112,   136,
    38,    -3,    85,    92,-32768,-32768,    98,-32768
};

static const short yypgoto[] = {   -27,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -10,    22,-32768,-32768,-32768,   -18,    34,-32768,-32768
};


#define	YYLAST		173


static const short yytable[] = {    44,
    62,    45,    22,    29,    15,    34,    35,    43,    30,    36,
    23,    45,    24,    51,    37,   127,    46,    25,    52,    38,
    66,    68,    70,    39,    40,   105,     2,    16,    65,    67,
    69,    63,    41,    83,    26,    47,    79,     3,     4,     5,
     6,     7,    27,    56,   -19,    91,    34,    85,    57,    64,
    36,    81,    89,    90,    92,     2,    99,   100,   101,   102,
    94,    95,    96,    97,    86,    87,     3,     4,     5,     6,
     7,    46,    82,    88,   103,   111,   112,   113,   110,    84,
   115,    75,    76,    77,    78,   118,    75,    76,    77,    78,
   107,    93,   117,     2,   119,    73,    74,   128,   104,   120,
    77,    78,   106,   124,     3,     4,     5,     6,     7,     0,
   123,    71,    72,    73,    74,   108,     0,   109,    75,    76,
    77,    78,     0,     0,   125,    71,    72,    73,    74,     0,
     0,   126,    75,    76,    77,    78,     0,   121,    71,    72,
    73,    74,     0,     0,   122,    71,    72,    73,    74,     0,
   103,    75,    76,    77,    78,   114,    71,    72,    73,    74,
     0,   116,    71,    72,    73,    74,    92,    75,    76,    77,
    78,     0,   115
};

static const short yycheck[] = {    18,
    28,    14,    33,    12,    11,     9,    10,    18,    17,    13,
    33,    14,    33,    15,    18,     0,    19,    33,    20,    23,
    39,    40,    41,    27,    28,    38,    11,    34,    39,    40,
    41,    36,    36,    61,    33,    38,    47,    22,    23,    24,
    25,    26,    34,    16,    33,    64,     9,    10,    21,    36,
    13,    27,    63,    64,    37,    11,    75,    76,    77,    78,
    71,    72,    73,    74,    27,    28,    22,    23,    24,    25,
    26,    19,    27,    36,    39,    86,    87,    88,    36,    35,
    37,    27,    28,    29,    30,   104,    27,    28,    29,    30,
    38,    37,   103,    11,   105,    29,    30,     0,    39,   110,
    29,    30,    81,   122,    22,    23,    24,    25,    26,    -1,
   121,    27,    28,    29,    30,    82,    -1,    35,    27,    28,
    29,    30,    -1,    -1,    40,    27,    28,    29,    30,    -1,
    -1,    40,    27,    28,    29,    30,    -1,    39,    27,    28,
    29,    30,    -1,    -1,    39,    27,    28,    29,    30,    -1,
    39,    27,    28,    29,    30,    37,    27,    28,    29,    30,
    -1,    37,    27,    28,    29,    30,    37,    27,    28,    29,
    30,    -1,    37
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
#line 100 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-1].identifier()); ;
    break;}
case 10:
#line 101 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 11:
#line 103 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-2].identifier(),yyvsp[-1].identifier());;
    break;}
case 12:
#line 104 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 13:
#line 108 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ set_pos(&yyvsp[0].prop_flag(),info); resolve_references(info); ;
    break;}
case 14:
#line 110 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_FLAG )
   {
     if( !yyvsp[-2].prop_flag().set_value( yyvsp[0].tok().flag() ) )
     {
       yyerr(info) << "error while setting property " << yyvsp[-2].prop_flag().get_name() << " to "
	     << yyvsp[0].tok().flag();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_FLAG );
     solve::explicite_reference( yyvsp[-2].prop_flag(), yyvsp[0].tok().op_flag(info) ); // establish reference
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 15:
#line 127 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.flag() = yyvsp[0].flag(); yyval.tok() = res;;
    break;}
case 16:
#line 128 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_flag(yyvsp[0].op_flag(info)); yyval.tok() = res;;
    break;}
case 17:
#line 132 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ set_pos(&yyvsp[0].prop_scalar(),info); resolve_references(info); ;
    break;}
case 18:
#line 134 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_SCALAR )
   {
     if( !yyvsp[-2].prop_scalar().set_value( yyvsp[0].tok().scalar() ) )
     {
       yyerr(info) << "error while setting property " << yyvsp[-2].prop_scalar().get_name() << " to "
	     << yyvsp[0].tok().scalar();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_SCALAR );
     // establish reference
     solve::explicite_reference( yyvsp[-2].prop_scalar(), yyvsp[0].tok().op_scalar(info) ); 
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 19:
#line 152 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.scalar() = yyvsp[0].scalar(); yyval.tok() = res;;
    break;}
case 20:
#line 153 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info);;
    break;}
case 21:
#line 157 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ set_pos(&yyvsp[0].prop_vector(),info); resolve_references(info); ;
    break;}
case 22:
#line 159 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_VECTOR )
   {
     if( !yyvsp[-2].prop_vector().set_value( yyvsp[0].tok().vector() ) )
     {
       yyerr(info) << "error while setting property " << yyvsp[-2].prop_vector().get_name() << " to "
	     << yyvsp[0].tok().vector();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_VECTOR );
     // establish reference
     solve::explicite_reference( yyvsp[-2].prop_vector(), yyvsp[0].tok().op_vector(info) ); 
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 23:
#line 177 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.vector() = yyvsp[0].vector(); yyval.tok() = res;;
    break;}
case 24:
#line 178 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_vector(yyvsp[0].op_vector(info)); yyval.tok() = res;;
    break;}
case 25:
#line 182 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ set_pos(&yyvsp[0].prop_matrix(),info); resolve_references(info); ;
    break;}
case 26:
#line 184 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_MATRIX )
   {
     if( !yyvsp[-2].prop_matrix().set_value( yyvsp[0].tok().matrix() ) )
     {
       yyerr(info) << "error while setting property " << yyvsp[-2].prop_matrix().get_name() << " to "
	     << yyvsp[0].tok().matrix();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_MATRIX );
     // establish reference
     solve::explicite_reference( yyvsp[-2].prop_matrix(), yyvsp[0].tok().op_matrix(info) ); 
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 27:
#line 202 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.matrix() = yyvsp[0].matrix(); yyval.tok() = res;;
    break;}
case 28:
#line 203 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_matrix(yyvsp[0].op_matrix(info)); yyval.tok() = res;;
    break;}
case 29:
#line 207 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ set_pos(&yyvsp[0].prop_string(),info); resolve_references(info); ;
    break;}
case 30:
#line 209 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ 
   if( yyvsp[0].tok().get_type() == TOK_STRING )
   {
     if( !yyvsp[-2].prop_string().set_value( yyvsp[0].tok().string() ) )
     {
       yyerr(info) << "error while setting property " << yyvsp[-2].prop_string().get_name() << " to "
	     << yyvsp[0].tok().string();
     }
   }
   else 
   {
     assert( yyvsp[0].tok().get_type() == TOK_OP_STRING );
     // establish reference
     solve::explicite_reference( yyvsp[-2].prop_string(), yyvsp[0].tok().op_string(info) ); 
   }
   resolve_properties(info); // resolve properties again
 ;
    break;}
case 31:
#line 227 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.string() = yyvsp[0].string(); yyval.tok() = res;;
    break;}
case 32:
#line 228 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{Token res;res.set_op_string(yyvsp[0].op_string(info)); yyval.tok() = res;;
    break;}
case 33:
#line 232 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.flag() = yyvsp[0].flag();;
    break;}
case 34:
#line 234 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar();;
    break;}
case 35:
#line 235 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar();;
    break;}
case 36:
#line 236 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar();;
    break;}
case 37:
#line 237 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar();;
    break;}
case 38:
#line 238 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = -yyvsp[0].scalar();;
    break;}
case 39:
#line 239 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[0].scalar();;
    break;}
case 40:
#line 240 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[-1].scalar();;
    break;}
case 41:
#line 241 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = sin(yyvsp[-1].scalar());;
    break;}
case 42:
#line 242 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = sqrt(yyvsp[-1].scalar());;
    break;}
case 43:
#line 243 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.scalar() = yyvsp[0].scalar();;
    break;}
case 44:
#line 246 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector();;
    break;}
case 45:
#line 248 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 46:
#line 249 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.vector() = yyvsp[0].vector();;
    break;}
case 47:
#line 252 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.matrix() = yyvsp[0].matrix();;
    break;}
case 48:
#line 254 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.string() = yyvsp[0].string();;
    break;}
case 49:
#line 258 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_flag(info) = yyvsp[0].op_flag(info);;
    break;}
case 50:
#line 261 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() + yyvsp[0].meta_op_scalar(info)();;
    break;}
case 51:
#line 262 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() - yyvsp[0].meta_op_scalar(info)();;
    break;}
case 52:
#line 263 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() * yyvsp[0].meta_op_scalar(info)();;
    break;}
case 53:
#line 264 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() / yyvsp[0].meta_op_scalar(info)();;
    break;}
case 54:
#line 265 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = -yyvsp[0].meta_op_scalar(info)();;
    break;}
case 55:
#line 266 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)();;
    break;}
case 56:
#line 267 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[-1].meta_op_scalar(info)();;
    break;}
case 57:
#line 268 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = sqrt(yyvsp[-1].meta_op_scalar(info)());;
    break;}
case 58:
#line 271 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = solve::const_op(yyvsp[0].scalar(),msg_consultant(info));;
    break;}
case 59:
#line 272 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[0].prop_scalar();;
    break;}
case 60:
#line 273 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = yyvsp[0].op_scalar(info);;
    break;}
case 61:
#line 276 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_vector(info) = yyvsp[-2].op_vector(info) + yyvsp[0].op_vector(info);;
    break;}
case 62:
#line 278 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	  yyerr(info) << "vector creation from operands not supported yet!";
	  assert(0); ;
    break;}
case 63:
#line 281 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_vector(info) = yyvsp[0].op_vector(info);;
    break;}
case 64:
#line 284 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_matrix(info) = yyvsp[0].op_matrix(info);;
    break;}
case 65:
#line 286 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.op_string(info) = yyvsp[0].op_string(info);;
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
#line 289 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

    int parse_adl( Prop_Tree_Node *node, adlparser_info *info,
		   std::string filename )
    {
      if( filename != "" ) info->open_file( filename );
      info->set_new_tree_node( node );
      info->id_resolver = &info->res_property;

      int ret = yyparse( static_cast<void*>(info) );

      return ret;
    }

  } // close namespace adlparser
} // close namespace anitmt
