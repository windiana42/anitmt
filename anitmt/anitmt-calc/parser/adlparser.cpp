#include "adlparser.hpp"
//-----------------------------------------------------------------------------
using namespace values;

#define PARSER_DEBUG
#ifdef PARSER_DEBUG
#define PARSER_DS(section, msg) {					\
	if (dmode) {							\
		for (unsigned i=0; i < ind; i++) logs << ' ';		\
		logs << "PARSE_" section ": " << (msg) << '\n';		\
                ind++;							\
	}								\
}
#define PARSER_DE {ind--;}
#else
#define PARSER_DS(section, msg)
#define PARSER_DE
#endif

anitmt::ADLParser::ADLParser(istream &is,
			     ostream &logs,
			     bool d) : fl(&is, &logs), dmode(d), logs(logs) {}

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

void anitmt::ADLParser::ExpectToken(const int tok) {
	if (fl.tok() != tok) throw EXParser("Syntax error. Got '"+
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
	default: throw EXParser("Number, vector or string expected.");
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
		  throw EXParser("{ or node name expected, got '"+NameOfToken(fl.tok())+"'.");
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
	case NODE:
		ParseNode(pt); break;
	case IDENTIFIER:
		ParseProperty(pt); break;
	default:
		throw EXParser("Node keyword or property "
			       "name (identifier) expected.");
	}
	PARSER_DE;
}
			
void anitmt::ADLParser::ParseTree(Prop_Tree_Node *pt) {
	ind=0;
	fl.GetNext();
	while (fl.tok() != END_OF_FILE) ParseEntry(pt);
}
//-----------------------------------------------------------------------------
