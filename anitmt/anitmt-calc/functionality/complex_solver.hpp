// ********************************************
// generated file by funcgen (www.anitmt.org)
// requires:
//   - libmessage
//   - libval
//   - libsolve
//   - libproptree
// ********************************************

#ifndef __functionality_complex_solver__
#define __functionality_complex_solver__

#include <val/val.hpp>
#include <solve/operand.hpp>
#include <solve/operator.hpp>
#include <solve/solver.hpp>
#include <proptree/property.hpp>
#include <proptree/proptree.hpp>
#include <message/message.hpp>

#include <list>
#include <string>
#include <map>
#include <math.h>

#include "base_func.hpp"
#include "solver.hpp"
#include "base_func.hpp"
#include "sp_curve.hpp"
namespace functionality
{
  // **********************
  // base type declarations
  // **********************


  // **************************
  // provider type declarations
  // **************************

  // ****************
  // provider classes

  // *****************
  // container classes


  // ********************
  // ********************
  // operator declartions
  // ********************
  // ********************

}
namespace solve
{

}
namespace functionality
{

  // ********************
  // ********************
  // solver declartions
  // ********************
  // ********************

  class _sc_accel_solver;
  class _sc_bezier_solver;
  class _sc_test_solver;
  class _sc_accel_solver : public message::Message_Reporter
  {
  public:

    _sc_accel_solver( solve::Operand< scalar >&s, solve::Operand< scalar >&t, solve::Operand< scalar >&a, solve::Operand< scalar >&v0, solve::Operand< scalar >&ve,  message::Message_Consultant* );

  private:
    solve::Operand< scalar >& _op_s;
    solve::Operand< scalar >& _op_t;
    solve::Operand< scalar >& _op_a;
    solve::Operand< scalar >& _op_v0;
    solve::Operand< scalar >& _op_ve;
    solve::Operand< scalar > _op_at;
    solve::Operand< scalar > _op_v0_ve;
    solve::Operand< scalar > _op_vt;
    solve::Operand< scalar > _op_ve2;
    solve::Operand< scalar > _op_v02;
    solve::Operand< scalar > _op_prod;
    solve::Operand< scalar > _op_as;
  };

  _sc_accel_solver* accel_solver( solve::Operand< scalar >&s, solve::Operand< scalar >&t, solve::Operand< scalar >&a, solve::Operand< scalar >&v0, solve::Operand< scalar >&ve, message::Message_Consultant * );

  class _sc_bezier_solver : public message::Message_Reporter
  {
  public:
    vector get_pos( stretch s );
  solve::Operand<bool> _av_get_pos___is_avail;

    _sc_bezier_solver( solve::Operand< vector >&p1, solve::Operand< vector >&p2, solve::Operand< vector >&p3, solve::Operand< vector >&p4, solve::Operand< scalar >&length,  message::Message_Consultant* );

  private:
    _sc_test_solver *test;
    solve::Operand< vector >& _op_p1;
    solve::Operand< vector >& _op_p2;
    solve::Operand< vector >& _op_p3;
    solve::Operand< vector >& _op_p4;
    solve::Operand< scalar >& _op_length;
    bezier_curve* bezier;
  };

  _sc_bezier_solver* bezier_solver( solve::Operand< vector >&p1, solve::Operand< vector >&p2, solve::Operand< vector >&p3, solve::Operand< vector >&p4, solve::Operand< scalar >&length, message::Message_Consultant * );

  class _sc_test_solver : public message::Message_Reporter
  {
  public:
    int cooler_wert(  );
  solve::Operand<bool> _av_cooler_wert___is_avail;

    _sc_test_solver(  message::Message_Consultant* );

  private:
  };

  _sc_test_solver* test_solver( message::Message_Consultant * );

  // ***************************
  // tree node type declarations
  // ***************************

  // *****************************
  // make nodes availible 

  void make_complex_solver_nodes_availible();
}
#endif
