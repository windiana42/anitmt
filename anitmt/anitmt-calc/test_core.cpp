#include <iostream>

#include "proptree.hpp"

using namespace std;
using namespace anitmt;

int test_anitmt()
{
  try{
    int errors = 0;
    errors += property_tree_test();
    cout << "Done..." << endl;
    if( errors )
      cout << errors << " Errors occured" << endl;
    return errors;
  }
  catch( EX e ){
    cout << "Fatal Error: " << e.get_name() << endl;
    return -1;
  }
}

//*****************
// main
//*****************

int main()
{
  return test_anitmt();
}
