
/*  A Bison parser, made from /home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	TOK_INVALID_ID	257
#define	TOK_ERROR	258
#define	TOK_DUMMY_OPERAND	259
#define	TOK_DUMMY_OPERATOR	260
#define	TOK_IS_EQUAL	261
#define	TOK_NOT_EQUAL	262
#define	TOK_MORE_EQUAL	263
#define	TOK_LESS_EQUAL	264
#define	TOK_FUNC_SIN	265
#define	TOK_FUNC_SQRT	266
#define	TOK_IDENTIFIER	267
#define	TOK_FLAG	268
#define	TOK_SCALAR	269
#define	TOK_VECTOR	270
#define	TOK_MATRIX	271
#define	TOK_STRING	272
#define	TOK_OP_FLAG	273
#define	TOK_OP_SCALAR	274
#define	TOK_OP_VECTOR	275
#define	TOK_OP_MATRIX	276
#define	TOK_OP_STRING	277
#define	TOK_PROP_FLAG	278
#define	TOK_PROP_SCALAR	279
#define	TOK_PROP_VECTOR	280
#define	TOK_PROP_MATRIX	281
#define	TOK_PROP_STRING	282
#define	lowest_precedence	283
#define	UMINUS	284
#define	OP_CONVERTION	285
#define	left_associated	286
#define	right_associated	287
#define	lower_precedence	288
#define	higher_precedence	289
#define	highest_precedence	290

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
				// including nessessary defines 

  // open namespaces
  namespace anitmt
  {
    namespace adlparser
    {

#define MAX_OLD_POSITIONS 	10

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		191
#define	YYFLAG		-32768
#define	YYNTBASE	49

#define YYTRANSLATE(x) ((unsigned)(x) <= 290 ? yytranslate[x] : 82)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    44,
    45,    32,    30,    47,    31,     2,    33,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    41,    46,
     2,    48,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    42,     2,    43,     2,     2,     2,     2,     2,
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
    27,    28,    29,    34,    35,    36,    37,    38,    39,    40
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     6,     8,    11,    14,    17,    20,    23,
    26,    27,    33,    34,    41,    42,    46,    48,    50,    52,
    53,    57,    59,    61,    63,    64,    68,    70,    72,    74,
    75,    79,    81,    83,    85,    86,    90,    92,    94,    96,
   100,   102,   106,   110,   114,   118,   121,   124,   128,   133,
   138,   140,   144,   152,   156,   158,   162,   164,   168,   170,
   174,   176,   178,   182,   186,   190,   194,   197,   200,   204,
   209,   211,   213,   215,   219,   227,   231,   233,   235,   239,
   241,   243,   247,   249,   251,   253,   256,   260,   262,   266,
   271,   275,   278,   281,   283,   285,   287,   289,   291
};

static const short yyrhs[] = {    -1,
    49,    50,     0,     1,     0,    51,     0,    54,    41,     0,
    57,    41,     0,    60,    41,     0,    63,    41,     0,    66,
    41,     0,     1,    41,     0,     0,    13,    42,    52,    49,
    43,     0,     0,    13,    13,    42,    53,    49,    43,     0,
     0,    24,    55,    56,     0,    69,     0,    74,     0,    79,
     0,     0,    25,    58,    59,     0,    70,     0,    75,     0,
    79,     0,     0,    26,    61,    62,     0,    71,     0,    76,
     0,    79,     0,     0,    27,    64,    65,     0,    72,     0,
    77,     0,    79,     0,     0,    28,    67,    68,     0,    73,
     0,    78,     0,    79,     0,    44,    14,    45,     0,    14,
     0,    70,    30,    70,     0,    70,    31,    70,     0,    70,
    32,    70,     0,    70,    33,    70,     0,    31,    70,     0,
    30,    70,     0,    44,    70,    45,     0,    11,    44,    70,
    45,     0,    12,    44,    70,    45,     0,    15,     0,    71,
    30,    71,     0,    46,    70,    47,    70,    47,    70,    48,
     0,    44,    71,    45,     0,    16,     0,    44,    17,    45,
     0,    17,     0,    44,    18,    45,     0,    18,     0,    44,
    19,    45,     0,    24,     0,    19,     0,    75,    30,    75,
     0,    75,    31,    75,     0,    75,    32,    75,     0,    75,
    33,    75,     0,    31,    75,     0,    30,    75,     0,    44,
    75,    45,     0,    12,    44,    75,    45,     0,    70,     0,
    25,     0,    20,     0,    76,    30,    76,     0,    46,    75,
    47,    75,    47,    75,    48,     0,    44,    21,    45,     0,
    26,     0,    21,     0,    44,    22,    45,     0,    27,     0,
    22,     0,    44,    23,    45,     0,    28,     0,    23,     0,
    80,     0,    80,     1,     0,    80,    81,    81,     0,     1,
     0,    80,    81,    80,     0,     5,    44,    80,    45,     0,
    44,    80,    45,     0,    31,     5,     0,    30,     5,     0,
     5,     0,     6,     0,    30,     0,    31,     0,    32,     0,
    33,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   130,   131,   132,   135,   137,   138,   139,   140,   141,   142,
   145,   147,   148,   149,   152,   154,   156,   158,   159,   162,
   164,   166,   168,   169,   172,   174,   176,   178,   179,   182,
   184,   186,   188,   189,   192,   194,   196,   198,   199,   203,
   205,   208,   210,   211,   212,   213,   214,   215,   216,   217,
   218,   221,   223,   225,   226,   229,   231,   234,   236,   239,
   241,   242,   246,   248,   249,   250,   251,   252,   253,   254,
   255,   257,   258,   261,   263,   267,   268,   269,   272,   274,
   275,   278,   280,   281,   284,   286,   288,   291,   295,   297,
   298,   299,   300,   301,   303,   305,   305,   305,   305
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","TOK_INVALID_ID",
"TOK_ERROR","TOK_DUMMY_OPERAND","TOK_DUMMY_OPERATOR","TOK_IS_EQUAL","TOK_NOT_EQUAL",
"TOK_MORE_EQUAL","TOK_LESS_EQUAL","TOK_FUNC_SIN","TOK_FUNC_SQRT","TOK_IDENTIFIER",
"TOK_FLAG","TOK_SCALAR","TOK_VECTOR","TOK_MATRIX","TOK_STRING","TOK_OP_FLAG",
"TOK_OP_SCALAR","TOK_OP_VECTOR","TOK_OP_MATRIX","TOK_OP_STRING","TOK_PROP_FLAG",
"TOK_PROP_SCALAR","TOK_PROP_VECTOR","TOK_PROP_MATRIX","TOK_PROP_STRING","lowest_precedence",
"'+'","'-'","'*'","'/'","UMINUS","OP_CONVERTION","left_associated","right_associated",
"lower_precedence","higher_precedence","highest_precedence","';'","'{'","'}'",
"'('","')'","'<'","','","'>'","tree_node_block","statement","child_declaration",
"@1","@2","flag_statement","@3","any_flag_exp","scalar_statement","@4","any_scalar_exp",
"vector_statement","@5","any_vector_exp","matrix_statement","@6","any_matrix_exp",
"string_statement","@7","any_string_exp","flag_exp","scalar_exp","vector_exp",
"matrix_exp","string_exp","op_flag_exp","op_scalar_exp","op_vector_exp","op_matrix_exp",
"op_string_exp","receive_dummy_exp","dummy_exp","dummy_operator", NULL
};
#endif

static const short yyr1[] = {     0,
    49,    49,    49,    50,    50,    50,    50,    50,    50,    50,
    52,    51,    53,    51,    55,    54,    56,    56,    56,    58,
    57,    59,    59,    59,    61,    60,    62,    62,    62,    64,
    63,    65,    65,    65,    67,    66,    68,    68,    68,    69,
    69,    70,    70,    70,    70,    70,    70,    70,    70,    70,
    70,    71,    71,    71,    71,    72,    72,    73,    73,    74,
    74,    74,    75,    75,    75,    75,    75,    75,    75,    75,
    75,    75,    75,    76,    76,    76,    76,    76,    77,    77,
    77,    78,    78,    78,    79,    79,    79,    79,    80,    80,
    80,    80,    80,    80,    81,    81,    81,    81,    81
};

static const short yyr2[] = {     0,
     0,     2,     1,     1,     2,     2,     2,     2,     2,     2,
     0,     5,     0,     6,     0,     3,     1,     1,     1,     0,
     3,     1,     1,     1,     0,     3,     1,     1,     1,     0,
     3,     1,     1,     1,     0,     3,     1,     1,     1,     3,
     1,     3,     3,     3,     3,     2,     2,     3,     4,     4,
     1,     3,     7,     3,     1,     3,     1,     3,     1,     3,
     1,     1,     3,     3,     3,     3,     2,     2,     3,     4,
     1,     1,     1,     3,     7,     3,     1,     1,     3,     1,
     1,     3,     1,     1,     1,     2,     3,     1,     3,     4,
     3,     2,     2,     1,     1,     1,     1,     1,     1
};

static const short yydefact[] = {     0,
     3,     0,     0,     0,    15,    20,    25,    30,    35,     2,
     4,     0,     0,     0,     0,     0,    10,     0,    11,     0,
     0,     0,     0,     0,     5,     6,     7,     8,     9,    13,
     0,    88,    94,    41,    62,    61,     0,     0,     0,    16,
    17,    18,    19,     0,     0,     0,    51,    73,    72,     0,
     0,     0,    21,    71,    23,    24,    55,    78,    77,     0,
     0,    26,    27,    28,    29,    57,    81,    80,     0,    31,
    32,    33,    34,    59,    84,    83,     0,    36,    37,    38,
    39,     0,     0,     0,    93,    92,     0,     0,     0,     0,
    86,    95,    96,    97,    98,    99,     0,     0,     0,     0,
     0,     0,    47,    68,    46,    67,    71,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    71,     0,     0,     0,     0,     0,     0,     0,     0,    12,
     0,    40,    60,    91,     0,    96,    97,    89,    87,     0,
     0,     0,     0,     0,    71,     0,    48,    69,    42,    43,
    44,    45,    71,    63,    64,    65,    66,    76,     0,    54,
     0,     0,     0,    52,     0,     0,    74,    56,    79,    58,
    82,    14,    90,     0,    47,    46,     0,    49,    50,    70,
     0,     0,     0,     0,     0,     0,     0,    53,    75,     0,
     0
};

static const short yydefgoto[] = {     2,
    10,    11,    31,    82,    12,    20,    40,    13,    21,    53,
    14,    22,    62,    15,    23,    70,    16,    24,    78,    41,
   153,   120,    71,    79,    42,   104,    64,    72,    80,    43,
    90,   135
};

static const short yypact[] = {   326,
-32768,   331,   -27,    21,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,    12,    23,    32,    33,    41,-32768,   -16,-32768,   149,
   122,    31,    28,   139,-32768,-32768,-32768,-32768,-32768,-32768,
   164,-32768,     2,-32768,-32768,-32768,    22,    78,   259,-32768,
-32768,-32768,-32768,    70,    61,    63,-32768,-32768,-32768,   217,
   238,   240,-32768,    68,    -8,-32768,-32768,-32768,-32768,   105,
   303,-32768,    82,    88,-32768,-32768,-32768,-32768,   257,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   281,-32768,-32768,-32768,
-32768,   164,   171,    60,-32768,-32768,    74,    75,    60,    11,
-32768,-32768,-32768,-32768,-32768,-32768,   261,   305,   303,   303,
   303,   303,-32768,-32768,-32768,-32768,    77,   342,   305,   305,
   305,   305,   303,   303,   303,   303,    79,   195,   305,   -10,
    85,    37,     5,   187,    80,    93,    94,    98,   193,-32768,
   277,-32768,-32768,-32768,    60,    22,    78,   140,-32768,   111,
   305,   305,   305,   358,   113,   362,-32768,-32768,   -17,   -17,
-32768,-32768,-32768,   -14,   -14,-32768,-32768,-32768,   330,-32768,
   305,   303,     5,-32768,   135,   303,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,   305,-32768,-32768,   366,-32768,-32768,-32768,
   334,   338,   382,   305,   303,   -20,    83,-32768,-32768,   159,
-32768
};

static const short yypgoto[] = {   -22,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -13,   -15,-32768,-32768,-32768,   -21,    36,-32768,-32768,   180,
   -18,   -43
};


#define	YYLAST		427


static const short yytable[] = {    55,
    97,    44,    44,    44,    44,    44,    63,    54,    83,   109,
   110,   111,   112,    17,   111,   112,    92,   115,   116,   123,
    57,   113,   114,   115,   116,    30,    85,   188,    32,   106,
   108,    32,    33,    18,   160,    33,   103,   105,   107,   122,
    93,    94,    95,    96,    66,    84,    57,   121,   163,    67,
   119,    58,    25,   139,    68,   134,    59,    37,    38,   129,
    37,    38,    19,    26,    33,   131,   113,   114,   115,   116,
    91,    69,    27,    28,    60,    92,    61,   146,   138,   106,
   108,    29,    86,   162,   144,   145,   103,   105,   107,    37,
    38,   154,   155,   156,   157,   149,   150,   151,   152,    93,
    94,    95,    96,    89,    98,   159,    99,   164,   -22,    33,
   -85,   123,   113,   114,   115,   116,   138,   124,   132,   133,
    57,   147,    32,   158,   168,   117,    33,   175,   176,   177,
   189,   161,    45,    46,    37,    38,    47,   169,   170,    32,
   182,    48,   171,    33,   122,    92,    49,   181,   118,    32,
   119,    50,    51,    33,   174,   117,    74,   179,   191,   167,
   183,    75,    34,   187,     1,    52,    76,    35,    37,    38,
   186,     3,    36,     0,     0,     0,    -1,     0,    37,    38,
     0,     0,    77,     4,     0,     0,     0,    -1,    -1,    -1,
    -1,    -1,    39,     3,     5,     6,     7,     8,     9,    33,
    56,    65,    73,    81,     0,     4,    -1,    58,     0,     0,
    57,     0,    59,   130,     0,     0,     5,     6,     7,     8,
     9,    85,     0,     0,    37,    38,     0,    45,    46,     0,
   165,    47,   166,     0,     0,   172,    48,     0,   118,     0,
   119,    49,    86,     0,    33,     0,   100,   101,    45,    46,
    45,    46,    47,     0,    47,     0,     0,    48,     0,    48,
   102,    33,    49,    33,    49,    33,    92,   100,   101,    50,
    51,     0,    87,   125,     0,     0,     0,    88,   126,     0,
     0,   102,    92,    52,     0,    33,    37,    38,    37,    38,
   136,   137,    95,    96,     0,     0,     0,     0,   127,     0,
    89,     0,    89,   128,    89,     0,    93,    94,    95,    96,
    37,    38,     0,    45,    46,    45,   140,    47,     0,    47,
     0,   173,    48,     0,    89,    -1,     1,    49,     0,     0,
   190,     3,   100,   101,   141,   142,     0,     0,    -1,     0,
     0,     0,     0,     4,     0,     0,   102,     0,   143,    -1,
    -1,    -1,    -1,    -1,     5,     6,     7,     8,     9,   109,
   110,   111,   112,   109,   110,   111,   112,   113,   114,   115,
   116,   113,   114,   115,   116,     0,   161,     0,     0,     0,
   184,     0,     0,     0,   185,     0,   148,   109,   110,   111,
   112,   113,   114,   115,   116,   109,   110,   111,   112,     0,
     0,     0,   178,     0,     0,     0,   180,     0,     0,     0,
   147,   109,   110,   111,   112,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,   179
};

static const short yycheck[] = {    21,
    44,    20,    21,    22,    23,    24,    22,    21,    31,    30,
    31,    32,    33,    41,    32,    33,     6,    32,    33,    30,
    16,    30,    31,    32,    33,    42,     5,    48,     1,    51,
    52,     1,     5,    13,    45,     5,    50,    51,    52,    61,
    30,    31,    32,    33,    17,    44,    16,    61,    44,    22,
    46,    21,    41,    97,    27,    45,    26,    30,    31,    82,
    30,    31,    42,    41,     5,    84,    30,    31,    32,    33,
     1,    44,    41,    41,    44,     6,    46,    99,    97,   101,
   102,    41,     5,    47,    98,    99,   100,   101,   102,    30,
    31,   113,   114,   115,   116,   109,   110,   111,   112,    30,
    31,    32,    33,    44,    44,   119,    44,   123,    41,     5,
    41,    30,    30,    31,    32,    33,   135,    30,    45,    45,
    16,    45,     1,    45,    45,    21,     5,   141,   142,   143,
    48,    47,    11,    12,    30,    31,    15,    45,    45,     1,
   162,    20,    45,     5,   166,     6,    25,   161,    44,     1,
    46,    30,    31,     5,    44,    21,    18,    45,     0,   124,
   174,    23,    14,   185,     1,    44,    28,    19,    30,    31,
   184,     1,    24,    -1,    -1,    -1,    13,    -1,    30,    31,
    -1,    -1,    44,    13,    -1,    -1,    -1,    24,    25,    26,
    27,    28,    44,     1,    24,    25,    26,    27,    28,     5,
    21,    22,    23,    24,    -1,    13,    43,    21,    -1,    -1,
    16,    -1,    26,    43,    -1,    -1,    24,    25,    26,    27,
    28,     5,    -1,    -1,    30,    31,    -1,    11,    12,    -1,
    44,    15,    46,    -1,    -1,    43,    20,    -1,    44,    -1,
    46,    25,     5,    -1,     5,    -1,    30,    31,    11,    12,
    11,    12,    15,    -1,    15,    -1,    -1,    20,    -1,    20,
    44,     5,    25,     5,    25,     5,     6,    30,    31,    30,
    31,    -1,    14,    17,    -1,    -1,    -1,    19,    22,    -1,
    -1,    44,     6,    44,    -1,     5,    30,    31,    30,    31,
    30,    31,    32,    33,    -1,    -1,    -1,    -1,    18,    -1,
    44,    -1,    44,    23,    44,    -1,    30,    31,    32,    33,
    30,    31,    -1,    11,    12,    11,    12,    15,    -1,    15,
    -1,    45,    20,    -1,    44,     0,     1,    25,    -1,    -1,
     0,     1,    30,    31,    30,    31,    -1,    -1,    13,    -1,
    -1,    -1,    -1,    13,    -1,    -1,    44,    -1,    44,    24,
    25,    26,    27,    28,    24,    25,    26,    27,    28,    30,
    31,    32,    33,    30,    31,    32,    33,    30,    31,    32,
    33,    30,    31,    32,    33,    -1,    47,    -1,    -1,    -1,
    47,    -1,    -1,    -1,    47,    -1,    45,    30,    31,    32,
    33,    30,    31,    32,    33,    30,    31,    32,    33,    -1,
    -1,    -1,    45,    -1,    -1,    -1,    45,    -1,    -1,    -1,
    45,    30,    31,    32,    33,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    45
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

case 3:
#line 132 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerrok; /* error recovery */ ;
    break;}
