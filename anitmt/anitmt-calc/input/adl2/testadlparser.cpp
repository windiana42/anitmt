#include <iostream>
#include <message/message.hpp>
#include <val/val.hpp>

#include "adlparser.hpp"
#include "expression.hpp"

#include <animation.hpp>
#include <save_filled.hpp>

#include <config.h>

#if(YYDEBUG)
namespace anitmt{
  namespace adlparser{
    extern int yydebug;
  }
}
#endif

using namespace anitmt::adlparser;

int main( int argc, char *argv[] )
{
  message::Message_Source_Identifier main_msg_id(0);
  message::Stream_Message_Handler handler(cerr,cout,cout);
  message::Message_Manager manager( &handler );
  message::Message_Consultant main_consultant( &manager, main_msg_id );
  message::Message_Reporter msg(&main_consultant);

  msg.verbose() << "************** Test expressions ****************";
  message::Stream_Message_Handler test_handler(cerr,cout,cout);
  message::Message_Manager test_manager( &test_handler );
  message::Message_Consultant test_consultant( &test_manager, main_msg_id );
  message::Message_Consultant *c = &test_consultant;
  message::Message_Reporter test_msg(&test_consultant);
  
  Any_Type v4(c);
  msg.verbose() << "scalar + scalar:";
  {
    Any_Type v1(values::Scalar(1),c);
    Any_Type v2(values::Scalar(2),c);
    Any_Type v3( v1 + v2 );
    v4 = v1 + v2;
  
    msg.verbose() << "  1 + 2 = " << v3;
    msg.verbose() << "  1 + 2 = " << v4;
    if( (v3.get_scalar() != 3) || (v4.get_scalar() != 3) )
      msg.error() << "!!! Error !!!";
    else
      msg.verbose() << "ok";
  }
  msg.verbose() << "vector + vector:";
  {
    Any_Type v1(values::Vector(1,2,3),c);
    Any_Type v2(values::Vector(-1,5,2),c);
    Any_Type v3( v1 + v2 );
    v4 = v1 + v2;
  
    msg.verbose() << "  <1,2,3> + <-1,5,2> = " << v3;
    msg.verbose() << "  <1,2,3> + <-1,5,2> = " << v4;
    if( (v3.get_vector() != values::Vector(0,7,5)) || 
	(v4.get_vector() != values::Vector(0,7,5)) )
      msg.error() << "!!! Error !!!";
    else
      msg.verbose() << "ok";
  }
  msg.verbose() << "matrix + matrix:";
  {
    Any_Type v1( mat_compose_rows(values::Vector(1,2,3),
				  values::Vector(2,3,4),
				  values::Vector(5,6,7)), c );
    values::Matrix mat2 = values::Neutral1();
    Any_Type v2( mat2,c );
    Any_Type v3( v1 + v2 );
    v4 = v1 + v2;
  
    msg.verbose() << "  [ [1,2,3,0], [2,3,4,0], [5,6,7,0], [0,0,0,1] ] "
		  << message::nl
		  << "+ [ [1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1] ] " 
		  << message::nl 
		  << "= " << v3;
    msg.verbose() << "  [ [1,2,3,0], [2,3,4,0], [5,6,7,0], [0,0,0,1] ] "
		  << message::nl
		  << "+ [ [1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1] ] " 
		  << message::nl 
		  << "= " << v4;

    if( (v3.get_matrix() != mat_compose_rows(vect::Vector<4>(2,2,3,0),
					     vect::Vector<4>(2,4,4,0),
					     vect::Vector<4>(5,6,8,0),
					     vect::Vector<4>(0,0,0,2))) || 
	(v4.get_matrix() != mat_compose_rows(vect::Vector<4>(2,2,3,0),
					     vect::Vector<4>(2,4,4,0),
					     vect::Vector<4>(5,6,8,0),
					     vect::Vector<4>(0,0,0,2))) )
      msg.error() << "!!! Error !!!";
    else
      msg.verbose() << "ok";
  }
  msg.verbose() << "string + string:";
  {
    Any_Type v1(values::String("bus"),c);
    Any_Type v2(values::String("phara-o"),c);
    Any_Type v3( v1 + v2 );
    v4 = v1 + v2;
  
    msg.verbose() << "  \"bus\" + \"phara-o\" = " << v3;
    msg.verbose() << "  \"bus\" + \"phara-o\" = " << v4;
    if( (v3.get_string() != "busphara-o") || 
	(v4.get_string() != "busphara-o") )
      msg.error() << "!!! Error !!!";
    else
      msg.verbose() << "ok";
  }
  Any_Type flag(values::Flag(true),c);
  Any_Type scalar(values::Scalar(3),c);
  Any_Type vector(values::Vector(1,2,3),c);
  values::Matrix mat;
  Any_Type matrix(mat,c);
  Any_Type string(values::String("ada"),c);

  msg.verbose() << "flag + ...: this should cause 4 * 2 errors:";
  flag + scalar;
  flag + vector;
  flag + matrix;
  flag + string;
  if( test_msg.get_num_errors() != 8 )
  {
    msg.error() << "wrong number of errors in the test: " 
		<< test_msg.get_num_errors();
    msg.error() << "!!! Error !!!";
  }
  else
  {
    msg.verbose() << "ok";
    test_msg.clear_num_messages();
  }

  msg.verbose() << "scalar + ...: this should cause 4 * 2 errors:";
  scalar + flag;
  scalar + vector;
  scalar + matrix;
  scalar + string;
  if( test_msg.get_num_errors() != 8 )
  {
    msg.error() << "wrong number of errors in the test: " 
		<< test_msg.get_num_errors();
    msg.error() << "!!! Error !!!";
  }
  else
  {
    msg.verbose() << "ok";
    test_msg.clear_num_messages();
  }

  msg.verbose() << "vector + ...: this should cause 4 * 2 errors:";
  vector + flag;
  vector + scalar;
  vector + matrix;
  vector + string;
  if( test_msg.get_num_errors() != 8 )
  {
    msg.error() << "wrong number of errors in the test: " 
		<< test_msg.get_num_errors();
    msg.error() << "!!! Error !!!";
  }
  else
  {
    msg.verbose() << "ok";
    test_msg.clear_num_messages();
  }

  msg.verbose() << "matrix + ...: this should cause 4 * 2 errors:";
  matrix + flag;
  matrix + scalar;
  matrix + vector;
  matrix + string;
  if( test_msg.get_num_errors() != 8 )
  {
    msg.error() << "wrong number of errors in the test: " 
		<< test_msg.get_num_errors();
    msg.error() << "!!! Error !!!";
  }
  else
  {
    msg.verbose() << "ok";
    test_msg.clear_num_messages();
  }

  msg.verbose() << "string + ...: this should cause 4 * 2 errors:";
  string + flag;
  string + scalar;
  string + vector;
  string + matrix;
  if( test_msg.get_num_errors() != 8 )
  {
    msg.error() << "wrong number of errors in the test: " 
		<< test_msg.get_num_errors();
    msg.error() << "!!! Error !!!";
  }
  else
  {
    msg.verbose() << "ok";
    test_msg.clear_num_messages();
  }
  
  /*
  msg.verbose() << "************** Test complete parser ****************";

  std::string infile = "";
  std::string outfile1 = "test_filled_adl.out";
  std::string outfile2 = "final_test_filled_adl.out";

  if( (argc > 1) && (argv[1][0] != '-') ) 
    infile = argv[1];
  else
  {
    std::cout << "Usage:" << std::endl;
    std::cout << "  testadlparser <infile> [<outfile1> [<outfile2> [-d]]]" 
	      << std::endl;
    return 0;
  }

  if( argc > 2 )
    outfile1 = argv[2];

  if( argc > 3 )
    outfile2 = argv[3];

#if(YYDEBUG)
  if( argc > 4 )
    if( !strcmp(argv[4],"-d") )
      anitmt::adlparser::yydebug = 1; // output debugging information
#endif

  anitmt::make_all_nodes_available();

  anitmt::Animation ani("test", &manager);

  try
  {
    msg.verbose() << "parsing pass 1...";
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass1 );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    msg.verbose() << "finish hierarchy...";
    ani.hierarchy_final_init();
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    msg.verbose() << "parsing pass 2...";
    anitmt::adlparser::parse_adl( &ani, &main_consultant, infile, 
				  anitmt::adlparser::pass2 );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
    anitmt::save_filled( outfile1, &ani );
    ani.pri_sys.invoke_all_Actions();
    anitmt::save_filled( outfile2, &ani );
    if( msg.get_num_errors() > 0 )
    {
      msg.verbose() << "  Errors: " << msg.get_num_errors()
		    << "  Warnings: " << msg.get_num_warnings();
      msg.clear_num_messages();
    }
  } 
  catch( anitmt::EX e ) 
  {
    std::cerr << "Exception: " << e.get_name() << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown Exception caught" << std::endl;
  }
  */
  if( msg.get_num_errors() )
  {
    std::cerr << "***********************" << std::endl;
    std::cerr << "Hey! there were Errors!" << std::endl;
    std::cerr << "***********************" << std::endl;
  }
  else
  {
    std::cout << "*************************" << std::endl;
    std::cout << "Everything seems to be ok" << std::endl;
  }
  return 0;
}
