%{
/*
 * ani-parser/grammar.yy
 * 
 * The AniVision programming language grammar and also 
 * the POV command comment grammar. 
 * This is part of the AniVision project. 
 * 
 * Copyright (c) 2003--2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <hlib/cplusplus.h>

#include <ani-parser/tree.h>
#include <pov-core/povtree.h>
#include <calccore/adtree.h>
#include "ani_parser.h"

// void yyerror (char const *msg);                 /* Yacc parsers.  */
// void yyerror (YYLTYPE *locp, char const *msg);  /* GLR parsers.   */

// replaced by %lex-param & Co
//#define YYLEX_PARAM (((Ani_Parser*)_ap)->lsb)
//#define YYPARSE_PARAM _ap   /* Ani_Parser */

#define YYERROR_VERBOSE 1
/* Special hack to get Ani_Parser in the error handler. */
#undef yyerror
#define yyerror(yylloc, _ap, mode, msg)  Ani_error(msg,_ap)   /* SPECIAL HACK */

/* Something is wrong with bison-1.875's __attribute__((__unused__)) */
/* for a label: it misses a semicolon at the end. DIRTY FIX: */
#define __attribute__(x) __attribute__(x);

#define YYLLOC_DEFAULT(Current, Rhs, N)\
	Current.pos0 = Rhs[1].pos0;  \
	Current.pos1 = Rhs[N].pos1;

// Yes, it's nice that there is an easy way to put 
// a bison parser into a namespace. 
namespace ANI
{

int yylex(YYSTYPE *lvalp,YYLTYPE *llocp,void *mode, ANI::Ani_Parser *fsc);
// static void yyerror(const char *s);
void Ani_error(const char *s,void *_ap);  /* SPECIAL HACK */

%}

%pure-parser
%defines
%name-prefix="Ani_"
%output="grammar.cc"
%locations
%error-verbose

// The following options are needed for newer newer bison versions that
// do not support #define YYLEX_PARAM & Co any more
%lex-param   {void *mode}     
%parse-param {void *mode}
%param       {ANI::Ani_Parser *_ap}

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
	const char *i_str_val;
	ANI::TreeNode *tn_none;
	ANI::TNIdentifierSimple *tn_identifier_simple;
	ANI::TNIdentifier *tn_identifier;
	ANI::TNValue *tn_val;
	ANI::TNArray *tn_array;
	ANI::TNOperatorFunction *tn_op_func;
	ANI::TNExpression *tn_expr;
	ANI::TNExpressionList *tn_expr_list;
	ANI::OperatorFuncID op_func_id;
	ANI::TNTypeSpecifier *tn_type_spec;
	ANI::TNFCallArgument *tn_fcall_arg;
	ANI::TNIterationStmt *tn_iter_stmt;
	ANI::TNSelectionStmt *tn_sel_stmt;
	ANI::TNExpressionStmt *tn_expr_stmt;
	ANI::AssignmentOpID assignment_op;
	ANI::TNAssignment *tn_assignment;
	ANI::TNStatement *tn_stmt;
	ANI::TNJumpStmt *tn_jump_stmt;
	ANI::TNDeclarationStmt *tn_decl_stmt;
	ANI::TNDeclarator *tn_declarator;
	ANI::TNFunctionDef *tn_func_def;
	ANI::TNFuncDefArgDecl *tn_fdef_arg;
	ANI::TNObject *tn_object;
	ANI::TNSetting *tn_setting;
	ANI::TNAnimation *tn_animation;
	/*-------------------------------------*/
	POV::POVTreeNode *pov_tn;
	POV::PTNExpression *pov_expr;
	POV::PTNAssignment *pov_assign;
	POV::PTNObjectEntry *pov_objent;
	/*-------------------------------------*/
	CC::ADTreeNode *ad_none;
	CC::ADTNAniDescBlock *ad_anidescblock;
	CC::ADTNTypedScope *ad_scope;
	CC::ADTNScopeEntry *ad_entry;
}

/* Special tokens specifying what to parse: */
%token TSS_ANIMATON   /* complete animation */
%token TSS_EXPR       /* expression */
%token TSS_POV_ENTRY  /* POVRay "command" entry @xxx(...) */

/* terminal... */
%token<int_val>   TS_INTEGER    "[integer]"
%token<float_val> TS_FLOAT      "[scalar]"
%token<str_val>   TS_STRING     "[string]"
%token<str_val>   TS_IDENTIFIER "[identifier]"

/*---------<GENERAL AND "ANI" MODE>---------*/

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
%token TS_STATIC  "static"

/* L = "literal"; POD-types: */
%token TSL_INT     "int"
%token TSL_SCALAR  "scalar"
%token TSL_RANGE   "range"
%token TSL_VECTOR  "vector"
%token TSL_MATRIX  "matrix"
%token TSL_STRING  "string"
%token TSL_ANYOBJ  "anyobj"

%token TS_OBJECT    "object"
%token TS_METHOD    "method"
%token TS_SETTING   "setting"
%token TS_ANIMATION "animation"

%token TS_THIS      "this"
%token TS_NULL      "NULL"

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

/*---------<POV MODE>---------*/

%token TSP_OBJECT  "@object"  /* Actually "object" but that would collide. */
%token TSP_PARAMS  "params"
%token TSP_DEFS    "defs"
%token TSP_MACRO   "macro"
%token TSP_DECLARE "declare"
%token TSP_INCLUDE "include"
%token TSP_TYPE    "type"
%token TSP_APPEND_RAW "append_raw"
%token TSP_FILEID  "@fileID"

/*-----------------------------------*/

%right TS_MAPPING
%right TS_COLONCOLON

%right TS_ASS_PLUS TS_ASS_MINUS TS_ASS_MUL TS_ASS_DIV

%nonassoc TS_DOTDOT

%right '?' ':'

%left TS_OROR
%left TS_ANDAND
%left TS_EQ TS_NEQ
%left TS_LE TS_GE '<' TS_G '>'

