#ifndef __ANITMT_CALC_INPUT_ADL_ADLPARSER_HPP
#define __ANITMT_CALC_INPUT_ADL_ADLPARSER_HPP
//-----------------------------------------------------------------------------
#include <message/message.hpp>
#include <error.hpp>
#include <animation.hpp>

#define yyFlexLexer ADLFlexLexer
#include <FlexLexer.h>

#include "support.hpp"
#include "input/input.hpp"

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
	class ADLParser : public message::Message_Reporter {
		/* FIXME: The internal parser routines are private stuff now, 
		   but for parser extensions it may make sense to make it protected! */
	public:
		/*!
		  \param fn name of input stream (FIXME: THIS IS UGLY!)
		  \param is input stream
		  \param c the message consultant that gets all complains
		*/
		ADLParser(const std::string fn,
			  std::istream &is,
			  message::Message_Consultant *c);
		/*! Parses a list of entries and creates a tree for the anitmt core
		  \param pt is the property tree the parser \
		  should merge it's information into */
		void ParseTree(Prop_Tree_Node *pt);
	private:
		void report_error(const string msg);
		VADLFlexLexer fl;
		const string fn;
		
		//! Indention for verbose parsing mode
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
	};

  
//! Input Interface for reading ADL files
	class ADL_Input : public Input_Interface {
		std::string filename;
		Animation *ani;
		message::Message_Consultant *c;
	public:
		//! create animation tree structure
		virtual void create_structure();
		//! create explicite references 
		virtual void insert_expl_ref(); 
		//! insert concrete values for properties
		virtual void insert_values(); 

		//! Create a new ADL input interface
		/*
		  \param filename - Name of .adl-file
		  \param ani - Animation structure to be filled
		  \param c - Message consultant that get's all errors etc. */
		ADL_Input( std::string filename, Animation *ani, message::Message_Consultant *c);
	};
}
//-----------------------------------------------------------------------------
#endif
