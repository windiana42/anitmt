#include <iostream>

#include "operator.hpp"
#include "solver.hpp"
#include "priority.hpp"
#include "proptree.hpp"

using namespace std;
using namespace anitmt;

int test_anitmt()
{
  try{
    int errors = 0;
    errors += operator_test();
    errors += solver_test();
    errors += action_system_test();
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