case 10:
#line 142 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerrok; /* error recovery */ ;
    break;}
case 11:
#line 146 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-1].identifier()); ;
    break;}
case 12:
#line 147 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 13:
#line 148 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_current_child(info,yyvsp[-2].identifier(),yyvsp[-1].identifier());;
    break;}
case 14:
#line 149 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ change_to_parent(info); ;
    break;}
case 15:
#line 153 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_flag(),info); ;
    break;}
case 16:
#line 154 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ flag_prop_declaration_finish(yyvsp[-2].prop_flag(),yyvsp[0].tok(),info); ;
    break;}
case 17:
#line 157 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 18:
#line 158 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_flag(info) = yyvsp[0].meta_op_flag(info)(); ;
    break;}
case 19:
#line 159 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 20:
#line 163 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_scalar(),info); ;
    break;}
case 21:
#line 164 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ scalar_prop_declaration_finish(yyvsp[-2].prop_scalar(),yyvsp[0].tok(),info); ;
    break;}
case 22:
#line 167 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 23:
#line 168 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 24:
#line 169 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 25:
#line 173 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_vector(),info); ;
    break;}
case 26:
#line 174 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ vector_prop_declaration_finish(yyvsp[-2].prop_vector(),yyvsp[0].tok(),info); ;
    break;}
