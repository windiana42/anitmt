#include "nodes.hpp"
void test(){
  anitmt::make_all_nodes_available();
}

#include <iostream>

#include "val.hpp"
#include "operand.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "priority.hpp"
#include "animation.hpp"
#include "scene.hpp"
#include "scalar.hpp"
#include "move.hpp"
#include "save_filled.hpp"


using namespace std;
using namespace anitmt;

int main()
{
  try{
    //*********************
    // property/solver test 
    //*********************
    {
      cout << endl;
      cout << "--------------" << endl;
      cout << "Solver Test..." << endl;
      cout << "--------------" << endl;

      Scalar_Property *s0 = new Scalar_Property(); // start stretch
      Scalar_Property *se = new Scalar_Property(); // end stretch
      Scalar_Property *s  = new Scalar_Property(); // differance stretch
      Scalar_Property *t  = new Scalar_Property(); // duration
      Scalar_Property *a  = new Scalar_Property(); // acceleration
      Scalar_Property *v0 = new Scalar_Property(); // startspeed
      Scalar_Property *ve = new Scalar_Property(); // endspeed
    
      new Accel_Solver( s, t, a, v0, ve );
      new Diff_Solver ( s, s0, se );
    
      cout << "s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 
	   << " ve=" << *ve << endl;

      cout << "startspeed 2 m/s ok?" << v0->set_if_ok(2) << endl; 
      cout << "s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 
	   << " ve=" << *ve << endl;

      cout << "acceleration 1 m/s^2 ok?" << a->set_if_ok(1) << endl;
      cout << "s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 
	   << " ve=" << *ve << endl;

      cout << "duration 2 ok?" << t->set_if_ok(2) << endl; 
      cout << "s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 
	   << " ve=" << *ve << endl;
    
    }
    //*********************
    // Action System test 
    //*********************
    {
      cout << endl;
      cout << "--------------" << endl;
      cout << "Action Test..." << endl;
      cout << "--------------" << endl;

      Scalar_Property *s0 = new Scalar_Property(); // start stretch
      Scalar_Property *se = new Scalar_Property(); // end stretch
      Scalar_Property *s  = new Scalar_Property(); // differance stretch
      Scalar_Property *t  = new Scalar_Property(); // duration
      Scalar_Property *a  = new Scalar_Property(); // acceleration
      Scalar_Property *v0 = new Scalar_Property(); // startspeed
      Scalar_Property *ve = new Scalar_Property(); // endspeed

      cout << " beginning" << endl;
      cout << "  s0="<< *s0 << " se=" << *se << " s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 << " ve=" << *ve << endl;

      new Accel_Solver( s, t, a, v0, ve );
      new Diff_Solver ( s, s0, se );

      Priority_System sys;
    
      cout << " Actions:" << endl;
      cout << "  Level  5:  a= 0.5" << endl;
      cout << "  Level  7:  t=  s0" << endl;
      cout << "  Level 10: s0=   1" << endl;
      establish_Default_Value( &sys,  5, a, values::Scalar(0.5) );
      establish_Default_Value( &sys, 10, s0, values::Scalar(1) );
      establish_Push_Connection( &sys, 7, s0, t ); // push just for fun

      cout << " unset status" << endl;
      cout << "  s0="<< *s0 << " se=" << *se << " s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 << " ve=" << *ve << endl;
      v0->set_if_ok( 0 );
      cout << " after v0=0" << endl;
      cout << "  s0="<< *s0 << " se=" << *se << " s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 << " ve=" << *ve << endl;
      sys.invoke_all_Actions();
      cout << " after result of actions" << endl;
      cout << "  s0="<< *s0 << " se=" << *se << " s=" << *s << " t=" <<  *t << " a=" << *a << " v0=" << *v0 << " ve=" << *ve << endl;
    }

    //*****************
    // tree create test
    //*****************
    {
      cout << endl;
      cout << "-----------------" << endl;
      cout << "Tree Node Test..." << endl;
      cout << "-----------------" << endl;

      cout << " Node name initialization..." << endl;
      anitmt::make_all_nodes_available();
    
      cout << " Building data hierarchy..." << endl;
      cout << "  ani dummy_name" << endl;
      Animation *ani = new Animation("dummy_name");
      cout << "    scene testscene" << endl;
      Prop_Tree_Node *tscene = ani->add_child( "scene", "testscene" );
      cout << "      scalar testval" << endl;
      Prop_Tree_Node *tscalar = tscene ->add_child( "scalar", "testval" );
      cout << "        linear testlinear1" << endl;
      Prop_Tree_Node *tlinear1 = tscalar->add_child( "linear", "testlinear1" );
      cout << "        linear testlinear2" << endl;
      Prop_Tree_Node *tlinear2 = tscalar->add_child( "linear", "testlinear2" );
    
      cout << " Setting values..." << endl;
      cout << "  scene.filename = \"test.scene\"" << endl;
      tscene->set_property( "filename", values::String("test.scene") );
    
      cout << "  testlinear1.starttime  = 0" << endl;
      tlinear1->set_property( "starttime",  values::Scalar(0) );
      cout << "  testlinear1.endtime    = 3" << endl;
      tlinear1->set_property( "endtime",    values::Scalar(3) );
      cout << "  testlinear1.endvalue   = 2" << endl;
      tlinear1->set_property( "endvalue",   values::Scalar(2) );
      cout << "  testlinear1.difference = 1" << endl;
      tlinear1->set_property( "difference", values::Scalar(1) );
      cout << "  testlinear2.endtime    = 10" << endl;
      tlinear2->set_property( "endtime",    values::Scalar(10) );
      cout << "  testlinear2.endvalue   = 1" << endl;
      tlinear2->set_property( "endvalue",   values::Scalar(1) );
    
      cout << " Save pre results..." << endl;
      save_filled( "test_pre.out", ani );

      cout << " Run actions..." << endl;
      ani->pri_sys.invoke_all_Actions(); // invoke actions

      cout << " Save final results..." << endl;
      save_filled( "test.out", ani );
    }

    //**************
    // Operand test
    //**************
    {
      cout << endl;
      cout << "------------------------" << endl;
      cout << "Operand/Operator Test..." << endl;
      cout << "------------------------" << endl;

      // calc 1 + 2 = 3
      {
	Operand<values::Scalar> &op = 
	  *(new Add_Operator<values::Scalar>
	    (*(new Constant<values::Scalar>(1)), 
	     *(new Constant<values::Scalar>(2))));

	if( op.is_solved() )
	  {
	    cout << "  1 + 2 = " << op.get_value() << " (3)" <<  endl;
	    assert( op.get_value() == 3 );
	  }
	else
	  cerr << "!!Error could not calc 1 + 2 ;)!! " << endl;
      }
      // calc (!1) + 2 = 2
      {
	Operand<values::Scalar> &op = 
	  *(new Add_Operator<values::Scalar>
	    (*(new Not_Operator<values::Scalar>
	       (*(new Constant<values::Scalar>(1)))), 
	     *(new Constant<values::Scalar>(2))));
	if( op.is_solved() )
	  {
	    cout << "  (!1) + 2 = " << op.get_value() << " (2)" << endl;
	    assert( op.get_value() == 2 );
	  } 
	else
	  cerr << "!!Error could not calc (!1) + 2 ;)!! " << endl;
      }

      // calc 2 + x (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op = 
	  *(new Add_Operator<values::Scalar>
	    (*(new Constant<values::Scalar>(2)), 
	     x));

	if( op.is_solved() )
	  cerr << "!!Error why can he solve 2 + x without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  2 + x(=5) = " << op.get_value() << "(7)" << endl;
	    assert( op.get_value() == 7 );
	  }
	else
	  cerr << "!!Error could not calc 2 + x !!" << endl;
      }


      // calc x + 2 (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op = 
	  *(new Add_Operator<values::Scalar>
	    (x,
	     *(new Constant<values::Scalar>(2))));

	if( op.is_solved() )
	  cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  x(=5) + 2 = " << op.get_value() << "(7)" << endl;
	    assert( op.get_value() == 7 );
	  }
	else
	  cerr << "!!Error could not calc x + 2 !!" << endl;
      }


      // calc (!x) + 2 (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op = 
	  *(new Add_Operator<values::Scalar>
	    (*(new Not_Operator<values::Scalar>(x)),
	     *(new Constant<values::Scalar>(2))));

	if( op.is_solved() )
	  cerr << "!!Error why can he solve (!x) + 2 without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  (!x(=5)) + 2 = " << op.get_value() << "(2)" << endl;
	    assert( op.get_value() == 2 );
	  }
	else
	  cerr << "!!Error could not calc (!x) + 2 !!" << endl;
      }


      // calc <1,2,3> + <5,4,7>     
      {
	Operand<values::Vector> &op = 
	  *(new Add_Operator<values::Vector>
	    (*(new Constant<values::Vector>( values::Vector(1,2,3) )),
	     *(new Constant<values::Vector>( values::Vector(5,4,7) ))));

	if( op.is_solved() )
	  {
	    cout << "  <1,2,3> + <5,4,7> = " << op.get_value() 
		 << " (<6,6,10>)" <<  endl;
	    assert( op.get_value() == values::Vector(6,6,10) );
	  }
	else
	  cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
      }

      // calc <1,2,3> + v
      {
	Operand<values::Vector> v; 
	Operand<values::Vector> &op = 
	  *(new Add_Operator<values::Vector>
	    (*(new Constant<values::Vector>( values::Vector(1,2,3) )),
	     v));

	if( op.is_solved() )
	  cerr << "!!Error why can he solve <1,2,3> + v without knowing v?!! " 
	       << endl;

	v.set_value( values::Vector(5,4,7) );

	if( op.is_solved() )
	  {
	    cout << "  <1,2,3> + v(=<5,4,7>) = " << op.get_value() 
		 << " (<6,6,10>)" <<  endl;
	    assert( op.get_value() == values::Vector(6,6,10) );
	  }
	else
	  cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
      }

      // operator test
      cout << " Tests with usage of C++-Operators" << endl;

      // calc 1 + 2 = 3
      {
	Operand<values::Scalar> &op = 
	  *(new Constant<values::Scalar>(1)) + 
	  *(new Constant<values::Scalar>(2));

	if( op.is_solved() )
	  {
	    cout << "  1 + 2 = " << op.get_value() << " (3)" <<  endl;
	    assert( op.get_value() == 3 );
	  }
	else
	  cerr << "!!Error could not calc 1 + 2 ;)!! " << endl;
      }
      // calc (!1) + 2 = 2
      {
	Operand<values::Scalar> &op = 
	  !*(new Constant<values::Scalar>(1)) + 
	  *(new Constant<values::Scalar>(2));

	if( op.is_solved() )
	  {
	    cout << "  (!1) + 2 = " << op.get_value() << " (2)" << endl;
	    assert( op.get_value() == 2 );
	  } 
	else
	  cerr << "!!Error could not calc (!1) + 2 ;)!! " << endl;
      }

      // calc 2 + x (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op = 2 + x;

	if( op.is_solved() )
	  cerr << "!!Error why can he solve 2 + x without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  2 + x(=5) = " << op.get_value() << "(7)" << endl;
	    assert( op.get_value() == 7 );
	  }
	else
	  cerr << "!!Error could not calc 2 + x !!" << endl;
      }


      // calc x + 2 (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op =  x + 2;

	if( op.is_solved() )
	  cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  x(=5) + 2 = " << op.get_value() << "(7)" << endl;
	    assert( op.get_value() == 7 );
	  }
	else
	  cerr << "!!Error could not calc x + 2 !!" << endl;
      }


      // calc (!x) + 2 (x = 5)    
      {
	Operand<values::Scalar> x;
	Operand<values::Scalar> &op = (!x) + 2;

	if( op.is_solved() )
	  cerr << "!!Error why can he solve (!x) + 2 without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( op.is_solved() )
	  {
	    cout << "  (!x(=5)) + 2 = " << op.get_value() << "(2)" << endl;
	    assert( op.get_value() == 2 );
	  }
	else
	  cerr << "!!Error could not calc (!x) + 2 !!" << endl;
      }


      // calc <1,2,3> + <5,4,7>     
      {
	Operand<values::Vector> &op = 
	  *(new Constant<values::Vector>( values::Vector(1,2,3) )) +
	  *(new Constant<values::Vector>( values::Vector(5,4,7) ));

	if( op.is_solved() )
	  {
	    cout << "  <1,2,3> + <5,4,7> = " << op.get_value() 
		 << " (<6,6,10>)" <<  endl;
	    assert( op.get_value() == values::Vector(6,6,10) );
	  }
	else
	  cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
      }

      // calc <1,2,3> + v
      {
	Operand<values::Vector> v; 
	Operand<values::Vector> &op = values::Vector(1,2,3) + v;

	if( op.is_solved() )
	  cerr << "!!Error why can he solve <1,2,3> + v without knowing v?!! " 
	       << endl;

	v.set_value( values::Vector(5,4,7) );

	if( op.is_solved() )
	  {
	    cout << "  <1,2,3> + v(=<5,4,7>) = " << op.get_value() 
		 << " (<6,6,10>)" <<  endl;
	    assert( op.get_value() == values::Vector(6,6,10) );
	  }
	else
	  cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
      }

      // property assignment test
      cout << " Tests Operand assignment to Property" << endl;

      // calc x + 2 (x = 5)    
      {
	Operand<values::Scalar> x;
	Type_Property<values::Scalar> prop;
	prop = x + 2;

	if( prop.is_solved() )
	  cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	       << endl;

	x.set_value( 5 );

	if( prop.is_solved() )
	  {
	    cout << "  x(=5) + 2 = " << prop << "(7)" << endl;
	    assert( prop == 7 );
	  }
	else
	  cerr << "!!Error could not calc x + 2 !!" << endl;
      }
    }
    cout << "Done..." << endl;
  }
  catch( EX e ){
    cout << "Error: " << e.get_name() << endl;
  }

  return 0;
}
