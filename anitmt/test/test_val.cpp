/*
 * test_val.cpp
 *
 * Routines to test value library. 
 *
 * Copyright (c) 2000 by Wolfgang Wieser. 
 *
 * This is a top-secret file..\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b
 * Use this file for anything you want.
 *
 */

#include <stdlib.h>
#include <unistd.h>

#include <val/val.hpp>

using namespace values;

int main()
{
	double da,db=1.1;
	Scalar sa,sb(1),sc(sb),sd(db);
	
	da=sb;
	if(1.00000000001==sb)
		{  std::cerr << "equal\n";  }
	if(!(sb!=1.00000000001))
		{  std::cerr << "equal2\n";  }
	sa=sb*sc;
	sb=sa-0.00000000001;
	if(sa==sb)
		{  std::cerr << "equal3" "\n";  }
	
	sd-=1.10000000001;
	if(!sd)
		{  std::cerr << "zero\n";  }
	if(double(sd)==0.0)
		{  std::cerr << "OOPS!!\n";  }
	
	sa=2.0;
	sb=2.0*sa;
	sc=(sa*=sb)*2.0;
	std::cerr << "res (16) =" << sc << "\n";
	
	sc=-sa;  std::cerr << "-" << sa << "=" << sc << "\n";
	sc=+sa;  std::cerr << "+" << sa << "=" << sc << "\n";
	
	std::cerr << "sizes=" << sizeof(vect::vector<3>) << "," << sizeof(Vector) << "\n";
	
	Vector va(1.0,6.0,3.0),vb(2.0,4.0,2.0),vc,vd(vb);
	std::cerr << vb << " -> abs, abs2: " << abs(vb) << ", " << abs2(vb) << "\n";
	
	std::cerr << va << "+" << vb << "=" << (va+vb) << "\n";
	std::cerr << va << "*" << sa << "=" << (va*sa) << "\n";
	std::cerr << vb << "/" << sa << "=" << (vb/sa) << "\n";
	
	vc=Vector(1.0,6.0,3.0000000001);
	std::cerr << va << " == " << vc << " -> " << (va==vc) << "\n";
	va=-vc;
	
	Vector v0;
	std::cerr << "IsNull(" << v0 << ") = " << !v0 << "\n";
	std::cerr << "IsNull(" << (v0+Vector(0,0,1)) << ") = " << !(v0+Vector(0,0,1)) << "\n";
	
	va=Vector(1.0,0.0,0.0);
	vb=va;
	std::cerr << "rotate(45):" << va << " -> " << rotateZ(rotateY(va,M_PI/4.0),M_PI/2.0) << "\n";
	vb.rotateY(M_PI/4.0).rotateZ(M_PI/2.0);
	std::cerr << "rotate(45):" << va << " -> " << vb << "\n";
	
	std::cerr << "Testing to_spherical/to_rectangular...";
	srandom(getpid());
	// Testing to_spherical, to_rectangular: 
	for(int i=0; i<100; i++)
	{
		Vector vva(rand()/1000.0,rand()/1000.0,rand()/1000.0);
		Vector sph1=to_spherical(vva);
		Vector rec1=to_rectangular(sph1);
		Vector sph2=vva;  sph2.to_spherical();
		Vector rec2=sph2; rec2.to_rectangular();
		if(sph1!=sph2)
		{  std::cerr << "\nto_spherical: different results: " << 
			sph1 << " " << sph2 << "\n";  }
		if(rec2!=rec1)
		{  std::cerr << "\nto_rectangular: different results. " << 
			rec2 << " " << rec1 << "\n";  }
		if(vva!=rec1)
		{  std::cerr << "\nto_spherical/to_rectangular non-inverse: " << 
			vva << " -> " << sph1 << " -> " << rec1 << "\n";  }
	}
	std::cerr << "done\n";
	
	// Speed test:
	//std::cerr << "Speed test..."; 
	//for(int i=0; i<100000; i++)
	//{
	//	va=vb+vc;
	//	va/=10.0;
	//	vc-=vb;
	//}
	//std::cerr << "passed.\n";
	
	va=Vector(2,4,5);
	vb=va;
	std::cerr << "normalize(" << va << ") = " << 
		normalize(va) << " = " << vb.normalize() << "\n";
	if(normalize(va) != vb || 
	   abs(vb) != 1.0)
	{  std::cerr << "*** ERROR!! normalize-abs=" << vb.abs() << "\n";  }
	
	std::cerr << "Testing abs...";
	if(va.abs() != abs(va))
	{  std::cerr << "abs() ERROR: " << va.abs() << " != " 
		<< abs(va) << "\n";  }
	else
	{  std::cerr << "done\n";  }
	
	va=Vector(-2,3.5,-7);
	vb=va;
	std::cerr << "Vector: " << va << "\n";
	std::cerr << "Mirrors: " << 
		mirrorX(va) << 
		mirrorY(va) << 
		mirrorZ(va) << 
		mirror(va) << "\n";
	std::cerr << "Mirrors: " << 
		va.mirrorX() << 
		va.mirrorX().mirrorY() << 
		va.mirrorY().mirrorZ() << 
		va.mirrorZ().mirror() << "\n";
	
	va=vb;
	std::cerr << "Trans: " << 
		transX(va,-6) << 
		transY(va,-6) << 
		transZ(va,-6) << "\n";
	std::cerr << "Trans: " << 
		va.transX(-6) << 
		va.transX(6).transY(-6) << 
		va.transY(6).transZ(-6) << "\n";
	
	va=vb;
	std::cerr << "Scale: " << 
		scaleX(va,-2) << 
		scaleY(va,-2) << 
		scaleZ(va,-2) << "\n";
	std::cerr << "Scale: " << 
		va.scaleX(-2) << 
		va.scaleX(-0.5).scaleY(-2) << 
		va.scaleY(-0.5).scaleZ(-2) << "\n";
	
	std::cerr << "Testing multiplication...";
	for(int i=0; i<100; i++)
	{
		va=Vector(rand()/1000,rand()/1000,rand()/1000);
		vb=Vector(rand()/1000,rand()/1000,rand()/1000);
		vc=cross(va,vb);
		sa=va*vc;
		sb=dot(vb,vc);
		sc=abs(va)*abs(vb)*(sin(angle(va,vb)));
		if(sa!=0 || sb!=0)
		{  std::cerr << "*** ERROR (scalar/vector mul) (" << 
			sa << ", " << sb << ")\n";  }
		else if(abs(vc)-sc > 0.01)
		{  std::cerr << "*** ERROR (scalar/vector mul) " << 
			abs(vc) << " != " << sc << "; delta=" << 
			abs(vc)-sc << "\n";  }
	}
	std::cerr << "done\n";
	
	Matrix ma,mb(Matrix::ident),mc(Matrix::null),md;
	
	ma(2,0,5.0)(2,2,7.0);
	std::cerr << ma;
	std::cerr << "idx:" << ma[0][0] << "," << ma[1][0] << "," 
	               << ma[2][0] << "," << ma[2][2] << "\n";
	
	std::cerr << "ident? (should be 1,0) " << !mb << "," << !ma << "\n";
	std::cerr << "null? (should be 1,0) " << mc.is_null() << "," << mb.is_null() << "\n";
	
	//std::cerr << ma*10 << ma/2 << 2*ma ;
	//ma*=10;  std::cerr << ma;  ma/=2;  std::cerr << ma;
	
	std::cerr << "Simple Matrix*Vector test...";
	va=Vector(3.77,8.91,-13.1);
	vb=va;
	mc=mb*2;
	std::cerr << mb << (mb*va);
	if(mb*va != vb || mc*va != vb*2.0 || va*mc != vb*2.0)
	{  std::cerr << "\n*** ERROR in Matrix*Vector (1).\n";  }
	else
	{
		va*=mc;
		vb*=2;
		if(va != vb)
		{  std::cerr << "\n*** ERROR in Matrix*Vector (2).\n";  }
		else
		{  std::cerr << "done\n";  }
	}
	
	std::cerr << "Matrix*Matrix / invert test...";
	for(int i=0; i<100; i++)
	{
		for(int c=0; c<4; c++)
			for(int r=0; r<4; r++)
				mc(c,r,rand()/1000.0);
		
		if(mc*mb != mc)
		{  std::cerr << "\n*** ERROR in Matrix*Matrix (1).\n";  }
		if(mb*mc != mc)
		{  std::cerr << "\n*** ERROR in Matrix*Matrix (2).\n";  }
		ma=mc;
		mc*=mb;
		if(mc != ma)
		{  std::cerr << "\n*** ERROR in Matrix*Matrix (3).\n";  }
		
		// mc = random;  ma = mc;  mb = identity matrix
		
		ma.invert();
		if(ma*mc!=mb)
		{  std::cerr << "\n*** ERROR in invert() (1).\n";  }
		
		ma=invert(mc);
		if(ma*mc!=mb)
		{  std::cerr << "\n*** ERROR in invert() (2).\n";  }
	}
	std::cerr << "done\n";
	
	std::cerr << "Matrix+-Matrix / -Matrix test...";
	for(int i=0; i<100; i++)
	{
		Matrix mrp,mrm,mrn;
		for(int c=0; c<4; c++)
			for(int r=0; r<4; r++)
			{
				double ra=rand()/1000.0,rb=rand()/1000.0;
				ma(c,r,ra);
				mb(c,r,rb);
				mrp(c,r,ra+rb);
				mrm(c,r,ra-rb);
				mrn(c,r,-ra);
			}
		md=ma;
		mc=ma+mb;
		md+=mb;
		if(mc!=md || mc!=mrp || mc*0.5!=(ma/2.0)+(mb*0.5))
		{  std::cerr << "\n*** ERROR in matrix+matrix\n";  }
		md=ma;
		mc=ma-mb;
		md-=mb;
		if(mc!=md || mc!=mrm || mc*0.5!=(ma/2.0)-(mb*0.5))
		{  std::cerr << "\n*** ERROR in matrix-matrix\n";  }
		if(-ma!=mrn || (-ma)*2.0 != (ma*(-2.0)))
		{  std::cerr << "\n*** ERROR in -matrix\n";  }
	}
	std::cerr << "done\n";
	
	return(0);
}