case 27:
#line 177 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 28:
#line 178 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 29:
#line 179 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 30:
#line 183 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_matrix(),info); ;
    break;}
case 31:
#line 184 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ matrix_prop_declaration_finish(yyvsp[-2].prop_matrix(),yyvsp[0].tok(),info); ;
    break;}
case 32:
#line 187 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 33:
#line 188 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_matrix(info) = yyvsp[0].meta_op_matrix(info)(); ;
    break;}
case 34:
#line 189 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 35:
#line 193 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ prop_declaration_start(yyvsp[0].prop_string(),info); ;
    break;}
case 36:
#line 194 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ string_prop_declaration_finish(yyvsp[-2].prop_string(),yyvsp[0].tok(),info); ;
    break;}
case 37:
#line 197 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 38:
#line 198 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_string(info) = yyvsp[0].meta_op_string(info)(); ;
    break;}
case 39:
#line 199 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.tok(); ;
    break;}
case 40:
#line 204 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[-1].flag(); ;
    break;}
case 41:
#line 205 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.flag() = yyvsp[0].flag(); ;
    break;}
case 42:
#line 209 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() + yyvsp[0].scalar(); ;
    break;}
case 43:
#line 210 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() - yyvsp[0].scalar(); ;
    break;}
case 44:
#line 211 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() * yyvsp[0].scalar(); ;
    break;}