%left '+' '-'
%left '*' '/' '%'
%right '^'

%left '.' 

/*-----------------------------------*/

%type<i_str_val> builtin_name_raw
%type<tn_val> literal 
%type<tn_identifier> name name_notabs builtin_name
%type<tn_identifier_simple> identifier
%type<tn_op_func> vector
%type<tn_array> array array_expr_list
%type<tn_expr> primary_expr postfix_expr unary_expr binary_expr cond_expr 
%type<tn_expr> range_expr expr initializer
%type<tn_expr_list> expr_list_ne fcall_expr_list_ne fcall_expr_list
%type<op_func_id> unary_op
%type<tn_type_spec> pod_type_name type_name array_spec
%type<tn_none> new_array_spec
%type<tn_fcall_arg> fcall_expr_list_ent
%type<tn_iter_stmt> iteration_stmt
%type<tn_sel_stmt> selection_stmt
%type<tn_stmt> _for_init_stmt statement compound_stmt ani_stmt
%type<tn_expr_stmt> expr_stmt assign_stmt _for_inc_stmt
%type<tn_assignment> assignment
%type<assignment_op> assign_op
%type<tn_jump_stmt> jump_stmt
%type<tn_none> statement_list_ne statement_list declarator_list_ne
%type<tn_decl_stmt> decl_stmt decl_stmt_nonstatic
%type<tn_declarator> declarator_noinit declarator
%type<tn_none> fdef_decl_list_ne fdef_decl_list 
%type<tn_func_def> function_def_nonstatic function_def obj_function_def
%type<tn_func_def> animation_def
%type<tn_fdef_arg> fdef_decl_list_ent
%type<tn_none> object_body_ent object_body_ne object_body
%type<tn_object> object_def
%type<tn_none> setting_body_ent setting_body_ne setting_body
%type<tn_setting> setting_def
%type<tn_none> top_node top_node_list_ne top_node_list STARTEXPR
%type<tn_animation> animation
%type<int_val> function_mod
/*--------------------------------------------------------------------------------*/
%type<pov_tn> pov_exprlist pov_exprlist_ne
%type<pov_tn> pov_assignment_list pov_assignment_list_ne
%type<pov_tn> pov_object_entry_list 
%type<pov_tn> pov_entry pov_entry_list_ne pov_entry_list
%type<pov_expr> pov_expr
%type<pov_objent> pov_object_entry
%type<pov_assign> pov_assignment_list_ent
/*--------------------------------------------------------------------------------*/
%type<tn_none> typed_scope_list typed_scope_list_ne
%type<ad_anidescblock> ani_block
%type<ad_scope> typed_scope typed_scope_expr
%type<ad_entry> typed_scope_ent
/*-----------------------------------*/

%start STARTEXPR

/* Grammar */
%%

/******************* EXPRESSION *******************/

literal:
	  TS_INTEGER
	  	{
			int v=$1;
			Value vv(v);
			$$=new TNValue(@1,vv);
		}
	| TS_FLOAT
		{
			double v=$1;
			Value vv(v);
			$$=new TNValue(@1,vv);
		}
	| TS_STRING
		{
			String s($1);
			Value vv(s);
			$$=new TNValue(@1,vv);
			/* Mark as used: */
			*($1)='\0';
		}
;

identifier:
	  TS_IDENTIFIER
		{
			TNIdentifierSimple *ids=_ap->_AllocIdentifier(@1,$1);
			$$=ids;
		}
;

name_notabs:
	  identifier
	  	{  $$=new TNIdentifier($1);  }
	| identifier TS_COLONCOLON name
		{
			$3->AddChild($1);
			$$=$3;
		}
;
name:
	  name_notabs  {  $$=$1;  }
	| TS_COLONCOLON name_notabs
		{
			/* NULL ref in this case. */
			RefString nullstr;
			$2->AddChild(new TNIdentifierSimple(@2,nullstr)); /* CORRECT. */
			$$=$2;
		}
;


builtin_name_raw:
	  TS_THIS    {  $$="this";  }
;
builtin_name:
	  builtin_name_raw
		{
			TNIdentifierSimple *ids=_ap->
				_AllocIdentifier(@1,(char*)($1),/*const_char_arg=*/1);
			$$=new TNIdentifier(ids);
		}
;

vector:
	  '<' expr_list_ne '>'
	  	{
			$$=new TNOperatorFunction(OF_Vector,@1,@3);
			$$->TransferChildrenFrom($2);
			delete $2;
		}
;

/* May not be empty! (Otherwise r/r conflict with empty compound expr {}.) */
array_expr_list:
	  expr_list_ne
	  	{
			$$=new TNArray();
			$$->TransferChildrenFrom($1);
		}
	| ',' expr_list_ne
	  	{
			$$=new TNArray();
			$$->TransferChildrenFrom($2);
			$$->SetExtraComma(1,0);
		}
	|     expr_list_ne ','
	  	{
			$$=new TNArray();
			$$->TransferChildrenFrom($1);
			$$->SetExtraComma(0,1);
		}
	| ',' expr_list_ne ','
	  	{
			$$=new TNArray();
			$$->TransferChildrenFrom($2);
			$$->SetExtraComma(1,1);
		}
;
array:
	  '{' array_expr_list '}'
	  	{
			$2->SetLocation(@1,@3);
			$$=$2;
		}
	/* Explicit empty array spec: */
	/* Cannot use '{' '}' because of s/r conflict with empty compound stmt. */
	| '{' '.' '}'
		{
			$$=new TNArray();
			$$->SetLocation(@1,@3);
		}
;

primary_expr:
	  literal       {  $$=$1;  }
	| vector        {  $$=$1;  }
	| array         {  $$=$1;  }
	| name          {  $$=new TNExpression($1);  }
	| builtin_name  {  $$=new TNExpression($1);  }
	| '(' expr ')'  {  $$=$2;  }
	| TS_NULL
		{
			TNIdentifierSimple *ids=_ap->
				_AllocIdentifier(@1,"NULL",/*const_char_arg=*/1);
			$$=new TNExpression(new TNIdentifier(ids));
		}
