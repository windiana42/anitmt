
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_ERROR	258
#define	TOK_DUMMY	259
#define	TOK_IS_EQUAL	260
#define	TOK_NOT_EQUAL	261
#define	TOK_MORE_EQUAL	262
#define	TOK_LESS_EQUAL	263
#define	TOK_FUNC_SIN	264
#define	TOK_FUNC_SQRT	265
#define	TOK_IDENTIFIER	266
#define	TOK_FLAG	267
#define	TOK_SCALAR	268
#define	TOK_VECTOR	269
#define	TOK_MATRIX	270
#define	TOK_STRING	271
#define	TOK_OP_FLAG	272
#define	TOK_OP_SCALAR	273
#define	TOK_OP_VECTOR	274
#define	TOK_OP_MATRIX	275
#define	TOK_OP_STRING	276
#define	TOK_PROP_FLAG	277
#define	TOK_PROP_SCALAR	278
#define	TOK_PROP_VECTOR	279
#define	TOK_PROP_MATRIX	280
#define	TOK_PROP_STRING	281
#define	UMINUS	282
#define	OP_CONVERTION	283

#line 19 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

#include <iostream>
#include <string>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/reference.hpp>
#include <message/message.hpp>

#include "token.hpp"
#include "parsinfo.hpp"
#include "adlparser.hpp"

#include "parser_functions.hpp"	// help functions for the parser

#define YYPARSE_PARAM info	// parameter to the parser (yyparser(void*))
#define YYLEX_PARAM info	// parameter to the lexer
#define YYLEX_PARAM_TYPE (parser_info&)	// -> yylex(parser_info &info)

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



#define	YYFINAL		161
#define	YYFLAG		-32768
#define	YYNTBASE	42

#define YYTRANSLATE(x) ((unsigned)(x) <= 283 ? yytranslate[x] : 72)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    37,
    38,    30,    28,    40,    29,     2,    31,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    34,    39,
     2,    41,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    35,     2,    36,     2,     2,     2,     2,     2,
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
    27,    32,    33
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     9,    12,    15,    18,    21,    24,
    25,    31,    32,    39,    40,    44,    46,    48,    50,    51,
    55,    57,    59,    61,    62,    66,    68,    70,    72,    73,
    77,    79,    81,    83,    84,    88,    90,    92,    94,    98,
   100,   104,   108,   112,   116,   119,   122,   126,   131,   136,
   138,   142,   150,   154,   156,   160,   162,   166,   168,   172,
   174,   176,   180,   184,   188,   192,   195,   198,   202,   207,
   209,   211,   213,   217,   225,   229,   231,   233,   237,   239,
   241,   245,   247
};

static const short yyrhs[] = {    -1,
    42,    43,     0,    44,     0,    47,    34,     0,    50,    34,
     0,    53,    34,     0,    56,    34,     0,    59,    34,     0,
     1,    34,     0,     0,    12,    35,    45,    42,    36,     0,
     0,    12,    12,    35,    46,    42,    36,     0,     0,    23,
    48,    49,     0,    62,     0,    67,     0,     5,     0,     0,
    24,    51,    52,     0,    63,     0,    68,     0,     5,     0,
     0,    25,    54,    55,     0,    64,     0,    69,     0,     5,
     0,     0,    26,    57,    58,     0,    65,     0,    70,     0,
     5,     0,     0,    27,    60,    61,     0,    66,     0,    71,
     0,     5,     0,    37,    13,    38,     0,    13,     0,    63,
    28,    63,     0,    63,    29,    63,     0,    63,    30,    63,
     0,    63,    31,    63,     0,    29,    63,     0,    28,    63,
     0,    37,    63,    38,     0,    10,    37,    63,    38,     0,
    11,    37,    63,    38,     0,    14,     0,    64,    28,    64,
     0,    39,    63,    40,    63,    40,    63,    41,     0,    37,
    64,    38,     0,    15,     0,    37,    16,    38,     0,    16,
     0,    37,    17,    38,     0,    17,     0,    37,    18,    38,
     0,    23,     0,    18,     0,    68,    28,    68,     0,    68,
    29,    68,     0,    68,    30,    68,     0,    68,    31,    68,
     0,    29,    68,     0,    28,    68,     0,    37,    68,    38,
     0,    11,    37,    68,    38,     0,    63,     0,    24,     0,
    19,     0,    69,    28,    69,     0,    39,    68,    40,    68,
    40,    68,    41,     0,    37,    20,    38,     0,    25,     0,
    20,     0,    37,    21,    38,     0,    26,     0,    21,     0,
    37,    22,    38,     0,    27,     0,    22,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   116,   117,   120,   122,   123,   124,   125,   126,   127,   130,
   132,   133,   134,   137,   139,   141,   143,   144,   147,   149,
   151,   153,   154,   157,   159,   161,   163,   164,   167,   169,
   171,   173,   174,   177,   179,   181,   183,   184,   188,   190,
   193,   195,   196,   197,   198,   199,   200,   201,   202,   203,
   206,   208,   210,   211,   214,   216,   219,   221,   224,   226,
   227,   231,   233,   234,   235,   236,   237,   238,   239,   240,
   242,   243,   246,   248,   252,   253,   254,   257,   259,   260,
   263,   265,   266
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_ERROR","TOK_DUMMY","TOK_IS_EQUAL","TOK_NOT_EQUAL","TOK_MORE_EQUAL","TOK_LESS_EQUAL",
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
    42,    42,    43,    43,    43,    43,    43,    43,    43,    45,
    44,    46,    44,    48,    47,    49,    49,    49,    51,    50,
    52,    52,    52,    54,    53,    55,    55,    55,    57,    56,
    58,    58,    58,    60,    59,    61,    61,    61,    62,    62,
    63,    63,    63,    63,    63,    63,    63,    63,    63,    63,
    64,    64,    64,    64,    65,    65,    66,    66,    67,    67,
    67,    68,    68,    68,    68,    68,    68,    68,    68,    68,
    68,    68,    69,    69,    69,    69,    69,    70,    70,    70,
    71,    71,    71
};

