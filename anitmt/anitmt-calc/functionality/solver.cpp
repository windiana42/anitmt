// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#include <solve/constraint.hpp>
#include "solver.hpp"

namespace functionality
{
  // ****************************
  // help functions
  // ****************************

  template<class T>
  inline T extract_status( std::pair<bool,T> value, bool &status, 
			   bool &any_false )
  {
    status = value.first;
    any_false |= !value.first;
    return value.second;
  }
  // ****************************
  // provider type implementation
  // ****************************

  // *****************
  // container classes

  // *****************************
  // *****************************
  // tree node type implementation
  // *****************************
  // *****************************

  // *****************************
  // make nodes availible 
  // *****************************

  void make_solver_nodes_availible()  {
  }
}