;

postfix_expr:
	  primary_expr  {  $$=$1;  }
	| postfix_expr '[' expr ']'              // array subscript
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_Subscript,@2,@4);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr '[' error ']'
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_Subscript,@2,@4);
			opfunc->AddOperator($1);
			$$=opfunc;
		}
	| postfix_expr '(' fcall_expr_list ')'   // function call
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_FCall,@2,@4);
			opfunc->AddOperator($1);
			opfunc->TransferChildrenFrom($3);
			delete $3;
			$$=opfunc;
		}
	| postfix_expr '(' error ')'
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_FCall,@2,@4);
			opfunc->AddOperator($1);
			$$=opfunc;
		}
	| pod_type_name '(' expr ')'             // cast to POD-type
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_PODCast,@2,@4);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr '.' name
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_MembSel,@2);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr '.' builtin_name
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_MembSel,@2);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr TS_MAPPING name
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_Mapping,@2);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr TS_MAPPING builtin_name
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_Mapping,@2);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			$$=opfunc;
		}
	| postfix_expr TS_PLUSPLUS
		{  $$=new TNOperatorFunction(OF_PostIncrement,@2,$1);  }
	| postfix_expr TS_MINUSMINUS
		{  $$=new TNOperatorFunction(OF_PostDecrement,@2,$1);  }
;

unary_op:
	  '+'  {  $$=OF_UnPlus;   }
	| '-'  {  $$=OF_UnMinus;  }
	| '!'  {  $$=OF_UnNot;    }
;

	/* First, queue it temporarily in a TN_None TreeNode. */
new_array_spec:
	  type_name '[' expr ']'
	  	{
			TNLocation loc(@2.pos0,@4.pos1);
			TNNewArraySpecifier *nspec=new TNNewArraySpecifier(loc,$3);
			$$=new TreeNode(TN_None/*special*/,$1,nspec);
		}
	| new_array_spec bracket_bracket
		{
			$1->AddChild(new TNNewArraySpecifier(@2));
			$$=$1;
		}
	| new_array_spec '[' expr ']'
		{
			TNLocation loc(@2.pos0,@4.pos1);
			$1->AddChild(new TNNewArraySpecifier(loc,$3));
			$$=$1;
		}
	| new_array_spec '[' error ']'
		{
			TNLocation loc(@2.pos0,@4.pos1);
			$1->AddChild(new TNNewArraySpecifier(loc));
			$$=$1;
		}
;
unary_expr:
	  postfix_expr   {  $$=$1;  }
	| TS_PLUSPLUS unary_expr
		{  $$=new TNOperatorFunction(OF_PreIncrement,@1,$2);  }
	| TS_MINUSMINUS unary_expr
		{  $$=new TNOperatorFunction(OF_PreDecrement,@1,$2);  }
	| unary_op unary_expr
		{  $$=new TNOperatorFunction($1,@1,$2);  }
	// alloc & dealloc FIXME
	| TS_NEW name '(' fcall_expr_list ')'  // allocation (no arrays)
		{
			TNOperatorFunction *opfunc=new TNOperatorFunction(OF_New,@1);
			opfunc->AddOperator($2);
			opfunc->TransferChildrenFrom($4);
			delete $4;
			$$=opfunc;
		}
	| TS_NEW new_array_spec   // allocation (arrays)
		{
			TNOperatorFunction *opfunc=new TNOperatorFunction(OF_NewArray,@1);
			opfunc->TransferChildrenFrom($2);
			delete $2;
			$$=opfunc;
		}
	| TS_DELETE unary_expr   // deallocation
		{  $$=new TNOperatorFunction(OF_Delete,@1,$2);  }
;

binary_expr:
	  unary_expr  {  $$=$1;  }
	| binary_expr '^' binary_expr
		{  $$=new TNOperatorFunction(OF_Pow,@2,$1,$3);  }
	| binary_expr '*' binary_expr
		{  $$=new TNOperatorFunction(OF_Mult,@2,$1,$3);  }
	| binary_expr '/' binary_expr
		{  $$=new TNOperatorFunction(OF_Div,@2,$1,$3);  }
	| binary_expr '%' binary_expr
		{  $$=new TNOperatorFunction(OF_Modulo,@2,$1,$3);  }
	| binary_expr '+' binary_expr
		{  $$=new TNOperatorFunction(OF_Add,@2,$1,$3);  }
	| binary_expr '-' binary_expr
		{  $$=new TNOperatorFunction(OF_Subtract,@2,$1,$3);  }
	| binary_expr '<' binary_expr
		{  $$=new TNOperatorFunction(OF_Lesser,@2,$1,$3);  }
	| '(' binary_expr '>' binary_expr ')'   // <-- s/r conflict with vector (w/o "()")
		{  $$=new TNOperatorFunction(OF_Greater,@3,$2,$4);  }
	| binary_expr TS_G binary_expr
		{  $$=new TNOperatorFunction(OF_Greater,@2,$1,$3);  }
	| binary_expr TS_LE binary_expr
		{  $$=new TNOperatorFunction(OF_LesserEq,@2,$1,$3);  }
	| binary_expr TS_GE binary_expr
		{  $$=new TNOperatorFunction(OF_GreaterEq,@2,$1,$3);  }
	| binary_expr TS_EQ binary_expr
		{  $$=new TNOperatorFunction(OF_Equal,@2,$1,$3);  }
	| binary_expr TS_NEQ binary_expr
		{  $$=new TNOperatorFunction(OF_NotEqual,@2,$1,$3);  }
	| binary_expr TS_ANDAND binary_expr
		{  $$=new TNOperatorFunction(OF_LogicalAnd,@2,$1,$3);  }
	| binary_expr TS_OROR binary_expr
		{  $$=new TNOperatorFunction(OF_LogicalOr,@2,$1,$3);  }
;

