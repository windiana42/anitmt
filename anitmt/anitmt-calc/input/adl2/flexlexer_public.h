// this file is included in the FlexLexer object of lib/flex/FlexLexer.h
adlparser_info *info;		/* scanner - parser informations */
Token *yylval;			/* to store the token in the parser */

void goto_initial_state();	/* forces the lexer to go to INITIAL state */
void dummy_statement_follows();	/* tells the lexer to parse a dummy statement*/
void set_input_stream( std::istream &in ); /* resets the input stream */
