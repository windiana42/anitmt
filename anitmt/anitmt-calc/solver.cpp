/*****************************************************************************/
/**   This file offers solver for properties          			    **/
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

#include <assert.h>
#include <algorithm>

#include "solver.hpp"

#include "operator.hpp"

namespace anitmt{
  //***************************************************
  // Product_Solver: solves a = b * c in any direction
  //***************************************************

  Product_Solver::T_A Product_Solver::calc_a( const T_B &b, const T_C &c )
  {
    return b * c;
  }
  Product_Solver::T_A Product_Solver::calc_b( const T_A &a, const T_C &c )
  {
    assert( c != 0 );
    return a / c;
  }
  Product_Solver::T_A Product_Solver::calc_c( const T_A &a, const T_B &b )
  {
    assert( b != 0 );
    return a / b;
  }

  bool Product_Solver::is_a_ok( const T_A &a, const T_B &b, const T_C &c, 
				bool avail_b, bool avail_c )
  {
    if( a != 0 )
    {
      if( avail_b ) if( b == 0 ) return false;
      if( avail_c ) if( c == 0 ) return false;
    }
    else // a == 0
    {
      if( avail_b && avail_c ) if( b != 0 && c != 0 ) return false;
    }
    return true;
  }
  bool Product_Solver::is_b_ok( const T_A &a, const T_B &b, const T_C &c, 
				bool avail_a, bool avail_c )
  {
    if( avail_a )
    {
      if( a != 0 )
      {
	if( b == 0 ) return false;
      }
      else // a == 0
      {
	if( avail_c ) if( c != 0 && b != 0 ) return false;
      }
    }
    return true;
  }
  bool Product_Solver::is_c_ok( const T_A &a, const T_B &b, const T_C &c, 
				bool avail_a, bool avail_b )
  {
    if( avail_a )
    {
      if( a != 0 )
      {
	if( c == 0 ) return false;
      }
      else // a == 0
      {
	if( avail_b ) if( b != 0 && c != 0 ) return false;
      }
    }
    return true;
  }

  bool Product_Solver::is_b_calcable( const T_A &, const T_C &c )
  { 
    return c != 0; 
  }
  bool Product_Solver::is_c_calcable( const T_A &, const T_B &b )
  { 
    return b != 0; 
  }
  
  bool Product_Solver::is_a_calcable_from_b( const T_B &b )
  {
    return b == 0;
  }
  bool Product_Solver::is_a_calcable_from_c( const T_C &c )
  {
    return c == 0;
  }
  
  Product_Solver::T_A Product_Solver::calc_a_from_b( const T_B &b )
  {
    assert( b == 0 );
    return 0;
  }
  Product_Solver::T_A Product_Solver::calc_a_from_c( const T_C &c )
  {
    assert( c == 0 );
    return 0;
  }

  Product_Solver::Product_Solver
  ( Operand<T_A> &a, Operand<T_A> &b, Operand<T_A> &c )
  : Basic_Solver_for_3_Operands<T_A,T_B,T_C>( a, b, c ) {}


  //*********************************************************
  // Accel_Solver: Solver for a constantly accelerated system
  //*********************************************************

  void establish_accel_solver( Operand<values::Scalar> &s, 
			       Operand<values::Scalar> &t, 
			       Operand<values::Scalar> &a, 
			       Operand<values::Scalar> &v0,
			       Operand<values::Scalar> &ve )
  {
    s = 0.5 * a * t*t + v0 * t;
    ve = v0 + a * t;
  }


  //***************************************************************************
  // property/solver test 
  //***************************************************************************