cond_expr:
	  binary_expr  {  $$=$1;  }
	| binary_expr '?' expr ':' cond_expr
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_IfElse,@2,@4);
			opfunc->AddOperator($1);
			opfunc->AddOperator($3);
			opfunc->AddOperator($5);
			$$=opfunc;
		}
	| binary_expr '?' ':' cond_expr
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_IfElse2,@2,@3);
			opfunc->AddOperator($1);
			opfunc->AddOperator($4);
			$$=opfunc;
		}
;

range_expr:
	  cond_expr  {  $$=$1;  }
	| cond_expr TS_DOTDOT cond_expr
		{
			$$=new TNOperatorFunction(OF_Range,@2,$1,$3);
		}
	|           TS_DOTDOT cond_expr
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_RangeNoA,@1);
			opfunc->AddOperator($2);
			$$=opfunc;
		}
	| cond_expr TS_DOTDOT
		{
			TNOperatorFunction *opfunc=
				new TNOperatorFunction(OF_RangeNoB,@2);
			opfunc->AddOperator($1);
			$$=opfunc;
		}
;

expr:
	  range_expr  {  $$=$1;  assert($$->NType()==TN_Expression);  }
;


expr_list_ne:
	  expr                   {  $$=new TNExpressionList($1);  }
	| expr_list_ne ',' expr  {  $$->AddExpression($3);  }
	| error ',' expr         {  $$=new TNExpressionList($3);  }
	| expr_list_ne ',' error ',' expr  {  $$->AddExpression($5);  }
;

fcall_expr_list_ent:
	  initializer           {  $$=new TNFCallArgument($1);  }
	| name '=' initializer  {  $$=new TNFCallArgument(@2,$1,$3);  }
;
fcall_expr_list_ne:
	  fcall_expr_list_ent   {  $$=new TNExpressionList($1);  }
	| fcall_expr_list_ne ',' fcall_expr_list_ent
		{  $1->AddExpression($3);  $$=$1;  }
	| error ',' expr        {  $$=new TNExpressionList($3);  }
	| fcall_expr_list_ne ',' error ',' fcall_expr_list_ent
		{  $1->AddExpression($5);  $$=$1;  }
;
fcall_expr_list:
	  /* empty */           {  $$=new TNExpressionList();  }
	| fcall_expr_list_ne    {  $$=$1;  }
;


/******************* STATEMENT *******************/

pod_type_name:
	  TSL_INT      {  $$=new TNTypeSpecifier(@1,Value::VTInteger);  }
	| TSL_SCALAR   {  $$=new TNTypeSpecifier(@1,Value::VTScalar);  }
	| TSL_RANGE    {  $$=new TNTypeSpecifier(@1,Value::VTRange);  }
	| TSL_VECTOR   {  $$=new TNTypeSpecifier(@1,Value::VTVector);  }
	| TSL_VECTOR '<' expr '>'
		{
			$$=new TNTypeSpecifier(@1,Value::VTVector);
			$$->AddExpressionT($3);
		}
	| TSL_MATRIX   {  $$=new TNTypeSpecifier(@1,Value::VTMatrix);  }
	| TSL_MATRIX '<' expr ',' expr '>'
		{
			$$=new TNTypeSpecifier(@1,Value::VTMatrix);
			$$->AddExpressionT($3);
			$$->AddExpressionT($5);
		}
	| TSL_STRING   {  $$=new TNTypeSpecifier(@1,Value::VTString);  }
;
type_name:
	  pod_type_name  {  $$=$1;  }
	| name           {  $$=new TNTypeSpecifier(@1,$1);  }
	| TSL_ANYOBJ     {  assert(!"!implemented");  }
;

	/* Alternative array spec. Needed to  */
	/* allow functions to return arrays.  */
	/* This may seem a bit like a hack.   */
bracket_bracket:
	'[' ']' 
;
array_spec:
	  bracket_bracket
		{  $$=new TNTypeSpecifier(@1,(TNTypeSpecifier*)NULL);  }
	| array_spec bracket_bracket
		{  $$=new TNTypeSpecifier(@2,$1);  }
;

initializer:
	  expr     {  $$=$1;  }
	| '{' '}'  { TNArray *arr=new TNArray(); arr->SetLocation(@1,@2); $$=arr; }
;

declarator_noinit:
	  name_notabs   /* <-- TS_IDENTIDIER */   {  $$=new TNDeclarator($1);  }
	| declarator_noinit '[' expr ']'
		{  $$=new TNDeclarator($1,TNDeclarator::DT_Array,$3);  }
	| declarator_noinit '[' error ']'
		{  $$=new TNDeclarator($1,TNDeclarator::DT_Array,NULL);  }
	| declarator_noinit '[' ']'
		{  $$=new TNDeclarator($1,TNDeclarator::DT_Array);  }
;
declarator:
	  declarator_noinit  {  $$=$1;  }
	| declarator_noinit '=' initializer
		{  $$=new TNDeclarator($1,TNDeclarator::DT_Initialize,$3);  }
;
/* Here again: Temporarily gather the TNDeclarators in a TN_None TreeNode */
/* before transferring them to the TNDeclarationStmt. */
declarator_list_ne:
	  declarator            {  $$=new TreeNode(TN_None/*special*/,$1);  }
	| declarator_list_ne ',' declarator  {  $1->AddChild($3);  $$=$1;  }
	| error ',' declarator  {  $$=new TreeNode(TN_None/*special*/,$3);  }
	| declarator_list_ne ',' error ',' declarator
		{  $1->AddChild($5);  $$=$1;  }
;
//declarator_list:
//	  /* empty */
//	| declarator_list_ne
//;

decl_stmt_nonstatic:
	  type_name declarator_list_ne ';'
	  	{
			$$=new TNDeclarationStmt($1,0,@3,TNLocation());
			$$->TransferChildrenFrom($2);
			delete $2;
		}
	| TS_CONST type_name declarator_list_ne ';'
		{
			$$=new TNDeclarationStmt($2,TNDeclarationStmt::IS_Const,@4,@1);
			$$->TransferChildrenFrom($3);
			delete $3;
		}