case 45:
#line 212 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-2].scalar() / yyvsp[0].scalar(); ;
    break;}
case 46:
#line 213 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = -yyvsp[0].scalar(); ;
    break;}
case 47:
#line 214 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 48:
#line 215 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[-1].scalar(); ;
    break;}
case 49:
#line 216 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sin(yyvsp[-1].scalar()); ;
    break;}
case 50:
#line 217 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = sqrt(yyvsp[-1].scalar()); ;
    break;}
case 51:
#line 218 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.scalar() = yyvsp[0].scalar(); ;
    break;}
case 52:
#line 222 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-2].vector() + yyvsp[0].vector(); ;
    break;}
case 53:
#line 224 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = values::Vector( yyvsp[-5].scalar(), yyvsp[-3].scalar(), yyvsp[-1].scalar() ); ;
    break;}
case 54:
#line 225 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[-1].vector(); ;
    break;}
case 55:
#line 226 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.vector() = yyvsp[0].vector(); ;
    break;}
case 56:
#line 230 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[-1].matrix(); ;
    break;}
case 57:
#line 231 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.matrix() = yyvsp[0].matrix(); ;
    break;}
case 58:
#line 235 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[-1].string(); ;
    break;}
case 59:
#line 236 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.string() = yyvsp[0].string(); ;
    break;}