static const short yyr2[] = {     0,
     0,     2,     1,     2,     2,     2,     2,     2,     2,     0,
     5,     0,     6,     0,     3,     1,     1,     1,     0,     3,
     1,     1,     1,     0,     3,     1,     1,     1,     0,     3,
     1,     1,     1,     0,     3,     1,     1,     1,     3,     1,
     3,     3,     3,     3,     2,     2,     3,     4,     4,     1,
     3,     7,     3,     1,     3,     1,     3,     1,     3,     1,
     1,     3,     3,     3,     3,     2,     2,     3,     4,     1,
     1,     1,     3,     7,     3,     1,     1,     3,     1,     1,
     3,     1,     1
};

static const short yydefact[] = {     1,
     0,     0,     0,    14,    19,    24,    29,    34,     2,     3,
     0,     0,     0,     0,     0,     9,     0,    10,     0,     0,
     0,     0,     0,     4,     5,     6,     7,     8,    12,     1,
    18,    40,    61,    60,     0,    15,    16,    17,    23,     0,
     0,    50,    72,    71,     0,     0,     0,    20,    70,    22,
    28,    54,    77,    76,     0,     0,    25,    26,    27,    33,
    56,    80,    79,     0,    30,    31,    32,    38,    58,    83,
    82,     0,    35,    36,    37,     1,     0,     0,     0,     0,
     0,    46,    67,    45,    66,    70,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,    70,
     0,     0,     0,     0,     0,     0,     0,     0,    11,    39,
    59,     0,     0,     0,     0,     0,    70,     0,    47,    68,
    41,    42,    43,    44,    70,    62,    63,    64,    65,    75,
     0,    53,     0,     0,    51,     0,     0,    73,    55,    78,
    57,    81,    13,     0,    46,    45,     0,    48,    49,    69,
     0,     0,     0,     0,     0,     0,     0,    52,    74,     0,
     0
};

static const short yydefgoto[] = {     1,
     9,    10,    30,    76,    11,    19,    36,    12,    20,    48,
    13,    21,    57,    14,    22,    65,    15,    23,    73,    37,
   125,    99,    66,    74,    38,   101,    59,    67,    75
};

static const short yypact[] = {-32768,
   125,   -31,    45,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -25,     8,    12,    22,    24,-32768,    31,-32768,     0,    99,
    -1,    34,   102,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    -6,-32768,-32768,-32768,-32768,   -22,
    47,-32768,-32768,-32768,   143,   143,   143,-32768,    48,   210,
-32768,-32768,-32768,-32768,    25,   143,-32768,    57,    63,-32768,
-32768,-32768,-32768,    -5,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    89,-32768,-32768,-32768,-32768,     5,    21,    60,   145,
   143,-32768,-32768,-32768,-32768,    61,   176,   145,   145,   145,
   145,   143,   143,   143,   143,    70,    28,   145,   -18,    72,
    23,    28,   121,    77,    78,    79,    82,    69,-32768,-32768,
-32768,    85,   145,   145,   145,   180,    92,   191,-32768,-32768,
    -9,    -9,-32768,-32768,-32768,    18,    18,-32768,-32768,-32768,
   155,-32768,   145,   143,-32768,   111,   143,-32768,-32768,-32768,
-32768,-32768,-32768,   145,-32768,-32768,   195,-32768,-32768,-32768,
   159,   163,   206,   145,   143,    59,   135,-32768,-32768,    97,
-32768
};

