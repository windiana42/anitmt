/*
 * test_val.cpp
 *
 * Routines to test value library. 
 *
 * Copyright (c) 2000--2002 by Wolfgang Wieser. 
 *
 * This is a top-secret file..\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b
 * Use this file for anything you want.
 *
 */

#include <stdlib.h>
#include <unistd.h>

#include "val.hpp"

#include <assert.h>

using namespace values;
using namespace std;

int main()
{
	double da,db=1.1;
	Scalar sa,sb(1),sc(sb),sd(db);
	
	da=sb;
	if(1.00000000001==sb)
		{  cerr << "equal\n";  }
	if(!(sb!=1.00000000001))
		{  cerr << "equal2\n";  }
	sa=sb*sc;
	sb=sa-0.00000000001;
	if(sa==sb)
		{  cerr << "equal3" "\n";  }
	
	sd-=1.10000000001;
	if(!sd)
		{  cerr << "zero\n";  }
	if(double(sd)==0.0)
		{  cerr << "OOPS!!\n";  }
	
	sa=2.0;
	sb=2.0*sa;
	sc=(sa*=sb)*2.0;
	cerr << "res (16) =" << sc << "\n";
	
	sc=-sa;  cerr << "-" << sa << "=" << sc << "\n";
	sc=+sa;  cerr << "+" << sa << "=" << sc << "\n";
	
	cerr << "sizes=" << sizeof(vect::Vector<3>) << "," << sizeof(Vector) << "\n";
	
	Vector va(1.0,6.0,3.0),vb(2.0,4.0,2.0),vc,vd(vb);
	cerr << vb << " -> abs, abs2: " << abs(vb) << ", " << abs2(vb) << "\n";
	
	cerr << va << "+" << vb << "=" << (va+vb) << "\n";
	cerr << va << "*" << sa << "=" << (va*sa) << "\n";
	cerr << vb << "/" << sa << "=" << (vb/sa) << "\n";
	
	vc=Vector(1.0,6.0,3.0000000001);
	cerr << va << " == " << vc << " -> " << (va==vc) << "\n";
	va=-vc;
	
	Vector v0;
	cerr << "IsNull(" << v0 << ") = " << !v0 << "\n";
	cerr << "IsNull(" << (v0+Vector(0,0,1)) << ") = " << !(v0+Vector(0,0,1)) << "\n";
	
	va=Vector(1.0,0.0,0.0);
	vb=va;
	cerr << "rotate(45):" << va << " -> " << vec_rotate_z(vec_rotate_y(va,M_PI/4.0),M_PI/2.0) << "\n";
	vb.rotate_y(M_PI/4.0).rotate_z(M_PI/2.0);
	cerr << "rotate(45):" << va << " -> " << vb << "\n";
	
	cerr << "Testing to_spherical/to_rectangular...";
	srandom(getpid());
	// Testing to_spherical, to_rectangular: 
	for(int i=0; i<100; i++)
	{
		Vector vva(rand()/1000.0,rand()/1000.0,rand()/1000.0);
		Vector sph1=vec_to_spherical(vva);
		Vector rec1=vec_to_rectangular(sph1);
		Vector sph2=vva;  sph2.to_spherical();
		Vector rec2=sph2; rec2.to_rectangular();
		if(sph1!=sph2)
		{  cerr << "\nto_spherical: different results: " << 
			sph1 << " " << sph2 << "\n";  }
		if(rec2!=rec1)
		{  cerr << "\nto_rectangular: different results. " << 
			rec2 << " " << rec1 << "\n";  }
		if(vva!=rec1)
		{  cerr << "\nto_spherical/to_rectangular non-inverse: " << 
			vva << " -> " << sph1 << " -> " << rec1 << "\n";  }
	}
	cerr << "done\n";
	
	// Speed test:
	//cerr << "Speed test..."; 
	//for(int i=0; i<100000; i++)
	//{
	//	va=vb+vc;
	//	va/=10.0;
	//	vc-=vb;
	//}
	//cerr << "passed.\n";
	
	va=Vector(2,4,5);
	vb=va;
	cerr << "normalize(" << va << ") = " << 
		vec_normalize(va) << " = " << vb.normalize() << "\n";
	if(vec_normalize(va) != vb || 
	   abs(vb) != 1.0)
	{  cerr << "*** ERROR!! normalize-abs=" << vb.abs() << "\n";  }
	
	cerr << "Testing abs...";
	if(va.abs() != abs(va))
	{  cerr << "abs() ERROR: " << va.abs() << " != " 
		<< abs(va) << "\n";  }
	else
	{  cerr << "done\n";  }
	
	va=Vector(-2,3.5,-7);
	vb=va;
	cerr << "Vector: " << va << "\n";
	cerr << "Mirrors: " << 
		vec_mirror(va,0) << 
		vec_mirror(va,1) << 
		vec_mirror(va,2) << 
		vec_mirror(va) << "\n";
	cerr << "Mirrors: " << 
		va.mirror(0) << 
		va.mirror(0).mirror(1) << 
		va.mirror(0).mirror(2) << 
		va.mirror(2).mirror() << "\n";
	
	va=vb;
	cerr << "Trans: " << 
		vec_translate(va,-6,0) << 
		vec_translate(va,-6,1) << 
		vec_translate(va,-6,2) << "\n";
	cerr << "Trans: " << 
		va.translate(-6,0) << 
		va.translate(6,0).translate(-6,1) << 
		va.translate(6,1).translate(-6,2) << "\n";
	
	va=vb;
	cerr << "Scale: " << 
		vec_scale(va,-2,0) << 
		vec_scale(va,-2,1) << 
		vec_scale(va,-2,2) << "\n";
	cerr << "Scale: " << 
		va.scale(-2,0) << 
		va.scale(-0.5,0).scale(-2,1) << 
		va.scale(-0.5,1).scale(-2,2) << "\n";
	
	cerr << "Testing multiplication...";
	int nfailed=0;
	for(int i=0; i<100; i++)
	{
		va=Vector(rand()/10000,rand()/10000,rand()/10000);
		vb=Vector(rand()/10000,rand()/10000,rand()/10000);
		vc=cross(va,vb);
		sa=va*vc;
		sb=dot(vb,vc);
		sc=abs(va)*abs(vb)*(sin(vec_angle(va,vb)));
		if(sa!=0.0 || sb!=0.0)
		{  cerr << "*** ERROR (scalar/vector mul) (" << 
			sa << ", " << sb << ")\n";  ++nfailed;  }
		else if(abs(vc)-sc > 0.01)
		{  cerr << "*** ERROR (scalar/vector mul) " << 
			abs(vc) << " != " << sc << "; delta=" << 
			abs(vc)-sc << "\n";  ++nfailed;  }
	}
	cerr << "done (" << nfailed << "/100)\n";
	
	Matrix ma,mb(Matrix::ident),mc(Matrix::null),md;
	
	ma[0][2]=5.0;
	ma[2][2]=7.0;
	cerr << ma;
	cerr << "idx:" << ma[0][0] << "," << ma[0][1] << "," 
	               << ma[0][2] << "," << ma[2][2] << "\n";
	
	cerr << "ident? (should be 1,0) " << !mb << "," << !ma << "\n";
	cerr << "null? (should be 1,0) " << mc.is_null() << "," << mb.is_null() << "\n";
	
	//cerr << ma*10 << ma/2 << 2*ma ;
	//ma*=10;  cerr << ma;  ma/=2;  cerr << ma;
	
	cerr << "Simple Matrix*Vector test...";
	va=Vector(3.77,8.91,-13.1);
	vb=va;
	mc=mb*2;
	cerr << mb << (mb*va);
	if(mb*va != vb || mc*va != vb*2.0 || va*mc != vb*2.0)
	{  cerr << "\n*** ERROR in Matrix*Vector (1).\n";  }
	else
	{
		va*=mc;
		vb*=2;
		if(va != vb)
		{  cerr << "\n*** ERROR in Matrix*Vector (2).\n";  }
		else
		{  cerr << "done\n";  }
	}
	
	cerr << "Matrix*Matrix / invert test...";
	for(int i=0; i<100; i++)
	{
		for(int r=0; r<4; r++)
			for(int c=0; c<4; c++)
				mc[r][c]=rand()/1000.0;
		
		if(mc*mb != mc)
		{  cerr << "\n*** ERROR in Matrix*Matrix (1).\n";  }
		if(mb*mc != mc)
		{  cerr << "\n*** ERROR in Matrix*Matrix (2).\n";  }
		ma=mc;
		mc*=mb;
		if(mc != ma)
		{  cerr << "\n*** ERROR in Matrix*Matrix (3).\n";  }
		
		// mc = random;  ma = mc;  mb = identity matrix
		
		ma.invert();
		if(ma*mc!=mb)
		{  cerr << "\n*** ERROR in invert() (1).\n";  }
		
		ma=mat_invert(mc);
		if(ma*mc!=mb)
		{  cerr << "\n*** ERROR in invert() (2).\n";  }
	}
	cerr << "done\n";
	
	cerr << "Matrix+-Matrix / -Matrix test...";
	for(int i=0; i<100; i++)
	{
		Matrix mrp,mrm,mrn;
		for(int c=0; c<4; c++)
			for(int r=0; r<4; r++)
			{
				double ra=rand()/1000.0,rb=rand()/1000.0;
				ma[r][c]=ra;
				mb[r][c]=rb;
				mrp[r][c]=ra+rb;
				mrm[r][c]=ra-rb;
				mrn[r][c]=-ra;
			}
		md=ma;
		mc=ma+mb;
		md+=mb;
		if(mc!=md || mc!=mrp || mc*0.5!=(ma/2.0)+(mb*0.5))
		{  cerr << "\n*** ERROR in matrix+matrix\n";  }
		md=ma;
		mc=ma-mb;
		md-=mb;
		if(mc!=md || mc!=mrm || mc*0.5!=(ma/2.0)-(mb*0.5))
		{  cerr << "\n*** ERROR in matrix-matrix\n";  }
		if(-ma!=mrn || (-ma)*2.0 != (ma*(-2.0)))
		{  cerr << "\n*** ERROR in -matrix\n";  }
	}
	cerr << "done\n";
	
	/*********************************************/
	
	cerr << "Some more test (assertions on failure)...";
	{
		ma=Matrix::null;
		mb=Matrix::ident;
		assert(ma==Neutral0());
		assert(!(ma!=Neutral0()));
		assert(mb==Neutral1());
		assert(!(mb!=Neutral1()));
		assert(ma!=Neutral1());
		assert(!(ma==Neutral1()));
		assert(mb!=Neutral0());
		assert(!(mb==Neutral0()));
		ma=Neutral1();
		assert(ma==Neutral1());
		mc=Matrix(Neutral0());
		mc=Matrix(Neutral1());
		
		va=Vector();
		assert(va==Neutral0());
		assert(!(va!=Neutral0()));
		va=Neutral0();
		va=Vector(Neutral0());
		
		sa=0.0;
		sb=1.0;
		assert(sa==Neutral0());
		assert(!(sa!=Neutral0()));
		assert(sb==Neutral1());
		assert(!(sb!=Neutral1()));
		assert(sa!=Neutral1());
		assert(!(sa==Neutral1()));
		assert(sb!=Neutral0());
		assert(!(sb==Neutral0()));
		sa=Neutral1();
		assert(sa==Neutral1());
		sc=Scalar(Neutral0());
		sc=Scalar(Neutral1());
		
		String x=Neutral0();
		assert(x==Neutral0());
	}
	cerr << " OK [" << sizeof(Neutral0) << "]" << endl;
	
	return(0);
}
