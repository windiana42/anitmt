#ifndef __ANITMT_CALC_PARSER_ADLPARSER_HPP
#define __ANITMT_CALC_PARSER_ADLPARSER_HPP
//-----------------------------------------------------------------------------
#include <error.hpp>
#include <val.hpp>
#include <animation.hpp>

#define yyFlexLexer ADLFlexLexer
#include <FlexLexer.h>

#include "support.hpp"

//-----------------------------------------------------------------------------
/*
  Note: Depending on the typical input structure to anitmt we should discuss
  this interface further. It's only the smallest possible subset, should work
  for today's simple files, but needs to be extended, of course.
  Ideas:
  - Finer grained parser control to
    support progress bars for gui etc. That means:
     - Status of file scan (in percent done)
     - Current line
  - Support of stream switching (e.g. support for preprocessor libraries)
*/

/*
  Notes for the parser itself:
  - support for recapturing in wrong files/collection of errors
*/
  
namespace anitmt {
	class EXParser : public EX { public: EXParser(const string msg) : EX(msg) {} };

        //! Parser for the adl format
	class ADLParser {
		/* FIXME: The internal parser routines are private stuff now, 
		   but for parser extensions it may make sense to make it protected! */
	private:
		VADLFlexLexer fl;

		//! Debugging mode flag
		bool dmode;
		//! Indention for debugging mode
		unsigned ind;

		values::Scalar ParseScalar();
		values::Vector ParseVector();
		values::String ParseString();

		void ExpectToken(const int tok);
		void ConsumeToken(const int tok);
		
		void ParseProperty(Prop_Tree_Node *pt);

		void ParseNode(Prop_Tree_Node *pt);
		//! Parse an entry, that is a node or a property in another node
		void ParseEntry(Prop_Tree_Node *pt);
	public:
		/*!
		  \param is input stream
		  \param logs parser log stream (for warnings etc.)
		  \param d debugging mode flag
		*/
		ADLParser(istream &is,
			  ostream &logs, bool d=false);
		/*! Parses a list of entries and creates a tree for the anitmt core
		  \param pt is the property tree the parser \
		  should merge it's information into */
		void ParseTree(Prop_Tree_Node *pt);
	};
}
//-----------------------------------------------------------------------------

#endif
