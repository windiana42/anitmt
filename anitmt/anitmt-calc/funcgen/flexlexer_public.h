// this file is included in the FlexLexer object of lib/flex/FlexLexer.h
afd_info *info;			/* scanner - parser informations */
Token *yylval;			/* to store the token in the parser */

void goto_initial_state();	/* forces the lexer to go to INITIAL state */
void goto_code_copy_mode();	/* switches to code copy mode */
void finish_mode();		/* switches to previous mode */
void set_input_stream( std::istream &in ); /* resets the input stream */
