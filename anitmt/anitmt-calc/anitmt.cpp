#include "nodes.hpp"
void test(){
  anitmt::make_all_nodes_availible();
}

#include <iostream>

#include "val.hpp"
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
    
      cout << "endspeed solved?" << ve->s() << endl;
      cout << "stretch solved?" << s->s() << endl;
    
      if( ve->s() )
	cout << "endspeed:" << *ve << endl;
      if( s->s() )
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

      anitmt::make_all_nodes_availible();
    
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

    cout << "Done..." << endl;
  }
  catch( EX e ){
    cout << "Error: " << e.get_name() << endl;
  }
  return 0;
}
