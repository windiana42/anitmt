#include <iostream>

#include "operand.hpp"
#include "operator.hpp"
#include "solver.hpp"
#include "priority.hpp"

#include "error.hpp"

using namespace std;

namespace solve {

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
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (*(new Constant<values::Scalar>(1)), 
	    *(new Constant<values::Scalar>(2))))).get_result();
    
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
	     (*(new Constant<values::Scalar>(1))))->get_result(), 
	    *(new Constant<values::Scalar>(2))))).get_result();
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (*(new Constant<values::Scalar>(2)), 
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   (x,
	    *(new Constant<values::Scalar>(2))))).get_result();

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
      Operand<values::Scalar> x;
      Operand<values::Scalar> &op = 
	(*(new Add_Operator<values::Scalar,values::Scalar,values::Scalar>
	   ((new Not_Operator<values::Scalar,values::Scalar>(x))->get_result(),
	    *(new Constant<values::Scalar>(2))))).get_result();

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
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v; 
      Operand<values::Vector> &op = 
	(*(new Add_Operator<values::Vector,values::Vector,values::Vector>
	   (*(new Constant<values::Vector>( values::Vector(1,2,3) )),
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
	*(new Constant<values::Scalar>(1)) + 
	*(new Constant<values::Scalar>(2));

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
	!(*(new Constant<values::Scalar>(1)) + 
	  *(new Constant<values::Scalar>(2)));

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
      Operand<values::Scalar> x;
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
      Operand<values::Scalar> x;
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
      Operand<values::Scalar> x;
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
      Operand<values::Vector> x;
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
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
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
	cerr << "!!Error: could not calc <1,2,3> + <5,4,7>!! " << endl;
	errors++;
      }
    }

    // calc <1,2,3> + v
    {
      Operand<values::Vector> v; 
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> op;
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
      Operand<values::Scalar> x; 
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
      Operand<values::Scalar> x; 
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
      Operand<values::Scalar> x; 
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
      Operand<values::Scalar> x; 
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
      Operand<values::Scalar> x; 
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
      Operand<values::Scalar> x;
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
      catch( EX )
      {
	cout << "  constraint( x(=1) == 2) rejected. OK" << endl;
      }
    }

    // calc +-sqrt( x(=25) ) 
    {
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
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
      catch( EX )
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> op;
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> y;
      Operand<values::Scalar> op;
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
      Operand<values::Scalar> x;
      Operand<values::Scalar> y = 2*x;
      Operand<values::Scalar> op;
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

  int solver_test()
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	    std::cout << " OK, H�??? ;)" << std::endl;
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	Operand<values::Scalar> a,b,c ; 
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
	    std::cout << " OK, H�??? ;)" << std::endl;
	  else
	  {
	    std::cout << " False!!!" << std::endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      std::cout << " Testing combined Solver... (a = (b+c) * d) [77 = (5+6) * 7]" 
	   << std::endl;
      {
	std::cout << "  inserting... ";
	Operand<values::Scalar> a,b,c,d,sum ; 
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
	Operand<values::Scalar> a,b,c,d,sum ; 
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
	Operand<values::Scalar> a,b,c,d,sum ; 
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
	Operand<values::Scalar> a,b,c,d,sum ; 
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
      Operand<values::Scalar> s0; // start stretch
      Operand<values::Scalar> se; // end stretch
      Operand<values::Scalar> s ; // differance stretch
      Operand<values::Scalar> t ; // duration
      Operand<values::Scalar> a ; // acceleration
      Operand<values::Scalar> v0; // startspeed
      Operand<values::Scalar> ve; // endspeed

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

  int action_system_test()
  {
    int errors = 0;

    std::cout << std::endl;
    std::cout << "-------------------------------" << std::endl;
    std::cout << "Priority Action System Test..." << std::endl;
    std::cout << "-------------------------------" << std::endl;

    Operand<values::Scalar> s0; // start stretch
    Operand<values::Scalar> se; // end stretch
    Operand<values::Scalar> s ; // differance stretch
    Operand<values::Scalar> t ; // duration
    Operand<values::Scalar> a ; // acceleration
    Operand<values::Scalar> v0; // startspeed
    Operand<values::Scalar> ve; // endspeed

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
  int reference_test()
  {
    int errors = 0;

    std::cout << std::endl;
    std::cout << "-------------------" << std::endl;
    std::cout << "Reference Test..." << std::endl;
    std::cout << "-------------------" << std::endl;

    

    return errors;
  }

  int test_solve()
  {
    try{
      int errors = 0;
      errors += operator_test();
      errors += solver_test();
      errors += action_system_test();
      errors += reference_test();
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