case 60:
#line 240 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[-1].op_flag(info);;
    break;}
case 61:
#line 241 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].prop_flag();;
    break;}
case 62:
#line 242 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_flag(info) = yyvsp[0].op_flag(info);;
    break;}
case 63:
#line 247 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() + yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 64:
#line 248 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() - yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 65:
#line 249 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() * yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 66:
#line 250 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-2].meta_op_scalar(info)() / yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 67:
#line 251 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = -yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 68:
#line 252 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].meta_op_scalar(info)(); ;
    break;}
case 69:
#line 253 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[-1].meta_op_scalar(info)(); ;
    break;}
case 70:
#line 254 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = sqrt(yyvsp[-1].meta_op_scalar(info)()); ;
    break;}
case 71:
#line 256 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_scalar(info) = solve::const_op(yyvsp[0].scalar(),msg_consultant(info));;
    break;}
case 72:
#line 257 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].prop_scalar(); ;
    break;}
case 73:
#line 258 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_scalar(info) = yyvsp[0].op_scalar(info); ;
    break;}
case 74:
#line 262 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-2].meta_op_vector(info)() + yyvsp[0].meta_op_vector(info)(); ;
    break;}
case 75:
#line 264 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ /*$$ = combine_Vector( $2, $4, $6 ); not supported yet*/ 
	yyerr(info) << "vector creation from operands not supported yet!";
	assert(0); ;
    break;}
