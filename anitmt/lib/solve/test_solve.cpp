#include <iostream>

#include <functionality/solver.hpp>
#include <message/message.hpp>

#include "operand.hpp"
#include "operator.hpp"
#include "solver.hpp"
#include "priority.hpp"
#include "constraint.hpp"
#include "reference.hpp"

using namespace std;

namespace solve {

  //***************************************************************************
  // operand/operator test
  //***************************************************************************

  int operator_test( message::Message_Consultant *msg )
  {
    int errors = 0;

    cout << endl;
    cout << "------------------------" << endl;
    cout << "Operand/Operator Test..." << endl;
    cout << "------------------------" << endl;

    // calc 1 + 2 = 3
    {
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (*(new Constant<values::Scalar>(1, msg)), 
	    *(new Constant<values::Scalar>(2, msg))))).get_result();
    
      if( op.is_solved() )
      {
	cout << "  1 + 2 = " << op.get_value() << " (3)" <<  endl;
	assert( op.get_value() == 3 );
      }
      else
      {
	cerr << "!!Error: could not calc 1 + 2 ;)!! " << endl;
	errors++;
      }
    }

    // calc (!1) + 2 = 2
    {
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   ((new Not_Operator<values::Scalar,values::Scalar>
	     (*(new Constant<values::Scalar>(1, msg))))->get_result(), 
	    *(new Constant<values::Scalar>(2, msg))))).get_result();
      if( op.is_solved() )
      {
	cout << "  (!1) + 2 = " << op.get_value() << " (2)" << endl;
	assert( op.get_value() == 2 );
      } 
      else
      {
	cerr << "!!Error: could not calc (!1) + 2 ;)!! " << endl;
	errors++;
      }
    }

    // calc 2 + x (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (*(new Constant<values::Scalar>(2, msg)), 
	    x))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve 2 + x without knowing x?!! " 
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
	cerr << "!!Error: could not calc 2 + x !!" << endl;
	errors++;
      }
    }


    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (x,
	    *(new Constant<values::Scalar>(2, msg))))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + 2 without knowing x?!! " 
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
	cerr << "!!Error: could not calc x + 2 !!" << endl;
	errors++;
      }
    }

    // calc (!x) + 2 (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   ((new Not_Operator<values::Scalar,values::Scalar>(x))->get_result(),
	    *(new Constant<values::Scalar>(2, msg))))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve (!x) + 2 without knowing x?!! " 
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
	cerr << "!!Error: could not calc (!x) + 2 !!" << endl;
	errors++;
      }
    }
    

    // calc <1,2,3> + <5,4,7>     
    {
      Operand<values::Vector> &op = 
	(*(new Add_Operator<values::Vector,values::Vector,values::Vector>
	   (*(new Constant<values::Vector>( values::Vector(1,2,3), msg )),
	    *(new Constant<values::Vector>( values::Vector(5,4,7), msg ))))).get_result();

      if( op.is_solved() )
      {
	cout << "  <1,2,3> + <5,4,7> = " << op.get_value() 
	     << " (<6,6,10>)" <<  endl;
	assert( op.get_value() == values::Vector(6,6,10) );
      }
      else
      {
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v(msg); 
      Operand<values::Vector> &op = 
	(*(new Add_Operator<values::Vector,values::Vector,values::Vector>
	   (*(new Constant<values::Vector>( values::Vector(1,2,3), msg )),
	    v))).get_result();

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve <1,2,3> + v without knowing v?!! " 
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
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // operator test
    cout << " Tests with usage of C++-Operators" << endl;

    // calc 1 + 2 = 3
    {
      Operand<values::Scalar> &op = 
	*(new Constant<values::Scalar>(1, msg)) + 
	*(new Constant<values::Scalar>(2, msg));

      if( op.is_solved() )
      {
	cout << "  1 + 2 = " << op.get_value() << " (3)" <<  endl;
	assert( op.get_value() == 3 );
      }
      else
      {
	cerr << "!!Error: could not calc 1 + 2 ;)!! " << endl;
	errors++;
      }
    }
    // calc !(1 + 2) = false
    {
      Operand<bool> &op = 
	!(*(new Constant<values::Scalar>(1, msg)) + 
	  *(new Constant<values::Scalar>(2, msg)));

      if( op.is_solved() )
      {
	cout << "  !(1 + 2) = " << op.get_value() << " (false)" << endl;
	assert( op.get_value() == false );
      } 
      else
      {
	cerr << "!!Error: could not calc !(1 + 2) !! " << endl;
	errors++;
      }
    }

    // calc 2 + x (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = 2 + x;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve 2 + x without knowing x?!! " 
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
	cerr << "!!Error: could not calc 2 + x !!" << endl;
	errors++;
      }
    }


    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op =  x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + 2 without knowing x?!! " 
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
	cerr << "!!Error: could not calc x + 2 !!" << endl;
	errors++;
      }
    }


    // calc !(x(=5) - 5) 
    {
      Operand<values::Scalar> x(msg);
      Operand<bool> &op = !(x - 5);

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve !(x - 5) without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  !(x(=5) - 5) = " << op.get_value() << "(true)" << endl;
	assert( op.get_value() == true );
      }
      else
      {
	cerr << "!!Error: could not calc !(x(=5) - 5) !!" << endl;
	errors++;
      }
    }

    // calc sqrt(abs(v(=<1.1,2.2,3.3>)))
    {
      Operand<values::Vector> x(msg);
      Operand<values::Scalar> &op = sqrt(abs(x));

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve sqrt(abs(v)) without knowing v?!! " 
	     << endl;
	errors++;
      }

      x.set_value( values::Vector(1.1,2.2,3.3) );

      if( op.is_solved() )
      {
	cout << "  sqrt(abs(v(=<1.1,2.2,3.3>))) = " << op.get_value() 
	     << "(2,02875)"
	     << endl;
	assert( op.get_value() == ::sqrt(::sqrt(1.1*1.1+2.2*2.2+3.3*3.3)) );
      }
      else
      {
	cerr << "!!Error: could not calc abs(v(=<1.1,2.2,3.3>)) !!" << endl;
	errors++;
      }
    }

    // calc sqrt(x(=-4)) 
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = sqrt(x);

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve sqrt(x) without knowing x?!! " 
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value( -4 ) )
	{
	  cerr << "!!Error: why does he accept sqrt(x(=-4))?!! " ;
	  if( op.is_solved() )
	    cerr << "Result: " << op.get_value();
	  cerr << endl;
	  errors++;
	}
	else
	{
	  cout << "  sqrt(x(=-4)) rejected. OK" << endl;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve sqrt(x) with rejected x?!! " ;
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cout << "  sqrt(x(=-4)) rejected with exception. OK" << endl;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve sqrt(x) with rejected x?!! " ;
	  errors++;
	}
      }
    }

    // calc sqrt(abs(x(=-123.45))) 
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = sqrt(abs(x));

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve sqrt(abs(x)) without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( -123.45 );

      if( op.is_solved() )
      {
	cout << "  sqrt(abs(x(=123.45))) = " << op.get_value() << "(11,1108)"
	     << endl;
	assert( op.get_value() == ::sqrt(123.45) );
      }
      else
      {
	cerr << "!!Error: could not calc sqrt(abs(x(=-123.45))) !!" << endl;
	errors++;
      }
    }

    // calc <1,2,3> + <5,4,7>     
    {
      Operand<values::Vector> &op = 
	*(new Constant<values::Vector>( values::Vector(1,2,3), msg )) +
	*(new Constant<values::Vector>( values::Vector(5,4,7), msg ));

      if( op.is_solved() )
      {
	cout << "  <1,2,3> + <5,4,7> = " << op.get_value() 
	     << " (<6,6,10>)" <<  endl;
	assert( op.get_value() == values::Vector(6,6,10) );
      }
      else
      {
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v(msg); 
      Operand<values::Vector> &op = values::Vector(1,2,3) + v;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve <1,2,3> + v without knowing v?!! " 
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
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> op(msg);
      op = x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + 2 without knowing x?!! " 
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
	cerr << "!!Error: could assign x + 2 to operand!!" << endl;
	errors++;
      }
    }
    
    // calc x * <1,2,3> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<values::Vector> &op = x * values::Vector(1,2,3);

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x * <1,2,3> without knowing x?!! " 
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
	cerr << "!!Error: could not calc x(=3) * <1,2,3>!! " << endl;
	errors++;
      }
    }

    // calc x * <0,0,0> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<values::Vector> &op = x * values::Vector(0,0,0);

      if( op.is_solved() )
      {
	cout << "  x * <0,0,0> = " << op.get_value() 
	     << " (<0,0,0>)" <<  endl;
	assert( op.get_value() == values::Vector(0,0,0) );
      }
      else
      {
	cerr << "!!Error: x needed for calculating x * <0,0,0>!! " << endl;
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
	cerr << "!!Error: could not calc x(=3) * <0,0,0>!! " << endl;
	errors++;
      }
    }

    // calc x * v
    {
      Operand<values::Scalar> x(msg); 
      Operand<values::Vector> v(msg); 
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
	cerr << "!!Error: v needed for calculating x(=0) * v!! " << endl;
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
	cerr << "!!Error: could not calc x(=0) * v(=<0,0,0>)!! " << endl;
	errors++;
      }
    }

    // calc 5 / x> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<values::Scalar> &op = 5 / x;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve 5 / x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 3 );

      if( op.is_solved() )
      {
	cout << "  5 / x(=3) = " << op.get_value() 
	     << " (1.66667)" <<  endl;
	assert( op.get_value() == 5./3. );
      }
      else
      {
	cerr << "!!Error: could not calc 5 / x(=3)!! " << endl;
	errors++;
      }
    }

    // calc 5 == x> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<bool> &op = (5 == x);

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve 5 == x without knowing x?!! " 
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
	cerr << "!!Error: could not calc 5 == x(=5)!! " << endl;
	errors++;
      }
    }

    // calc x(=5.1) == 5> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<bool> &op = x == 5;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x == 5 without knowing x?!! " 
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
	cerr << "!!Error: could not calc x(=5.1) == 5!! " << endl;
	errors++;
      }
    }

    // calc 5 != x(=0)> 
    {
      Operand<values::Scalar> x(msg); 
      Operand<bool> &op = 5 != x;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve 5 != x without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 0 );

      if( op.is_solved() )
      {
	cout << "  5 != x(=0) = " << op.get_value() 
	     << " (true)" <<  endl;
	assert( op.get_value() == true );
      }
      else
      {
	cerr << "!!Error: could not calc 5 != x(=0)!! " << endl;
	errors++;
      }
    }

    // calc constraint( x(=1) == 2 )
    {
      Operand<values::Scalar> x(msg);
      constraint( x == 2 );

      try
      {
        if( x.set_value( 1 ) )
	{
	  cerr << "!!Error: why does he accept constraint( x(=1) == 2 )?!!"
	       << endl;
	  errors++;
	}
	else
	{
	  cout << "  constraint( x(=1) == 2) rejected. OK" << endl;
	}
      }
      catch( ... )
      {
	cout << "  constraint( x(=1) == 2) rejected. OK" << endl;
      }
    }

    // calc +-sqrt( x(=25) ) 
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = plus_minus( sqrt(x) );

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve +-sqrt(x(=25)) without knowing x?!!"
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value(25) )
	{
	  if( op.is_solved() )
	  {
	    cout << "  +-sqrt(x(=25)) = " << op.get_value() 
		 << " (5)" <<  endl;
	    assert( op.get_value() == 5 );
	  }
	  else
	  {
	    cerr << "!!Error: could not calc +-sqrt(x(=25))!!" << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "!!Error: why was +-sqrt(x(=25)) rejected?!! " ;
	  errors++;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve +-sqrt(x) with rejected x?!!";
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cerr << "!!Error: why was +-sqrt(x(=25)) rejected with "
	  "exception?!!";
	errors++;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve +-sqrt(x) with rejected x?!!";
	  errors++;
	}
      }
    }

    // calc +-sqrt( x(=25) ) mit constraint( result < 0 )
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = plus_minus( sqrt(x) );
      constraint( op < 0 );

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve +-sqrt(x(=25))(<0) without knowing "
	  "x?!!"
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value(25) )
	{
	  if( op.is_solved() )
	  {
	    cout << "  +-sqrt(x(=25)) (<0) = " << op.get_value() 
		 << " (-5)" <<  endl;
	    assert( op.get_value() == -5 );
	  }
	  else
	  {
	    cerr << "!!Error: could not calc +-sqrt(x(=25)) (<0)!!" << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "!!Error: why was +-sqrt(x(=25))(<0) rejected?!! " ;
	  errors++;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve +-sqrt(x) (<0) with rejected "
	      "x?!!";
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cerr << "!!Error: why was +-sqrt(x(=25))(<0) rejected with "
	  "exception?!!";
	errors++;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve +-sqrt(x)(<0) with rejected x?!!";
	  errors++;
	}
      }
    }

    // calc +-sqrt( x(=25) ) mit constraint( result > 0 )
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = plus_minus( sqrt(x) );
      constraint( op > 0 );

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve +-sqrt(x(=25))(>0) without knowing "
	  "x?!!"
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value(25) )
	{
	  if( op.is_solved() )
	  {
	    cout << "  +-sqrt(x(=25)) (>0) = " << op.get_value() 
		 << " (5)" <<  endl;
	    assert( op.get_value() == 5 );
	  }
	  else
	  {
	    cerr << "!!Error: could not calc +-sqrt(x(=25)) (>0)!!" << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "!!Error: why was +-sqrt(x(=25)) (>0) rejected?!! " ;
	  errors++;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve +-sqrt(x) (>0) with rejected "
	      "x?!!";
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cerr << "!!Error: why was +-sqrt(x(=25)) (>0) rejected with "
	  "exception?!!";
	errors++;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve +-sqrt(x)(>0) with rejected x?!!";
	  errors++;
	}
      }
    }

    // calc +-( 5 +- x(=2) ) mit constraint( result <= 3 )
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &op = plus_minus( 5 + plus_minus(x) );
      constraint( op <= 3 );

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3) without "
	  "knowing x?!!"
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value(2) )
	{
	  if( op.is_solved() )
	  {
	    cout << "  +-(5 +-x(=2))(<=3) = " << op.get_value() 
		 << " (-7)" <<  endl;
	    assert( op.get_value() == -7 );
	  }
	  else
	  {
	    cerr << "!!Error: could not calc +-(5 +-x(=2))(<=3)!!" << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "!!Error: why was +-(5 +-x(=2))(<=3) rejected?!! " ;
	  errors++;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3) with "
	      "rejected x?!!";
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cerr << "!!Error: why was +-(5 +-x(=2))(<=3) rejected with "
	  "exception?!!";
	errors++;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3) with rejected "
	    "x?!!";
	  errors++;
	}
      }
    }

    // calc +-( 5 +- x(=2) ) mit ( result <= 3), (+-x<0)
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> &pmx = plus_minus(x);
      Operand<values::Scalar> &op = plus_minus( 5 + pmx );
      constraint( op <= 3 );
      constraint( pmx < 0 );

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3),(+-x<0) without"
	  " knowing x?!!"
	     << endl;
	errors++;
      }

      try
      {
        if( x.set_value(2) )
	{
	  if( op.is_solved() )
	  {
	    cout << "  +-(5 +-x(=2))(<=3),(+-x<0) = " << op.get_value() 
		 << " (3)" <<  endl;
	    assert( op.get_value() == 3 );
	  }
	  else
	  {
	    cerr << "!!Error: could not calc +-(5 +-x(=2))(<=3),(+-x<0)!!" 
		 << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "!!Error: why was +-(5 +-x(=2))(<=3),(+-x<0) rejected?!! " ;
	  errors++;
	  if( op.is_solved() )
	  {
	    cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3),(+-x<0) with"
	      " rejected x?!!";
	    errors++;
	  }
	}
      }
      catch( ... )
      {
	cerr << "!!Error: why was +-(5 +-x(=2))(<=3),(+-x<0) rejected with "
	  "exception?!!";
	errors++;
	if( op.is_solved() )
	{
	  cerr << "!!Error: why can he solve +-(5 +-x(=2))(<=3),(+-x<0) with"
	    " rejected x?!!";
	  errors++;
	}
      }
    }

    // operand assignment test
    cout << " Tests Operand assignment to Operand" << endl;

    // calc x + 2 (x = 5)    
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> op(msg);
      op = x + 2;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + 2 without knowing x?!! " 
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
	cerr << "!!Error: could assign x + 2 to operand!!" << endl;
	errors++;
      }
    }

    // calc x + y (x=5) (y=2*x)
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> y(msg);
      Operand<values::Scalar> op(msg);
      y = 2*x;
      op = x + y;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + y(=2*x) without knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + y(=2*x) = " << op << "(15)" << endl;
	assert( op.get_value() == 15 );
      }
      else
      {
	cerr << "!!Error: could assign x(=5) + y(=2*x) to operand!!" << endl;
	errors++;
      }
    }

    // calc x + y (x=5) (y=2*x) (already initialized y)
    {
      Operand<values::Scalar> x(msg);
      Operand<values::Scalar> y = 2*x;
      Operand<values::Scalar> op(msg);
      op = x + y;

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve x + y(=2*x)(init) without knowing"
	  " x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( 5 );

      if( op.is_solved() )
      {
	cout << "  x(=5) + y(=2*x)(init) = " << op << "(15)" << endl;
	assert( op.get_value() == 15 );
      }
      else
      {
	cerr << "!!Error: could note assign x(=5) + y(=2*x)(init) to operand!!"
	     << endl;
	errors++;
      }
    }
    
    return errors;
  }

  //***************************************************************************
  // property/solver test 
  //***************************************************************************

  int solver_test( message::Message_Consultant *msg )
  {
    int errors = 0;

    std::cout << std::endl;
    std::cout << "--------------" << std::endl;
    std::cout << "Solver Test..." << std::endl;
    std::cout << "--------------" << std::endl;

    {
      //**********************************************************************
      std::cout << " Testing Sum Solver... (a = b + c) [12 = 5 + 7]" << std::endl;
      {
	std::cout << "  solve for a: ";
	Operand<values::Scalar> a(msg),b(msg),c(msg) ; 
	sum_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  std::cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  std::cerr << "Error: a unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << a;
	  if( a.get_value() == 12 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for b: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	sum_solver( a, b, c );
	if(!a.set_value( 12 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << b;
	  if( b.get_value() == 5 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for c: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	sum_solver( a, b, c );
	if(!a.set_value( 12 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  std::cerr << "Error: c unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << c;
	  if( c.get_value() == 7 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing Product Solver... (a = b * c) [35 = 5 * 7]" << std::endl;
      {
	std::cout << "  solve for a: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  std::cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  std::cerr << "Error: a unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << a;
	  if( a.get_value() == 35 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for b: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!a.set_value( 35 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << b;
	  if( b.get_value() == 5 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for c: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!a.set_value( 35 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  std::cerr << "Error: c unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << c;
	  if( c.get_value() == 7 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing Product Solver... (a = b * c) [0 = 0 * 7]" 
	   << std::endl;
      {
	std::cout << "  solve for a: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  std::cout << a;
	  if( a.get_value() == 0 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
	else
	{
	  std::cerr << "Error: b=0 is not enough!!! Anyway  a="; 
	  errors++;

	  if(!c.set_value( 7 ) )
	  {
	    std::cerr << "Error: could not set c!!! Anyway  a="; 
	    errors++;
	  }
	  if(!a.is_solved() )
	  {
	    std::cerr << "Error: a unsolved!!!" << std::endl; 
	    errors++;
	  }
	  else
	  {
	    std::cout << a;
	    if( a.get_value() == 0 )
	      std::cout << " OK" << std::endl;
	    else
	    {
	      std::cout << " False!!!" << std::endl;
	      errors++;
	    }
	  }
	}
      }
      {
	std::cout << "  solve for b: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << b;
	  if( b.get_value() == 0 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for c: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  std::cout << "cannot be solved: OK" << std::endl; 
	}
	else
	{
	  std::cerr << "Error: c unsolvable!!! c="; 
	  errors++;

	  std::cout << c;
	  if( c.get_value() == 7 )
	    std::cout << " OK, Hä??? ;)" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing Product Solver... (a = b * c) [35 = 5 * 7] {reversed}" 
	   << std::endl;
      {
	std::cout << "  solve for a: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  std::cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  std::cerr << "Error: a unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << a;
	  if( a.get_value() == 35 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for b: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 35 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << b;
	  if( b.get_value() == 5 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for c: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 35 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if(!c.is_solved() )
	{
	  std::cerr << "Error: c unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << c;
	  if( c.get_value() == 7 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing Product Solver... (a = b * c) [0 = 0 * 7] {reversed}" 
	   << std::endl;
      {
	std::cout << "  solve for a: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  std::cerr << "Error: a solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if(!a.is_solved() )
	{
	  std::cerr << "Error: a unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << a;
	  if( a.get_value() == 0 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for b: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << b;
	  if( b.get_value() == 0 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  solve for c: ";
	Operand<values::Scalar> a(msg), b(msg), c(msg) ; 
	product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  std::cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  std::cout << "cannot be solved: OK" << std::endl; 
	}
	else
	{
	  std::cerr << "Error: c unsolvable!!! c="; 
	  errors++;

	  std::cout << c;
	  if( c.get_value() == 7 )
	    std::cout << " OK, Hä??? ;)" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " is_solved Operator..." << std::endl;
      {
	Operand<values::Scalar> a(msg);
	Operand<bool> res(msg);
	res = is_solved( a );
	if( res.is_solved() )
	{
	  std::cout << "Error: solved without knowing Argument"
		    << std::endl;
	}
	a.set_value(1);
	if( !res.is_solved() )
	{
	  std::cout << "Error: did not work show solved status" << std::endl;
	}
	else
	  std::cout << "  OK" << std::endl;
      }
      {
	Operand<values::Scalar> a(msg);
	Operand<bool> res(msg);
	a.set_value(1);
	res = is_solved( a );
	if( !res.is_solved() )
	{
	  std::cout << "Error: did not work with initially set operand" 
		    << std::endl;
	}
	else
	  std::cout << "  works with already set operand: OK" << std::endl;
      }
      //**********************************************************************
      std::cout << " Multi AND Operator..." << std::endl;
      {
	Multi_And_Operator *and = new Multi_And_Operator(msg);
	Operand<values::Scalar> a(msg), b(msg), c(msg); 
	Operand<bool> res(msg);
	res = and->get_result();
	and->add_operand( is_solved( a ) );
	and->add_operand( is_solved( b ) );
	and->add_operand( is_solved( c ) );
	and->finish_adding();
	a.set_value(1);
	b.set_value(1);
	if( res.is_solved() )
	{
	  std::cout << "Error: solved without knowing 3rd Argument" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  c.set_value(1);
	  if( !res.is_solved() )
	  {
	    std::cout << "Error: did not work with 3 true" << std::endl;
	    errors++;
	  }
	  else
	  {
	    if( res.get_value() == false )
	    {
	      std::cout << "Error: 3 true and = false" << std::endl;
	      errors++;
	    }
	    else
	    {
	      std::cout << "  3 true and = true: OK" << std::endl;
	    }
	  }
	}
      }
      {
	Multi_And_Operator *and = new Multi_And_Operator(msg);
	Operand<values::Scalar> a(msg), b(msg), c(msg); 
	Operand<bool> res(msg);
	res = and->get_result();
	a.set_value(1);
	and->add_operand( is_solved( a ) );
	and->add_operand( const_op(true,msg) );
	and->add_operand( is_solved( b ) );
	and->add_operand( is_solved( c ) );
	and->finish_adding();
	b.set_value(1);
	if( res.is_solved() )
	{
	  std::cout << "Error: solved without knowing 4th Argument" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  c.set_value(1);
	  if( !res.is_solved() )
	  {
	    std::cout << "Error: did not work with adding solved values" 
		      << std::endl;
	    errors++;
	  }
	  else
	  {
	    if( res.get_value() == false )
	    {
	      std::cout << "Error: 4 true and = false" << std::endl;
	      errors++;
	    }
	    else
	    {
	      std::cout << "  works with adding solved values: OK" 
			<< std::endl;
	    }
	  }
	}
      }
      {
	Multi_And_Operator *and = new Multi_And_Operator(msg);
	Operand<values::Scalar> a(msg); 
	Operand<bool> res(msg);
	res = and->get_result();
	a.set_value(1);
	and->add_operand( is_solved( a ) );
	and->add_operand( const_op(true,msg) );
	and->finish_adding();
	if( !res.is_solved() )
	{
	  std::cout << "Error: didn't solve adding only solved operands" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  if( res.get_value() == false )
	  {
	    std::cout << "Error: 2 true and = false" << std::endl;
	    errors++;
	  }
	  else
	  {
	    std::cout << "  works with adding only solved values: OK" 
		      << std::endl;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing combined Solver... (a = (b+c) * d) [77 = (5+6) * 7]" 
	   << std::endl;
      {
	std::cout << "  inserting... ";
	Operand<values::Scalar> a(msg), b(msg), c(msg), d(msg), sum(msg) ; 
	sum_solver( sum, b, c );     // sum = b + c
	product_solver( a, sum, d ); // a = sum * d
	if(!c.set_value( 6 ) )
	{
	  std::cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "c=6,";
	}
	if(!d.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "d=7,";
	}
	if( a.is_solved() )
	{
	  std::cerr << "Error: a solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!! "; 
	  errors++;
	}
	else
	{
	  std::cout << "b=5 ";
	}

	if(!a.is_solved() )
	{
	  std::cerr << "Error: a unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << "-> a=" << a;
	  if( a.get_value() == 77 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  inserting... ";
	Operand<values::Scalar> a(msg), b(msg), c(msg), d(msg), sum(msg) ; 
	sum_solver( sum, b, c );     // sum = b + c
	product_solver( a, sum, d ); // a = sum * d
	if(!d.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "d=7,";
	}
	if(!c.set_value( 6 ) )
	{
	  std::cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "c=6,";
	}
	if( b.is_solved() )
	{
	  std::cerr << "Error: b solved too early!!!,"; 
	  errors++;
	}

	if(!a.set_value( 77 ) )
	{
	  std::cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  std::cout << "a=77 ";
	}

	if(!b.is_solved() )
	{
	  std::cerr << "Error: b unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << "-> b=" << b;
	  if( b.get_value() == 5 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  inserting... ";
	Operand<values::Scalar> a(msg), b(msg), c(msg), d(msg), sum(msg) ; 
	sum_solver( sum, b, c );     // sum = b + c
	product_solver( a, sum, d ); // a = sum * d
	if(!a.set_value( 77 ) )
	{
	  std::cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  std::cout << "a=77 ";
	}
	if(!d.set_value( 7 ) )
	{
	  std::cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "d=7,";
	}
	if( c.is_solved() )
	{
	  std::cerr << "Error: c solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "b=5,";
	}

	if(!c.is_solved() )
	{
	  std::cerr << "Error: c unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << "-> c=" << c;
	  if( c.get_value() == 6 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      {
	std::cout << "  inserting... ";
	Operand<values::Scalar> a(msg), b(msg), c(msg), d(msg), sum(msg) ; 
	sum_solver( sum, b, c );     // sum = b + c
	product_solver( a, sum, d ); // a = sum * d
	if(!c.set_value( 6 ) )
	{
	  std::cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "c=6,";
	}
	if(!a.set_value( 77 ) )
	{
	  std::cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  std::cout << "a=77 ";
	}
	if( d.is_solved() )
	{
	  std::cerr << "Error: d solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  std::cerr << "Error: could not set b!!!,"; 
	  errors++;
	}
	else
	{
	  std::cout << "b=5,";
	}

	if(!d.is_solved() )
	{
	  std::cerr << "Error: d unsolved!!!" << std::endl; 
	  errors++;
	}
	else
	{
	  std::cout << "-> d=" << d;
	  if( d.get_value() == 7 )
	    std::cout << " OK" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
    }
    //**********************************************************************
    //**********************************************************************
    {
      Operand<values::Scalar> s0(msg); // start stretch
      Operand<values::Scalar> se(msg); // end stretch
      Operand<values::Scalar> s (msg); // differance stretch
      Operand<values::Scalar> t (msg); // duration
      Operand<values::Scalar> a (msg); // acceleration
      Operand<values::Scalar> v0(msg); // startspeed
      Operand<values::Scalar> ve(msg); // endspeed

      accel_solver( s, t, a, v0, ve );
      sum_solver( se, s, s0 );

      std::cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;
    
      std::cout << "startstretch ok?" << s0.set_value(0) << std::endl; 

      std::cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;

      std::cout << "startspeed 2 m/s ok?" << v0.set_value(2) << std::endl; 
      std::cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;

      std::cout << "acceleration 1 m/s^2 ok?" << a.set_value(1) << std::endl;
      std::cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;

      std::cout << "duration 2 ok?" << t.set_value(2) << std::endl; 
      std::cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;
    }
    return errors;
  }


  //***************************************************************************
  // Action System test 
  //***************************************************************************

  int action_system_test( message::Message_Consultant *msg )
  {
    int errors = 0;

    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Priority Action System Test..." << std::endl;
    std::cout << "-------------------------------" << std::endl;

    Operand<values::Scalar> s0(msg); // start stretch
    Operand<values::Scalar> se(msg); // end stretch
    Operand<values::Scalar> s (msg); // differance stretch
    Operand<values::Scalar> t (msg); // duration
    Operand<values::Scalar> a (msg); // acceleration
    Operand<values::Scalar> v0(msg); // startspeed
    Operand<values::Scalar> ve(msg); // endspeed

    std::cout << " beginning" << std::endl;
    std::cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;

    accel_solver( s, t, a, v0, ve );
    sum_solver( se, s, s0 );

    Priority_System sys;
    
    std::cout << " Actions:" << std::endl;
    std::cout << "  Level  5:  a= 0.5" << std::endl;
    std::cout << "  Level  7:  t=  s0" << std::endl;
    std::cout << "  Level 10: s0=   1" << std::endl;
    establish_Default_Value( &sys,  5, a, values::Scalar(0.5) );
    establish_Default_Value( &sys, 10, s0, values::Scalar(1) );
    establish_Push_Connection( &sys, 7, s0, t ); // push just for fun

    std::cout << " unset status" << std::endl;
    std::cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;
    v0.set_value( 0 );
    std::cout << " after v0=0" << std::endl;
    std::cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;
    sys.invoke_all_Actions();
    std::cout << " after result of actions" << std::endl;
    std::cout << "  s0="<< s0 << " se=" << se << " s=" << s << " t=" <<  t << " a=" << a << " v0=" << v0 << " ve=" << ve << std::endl;

    return errors;
  }

  //***************************************************************************
  // operand/operator test
  //***************************************************************************
  int reference_test( message::Message_Consultant *msg )
  {
    int errors = 0;

    std::cout << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "Reference Test..." << std::endl;
    std::cout << "-------------------" << std::endl;

    { // basic reference test: dest = src * 3
      Operand<values::Scalar> src(msg);
      Operand<values::Scalar> dest(msg);
      Operand<values::Scalar> expression = src * const_op(values::Scalar(3), 
							  msg);
      explicite_reference( dest, expression );
      
      if( src.set_value( 11 ) )
      {
	if( !dest.is_solved() )
	{
	  std::cout << "Error: couldn't solve reference x <= y(=11) * 3" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  if( dest.get_value() != 33 )
	  {
	    std::cout << "Error: wrong result for reference x <= y(=11) * 3: " 
		      << dest.get_value() 
		      << std::endl;
	    errors++;
	  }
	  else
	  {
	    std::cout << "x <= y(=11) * 3 = 33, OK"<< std::endl;
	  }
	}
      }
      else
      {
	std::cout << "Error: Value 11 rejected by reference x <= y * 3" 
		  << std::endl;
	errors++;
      }
    }

    { // immediate reference assign test: dest = src + 3
      Operand<values::Scalar> src(msg);
      Operand<values::Scalar> dest(msg);
      Operand<values::Scalar> expression = src + const_op(values::Scalar(3), 
							  msg);
      assert( src.set_value(3) );
      explicite_reference( dest, expression );
      
      if( !dest.is_solved() )
      {
	std::cout << "Error: couldn't solve reference while initialization "
		  << "x <= y(=3) + 3" 
		  << std::endl;
	errors++;
      }
      else
      {
	if( dest.get_value() != 6 )
	{
	  std::cout << "Error: wrong result for immediate reference solve "
		    << "x <= y(=3) + 3: " 
		    << dest.get_value() 
		    << std::endl;
	    errors++;
	}
	else
	{
	  std::cout << "x <= y(=3) + 3 = 6, OK"<< std::endl;
	}
      }
    }

    { // user override test
      Operand<values::Scalar> src(msg);
      Operand<values::Scalar> dest(msg);
      Operand<values::Scalar> expression = src * const_op(values::Scalar(3), 
							  msg);
      explicite_reference( dest, expression );
      
      if( dest.set_value( 5 ) )
      {
	if( (!dest.is_solved()) || (dest.get_value() != 5) )
	{
	  std::cout << "Error: override value 5 didn't reach dest" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  std::cout << "User may override explicite references: ok" 
		    << std::endl;
	}
      }
      else
      {
	std::cout << "Error: User may NOT override explicite references" 
		  << std::endl;
	errors++;
      }
    }

    { // action block test
      Operand<values::Scalar> src(msg);
      Operand<values::Scalar> dest(msg);
      Operand<values::Scalar> expression = src * const_op(values::Scalar(3), 
							  msg);
      explicite_reference( dest, expression );
      
      Priority_System sys;
   
      establish_Default_Value( &sys,  1, dest, values::Scalar(-5) );
      // this should be the only accepted default value
      establish_Default_Value( &sys,  2, src,  values::Scalar(10) );      
      establish_Default_Value( &sys,  3, dest, values::Scalar(-3) );

      sys.invoke_all_Actions();

      
      if( !dest.is_solved() )
      {
	std::cout << "Error: all default values were rejected" 
		  << std::endl;
	errors++;
      }
      else
      {
	if( dest.get_value() == -5 )
	{
	  std::cout << "Error: destination default wasn't rejected by reference" 
		    << std::endl;
	  errors++;
	}
	else
	{
	  if( dest.get_value() != 30 )
	  {
	    std::cout << "Error: strange override/reject behaviour of reference" 
		      << std::endl;
	    errors++;
	  }
	  else
	  {
	    std::cout << "explicite references block defaults: ok" 
		      << std::endl;
	  }
	}
      }
    }

    return errors;
  }

  int test_solve()
  {
    message::Stream_Message_Handler msg_handler(cerr,cout,cout);
    message::Message_Manager msg_manager(&msg_handler);
    message::Message_Consultant msg_consultant(&msg_manager,0);
    try{
      int errors = 0;
      errors += operator_test(&msg_consultant);
      errors += solver_test(&msg_consultant);
      errors += action_system_test(&msg_consultant);
      errors += reference_test(&msg_consultant);
      cout << "Done..." << endl;
      if( errors )
	cout << errors << " Errors occured" << endl;
      return errors;
    }
    catch( ... ){
      cout << "Fatal Error! " << endl;
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
