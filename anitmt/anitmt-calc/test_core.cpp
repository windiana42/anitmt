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
    return 0;
  }
  catch( EX e ){
    cout << "Error: " << e.get_name() << endl;
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
