/* ADL scanner/parser test program ... reads input and writes matching tokens. */
/* Copyright '01 Onno Kortmann */
/* License: GNU GPL */

#include <nodes.hpp>
#include <save_filled.hpp>

#include "adlparser.hpp"
#include <message/message.hpp>

using namespace anitmt;
using namespace message;

//#define SCANNER_TEST


void ScannerTest(Message_Consultant *c) {
	VADLFlexLexer l(std::string("stdin"), &std::cin, c);
	l.set_debug(true);
	l.yylval.num=0; 
	int tok;
	while ((tok=l.yylex()) != END_OF_FILE) {
		std::cout << "String value: " << l.yylval.str << '\n'
		     << "Number value: " << l.yylval.num << '\n';
	}
}

void ParserTest(Message_Consultant *c) {
	try {
		make_all_nodes_available();
		ADLParser p(std::string("stdin"), std::cin, c);
		
		Stream_Message_Handler msg_handler(std::cerr,std::cout,std::cout);
		Message_Manager msg_manager( &msg_handler );
		Animation *ani=new Animation("dummy_name", &msg_manager);
		p.ParseTree(ani);
		ani->pri_sys.invoke_all_Actions();
		std::cerr << "Using \"save_filled\" to save values...\n";
		save_filled("adltest.out", ani);
	}
	catch (EX &e) {
		std::cerr << "Error: " << e.get_name() << '\n';
	}
	catch (...) {
		std::cerr << "Error: UNKNOWN EXCEPTION!!\n";
	}
}

int main() {
	// Copied from testmessage.cpp
	Stream_Message_Handler handler(std::cerr,std::cout,std::cout);
	Message_Manager manager(&handler);
	Message_Consultant c(&manager, 0);
#ifdef SCANNER_TEST
	ScannerTest(&c);
#else
	ParserTest(&c);
#endif
}