;
decl_stmt:
	  decl_stmt_nonstatic
	  	{  $$=$1;  }
	| TS_STATIC decl_stmt_nonstatic
		{
			$2->attrib|=TNDeclarationStmt::IS_Static;
			$2->aloc.pos0=@1.pos0;
			$$=$2;
		}
;

compound_stmt:
	  '{' statement_list '}'
	  	{
			$$=new TNCompoundStmt(@1,@3);
			$$->TransferChildrenFrom($2);
			delete $2;
		}
	| '{' error '}'  {  $$=new TNCompoundStmt(@1,@3);  }
;

selection_stmt:
	  TS_IF '(' expr ')' statement
	  	{  $$=new TNSelectionStmt(@1,$3,$5);  }
	| TS_IF '(' error ')' statement
	  	{  $$=new TNSelectionStmt(@1,NULL,$5);  }
	| TS_IF '(' expr ')' statement TS_ELSE statement
		{  $$=new TNSelectionStmt(@1,$3,$5,@6,$7);  }
	| TS_IF '(' error ')' statement TS_ELSE statement
		{  $$=new TNSelectionStmt(@1,NULL,$5,@6,$7);  }
;

_for_init_stmt:
	  expr_stmt    {  $$=$1;  }
	| assign_stmt  {  $$=$1;  }
	| decl_stmt    {  $$=$1;  }
;
_for_inc_stmt:
	  /* empty */  {  $$=new TNExpressionStmt(TNLocation()/*, empty */);  }
	| expr         {  $$=new TNExpressionStmt(TNLocation(),$1);  }
	| assignment
		{
			TNAssignment *ass=$1;
			$$=new TNExpressionStmt(TNLocation(),ass);
		}
;
iteration_stmt:
	  TS_FOR '(' _for_init_stmt expr_stmt _for_inc_stmt ')' statement
	  	{
			TNExpression *exp=$4->ExportFirstChild();
			assert(!exp || (exp->NType()==TN_Expression && 
				exp->ExprType()!=TNE_Assignment));
			delete $4;
			$$=new TNIterationStmt(TNIterationStmt::IT_For,@1,$7,exp);
			$$->SetForLoopStmts($3,$5);
		}
	| TS_WHILE '(' expr ')' statement
	  	{  $$=new TNIterationStmt(TNIterationStmt::IT_While,@1,$5,$3);  }
	| TS_WHILE '(' error ')' statement
	  	{  $$=new TNIterationStmt(TNIterationStmt::IT_While,@1,$5,NULL);  }
	| TS_DO statement TS_WHILE '(' expr ')' ';'
	  	{  $$=new TNIterationStmt(TNIterationStmt::IT_DoWhile,@1,@3,$2,$5);  }
	| TS_DO statement TS_WHILE '(' error ')' ';'
	  	{  $$=new TNIterationStmt(TNIterationStmt::IT_DoWhile,@1,@3,$2,NULL);  }
;

jump_stmt:
	  TS_BREAK ';'
	  	{  $$=new TNJumpStmt(@1,TNJumpStmt::JT_Break,@2);  }
	| TS_RETURN expr ';'
		{  $$=new TNJumpStmt(@1,TNJumpStmt::JT_Return,@3,$2);  }
	| TS_RETURN ';'
		{  $$=new TNJumpStmt(@1,TNJumpStmt::JT_Return,@2);  }
;

assign_op: 
	  '='            {  $$=AO_Set;  	 }
	| TS_ASS_PLUS    {  $$=AO_Add;  	 }
	| TS_ASS_MINUS   {  $$=AO_Subtract;  }
	| TS_ASS_MUL     {  $$=AO_Multiply;  }
	| TS_ASS_DIV     {  $$=AO_Divide;	 }
;
assignment:
	  unary_expr assign_op initializer  /* was: expr */
	  	{  $$=new TNAssignment($2,@2,$1,$3);  }
;
assign_stmt:
	  assignment ';'
	  	{  TNAssignment *ass=$1; $$=new TNExpressionStmt(@2,ass);  }
;
expr_stmt:
	  /* empty */ ';'   {  $$=new TNExpressionStmt(@1/*, empty */);  }
	| expr ';'          {  $$=new TNExpressionStmt(@2,$1);  }
;

ani_stmt:
	  '%' ani_block ';'
		{  $$=new TNAniDescStmt(@1,@3,$2);  }
	| '%' cond_expr ani_block ';'
		{  $$=new TNAniDescStmt(@1,@4,$3,$2);  }
	| '%' name ':' ani_block ';'
		{  $$=new TNAniDescStmt(@1,@5,$4,NULL,$2);  }
	| '%' name ':' cond_expr ani_block ';'
		{  $$=new TNAniDescStmt(@1,@6,$5,$4,$2);  }
;

statement:
	  expr_stmt        {  $$=$1;  }
	| assign_stmt      {  $$=$1;  }
	| decl_stmt        {  $$=$1;  }
	| compound_stmt    {  $$=$1;  }
	| selection_stmt   {  $$=$1;  }
	| iteration_stmt   {  $$=$1;  }
	| jump_stmt        {  $$=$1;  }
	| ani_stmt         {  $$=$1;  }
	| error ';'        {  $$=new TNExpressionStmt(@2/*, empty */);  }
;

/* NOTE: Statement_list is only used inside compound stmt. */
/*       I first gather the children in a simple TreeNode, then */
/*       transfer them to the TNCompoundStmt object. */
statement_list_ne:
	  statement                    {  $$=new TreeNode(TN_None/*special*/,$1);  }
	| statement_list_ne statement  {  $1->AddChild($2);  $$=$1;  }
;
statement_list:
	  /* empty */         {  $$=new TreeNode(TN_None/*special*/);  }
	| statement_list_ne   {  $$=$1;  }
;


/******************* OBJECT *******************/

