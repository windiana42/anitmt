/* ADL scanner/parser test program ... reads input and writes matching tokens. */
/* Copyright '01 Onno Kortmann */
/* License: GNU GPL */

#define yyFlexLexer ADLFlexLexer
#include <FlexLexer.h>
#include <nodes.hpp>
#include <save_filled.hpp>
#include "support.hpp"
#include "parser.hpp"

using namespace anitmt;

//#define SCANNER_TEST


void ScannerTest() {
	VADLFlexLexer l(&cin, &cout);
	l.set_debug(true);
	l.yylval.num=0; 
	int tok;
	while ((tok=l.yylex()) != END_OF_FILE) {
		cout << "String value: " << l.yylval.str << '\n'
		     << "Number value: " << l.yylval.num << '\n';
	}
}

void ParserTest() {
	try {
		make_all_nodes_available();
		ADLParser p(cin, cout, true);
		Animation *ani=new Animation("dummy_name");
		p.ParseTree(ani);
		cerr << "Using \"save_filled\" to save values...\n";
		save_filled("adltest.out", ani);
	}
	catch (EX &e) {
		cerr << "Error: " << e.get_name() << '\n';
	}
	catch (...) {
		cerr << "Error: UNKNOWN EXCEPTION!!\n";
	}
}

int main() {
#ifdef SCANNER_TEST
	ScannerTest();
#else
	ParserTest();
#endif
}
