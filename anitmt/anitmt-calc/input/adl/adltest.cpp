/* ADL scanner/parser test program ... reads input and writes matching tokens. */
/* Copyright '01 Onno Kortmann */
/* License: GNU GPL */

#include <nodes.hpp>
#include <save_filled.hpp>

#include "adlparser.hpp"

using namespace anitmt;
using namespace message;

//#define SCANNER_TEST


void ScannerTest(Message_Consultant *c) {
	VADLFlexLexer l(string("stdin"), &cin, c);
	l.set_debug(true);
	l.yylval.num=0; 
	int tok;
	while ((tok=l.yylex()) != END_OF_FILE) {
		cout << "String value: " << l.yylval.str << '\n'
		     << "Number value: " << l.yylval.num << '\n';
	}
}

void ParserTest(Message_Consultant *c) {
	try {
		make_all_nodes_available();
		ADLParser p(string("stdin"), cin, c);
		Animation *ani=new Animation("dummy_name");
		p.ParseTree(ani);
		ani->pri_sys.invoke_all_Actions();
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
	// Copied from testmessage.cpp
	Stream_Message_Handler handler(cerr,cout,cout);
	Message_Manager manager(&handler);
	Message_Consultant c(&manager, 0);
#ifdef SCANNER_TEST
	ScannerTest(&c);
#else
	ParserTest(&c);
#endif
}
