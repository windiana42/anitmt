
#include <iostream>
#include "message.hpp"

#define DoNoendTest 0

#if DoNoendTest
  // This is only needed for usleep() below. 
  // Comment out if not available. 
  #include <unistd.h>
#endif

using namespace message;

class Test_Position : public Abstract_Position {
public:
  void write2stream( std::ostream &os, int detail ) const
  {
    os << "perhaps here:";
    for( int i=detail; i>0; i-- )
      os << i << ":";
    os << " ";
  }
};

enum Error_Source_Types{ Common_Test, Perticular_Part };

class test : public Message_Reporter {
public:
  void complain(){
    verbose(0) << "------------------------";
    verbose(0) << "Stream test following...";

    error ( new Test_Position() ) << "Error Message!!!";
    warn( new Test_Position() ) << "Warning Message!!!";
    verbose() << "Hey! I verbose this shit";
    for( int i=1; i<=4; i++ )
      verbose(i) << "Hey! I verbose this shit on level " << i;
    for( int i=1; i<=4; i++ )
      if( is_verbose(i) )
	verbose(i) << "Fast Hey! I verbose this shit on level " << i;

    verbose(2) << "Hey! I verbose this shit on level 2";
    verbose(3) << "Hey! I verbose this shit on level 3";
    verbose(4) << "Hey! I verbose this shit on level 4";

    verbose(0) << "...done";
    verbose(0) << "--------------------------";
    verbose(0) << "Sequence test following...";

    verbose(2)                 << " 1";
    error( GLOB::no_position ) << " 2";
    verbose(1)                 << " 3";
    warn( GLOB::no_position )  << " 4";
    error( GLOB::no_position ) << " 5";
    verbose(3)                 << " 6";
    warn( GLOB::no_position )  << " 7";
    warn( GLOB::no_position )  << " 8";
    verbose(4)                 << " 9";
    verbose(1)                 << "10";
    verbose(2)                 << "11";
    error( GLOB::no_position ) << "12";
    
    verbose(0) << "...done";
    verbose(0) << "--------------------";
    verbose(0) << "Detail level test...";
    for( int i=0; i<=5; i++ )
      error(new Test_Position(),i) << "position detail level is " << i;
    verbose(0) << "...done";
	
	verbose(0) << "testing long message...";
	std::string tmp="message";
	error(new Test_Position(),0) << "this" << " is " << 1 << " long " << tmp <<
	  " written " << 't' << 'o' << " the con" << "sole" << '.';
	verbose(0) << "...done";
   verbose(0, new File_Position("a-file-that-never-existed.txt", 73, 75))
	   << "verbose output with a file position\n";

    #if DoNoendTest
    // Last check: noend: 
    verbose() << "Doing some strange computation." << noend;
    for(int i=0; i<5; i++)
    {
      // Use something else which takes some time if you do not have usleep(). 
      usleep(100000);
      verbose() << "." << noend;
      if(i==2) error(new File_Position("here")) << "he!!";
    }
    verbose() << "done";
    #endif
  }
  test( Message_Consultant *consultant, int level ) 
    : Message_Reporter( consultant ) 
  {
    if( level >= 0 )
      consultant->set_verbose_level(level);
  }
};

int main()
{
  Stream_Message_Handler handler(cerr,cout,cout);
  Message_Manager manager( &handler );
  Message_Consultant main_consultant( &manager, Common_Test );
  for( int i=-1; i<=5; i++ )
  {
    cerr << "================================================" << endl;
    if( i== -1 )
      cout << "== Standard verbose level copied" << endl;
    else
      cout << "== Verbose level " << i << ": " << endl;
    Message_Consultant consultant = main_consultant;
    test t(&consultant,i);
    t.complain();
  }
  return 0;
}
