/*****************************************************************************/
/**   This file offers operand/operator tree objects			    **/
/*****************************************************************************/
/**									    **/
/** Author: Martin Trautmann						    **/
/**									    **/
/** EMail:   martintrautmann@gmx.de					    **/
/**									    **/
/** License: GPL - free and without any warranty - read COPYING             **/
/**									    **/
/** Package: AniTMT							    **/
/**									    **/
/*****************************************************************************/

#include "operator.hpp"

#include <iostream>
#include <assert.h>

#include "operand.hpp"
#include "property.hpp"

using namespace std;

namespace anitmt
{
  //***************************************************************************
  // operand/operator test
  //***************************************************************************

  int operator_test()
  {
    int errors = 0;

    cout << endl;
    cout << "------------------------" << endl;
    cout << "Operand/Operator Test..." << endl;
    cout << "------------------------" << endl;
  
    // calc 1 + 2 = 3
    {
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar>
	   (*(new Constant<values::Scalar>(1)), 
	    *(new Constant<values::Scalar>(2))))).get_result();
    
      if( op.is_solved() )
      {
	cout << "  1 + 2 = " << op.get_value() << " (3)" <<  endl;
	assert( op.get_value() == 3 );
      }
      else
      {
	cerr << "!!Error could not calc 1 + 2 ;)!! " << endl;
	errors++;
      }
    }

    // calc (!1) + 2 = 2
    {
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar>
	   ((new Not_Operator<values::Scalar>
	     (*(new Constant<values::Scalar>(1))))->get_result(), 
	    *(new Constant<values::Scalar>(2))))).get_result();
      if( op.is_solved() )
      {
	cout << "  (!1) + 2 = " << op.get_value() << " (2)" << endl;
	assert( op.get_value() == 2 );
      } 
      else
      {
	cerr << "!!Error could not calc (!1) + 2 ;)!! " << endl;
	errors++;
      }
    }

    // calc 2 + x (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar>
	   (*(new Constant<values::Scalar>(2)), 
	    x))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve 2 + x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  2 + x(=5) = " << op.get_value() << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could not calc 2 + x !!" << endl;
	errors++;
      }
    }


    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar>
	   (x,
	    *(new Constant<values::Scalar>(2))))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + 2 = " << op.get_value() << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could not calc x + 2 !!" << endl;
	errors++;
      }
    }


    // calc (!x) + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar>
	   ((new Not_Operator<values::Scalar>(x))->get_result(),
	    *(new Constant<values::Scalar>(2))))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve (!x) + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  (!x(=5)) + 2 = " << op.get_value() << "(2)" << endl;
	assert( op.get_value() == 2 );
      }
      else
      {
	cerr << "!!Error could not calc (!x) + 2 !!" << endl;
	errors++;
      }
    }


    // calc <1,2,3> + <5,4,7>     
    {
      Operand<values::Vector> &op = 
	(*(new Add_Operator<values::Vector>
	   (*(new Constant<values::Vector>( values::Vector(1,2,3) )),
	    *(new Constant<values::Vector>( values::Vector(5,4,7) ))))).get_result();

      if( op.is_solved() )
      {
	cout << "  <1,2,3> + <5,4,7> = " << op.get_value() 
	     << " (<6,6,10>)" <<  endl;
	assert( op.get_value() == values::Vector(6,6,10) );
      }
      else
      {
	cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v; 
      Operand<values::Vector> &op = 
	(*(new Add_Operator<values::Vector>
	   (*(new Constant<values::Vector>( values::Vector(1,2,3) )),
	    v))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve <1,2,3> + v without knowing v?!! " 
	     << endl;
	errors++;
      }

      v.set_value( values::Vector(5,4,7) );

      if( op.is_solved() )
      {
	cout << "  <1,2,3> + v(=<5,4,7>) = " << op.get_value() 
	     << " (<6,6,10>)" <<  endl;
	assert( op.get_value() == values::Vector(6,6,10) );
      }
      else
      {
	cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
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
      {
	cerr << "!!Error could not calc 1 + 2 ;)!! " << endl;
	errors++;
      }
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
      {
	cerr << "!!Error could not calc (!1) + 2 ;)!! " << endl;
	errors++;
      }
    }

    // calc 2 + x (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 2 + x;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve 2 + x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  2 + x(=5) = " << op.get_value() << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could not calc 2 + x !!" << endl;
	errors++;
      }
    }


    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op =  x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + 2 = " << op.get_value() << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could not calc x + 2 !!" << endl;
	errors++;
      }
    }


    // calc (!x) + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = (!x) + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve (!x) + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  (!x(=5)) + 2 = " << op.get_value() << "(2)" << endl;
	assert( op.get_value() == 2 );
      }
      else
      {
	cerr << "!!Error could not calc (!x) + 2 !!" << endl;
	errors++;
      }
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
      {
	cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v; 
      Operand<values::Vector> &op = values::Vector(1,2,3) + v;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve <1,2,3> + v without knowing v?!! " 
	     << endl;
	errors++;
      }

      v.set_value( values::Vector(5,4,7) );

      if( op.is_solved() )
      {
	cout << "  <1,2,3> + v(=<5,4,7>) = " << op.get_value() 
	     << " (<6,6,10>)" <<  endl;
	assert( op.get_value() == values::Vector(6,6,10) );
      }
      else
      {
	cerr << "!!Error could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // property assignment test
    cout << " Tests Operand assignment to Operand" << endl;

    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> op;
      op = x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + 2 = " << op << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could assign x + 2 to operand!!" << endl;
	errors++;
      }
    }
    
    // calc x * <1,2,3> 
    {
      Operand<values::Scalar> x; 
      Operand<values::Vector> &op = x * values::Vector(1,2,3);

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x * <1,2,3> without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 3 );

      if( op.is_solved() )
      {
	cout << "  x(=3) * <1,2,3> = " << op.get_value() 
	     << " (<3,6,9>)" <<  endl;
	assert( op.get_value() == values::Vector(3,6,9) );
      }
      else
      {
	cerr << "!!Error could not calc x(=3) * <1,2,3>!! " << endl;
	errors++;
      }
    }

    // calc x * <0,0,0> 
    {
      Operand<values::Scalar> x; 
      Operand<values::Vector> &op = x * values::Vector(0,0,0);

      if( op.is_solved() )
      {
	cout << "  x * <0,0,0> = " << op.get_value() 
	     << " (<0,0,0>)" <<  endl;
	assert( op.get_value() == values::Vector(0,0,0) );
      }
      else
      {
	cerr << "!!Error x needed for calculating x * <0,0,0>!! " << endl;
	errors++;
      }
      
      x.set_value( 3 );

      if( op.is_solved() )
      {
	cout << "  x(=3) * <0,0,0> = " << op.get_value() 
	     << " (<0,0,0>)" <<  endl;
	assert( op.get_value() == values::Vector(0,0,0) );
      }
      else
      {
	cerr << "!!Error could not calc x(=3) * <0,0,0>!! " << endl;
	errors++;
      }
    }

    // calc x * v
    {
      Operand<values::Scalar> x; 
      Operand<values::Vector> v; 
      Operand<values::Vector> &op = x * v;
      x.set_value( 0 );

      if( op.is_solved() )
      {
	cout << "  x(=0) * v = " << op.get_value() 
	     << " (<0,0,0>)" <<  endl;
	assert( op.get_value() == values::Vector(0,0,0) );
      }
      else
      {
	cerr << "!!Error v needed for calculating x(=0) * v!! " << endl;
	errors++;
      }
      
      v.set_value( values::Vector(0,0,0) );

      if( op.is_solved() )
      {
	cout << "  x(=0) * v(=<0,0,0>) = " << op.get_value() 
	     << " (<0,0,0>)" <<  endl;
	assert( op.get_value() == values::Vector(0,0,0) );
      }
      else
      {
	cerr << "!!Error could not calc x(=0) * v(=<0,0,0>)!! " << endl;
	errors++;
      }
    }

    // calc 5 / x> 
    {
      Operand<values::Scalar> x; 
      Operand<values::Scalar> &op = 5 / x;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve 5 / x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 3 );

      if( op.is_solved() )
      {
	cout << "  5 / x(=3) = " << op.get_value() 
	     << " (1.666...)" <<  endl;
	assert( op.get_value() == 5/3 );
      }
      else
      {
	cerr << "!!Error could not calc 5 / x(=3)!! " << endl;
	errors++;
      }
    }

    // calc 5 == x> 
    {
      Operand<values::Scalar> x; 
      Operand<bool> &op = 5 == x;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve 5 == x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  5 == x(=5) = " << op.get_value() 
	     << " (true)" <<  endl;
	assert( op.get_value() == true );
      }
      else
      {
	cerr << "!!Error could not calc 5 == x(=5)!! " << endl;
	errors++;
      }
    }

    // calc x == 5> 
    {
      Operand<values::Scalar> x; 
      Operand<bool> &op = x == 5;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x == 5 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5.1 );

      if( op.is_solved() )
      {
	cout << "  x(=5.1) == 5 = " << op.get_value() 
	     << " (false)" <<  endl;
	assert( op.get_value() == false );
      }
      else
      {
	cerr << "!!Error could not calc x(=5.1) == 5!! " << endl;
	errors++;
      }
    }

    // operand assignment test
    cout << " Tests Operand assignment to Operand" << endl;

    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x;
      Operand<values::Scalar> op;
      op = x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error why can he solve x + 2 without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + 2 = " << op << "(7)" << endl;
	assert( op.get_value() == 7 );
      }
      else
      {
	cerr << "!!Error could assign x + 2 to operand!!" << endl;
	errors++;
      }
    }
    
    // property assignment test
    cout << " Tests Operand assignment to Property" << endl;

    // calc 5 + 2    
    {
      Type_Property<values::Scalar> prop;
      prop = const_op( values::Scalar(5) ) + 2;

      if( prop.is_solved() )
      {
	cout << "  5 + 2 = " << prop << "(7)" << endl;
	assert( prop() == 7 );
      }
      else
      {
	cerr << "!!Error could not assign 5 + 2 to property!!" << endl;
	errors++;
      }
    }
    return errors;
  }
}
