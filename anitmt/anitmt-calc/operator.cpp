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
#include <math.h>

#include "operand.hpp"
#include "property.hpp"
#include "constraint.hpp"

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

    // calc abs(v(=<1.1,2.2,3.3>)) 
    {
      Operand<values::Vector> x;
      Operand<values::Scalar> &op = sqrt(abs(x));

      if( op.is_solved() )
      {
	cerr << "!!Error: why can he solve abs(v(=<1.1,2.2,3.3>)) without" 
	  " knowing x?!! " 
	     << endl;
	errors++;
      }

      x.set_value( values::Vector(1.1,2.2,3.3) );

      if( op.is_solved() )
      {
	cout << "  abs(v(=<1.1,2.2,3.3>)) = " << op.get_value() 
	     << "(4,115823125451)"
	     << endl;
	assert( op.get_value() == ::sqrt(1.1*1.1+2.2*2.2+3.3*3.3) );
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
	cout << "  sqrt(abs(x(=123.45))) = " << op.get_value() << "(11,11081)"
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
	     << " (1.666...)" <<  endl;
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
		 << " (-5)" <<  endl;
	    assert( op.get_value() == -5 );
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
	cerr << "!!Error: could not assign 5 + 2 to property!!" << endl;
	errors++;
      }
    }
    return errors;
  }
}