fdef_decl_list_ent:
	  type_name declarator      {  $$=new TNFuncDefArgDecl($1,$2);  }
// NOTE: This works, but I do not allow it here. 
//       People shall use one way to declare arrays 
//       ("foo(int x[])") and not be able to choose 
//       (and use "foo(int[] x)" instead). 
//       [Yes you can even use "int[]x[]" without trouble...]
//	| type_name array_spec declarator
//		{
//			/* Add array type as tree leaf. */
//			TreeNode *tn=$2;  /* = TNTypeSpecifier */
//			while(!tn->down.is_empty())  tn=tn->down.first();
//			tn->AddChild($1);
//			$$=new TNFuncDefArgDecl($2,$3);
//		}
;
/* NOTE: I first gather the func args in a simple TreeNode, then */
/*       transfer them to the TNFunctionDef object. */
fdef_decl_list_ne:
	  fdef_decl_list_ent
	  	{  $$=new TreeNode(TN_None/*special*/,$1);  }
	| fdef_decl_list_ne ',' fdef_decl_list_ent
		{  $1->AddChild($3);  $$=$1;  }
	| error ',' fdef_decl_list_ent
		{  $$=new TreeNode(TN_None/*special*/,$3);  }
	| fdef_decl_list_ne ',' error ',' fdef_decl_list_ent
		{  $1->AddChild($5);  $$=$1;  }
;
fdef_decl_list:
	  /* empty */         {  $$=new TreeNode(TN_None/*special*/);  }
	| fdef_decl_list_ne   {  $$=$1;  }
;
function_mod:
	  /* empty */   {  $$=0;  }
	| TS_CONST      {  $$=1;  }
;
function_def_nonstatic:
	  /*void*/  name_notabs '(' fdef_decl_list ')' function_mod compound_stmt
		{
			TNTypeSpecifier *rtype=new TNTypeSpecifier(@1,
				TNTypeSpecifier::VoidType);
			$$=_ap->_CreateFunctionDef($1,rtype,$6,
				0,TNLocation(),$5,@5,$3);
		}
	| type_name name_notabs '(' fdef_decl_list ')' function_mod compound_stmt
		{
			$$=_ap->_CreateFunctionDef($2,$1,$7,
				0,TNLocation(),$6,@6,$4);
		}
	| type_name array_spec name_notabs '(' fdef_decl_list ')' function_mod 
	  compound_stmt
		{
			/* Add array type as tree leaf. */
			TreeNode *tn=$2;  /* = TNTypeSpecifier */
			while(!tn->down.is_empty())  tn=tn->down.first();
			tn->AddChild($1);
			$$=_ap->_CreateFunctionDef($3,$2,$8,
				0,TNLocation(),$7,@7,$5);
		}
;
function_def:
	  function_def_nonstatic
		{  $$=$1;  }
	| TS_STATIC function_def_nonstatic
		{
			$2->attrib|=TNFunctionDef::IS_Static;
			$2->aloc0=@1;
			$$=$2;
		}
;
obj_function_def:
	  function_def            {  $$=$1;  }
	| TS_METHOD function_def  {  $$=$2;  }
	| '~' name_notabs '(' ')' compound_stmt   /* destructor */
		{
			$$=new TNFunctionDef($2,/*ret_type=*/NULL/*<-- special*/,$5);
			$$->attrib|=TNFunctionDef::IS_Destructor;
		}
;
/* Again the temporary gathering in TN_None TreeNodes... */
object_body_ent:
	  obj_function_def   {  $$=$1;  }
	| decl_stmt          {  $$=$1;  }
;
object_body_ne:
	  object_body_ent
	  	{  $$=new TreeNode(TN_None/*special*/,$1);  }
	| object_body_ne object_body_ent
		{  $1->AddChild($2);  $$=$1;  }
;
object_body:
	  /* empty */      {  $$=new TreeNode(TN_None/*special*/);  }
	| object_body_ne   {  $$=$1;  }
