#include <iostream>

//#include <proptree/proptree.hpp>

using namespace std;
//using namespace anitmt;

int test_anitmt()
{
  int errors = 0;
  //errors += property_tree_test(); can be tested by proptree library itself
  cout << "Done..." << endl;
  if( errors )
    cout << errors << " Errors occured" << endl;
  return errors;
}

//*****************
// main
//*****************

int main()
{
  return test_anitmt();
}
