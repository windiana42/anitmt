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
      cout << endl << "Solver Test..." << endl;

      Scalar_Property *s0 = new Scalar_Property(); // start stretch
      Scalar_Property *se = new Scalar_Property(); // end stretch
      Scalar_Property *s  = new Scalar_Property(); // differance stretch
      Scalar_Property *t  = new Scalar_Property(); // duration
      Scalar_Property *a  = new Scalar_Property(); // acceleration
      Scalar_Property *v0 = new Scalar_Property(); // startspeed
      Scalar_Property *ve = new Scalar_Property(); // endspeed
    
      new Accel_Solver( s, t, a, v0, ve );
      new Diff_Solver ( s, s0, se );
    
      cout << *s << " " <<  *t << " " << *a << " " << *v0 << " " << *ve << endl;
      cout << "startspeed 2 m/s ok?" << v0->set_if_ok(2) << endl; 
      cout << *s << " " <<  *t << " " << *a << " " << *v0 << " " << *ve << endl;
      cout << "acceleration 1 m/s^2 ok?" << a->set_if_ok(1) << endl;
      cout << *s << " " <<  *t << " " << *a << " " << *v0 << " " << *ve << endl;
      cout << "duration 2 ok?" << t->set_if_ok(2) << endl; 
      cout << *s << " " <<  *t << " " << *a << " " << *v0 << " " << *ve << endl;
    
      cout << "endspeed solved?" << ve->is_solved() << endl;
      cout << "stretch solved?" << s->is_solved() << endl;
    
      if( ve->is_solved() )
	cout << "endspeed:" << *ve << endl;
      if( s->is_solved() )
	cout << "stretch:" << *s << endl;
    }
    //*********************
    // Action System test 
    //*********************
    {
      cout << endl << "Action Test..." << endl;

      Scalar_Property *s0 = new Scalar_Property(); // start stretch
      Scalar_Property *se = new Scalar_Property(); // end stretch
      Scalar_Property *s  = new Scalar_Property(); // differance stretch
      Scalar_Property *t  = new Scalar_Property(); // duration
      Scalar_Property *a  = new Scalar_Property(); // acceleration
      Scalar_Property *v0 = new Scalar_Property(); // startspeed
      Scalar_Property *ve = new Scalar_Property(); // endspeed

      new Accel_Solver( s, t, a, v0, ve );
      new Diff_Solver ( s, s0, se );

      Priority_System sys;
    
      establish_Default_Value( &sys,  5, a, values::Scalar(0) );
      establish_Default_Value( &sys, 10, s0, values::Scalar(0) );
      establish_Push_Connection( &sys, 7, s0, t ); // push just for fun

      cout << "s0:"<< *s0 << " se:" << *se << " s:" << *s << " t:" <<  *t << " a:" << *a << " v0:" << *v0 << " ve:" << *ve << endl;
      v0->set_if_ok( 0 );
      cout << "s0:"<< *s0 << " se:" << *se << " s:" << *s << " t:" <<  *t << " a:" << *a << " v0:" << *v0 << " ve:" << *ve << endl;
      sys.invoke_all_Actions();
      cout << "s0:"<< *s0 << " se:" << *se << " s:" << *s << " t:" <<  *t << " a:" << *a << " v0:" << *v0 << " ve:" << *ve << endl;
    }

    //*****************
    // tree create test
    //*****************
    {
      cout << endl << "Tree Node Test..." << endl;

      anitmt::make_all_nodes_available();
    
      Animation *ani = new Animation("dummy_name");
    
      Prop_Tree_Node *tscene = ani->add_child( "scene", "testscene" );
    
      Prop_Tree_Node *tscalar = tscene ->add_child( "scalar", "testval" );
      Prop_Tree_Node *tlinear1 = tscalar->add_child( "linear", "testlinear1" );
      Prop_Tree_Node *tlinear2 = tscalar->add_child( "linear", "testlinear2" );
    
      tscene->set_property( "filename", values::String("test.scene") );
    
      tlinear1->set_property( "starttime",  values::Scalar(0) );
      tlinear1->set_property( "endtime",    values::Scalar(3) );
      tlinear1->set_property( "endvalue",   values::Scalar(2) );
      tlinear1->set_property( "difference", values::Scalar(1) );
      tlinear2->set_property( "endtime",    values::Scalar(10) );
      tlinear2->set_property( "endvalue",   values::Scalar(1) );
    
      ani->pri_sys.invoke_all_Actions(); // invoke actions

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