static const short yypgoto[] = {   -29,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -12,   -19,-32768,-32768,-32768,   -20,    30,-32768,-32768
};


#define	YYLAST		244


static const short yytable[] = {    50,
    77,    58,    16,    51,    31,     2,    78,    49,    24,   102,
   104,    79,    32,    52,    80,   105,     3,    33,    53,   132,
    90,    91,    34,    54,    83,    85,    87,     4,     5,     6,
     7,     8,    82,    84,    86,    55,    35,    56,    60,    52,
   109,    25,    52,   100,    96,    26,   108,    94,    95,    61,
    92,    93,    94,    95,    62,    27,    17,    28,   110,    63,
   118,    97,   134,    98,    97,    29,    98,   116,   117,     2,
    64,   126,   127,   128,   129,   121,   122,   123,   124,    18,
     3,   -21,   135,    81,   102,   131,    88,    89,    90,    91,
   103,     4,     5,     6,     7,     8,   161,   111,   119,   158,
   145,   146,   147,    39,   143,   106,    68,   130,    40,    41,
   107,   133,    42,   152,   139,   140,   141,    43,    69,   142,
   151,   144,    44,    70,   160,     2,    45,    46,    71,   149,
    96,   153,   138,     0,   157,    47,     3,     0,    72,     0,
    53,   156,     0,     0,     0,    54,     0,     4,     5,     6,
     7,     8,    40,    41,    40,   112,    42,   136,    42,   137,
     0,    43,    92,    93,    94,    95,    44,     0,     0,     0,
    45,    46,   113,   114,     0,   159,     0,     0,     0,    47,
     0,   115,    88,    89,    90,    91,    88,    89,    90,    91,
    92,    93,    94,    95,   133,     0,     0,     0,   154,     0,
     0,     0,   155,    92,    93,    94,    95,    88,    89,    90,
    91,     0,     0,   120,     0,     0,     0,   148,    92,    93,
    94,    95,    88,    89,    90,    91,     0,     0,   150,     0,
     0,     0,   119,    88,    89,    90,    91,    92,    93,    94,
    95,     0,     0,   149
};

static const short yycheck[] = {    20,
    30,    21,    34,     5,     5,     1,    13,    20,    34,    28,
    16,    18,    13,    15,    37,    21,    12,    18,    20,    38,
    30,    31,    23,    25,    45,    46,    47,    23,    24,    25,
    26,    27,    45,    46,    47,    37,    37,    39,     5,    15,
    36,    34,    15,    56,    20,    34,    76,    30,    31,    16,
    28,    29,    30,    31,    21,    34,    12,    34,    38,    26,
    81,    37,    40,    39,    37,    35,    39,    80,    81,     1,
    37,    92,    93,    94,    95,    88,    89,    90,    91,    35,
    12,    34,   102,    37,    28,    98,    28,    29,    30,    31,
    28,    23,    24,    25,    26,    27,     0,    38,    38,    41,
   113,   114,   115,     5,    36,    17,     5,    38,    10,    11,
    22,    40,    14,   134,    38,    38,    38,    19,    17,    38,
   133,    37,    24,    22,     0,     1,    28,    29,    27,    38,
    20,   144,   103,    -1,   155,    37,    12,    -1,    37,    -1,
    20,   154,    -1,    -1,    -1,    25,    -1,    23,    24,    25,
    26,    27,    10,    11,    10,    11,    14,    37,    14,    39,
    -1,    19,    28,    29,    30,    31,    24,    -1,    -1,    -1,
    28,    29,    28,    29,    -1,    41,    -1,    -1,    -1,    37,
    -1,    37,    28,    29,    30,    31,    28,    29,    30,    31,
    28,    29,    30,    31,    40,    -1,    -1,    -1,    40,    -1,
    -1,    -1,    40,    28,    29,    30,    31,    28,    29,    30,
    31,    -1,    -1,    38,    -1,    -1,    -1,    38,    28,    29,
    30,    31,    28,    29,    30,    31,    -1,    -1,    38,    -1,
    -1,    -1,    38,    28,    29,    30,    31,    28,    29,    30,
    31,    -1,    -1,    38
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
#line 127 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerrok; /* error recovery */ ;
    break;}
case 10:
#line 131 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-1].identifier()); ;
    break;}
case 11:
#line 132 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 12:
#line 133 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-2].identifier(),yyvsp[-1].identifier());;
    break;}
case 13:
#line 134 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 14:
#line 138 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_flag(),info); ;
    break;}
case 15:
#line 139 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ flag_prop_declaration_finish(yyvsp[-2].prop_flag(),yyvsp[0].tok(),info); ;
    break;}