  int solver_test()
  {
    int errors = 0;

    cout << endl;
    cout << "--------------" << endl;
    cout << "Solver Test..." << endl;
    cout << "--------------" << endl;

    {
      //**********************************************************************
      cout << " Testing Sum Solver... (a = b + c) [12 = 5 + 7]" << endl;
      {
	cout << "  solve for a: ";
	Operand<values::Scalar> a,b,c ; 
	establish_sum_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  cerr << "Error: a unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << a;
	  if( a.get_value() == 12 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for b: ";
	Operand<values::Scalar> a,b,c ; 
	establish_sum_solver( a, b, c );
	if(!a.set_value( 12 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << b;
	  if( b.get_value() == 5 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for c: ";
	Operand<values::Scalar> a,b,c ; 
	establish_sum_solver( a, b, c );
	if(!a.set_value( 12 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  cerr << "Error: c unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << c;
	  if( c.get_value() == 7 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      cout << " Testing Product Solver... (a = b * c) [35 = 5 * 7]" << endl;
      {
	cout << "  solve for a: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  cerr << "Error: a unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << a;
	  if( a.get_value() == 35 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for b: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!a.set_value( 35 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << b;
	  if( b.get_value() == 5 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for c: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!a.set_value( 35 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  cerr << "Error: c unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << c;
	  if( c.get_value() == 7 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      cout << " Testing Product Solver... (a = b * c) [0 = 0 * 7]" 
	   << endl;
      {
	cout << "  solve for a: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  cout << a;
	  if( a.get_value() == 0 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
	else
	{
	  cerr << "Error: b=0 is not enough!!! Anyway  a="; 
	  errors++;

	  if(!c.set_value( 7 ) )
	  {
	    cerr << "Error: could not set c!!! Anyway  a="; 
	    errors++;
	  }
	  if(!a.is_solved() )
	  {
	    cerr << "Error: a unsolved!!!" << endl; 
	    errors++;
	  }
	  else
	  {
	    cout << a;
	    if( a.get_value() == 0 )
	      cout << " OK" << endl;
	    else
	    {
	      cout << " False!!!" << endl;
	      errors++;
	    }
	  }
	}
      }
      {
	cout << "  solve for b: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << b;
	  if( b.get_value() == 0 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for c: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  cout << "cannot be solved: OK" << endl; 
	}
	else
	{
	  cerr << "Error: c unsolvable!!! c="; 
	  errors++;

	  cout << c;
	  if( c.get_value() == 7 )
	    cout << " OK, Hä??? ;)" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      cout << " Testing Product Solver... (a = b * c) [35 = 5 * 7] {reversed}" 
	   << endl;
      {
	cout << "  solve for a: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  cerr << "Error: a solved without knowing c!!! a="; 
	  errors++;
	}
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if( !a.is_solved() )
	{
	  cerr << "Error: a unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << a;
	  if( a.get_value() == 35 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for b: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 35 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << b;
	  if( b.get_value() == 5 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for c: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 35 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if(!c.is_solved() )
	{
	  cerr << "Error: c unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << c;
	  if( c.get_value() == 7 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      cout << " Testing Product Solver... (a = b * c) [0 = 0 * 7] {reversed}" 
	   << endl;
      {
	cout << "  solve for a: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  a="; 
	  errors++;
	}
	if( a.is_solved() )
	{
	  cerr << "Error: a solved without knowing b!!! c="; 
	  errors++;
	}
	if(!b.set_value( 0 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  a="; 
	  errors++;
	}
	if(!a.is_solved() )
	{
	  cerr << "Error: a unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << a;
	  if( a.get_value() == 0 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for b: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!c.set_value( 7 ) )
	{
	  cerr << "Error: could not set c!!! Anyway  b="; 
	  errors++;
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved without knowing c!!! b="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  b="; 
	  errors++;
	}
	if( !b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << b;
	  if( b.get_value() == 0 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  solve for c: ";
	Operand<values::Scalar> a,b,c ; 
	establish_product_solver( a, b, c );
	if(!b.set_value( 0 ) )
	{
	  cerr << "Error: could not set b!!! Anyway  c="; 
	  errors++;
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved without knowing b!!! c="; 
	  errors++;
	}
	if(!a.set_value( 0 ) )
	{
	  cerr << "Error: could not set a!!! Anyway  c="; 
	  errors++;
	}
	if( !c.is_solved() )
	{
	  cout << "cannot be solved: OK" << endl; 
	}
	else
	{
	  cerr << "Error: c unsolvable!!! c="; 
	  errors++;

	  cout << c;
	  if( c.get_value() == 7 )
	    cout << " OK, Hä??? ;)" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      //**********************************************************************
      cout << " Testing combined Solver... (a = (b+c) * d) [77 = (5+6) * 7]" 
	   << endl;
      {
	cout << "  inserting... ";
	Operand<values::Scalar> a,b,c,d,sum ; 
	establish_sum_solver( sum, b, c );     // sum = b + c
	establish_product_solver( a, sum, d ); // a = sum * d
	if(!c.set_value( 6 ) )
	{
	  cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "c=6,";
	}
	if(!d.set_value( 7 ) )
	{
	  cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "d=7,";
	}
	if( a.is_solved() )
	{
	  cerr << "Error: a solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!! "; 
	  errors++;
	}
	else
	{
	  cout << "b=5 ";
	}

	if(!a.is_solved() )
	{
	  cerr << "Error: a unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << "-> a=" << a;
	  if( a.get_value() == 77 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  inserting... ";
	Operand<values::Scalar> a,b,c,d,sum ; 
	establish_sum_solver( sum, b, c );     // sum = b + c
	establish_product_solver( a, sum, d ); // a = sum * d
	if(!d.set_value( 7 ) )
	{
	  cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "d=7,";
	}
	if(!c.set_value( 6 ) )
	{
	  cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "c=6,";
	}
	if( b.is_solved() )
	{
	  cerr << "Error: b solved too early!!!,"; 
	  errors++;
	}

	if(!a.set_value( 77 ) )
	{
	  cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  cout << "a=77 ";
	}

	if(!b.is_solved() )
	{
	  cerr << "Error: b unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << "-> b=" << b;
	  if( b.get_value() == 5 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  inserting... ";
	Operand<values::Scalar> a,b,c,d,sum ; 
	establish_sum_solver( sum, b, c );     // sum = b + c
	establish_product_solver( a, sum, d ); // a = sum * d
	if(!a.set_value( 77 ) )
	{
	  cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  cout << "a=77 ";
	}
	if(!d.set_value( 7 ) )
	{
	  cerr << "Error: could not set d!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "d=7,";
	}
	if( c.is_solved() )
	{
	  cerr << "Error: c solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "b=5,";
	}

	if(!c.is_solved() )
	{
	  cerr << "Error: c unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << "-> c=" << c;
	  if( c.get_value() == 6 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
	    errors++;
	  }
	}
      }
      {
	cout << "  inserting... ";
	Operand<values::Scalar> a,b,c,d,sum ; 
	establish_sum_solver( sum, b, c );     // sum = b + c
	establish_product_solver( a, sum, d ); // a = sum * d
	if(!c.set_value( 6 ) )
	{
	  cerr << "Error: could not set c!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "c=6,";
	}
	if(!a.set_value( 77 ) )
	{
	  cerr << "Error: could not set a!!! "; 
	  errors++;
	}
	else
	{
	  cout << "a=77 ";
	}
	if( d.is_solved() )
	{
	  cerr << "Error: d solved too early!!!,"; 
	  errors++;
	}

	if(!b.set_value( 5 ) )
	{
	  cerr << "Error: could not set b!!!,"; 
	  errors++;
	}
	else
	{
	  cout << "b=5,";
	}

	if(!d.is_solved() )
	{
	  cerr << "Error: d unsolved!!!" << endl; 
	  errors++;
	}
	else
	{
	  cout << "-> d=" << d;
	  if( d.get_value() == 7 )
	    cout << " OK" << endl;
	  else
	  {
	    cout << " False!!!" << endl;
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

      establish_accel_solver( s, t, a, v0, ve );
      establish_sum_solver( se, s, s0 );

      cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;
    
      cout << "startstretch ok?" << s0.set_value(0) << endl; 

      cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;

      cout << "startspeed 2 m/s ok?" << v0.set_value(2) << endl; 
      cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;

      cout << "acceleration 1 m/s^2 ok?" << a.set_value(1) << endl;
      cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;

      cout << "duration 2 ok?" << t.set_value(2) << endl; 
      cout << "s0=" << s0 << " s=" << s << " se=" << se << " t=" <<  t 
	   << " a=" << a << " v0=" << v0 << " ve=" << ve << endl;
    }
    return errors;
  }

}  