;
object_def:
	  TS_OBJECT name '{' object_body '}'
	  	{
			$$=new TNObject($2,@3,@5);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| TS_OBJECT name '{' error '}'
		{  $$=new TNObject($2,@3,@5);  }
;


/******************* SETTING *******************/

/* Again the temporary gathering in TN_None TreeNodes... */
setting_body_ent:
	  function_def   {  $$=$1;  }
	| decl_stmt      {  $$=$1;  }
	| object_def     {  $$=$1;  }
	// FIXME: this needs work. 
;
setting_body_ne:
	  setting_body_ent
	  	{  $$=new TreeNode(TN_None/*special*/,$1);  }
	| setting_body_ne setting_body_ent
		{  $1->AddChild($2);  $$=$1;  }
;
setting_body:
	  /* empty */       {  $$=new TreeNode(TN_None/*special*/);  }
	| setting_body_ne   {  $$=$1;  }
;
setting_def:
	  TS_SETTING name '{' setting_body '}'
	  	{
			$$=new TNSetting($2,@3,@5);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| TS_SETTING name '{' error '}'
		{  $$=new TNSetting($2,@3,@5);  }
;

/******************* ANIMATION *******************/

animation_def:
	  TS_ANIMATION name compound_stmt
		{
			$$=new TNFunctionDef($2,/*ret_type=*/NULL/*<-- special*/,$3);
			$$->attrib|=TNFunctionDef::IS_Animation;
		}
;

/******************* ANI MOVE/CHANGE... BLOCK *******************/

typed_scope:
	  identifier '{' typed_scope_list '}'
		{
			CC::ADTNTypedScope *sc=new CC::ADTNTypedScope($1,@2,@4);
			sc->TransferScopeEntries($3);  delete $3;
			$$=sc;
		}
	| identifier name '{' typed_scope_list '}'
		{
			CC::ADTNTypedScope *sc=new CC::ADTNTypedScope($1,@3,@5,$2);
			sc->TransferScopeEntries($4);  delete $4;
			$$=sc;
		}
	| identifier '{' error '}'
		{  $$=new CC::ADTNTypedScope($1,@2,@4);  }
	| identifier name '{' error '}'
		{  $$=new CC::ADTNTypedScope($1,@3,@5,$2);  }
;
typed_scope_expr:
	  typed_scope   {  $$=$1;  }
;
typed_scope_ent:
	  typed_scope_expr            {  $$=new CC::ADTNScopeEntry($1);  }
	| name '=' typed_scope_expr   {  $$=new CC::ADTNScopeEntry($3,$1,&@2);  }
	| expr ';'                    {  $$=new CC::ADTNScopeEntry($1);  }
	| name '=' expr ';'           {  $$=new CC::ADTNScopeEntry($3,$1,&@2);  }
;
/* Again the temporary gathering in TN_None TreeNodes... */
typed_scope_list_ne:
	  typed_scope_ent
		{  $$=new TreeNode(TN_None/*special*/,$1);  }
	| typed_scope_list_ne typed_scope_ent
		{  $1->AddChild($2);  $$=$1;  }
;
typed_scope_list:
	  /* empty */          {  $$=new TreeNode(TN_None/*special*/);  }
	| typed_scope_list_ne  {  $$=$1;  }
;

ani_block:
	  '{' typed_scope_list '}'
		{
			$$=new CC::ADTNAniDescBlock(@1,@3);
			$$->TransferChildrenFrom($2);
			delete $2;
		}
	| '{' error '}'
		{
			$$=new CC::ADTNAniDescBlock(@1,@3);
		}
;

/******************* START *******************/

top_node:
	  setting_def    {  $$=$1;  }
	| object_def     {  $$=$1;  }
	| function_def   {  $$=$1;  }
	| decl_stmt      {  $$=$1;  }
	| animation_def  {  $$=$1;  }
;
/* Again a TN_None temporary list. */
top_node_list_ne:
	  top_node
	  	{  $$=new TreeNode(TN_None/*special*/,$1);  }
	| top_node_list_ne top_node
		{  $1->AddChild($2);  $$=$1;  }
	| top_node_list_ne error top_node
		{  $1->AddChild($3);  $$=$1;  }
;
top_node_list:
	  /* empty */       {  $$=new TreeNode(TN_None/*special*/);  }
	| top_node_list_ne  {  $$=$1;  }
;
animation:
	  top_node_list
		{
			$$=new TNAnimation();
			$$->TransferChildrenFrom($1);
		}
;

/*----------------------------------------------------------------------------*/

/* expresion list; ALWAYS USE TransferChildrenFrom ON THAT (is a PTN_None) */
pov_exprlist:
	  /* empty */      {  $$=new POV::POVTreeNode(POV::PTN_None);  }
	| pov_exprlist_ne  {  $$=$1;  }
;
pov_exprlist_ne:
	  pov_expr
		{  $$=new POV::POVTreeNode($1);  }
	| pov_exprlist_ne ',' pov_expr
		{  $1->AddChild($3);  $$=$1;  }
	| error ',' pov_expr
		{  $$=new POV::POVTreeNode($3);  }
	| pov_exprlist_ne ',' error ',' pov_expr
		{  $1->AddChild($5);  $$=$1;  }
;

pov_expr:
	{
		_ap->lsb->SetMainStartCond(LexerScannerBase::SC_ANI);
	}
	expr
	{
		_ap->lsb->SetMainStartCond(LexerScannerBase::SC_POV);
		
		$$=new POV::PTNExpression($2);
	}
;

/* a list of "identifier = expr" entries */
/* ALWAYS USE TransferChildrenFrom ON THAT (is a PTN_None) */
pov_assignment_list:
	  /*empty */              {  $$=new POV::POVTreeNode(POV::PTN_None);  }
	| pov_assignment_list_ne  {  $$=$1;  }
;
pov_assignment_list_ne:
	  pov_assignment_list_ent
		{  $$=new POV::POVTreeNode($1);  }
	| pov_assignment_list_ne ',' pov_assignment_list_ent
		{  $1->AddChild($3);  $$=$1;  }
	| error ',' pov_assignment_list_ent
		{  $$=new POV::POVTreeNode($3);  }
	| pov_assignment_list_ne ',' error ',' pov_assignment_list_ent
		{  $1->AddChild($5);  $$=$1;  }
;
pov_assignment_list_ent:
	name '=' pov_expr   {  $$=new POV::PTNAssignment($1,$3);  }
;

/* Entry in POV @object() spec: */
pov_object_entry:
	  TSP_MACRO '=' name
		{  $$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Macro,@1,$3);  }
	| TSP_DECLARE '=' name
		{  $$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Declare,@1,$3);  }
	| TSP_PARAMS '=' '{' pov_exprlist '}'
		{
			$$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Params,@1);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| TSP_DEFS '=' '{' pov_assignment_list '}'
		{
			$$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Defs,@1);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| TSP_INCLUDE '=' '{' pov_exprlist '}'
		{
			$$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Include,@1);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| TSP_TYPE '=' pov_expr
		{  $$=new POV::PTNObjectEntry(POV::PTNObjectEntry::AS_Type,@1,$3);  }
	| TSP_APPEND_RAW '=' pov_expr
		{  $$=new POV::PTNObjectEntry(
			POV::PTNObjectEntry::AS_AppendRaw,@1,$3);  }
;

/* ALWAYS USE TransferChildrenFrom ON THAT (is a PTN_None) */
pov_object_entry_list:
	  pov_object_entry
		{  $$=new POV::POVTreeNode($1);  }
	| pov_object_entry_list ',' pov_object_entry
		{  $1->AddChild($3);  $$=$1;  }
	| error ',' pov_object_entry
		{  $$=new POV::POVTreeNode($3);  }
	| pov_object_entry_list ',' error ',' pov_object_entry
		{  $1->AddChild($5);  $$=$1;  }
;