case 16:
#line 142 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 17:
#line 143 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_flag(info) = yyvsp[0].meta_op_flag(info)(); ;
    break;}
case 18:
#line 144 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 19:
#line 148 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_scalar(),info); ;
    break;}
case 20:
#line 149 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ scalar_prop_declaration_finish(yyvsp[-2].prop_scalar(),yyvsp[0].tok(),info); ;
    break;}
case 21:
#line 152 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 22:
#line 153 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 23:
#line 154 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 24:
#line 158 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_vector(),info); ;
    break;}
case 25:
#line 159 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ vector_prop_declaration_finish(yyvsp[-2].prop_vector(),yyvsp[0].tok(),info); ;
    break;}
case 26:
#line 162 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 27:
#line 163 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 28:
#line 164 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 29:
#line 168 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_matrix(),info); ;
    break;}
case 30:
#line 169 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ matrix_prop_declaration_finish(yyvsp[-2].prop_matrix(),yyvsp[0].tok(),info); ;
    break;}
case 31:
#line 172 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 32:
#line 173 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_matrix(info) = yyvsp[0].meta_op_matrix(info)(); ;
    break;}
case 33:
#line 174 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 34:
#line 178 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_string(),info); ;
    break;}
case 35:
#line 179 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ string_prop_declaration_finish(yyvsp[-2].prop_string(),yyvsp[0].tok(),info); ;
    break;}
case 36:
#line 182 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 37:
#line 183 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_string(info) = yyvsp[0].meta_op_string(info)(); ;
    break;}
case 38:
#line 184 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 39:
#line 189 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[-1].flag(); ;
    break;}
case 40:
#line 190 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 41:
#line 194 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar(); ;
    break;}
case 42:
#line 195 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar(); ;
    break;}
case 43:
#line 196 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar(); ;
    break;}
case 44:
#line 197 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar(); ;
    break;}
case 45:
#line 198 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = -yyvsp[0].scalar(); ;
    break;}
case 46:
#line 199 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 47:
#line 200 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-1].scalar(); ;
    break;}
case 48:
#line 201 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sin(yyvsp[-1].scalar()); ;
    break;}
case 49:
#line 202 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sqrt(yyvsp[-1].scalar()); ;
    break;}
case 50:
#line 203 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 51:
#line 207 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector(); ;
    break;}
case 52:
#line 209 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 53:
#line 210 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-1].vector(); ;
    break;}
case 54:
#line 211 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 55:
#line 215 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[-1].matrix(); ;
    break;}
case 56:
#line 216 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 57:
#line 220 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[-1].string(); ;
    break;}
case 58:
#line 221 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 59:
#line 225 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[-1].op_flag(info);;
    break;}
case 60:
#line 226 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].prop_flag();;
    break;}
case 61:
#line 227 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].op_flag(info);;
    break;}
case 62:
#line 232 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() + yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 63:
#line 233 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() - yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 64:
#line 234 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() * yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 65:
#line 235 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() / yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 66:
#line 236 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = -yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 67:
#line 237 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 68:
#line 238 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-1].meta_op_scalar(info)(); ;
    break;}
case 69:
#line 239 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = sqrt(yyvsp[-1].meta_op_scalar(info)()); ;
    break;}
case 70:
#line 241 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = solve::const_op(yyvsp[0].scalar(),msg_consultant(info));;
    break;}
case 71:
#line 242 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].prop_scalar(); ;
    break;}
case 72:
#line 243 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].op_scalar(info); ;
    break;}
case 73:
#line 247 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-2].meta_op_vector(info)() + yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 74:
#line 249 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	yyerr(info) << "vector creation from operands not supported yet!";
	assert(0); ;
    break;}
case 75:
#line 252 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-1].op_vector(info); ;
    break;}
case 76:
#line 253 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].prop_vector(); ;
    break;}
case 77:
#line 254 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].op_vector(info); ;
    break;}
case 78:
#line 258 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[-1].op_matrix(info);;
    break;}
case 79:
#line 259 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].prop_matrix();;
    break;}
case 80:
#line 260 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].op_matrix(info);;
    break;}
case 81:
#line 264 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[-1].op_string(info);;
    break;}
case 82:
#line 265 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].prop_string();;
    break;}
case 83:
#line 266 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].op_string(info);;
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
#line 269 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

    int parse_adl( Prop_Tree_Node *node, message::Message_Consultant *c, 
		   std::string filename, pass_type pass )
    {
      adlparser_info info(c);
      info.id_resolver = &info.res_property;
      
      info.open_file( filename );
      info.set_new_tree_node( node );
      info.set_pass(pass);
      int ret = yyparse( static_cast<void*>(&info) );
      info.close_file();
      
      return ret;
    }

  } // close namespace adlparser
} // close namespace anitmt
