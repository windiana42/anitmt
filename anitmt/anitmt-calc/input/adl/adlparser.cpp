#include "adlparser.hpp"

#include <fstream>
//-----------------------------------------------------------------------------
using namespace values;
using namespace message;

static const int verb_lvl=0; //FIXME ...

#define PARSER_DEBUG
#ifdef PARSER_DEBUG
#define PARSER_DS(section, msg) {					\
	verbose(verb_lvl, new File_Position(fn, fl.lineno()))		\
	        << "PARSE_" section ": " << (msg) << '\n';		\
	ind++;								\
}									


#define PARSER_DE {ind--;}
#else
#define PARSER_DS(section, msg)
#define PARSER_DE
#endif

anitmt::ADLParser::ADLParser(const std::string _fn,
	istream &is,
	message::Message_Consultant *c) :
	Message_Reporter(c), fl(_fn, &is, c), fn(_fn) {}

Scalar anitmt::ADLParser::ParseScalar() {
	ExpectToken(NUMBER);
	PARSER_DS("SCALAR", fl.yylval.num);
	Scalar ret=fl.yylval.num;
	fl.GetNext();
	PARSER_DE;
	return ret;
}

Vector anitmt::ADLParser::ParseVector() {
	ConsumeToken(OP_VECTOR);
	PARSER_DS("VECTOR", "");
	fl.GetNext();
	Scalar x, y, z;
	x=ParseScalar(); ConsumeToken(COMMA);
	y=ParseScalar(); ConsumeToken(COMMA);
	z=ParseScalar();
	ConsumeToken(CL_VECTOR);
	Vector ret=Vector(x, y, z);
	PARSER_DE;
	return ret;
}

String anitmt::ADLParser::ParseString() {
	ExpectToken(STRING);
	PARSER_DS("STRING", fl.yylval.str); 
	String ret=fl.yylval.str;
	fl.GetNext();
	PARSER_DE;
	return ret;
}

void anitmt::ADLParser::report_error(const string msg) {
		error(new File_Position(fn, fl.lineno())) << msg;
		throw EXParser(msg);
}

void anitmt::ADLParser::ExpectToken(const int tok) {
	if (fl.tok() != tok)
		report_error("Syntax error. Got '"+
			     NameOfToken(fl.tok())+
			     "' wanted '"+NameOfToken(tok)+"'");
}

void anitmt::ADLParser::ConsumeToken(const int tok) {
	PARSER_DS("CONSUMETOKEN", "");
	// Check for the token
	ExpectToken(tok);
	// Consume it.
	fl.GetNext();
	PARSER_DE;
}


void anitmt::ADLParser::ParseProperty(Prop_Tree_Node *pt) {
	PARSER_DS("PROPERTY", "name="+fl.yylval.str);
	// Save name of property
	string name=fl.yylval.str; fl.GetNext();

	// Get value and insert it:
	switch (fl.tok()) {
	case NUMBER:	pt->set_property(name, ParseScalar()); break;
	case OP_VECTOR:	pt->set_property(name, ParseVector()); break;
	case STRING:	pt->set_property(name, ParseString()); break;
	case N_A:	fl.GetNext(); break; // No real property.
	default: report_error("Number, vector or string expected.");
	}
	ConsumeToken(SEMICOLON);
	PARSER_DE;
}

void anitmt::ADLParser::ParseNode(Prop_Tree_Node *pt) {
	string type=fl.yylval.str; fl.GetNext();
        // Ok, current token must be the name of the node or "{"
	string name("dummy_name");
	switch (fl.tok()) {
	case OP_SECTION:
		break;
	case IDENTIFIER:
		name=fl.yylval.str; fl.GetNext();
		// Current token must be "{"...
		ExpectToken(OP_SECTION);
		break;
	default:
		  report_error("{ or node name expected, got '"+NameOfToken(fl.tok())+"'.");
		  break;
	}
	fl.GetNext();
	PARSER_DS("NODE", "type="+type+", name="+name);
	// Parse entries...
	Prop_Tree_Node *c=pt->add_child(type, name);
	for (;;) {
		if (fl.tok()==CL_SECTION) break;
		ParseEntry(c);
	}
	fl.GetNext();
	PARSER_DE;
}

void anitmt::ADLParser::ParseEntry(Prop_Tree_Node *pt) {
	PARSER_DS("ENTRY", "");
	/* Current token ... must be a node keyword or an identifier,
	   check for it and parse the content */
	switch (fl.tok()) {
	case IDENTIFIER:
		if (fl.yylval.str=="scalar" ||
		    fl.yylval.str=="linear" ||
		    fl.yylval.str=="scene")
			ParseNode(pt);
		else ParseProperty(pt);
		break;
	default:
		report_error("Node keyword or property "
			     "name (identifier) expected.");
	}
	PARSER_DE;
}
			
void anitmt::ADLParser::ParseTree(Prop_Tree_Node *pt) {
	ind=0;
	fl.GetNext();
	while (fl.tok() != END_OF_FILE) ParseEntry(pt);
}

//! create animation tree structure
void anitmt::ADL_Input::create_structure()
{
	ifstream in(filename.c_str());
	ADLParser p(filename, in, c);	
	p.ParseTree(ani ); // !!! shouldn't insert values yet !!!
}

//! create explicite references 
void anitmt::ADL_Input::insert_expl_ref()
{
				// !!! should read/insert explicite references 
}

//! insert concrete values for properties
void anitmt::ADL_Input::insert_values()
{
				// !!! should read/insert property values !!! 
}

anitmt::ADL_Input::ADL_Input(std::string file,
			     Animation *a,
			     message::Message_Consultant *_c) 
	:  filename(file), ani(a), c(_c) {}


//-----------------------------------------------------------------------------