/* POV command comment entry: */
pov_entry:
	  '@' TSP_OBJECT '(' pov_object_entry_list ')'
	  	{
			$$=new POV::PTNObjectSpec(@1);
			$$->TransferChildrenFrom($4);
			delete $4;
		}
	| '@' TSP_FILEID '=' name
		{
			$$=new POV::PTNGeneralCmd(POV::PTNGeneralCmd::PC_FileID,@2,$4);
		}
;
/* ALWAYS USE TransferChildrenFrom ON THAT (is a PTN_None) */
pov_entry_list:
	  /* empty */        {  $$=new POV::POVTreeNode(POV::PTN_None);  }
	| pov_entry_list_ne  {  $$=$1;  }
	| error              {  $$=new POV::POVTreeNode(POV::PTN_None);  }
;
pov_entry_list_ne:
	  pov_entry
		{  $$=new POV::POVTreeNode($1);  }
	| pov_entry_list_ne pov_entry
		{  $1->AddChild($2);  $$=$1;  }
	| error pov_entry
		{  $$=new POV::POVTreeNode($2);  }
	| pov_entry_list_ne error pov_entry
		{  $1->AddChild($3);  $$=$1;  }
;

/*----------------------------------------------------------------------------*/

STARTEXPR:
	  TSS_ANIMATON animation
	  	{
			Ani_Parser *ap=(Ani_Parser *)_ap;
			if(yynerrs)
			{  assert(ap->n_errors);  }
			
			assert(! ap->root );
			ap->root = $2;
			ap->_set_root=$2;
			$$ = NULL;
		}
	| TSS_EXPR expr
		{
			Ani_Parser *ap=(Ani_Parser *)_ap;
			if(yynerrs)
			{  assert(ap->n_errors);  }
			
			assert(! ap->root );
			ap->root = $2;
			ap->_set_root=$2;
			$$ = NULL;
		}
	| TSS_POV_ENTRY pov_entry_list
		{
			Ani_Parser *ap=(Ani_Parser *)_ap;
			if(yynerrs)
			{  assert(ap->n_errors);  }
			
			assert(! ap->pov_root );
			ap->pov_root = $2;
			ap->_set_root=$2;
			$$ = NULL;
		}
;

%%

int yylex(YYSTYPE *lvalp,YYLTYPE *llocp,void *mode, ANI::Ani_Parser *parser)
{
	return(parser->lsb->YYLex(lvalp,llocp));
}


/******************************************************************************/
/* NOTE: Only the code of Ani_Parser which belongs immediately to the         */
/*       grammar/parser is located below here. The rest is in ani_parser.cc.  */
/*----------------------------------------------------------------------------*/


TNFunctionDef *Ani_Parser::_CreateFunctionDef(
	TNIdentifier *_name,TNTypeSpecifier *_ret_type,TNStatement *_func_body,
	bool is_static,const TNLocation _aloc0,
	bool is_const,const TNLocation _aloc1,
	TreeNode *arglist)
{
	TNFunctionDef *fdef=new TNFunctionDef(_name,_ret_type,_func_body);
	if(is_static)
	{  fdef->attrib|=TNFunctionDef::IS_Static;  fdef->aloc0=_aloc0;  }
	if(is_const)
	{  fdef->attrib|=TNFunctionDef::IS_Const;  fdef->aloc1=_aloc1;  }
	fdef->SetFuncArguments(arglist);
	delete arglist;
	return(fdef);
}


TNIdentifierSimple *Ani_Parser::_AllocIdentifier(const YYLTYPE &pos,char *str,
	bool const_char_arg)
{
	const int use_hash=1;
	
	RefString s;
	if(use_hash)
	{
		/* See if we already know that identifier: */
		int *valptr;
		int rv=identifier_hash.LookupRefString(str,&s,&valptr,/*lru_requeue=*/1);
		if(!rv)
		{
			/* Hash hit. */
			++(*valptr);
			++hash_hits;
			hit_ents_size+=s.len()+1;
		}
		else if(rv==1)
		{
			/* Not yet known. Must store it: */
			s.set(str);
			int val=0;
			rv=identifier_hash.store(s,val,/*lru_requeue=*/1,/*dont_search=*/1);
			if(rv<0)  CheckMalloc(NULL);
			++hash_entries;
			hash_ents_size+=s.len()+1;
		}
		else assert(0);
	}
	else
	{
		s.set(str);
	}
	
	/* Mark as used: */
	if(!const_char_arg)
	{  *str='\0';  }
	
	TNIdentifierSimple *ids=new TNIdentifierSimple(pos,s);
	return(ids);
}


/*static void yyerror(const char *s)
{
	fprintf(stderr,"bison: %s\n",s);
}*/
void Ani_error(const char *s,void *_ap)  /* SPECIAL HACK */
{
	Ani_Parser *ap=(Ani_Parser*)_ap;
	
	Error(ap->lsb->GetCLoc(),"%s\n",s);
	
	++ap->n_errors;
}


int Ani_Parser::_YYParse()
{
	_set_root=NULL;
	int rv=yyparse((void*)0, this);
	
	/* Free all those nodes which were left laying around */
	/* when errors occured.                               */
	TreeNode::FreeAllHeadNodes(/*except=*/_set_root);
	_set_root=NULL;
	
	/* Don't need hash any more: */
	/* HOWEVER: Do not remove it: we may still parse some POV code 
	 * or so... the later we clear it the better. */
	/*_ClearHash()*/
	
	if(rv)  rv=2;
	else if(n_errors || lsb->NErrors())  rv=1;
	return(rv);
}

void Ani_Parser::_ClearHash()
{
	/* Print some statistics: */
	fprintf(stderr,"%s: Parser hash: %d entries, %d hits, "
		"%u bytes used, %u bytes saved\n",
		last_purpose,
		hash_entries,hash_hits,
		hash_ents_size,hit_ents_size);
	assert(hash_entries==identifier_hash.count());
	
	identifier_hash.clear();
	hash_entries=0;
	hash_hits=0;
	hash_ents_size=0;
	hit_ents_size=0;
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

%%
#endif