case 76:
#line 267 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[-1].op_vector(info); ;
    break;}
case 77:
#line 268 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].prop_vector(); ;
    break;}
case 78:
#line 269 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyval.meta_op_vector(info) = yyvsp[0].op_vector(info); ;
    break;}
case 79:
#line 273 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[-1].op_matrix(info);;
    break;}
case 80:
#line 274 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].prop_matrix();;
    break;}
case 81:
#line 275 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_matrix(info) = yyvsp[0].op_matrix(info);;
    break;}
case 82:
#line 279 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[-1].op_string(info);;
    break;}
case 83:
#line 280 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].prop_string();;
    break;}
case 84:
#line 281 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{yyval.meta_op_string(info) = yyvsp[0].op_string(info);;
    break;}
case 86:
#line 286 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerr(info,2) << "operator or ';' expected"; 
			initialize_lexer(info); yyerrok; ;
    break;}
case 87:
#line 289 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerr(info,2)<< "operand expected instead of operator";
 			initialize_lexer(info); yyerrok; ;
    break;}
case 88:
#line 292 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"
{ yyerr(info,2) << "operand expected"; 
			initialize_lexer(info); yyerrok; ;
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
#line 306 "/home/martin/Programmieren/sourceforge/anitmt/anitmt-calc/input/adl2/./parser.yy"

    int parse_adl( Prop_Tree_Node *node, message::Message_Consultant *c, 
		   std::string filename, pass_type pass )
    {
      adlparser_info info(c);
      info.set_max_old_positions(MAX_OLD_POSITIONS);
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
