%{
#include <stdio.h>

#include "parser.h"

#define YYLEX_PARAM (((Ani_Parser*)_fp)->als)
#define YYPARSE_PARAM _fp   /* Ani_Parser */
#define YYERROR_VERBOSE 1
/* Special hack to get Ani_Parser in the error handler. */
#undef yyerror
#define yyerror(msg)  Ani_error(msg,_fp)   /* SPECIAL HACK */

#define YYLLOC_DEFAULT(Current, Rhs, N)  Current.pos1 = Rhs[N].pos1;

// Yes, it's nice that there is an easy way to put 
// a bison parser into a namespace. 
namespace ANI
{

static int yylex(YYSTYPE *lvalp,YYLTYPE *llocp,void *fsc);
// static void yyerror(const char *s);
void Ani_error(const char *s,void *_fp);  /* SPECIAL HACK */

%}

%pure_parser
%defines
%name-prefix="Ani_"
%output="grammar.cc"
%locations

/* Expect 1 shift/reduce conflict -- it's the dangling else and */
/* (IMO) resolved correctly (shift the else). */
/* For the if(error) rule, we get the same conflict the second time. */
%expect 2


/* NOTE: All pointers in the union are allocated via operator new.     */
/*       str_val allocated via LStrNDup() and just needs to be copied; */
/*               DO NOT LFree() str_val; that's done by the lexter.    */
/* NOTE: TokenEntry::~TokenEntry() MUST KNOW THAT. */
%union {
	int int_val;              /* fom lexer */
	double float_val;         /* fom lexer */
	char *str_val;            /* fom lexer ---> TokenEntry::~TokenEntry()!!! */
	/* NOT lexer-generated elements: */
	ANI::TreeNode *tn_none;
	ANI::TNIdentifier *tn_identifier;
	ANI::TNValue *tn_val;
	ANI::TNList *tn_list;
	ANI::TNOperatorFunction *tn_op_func;
	ANI::TNExpression *tn_expr;
	ANI::TNExpressionList *tn_expr_list;
	ANI::OperatorFuncID op_func_id;
	ANI::TNTypeSpecifier *tn_type_spec;
	ANI::TNFCallArgument *tn_fcall_arg;
	ANI::TNIterationStmt *tn_iter_stmt;
	ANI::TNSelectionStmt *tn_sel_stmt;
	ANI::TNExpressionStmt *tn_expr_stmt;
	ANI::TNAssignment::AssignmentOp assignment_op;
	ANI::TNAssignment *tn_assignment;
	ANI::TNStatement *tn_stmt;
	ANI::TNJumpStmt *tn_jump_stmt;
	ANI::TNDeclarationStmt *tn_decl_stmt;
	ANI::TNDeclarator *tn_declarator;
}

/* terminal... */
%token<int_val>   TS_INTEGER    "[integer]"
%token<float_val> TS_FLOAT      "[scalar]"
%token<str_val>   TS_STRING     "[string]"
%token<str_val>   TS_IDENTIFIER "[identifier]"

/* Language tokens: */
%token TS_NEW     "new"
%token TS_DELETE  "delete"
%token TS_IF      "if"
%token TS_ELSE    "else"
%token TS_FOR     "for"
%token TS_DO      "do"
%token TS_WHILE   "while"
%token TS_BREAK   "break"
%token TS_RETURN  "return"
%token TS_CONST   "const"

/* L = "literal"; POD-types: */
%token TSL_INT     "int"
%token TSL_SCALAR  "scalar"
%token TSL_RANGE   "range"
%token TSL_VECTOR  "vector"
%token TSL_MATRIX  "matrix"
%token TSL_STRING  "string"

%token TS_OBJECT   "object"
%token TS_METHOD   "method"


/* non-terminal... */
/*%type<tnid> identifier
%type<code> unop assignop
MORE....*/

/* Special tokens specifying what to parse: */
/*%left TSS_EXPR   */  /* expression */

%token TS_PLUSPLUS "++"
%token TS_MINUSMINUS "--" 
%token TS_MAPPING "->"

%token TS_COLONCOLON "::"

%token TS_ASS_PLUS "+="
%token TS_ASS_MINUS "-="
%token TS_ASS_MUL "*="
%token TS_ASS_DIV "/="

%token TS_DOTDOT ".."

%token TS_OROR   "||"
%token TS_ANDAND "&&"
%token TS_EQ "=="
%token TS_NEQ "!="
%token TS_LE "<="
%token TS_GE ">="
%token TS_G "> " '>'

/*-----------------------------------*/

%right TS_MAPPING
%right TS_COLONCOLON

%right TS_ASS_PLUS TS_ASS_MINUS TS_ASS_MUL TS_ASS_DIV

%nonassoc TS_DOTDOT

%right '?' ':'

%left TS_OROR
%left TS_ANDAND
%left TS_EQ TS_NEQ "!="
%left TS_LE TS_GE '<' TS_G '>'

%left '+' '-'
%left '*' '/' '%'
%right '^'

%left '.' 

/*-----------------------------------*/

%start STARTEXPR

/* Grammar */
%%

/******************* EXPRESSION *******************/

literal:
	  TS_INTEGER  
	| TS_FLOAT
	| TS_STRING
;

name:
	  TS_IDENTIFIER
	| TS_IDENTIFIER TS_COLONCOLON name
;

vector:
	  '<' expr_list_ne '>'
;

array_expr_list:
	  expr_list_ne
	| ',' expr_list_ne
	|     expr_list_ne ','
	| ',' expr_list_ne ','
;
array:
	  '{' array_expr_list '}'
	/* Explicit empty array spec: */
	/* Cannot use '{' '}' because of s/r conflict with empty compound stmt. */
	| '{' '.' '}'
;

primary_expr:
	  literal
	| vector
	| array
	| TS_COLONCOLON name /*<- was: TS_IDENTIFIER*/
	| name
	| '(' expr ')'
;

postfix_expr:
	  primary_expr
	| postfix_expr '[' expr ']'              // array subscript
	| postfix_expr '[' error ']'
	| postfix_expr '(' fcall_expr_list ')'   // function call
	| postfix_expr '(' error ')'
	| pod_type_name '(' expr ')'             // cast to POD-type
	| postfix_expr '.' name
	| postfix_expr TS_MAPPING name
	| postfix_expr TS_PLUSPLUS
	| postfix_expr TS_MINUSMINUS
;

unary_op:
	  '+'
	| '-'
	| '!'
;

unary_expr:
	  postfix_expr
	| TS_PLUSPLUS unary_expr
	| TS_MINUSMINUS unary_expr
	| unary_op unary_expr
	// alloc & dealloc FIXME
	| TS_NEW name '(' fcall_expr_list ')'  // allocation (no arrays)
	| TS_DELETE unary_expr   // deallocation
;

binary_expr:
	  unary_expr
	| binary_expr '^' binary_expr
	| binary_expr '*' binary_expr
	| binary_expr '/' binary_expr
	| binary_expr '%' binary_expr
	| binary_expr '+' binary_expr
	| binary_expr '-' binary_expr
	| binary_expr '<' binary_expr
	| '(' binary_expr '>' binary_expr ')'   // <-- s/r conflict with vector (w/o "()")
	| binary_expr TS_G binary_expr
	| binary_expr TS_LE binary_expr
	| binary_expr TS_GE binary_expr
	| binary_expr TS_EQ binary_expr
	| binary_expr TS_NEQ binary_expr
	| binary_expr TS_ANDAND binary_expr
	| binary_expr TS_OROR binary_expr
;

cond_expr:
	  binary_expr
	| binary_expr '?' expr ':' cond_expr
	| binary_expr '?' ':' cond_expr
;

range_expr:
	  cond_expr
	| cond_expr TS_DOTDOT cond_expr
	|           TS_DOTDOT cond_expr
	| cond_expr TS_DOTDOT
;

expr:
	  range_expr
;


expr_list_ne:
	  expr
	| expr_list_ne ',' expr
	| error ',' expr
;

fcall_expr_list_ent:
	  initializer
	| name '=' initializer
;
fcall_expr_list_ne:
	  fcall_expr_list_ent
	| fcall_expr_list_ne ',' fcall_expr_list_ent
	| error ',' expr
;
fcall_expr_list:
	  /* empty */
	| fcall_expr_list_ne
;


/******************* STATEMENT *******************/

pod_type_name:
	  TSL_INT
	| TSL_SCALAR
	| TSL_RANGE
	| TSL_VECTOR
	| TSL_VECTOR '<' expr '>'
	| TSL_MATRIX
	| TSL_MATRIX '<' expr ',' expr '>'
	| TSL_STRING
;
type_name:
	  pod_type_name
	| name
;

initializer:
	  expr
	| '{' '}'
;

declarator_noinit:
	  name   /* <-- TS_IDENTIDIER */
	| declarator_noinit '[' expr ']'
	| declarator_noinit '[' error ']'
	| declarator_noinit '[' ']'
;
declarator:
	  declarator_noinit
	| declarator_noinit '=' initializer
;
/* Here again: Temporarily gather the TNDeclarators in a TN_None TreeNode */
/* before transferring them to the TNDeclarationStmt. */
declarator_list_ne:
	  declarator
	| declarator_list_ne ',' declarator
	| error ',' declarator
;
//declarator_list:
//	  /* empty */
//	| declarator_list_ne
//;
decl_stmt:
	  type_name declarator_list_ne ';'
	| TS_CONST type_name declarator_list_ne ';'
;


compound_stmt:
	  '{' statement_list '}'
	| '{' error '}'
;

selection_stmt:
	  TS_IF '(' expr ')' statement
	| TS_IF '(' error ')' statement
	| TS_IF '(' expr ')' statement TS_ELSE statement
	| TS_IF '(' error ')' statement TS_ELSE statement
;

_for_init_stmt:
	  expr_stmt  
	| assign_stmt
	| decl_stmt  
;
_for_inc_stmt:
	  /* empty */
	| expr       
	| assignment 
;
iteration_stmt:
	  TS_FOR '(' _for_init_stmt expr_stmt _for_inc_stmt ')' statement
	| TS_WHILE '(' expr ')' statement
	| TS_WHILE '(' error ')' statement
	| TS_DO statement TS_WHILE '(' expr ')' ';'
	| TS_DO statement TS_WHILE '(' error ')' ';'
;

jump_stmt:
	  TS_BREAK ';'
	| TS_RETURN expr ';'
;

assign_op: 
	  '='         
	| TS_ASS_PLUS 
	| TS_ASS_MINUS
	| TS_ASS_MUL  
	| TS_ASS_DIV  
;
assignment:
	  unary_expr assign_op initializer  /* was: expr */
;
assign_stmt:
	  assignment ';'
;
expr_stmt:
	  /* empty */ ';'
	| expr ';'       
;

statement:
	  expr_stmt      
	| assign_stmt    
	| decl_stmt      
	| compound_stmt  
	| selection_stmt 
	| iteration_stmt 
	| jump_stmt      
	| error ';'      
;

/* NOTE: Statement_list is only used inside compound stmt. */
/*       I first gather the children in a simple TreeNode, then */
/*       transfer them to the TNCompoundStmt object. */
statement_list_ne:
	  statement                  
	| statement_list_ne statement
;
statement_list:
	  /* empty */  
	| statement_list_ne
;


/******************* OBJECT *******************/

fdef_decl_list_ent:
	  type_name declarator
//	| type_name '&' declarator
;
fdef_decl_list_ne:
	  fdef_decl_list_ent
	| fdef_decl_list_ne ',' fdef_decl_list_ent
	| error ',' fdef_decl_list_ent
;
fdef_decl_list:
	  /* empty */
	| fdef_decl_list_ne
;
obj_function_def:
	                      name '(' fdef_decl_list ')' compound_stmt
	|           type_name name '(' fdef_decl_list ')' compound_stmt
	| TS_METHOD           name '(' fdef_decl_list ')' compound_stmt
	| TS_METHOD type_name name '(' fdef_decl_list ')' compound_stmt
	|    '~'              name '(' ')' compound_stmt   /* destructor */
;
object_body_ent:
	  obj_function_def
	| decl_stmt
;
object_body_ne:
	  object_body_ent
	| object_body_ne object_body_ent
;
object_body:
	  /* empty */
	| object_body_ne
;
object_def:
	  TS_OBJECT name '{' object_body '}'
	| TS_OBJECT name '{' error '}'
;


/*******************  *******************/

/*do_expr:
	  object_def
	| do_expr object_def
;*/
STARTEXPR:
	  //do_expr
	  object_def
;

%%

static int yylex(YYSTYPE *lvalp,YYLTYPE *llocp,void *_als)
{
	AniLexerScanner *als=(AniLexerScanner *)_als;
	return(als->YYLex(lvalp,llocp));
}


/*static void yyerror(const char *s)
{
	fprintf(stderr,"bison: %s\n",s);
}*/
void Ani_error(const char *s,void *_fp)  /* SPECIAL HACK */
{
	Ani_Parser *fp=(Ani_Parser*)_fp;
	
	#warning "Need case pos0.path!=pos1.path"
	fprintf(stderr,"bison: (lexer@%s:%d:%d..%d:%d) %s\n",
		fp->als->cloc.pos0.GetPath().str(),
		fp->als->cloc.pos0.GetLine(),fp->als->cloc.pos0.GetLPos(),
		fp->als->cloc.pos1.GetLine(),fp->als->cloc.pos1.GetLPos(),
		s);
}


int Ani_Parser::_YYParse()
{
	int rv=yyparse(this);
	
	/* Free all those nodes which were left laying around */
	/* when errors occured.                               */
	TreeNode::FreeAllHeadNodes(/*except=*/root);
	
	if(rv)  rv=2;
	else if(n_errors)  rv=1;
	return(rv);
}

}  // end of namespace ANI



