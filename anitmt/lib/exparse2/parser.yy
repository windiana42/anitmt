/* Test parser */

%{
#include <iostream>

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <message/message.hpp>

#include "exparser.hpp"

#define YYPARSE_PARAM info
#define YYLEX_PARAM info
#define YYLEX_PARAM_TYPE (parser_info&)
// interface to lexer
#define yyFlexLexer exparser_FlexLexer
#include <FlexLexer.h>
  
  namespace exparser
  {
    ::exparser_FlexLexer lexer;

    #define YYSTYPE Token
    int yylex( Token *lvalp, void *info )
    {
      lexer.info = static_cast<parser_info*> (info);
      lexer.yylval = lvalp;	// lvalue variable to return token value
      return lexer.yylex();
    }

    // redefine error output
#define yyerror( s ) ( ((parser_info*)info)-> \
  msg.error(new message::File_Position("unknown", lexer.lineno() )) << s, 1 )

    /*		       
    int yyerror( char *s )
    {
      cerr << "error: " << s;
      return 0;
    }
    */

/*
  int yylex()
  {
    return ::yylex();
  }
  int yylex()
  {
    return lexer.yylex();
  }
*/
%}

%pure_parser

%token TOK_INVALID_ID
// multi character operators
%token TOK_IS_EQUAL TOK_NOT_EQUAL TOK_MORE_EQUAL TOK_LESS_EQUAL 
// functions
%token TOK_FUNC_SIN
// type tokens
%token <op_flag()>   TOK_OP_FLAG
%token <op_scalar()> TOK_OP_SCALAR
%token <op_vector()> TOK_OP_VECTOR
%token <op_matrix()> TOK_OP_MATRIX
%token <op_string()> TOK_OP_STRING
%token <flag()>   TOK_FLAG
%token <scalar()> TOK_SCALAR
%token <vector()> TOK_VECTOR
%token <matrix()> TOK_MATRIX
%token <string()> TOK_STRING
// types of rules
%type <flag()>	 flag_exp 
%type <scalar()> scalar_exp 
%type <vector()> vector_exp 
%type <matrix()> matrix_exp 
%type <string()> string_exp 
%type <op_flag()>   op_flag_exp 
%type <op_scalar()> op_scalar_exp 
%type <op_vector()> op_vector_exp 
%type <op_matrix()> op_matrix_exp 
%type <op_string()> op_string_exp 

%left '+' '-'
%left '*' '/'
%nonassoc UMINUS
%nonassoc OP_CONVERTION 

%%
any_statement: scalar_statement
	| vector_statement
	;

scalar_statement: scalar_exp ';'
 { 
   Token result; result.scalar() = $1;
   static_cast<parser_info*>(info)->result = result; 
   return 0;
 }
	;
vector_statement: vector_exp ';'
 { 
   Token result; result.vector() = $1;
   static_cast<parser_info*>(info)->result = result; 
   return 0;
 }
	;

flag_exp: TOK_FLAG {$$ = $1;}

scalar_exp: scalar_exp '+' scalar_exp {$$ = $1 + $3;} 
	  | scalar_exp '-' scalar_exp {$$ = $1 - $3;}
	  | scalar_exp '*' scalar_exp {$$ = $1 * $3;}
  	  | scalar_exp '/' scalar_exp {$$ = $1 / $3;}
	  | '-' scalar_exp %prec UMINUS {$$ = -$2;}
	  | TOK_FUNC_SIN '(' scalar_exp ')' {$$ = sin($3);}
          | TOK_SCALAR {$$ = $1;}
	;

vector_exp: vector_exp '+' vector_exp {$$ = $1 + $3;} 
	  | '<' scalar_exp ',' scalar_exp ',' scalar_exp '>' { $$ = values::Vector( $2, $4, $6 ); }
	  | TOK_VECTOR {$$ = $1;}
	;

matrix_exp: TOK_MATRIX {$$ = $1;}
	;
string_exp: TOK_STRING {$$ = $1;}
	;


op_flag_exp: TOK_OP_FLAG {$$ = $1;}
	;

op_scalar_exp: op_scalar_exp '+' op_scalar_exp {$$ = $1 + $3;} 
	  | op_scalar_exp '-' op_scalar_exp {$$ = $1 - $3;}
	  | op_scalar_exp '*' op_scalar_exp {$$ = $1 * $3;}
  	  | op_scalar_exp '/' op_scalar_exp {$$ = $1 / $3;}
	  | '-' op_scalar_exp %prec UMINUS  {$$ = -$2;}
          | TOK_OP_SCALAR %prec OP_CONVERTION {$$ = $1;}
	;

op_vector_exp: op_vector_exp '+' op_vector_exp {$$ = $1 + $3;} 
	  | '<' op_scalar_exp ',' op_scalar_exp ',' op_scalar_exp '>' 
	{ /*$$ = values::Vector( $2, $4, $6 ); not supported yet*/ 
	  yyerror("vector creation from operands not supported yet!");
	  assert(0); }
	  | TOK_OP_VECTOR {$$ = $1;}
	;

op_matrix_exp: TOK_OP_MATRIX {$$ = $1;}
	;
op_string_exp: TOK_OP_STRING {$$ = $1;}
	;

%%

  Token get_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    return info.result;
  }
 
  solve::Operand<values::Scalar> &get_scalar_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    switch(info.result.get_type())
    {
    case TOK_SCALAR:
      std::cout << info.result.scalar() << std::endl;
      return solve::const_op( info.result.scalar() );	
      break;
    case TOK_OP_SCALAR:
      std::cout << info.result.op_scalar()() << std::endl;
      return info.result.op_scalar();	
      break;
    case TOK_VECTOR:
      std::cout << info.result.vector() << std::endl;
      std::cerr << "wrong type!" << std::endl; // should use info.msg.error()
      break;
    default:
      std::cerr << "fatal error: unknown expression type" << std::endl;
      assert(0);
    }
  }

  solve::Operand<values::Vector> &get_vector_expression( parser_info &info )
  {
    yyparse( static_cast<void*>(&info) );
    switch(info.result.get_type())
    {
    case TOK_SCALAR:
      std::cout << info.result.scalar() << std::endl;
      break;
    case TOK_VECTOR:
      std::cout << info.result.vector() << std::endl;
      return solve::const_op( info.result.vector() );	
      break;
    default:
      std::cerr << "fatal error: unknown expression type" << std::endl;
      assert(0);
    }
  }
}
