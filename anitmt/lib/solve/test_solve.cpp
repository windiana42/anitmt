#include <iostream>

#include "operand.hpp"
#include "operator.hpp"
#include "solver.hpp"
#include "priority.hpp"

#include "error.hpp"

using namespace std;

namespace solve {
  int test_solve()
  {
    try{
      int errors = 0;
      errors += operator_test();
      errors += solver_test();
      errors += action_system_test();
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
}

//*****************
// main
//*****************

int main()
{
  return solve::test_solve();
}