#if 0
	| TS_SWITCH '(' expr case_list ')'
		{
			$$ = new TNFunction(IFM_Switch,new TreeNode($3,$4));
		}

/* 
 * SWITCH: This is thought in the following way: 
 * a = switch( expr @valA: exprA @valB: exprB @: defexpr)
 *   Evaluate expr and check if expr == valA  (valA may be a range!)
 *   If yes, set a = exprA, else check @valB: In case they match, 
 *   set a = valB, otherwise use the default and set a = defexpr. 
 * You may also use several values for one expression: 
 * a = switch( expr @valA: @valB: @valC: expr0 @: expr1 )
 *   Assign a = expr0 if expr matches valA, valB or valC; otherwise 
 *   assign expr1 to a. 
 * The rules are matched in the oder they appear. 
 * If no rule matches, the default is used. The default is implicitly 
 * always put at the end. Each switch MUST have a default. 
 * NOTE: "@:" is not allowed (i.e. no need for rule "'@' ':'" below), 
 *       because you the default MUST have a value. In case you want to 
 *       do sth like "switch(a @: @1: exprA)", (use exprA for a=1 and 
 *       as default) you can easily re-arrange that to 
 **      "switch(a @1: @: exprA)". 
 */

case_rule:
	  '@' expr ':' expr  {  $$ = new TNFunction(IFC_Case,new TreeNode($2,$4));  }
	| '@' expr ':'       {  $$ = new TNFunction(IFC_CaseNA,new TreeNode($2));  }
	| '@'      ':' expr  {  $$ = new TNFunction(IFC_CaseNC,new TreeNode($3));  }
;

case_list:
	  case_rule             {  $$ = new TreeNode($1);  }
	| case_list case_rule   {  $1->AddChild($2);  $$ = $1;  }
;

	/* Start wrapper: just start_sym and puts the tree into a Ani_Parser. */
start_wrapper: start_sym
		{
			Ani_Parser *fp=(Ani_Parser *)_fp;
			fp->n_errors=yynerrs;
			
			assert(! fp->root );
			fp->root = $1;
			$$ = NULL;
		}
;

%%
#endif
