#include "nodes.hpp"
void test(){
  anitmt::make_all_nodes_availible();
}

#include <iostream>

#include "val.hpp"
#include "property.hpp"
#include "solver.hpp"
#include "animation.hpp"
#include "scene.hpp"
#include "scalar.hpp"
#include "move.hpp"
#include "save_filled.hpp"


using namespace std;
using namespace anitmt;

int main()
{
  //*********************
  // property/solver test 
  //*********************

  Scalar_Property *s0 = new Scalar_Property(); // start stretch
  Scalar_Property *se = new Scalar_Property(); // end stretch
  Scalar_Property *s = new Scalar_Property(); // differance stretch
  Scalar_Property *t = new Scalar_Property(); // duration
  Scalar_Property *a = new Scalar_Property(); // acceleration
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

  //*****************
  // tree create test
  //*****************

  anitmt::make_all_nodes_availible();

  Animation *ani = new Animation("dummy_name");

  Prop_Tree_Node *tscene = ani->add_child( "scene", "testscene" );

  Prop_Tree_Node *tscalar = tscene ->add_child( "scalar", "testval" );
  Prop_Tree_Node *tlinear = tscalar->add_child( "linear", "testlinear" );

  tscene->set_property( "filename", values::String("test.scene") );
  
  tlinear->set_property( "starttime",  values::Scalar(0) );
  tlinear->set_property( "endtime",    values::Scalar(3) );
  tlinear->set_property( "endvalue",   values::Scalar(2) );
  tlinear->set_property( "difference", values::Scalar(1) );

  save_filled( "test.out", ani );

  return 0;
}
