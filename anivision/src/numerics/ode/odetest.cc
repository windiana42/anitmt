/*
 * odetest.cc
 * 
 * Numerical ODE (ordinary differential equation) solver tester 
 * with simple Qt & QTX-based GUI. 
 * 
 * Note: This is a quick test program. IF YOU INTEND TO SEE GOOD 
 *       PROGRAMMING, DO NOT LOOK AT THIS FILE. 
 * 
 * Copyright (c) 2004 by Wolfgang Wieser (wwieser@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <Qt/qapplication.h>
#include <Qt/qpainter.h>
#include <Qt/qbrush.h>
#include <Qt/qlayout.h>
#include <Qt/qlabel.h>
#include <Qt/qcdestyle.h>
#include <Qt/qevent.h>

#include <QTX/xpainter.h>

// gcc -W -Wall -O2 odetest.cc -o odetest -fno-rtti -fno-exceptions -I. -IQt -fmessage-length=$COLUMNS -lqt-mt -L/opt/Qt/lib libqtxlib.a -I../.. -I../../../../hlib-build/include/ ../../../../anivision-build/src/numerics/ode/libnumerics_ode.a ../../../../anivision-build/src/numerics/libnumerics.a ../../../../anivision-build/src/numerics/interpolate/libnumerics_interpolate.a ../../../../anivision-build/src/numerics/linalg/libnumerics_linalg.a ../../../../hlib-build/libhlib.a


using namespace QTX;

#include <numerics/function.h>
#include <numerics/linalg/linalg.h>
#include <numerics/ode/odesolver.h>
#include <numerics/ode/odedrv_rk.h>
#include <numerics/ode/odedrv_bs.h>
#include <numerics/ode/odedrv_rkf456.h>

//------------------------------------------------------------------------------

const double m1=1.0,m2=1.0;
const double l1=100.0,l2=100.0;
const double g=9.81;

inline double SQR(double x)
	{  return(x*x);  }

class MyODEFunc : public NUM::Function_ODE
{
	protected:
		// [overriding virtual]
		void function(double /*x*/,const double *y,double *result)
		{
			register double phi=y[0],phip=y[1],psi=y[2],psip=y[3];
			
			double tmp = m2*l1*l2*sin(phi-psi);
			double a = -tmp*psip*psip - g*(m1+m2)*l1*sin(phi);
			double d =  tmp*phip*phip - g*    m2 *l2*sin(psi);
			double b = l1*l1*(m1+m2);
			double c = m2*l1*l2*cos(phi-psi);
			double e = l2*l2*m2;
			double det = (b*e-c*c);
			
			result[0]=phip;
			result[1]=(a*e-c*d)/det;
			result[2]=psip;
			result[3]=(b*d-c*a)/det;
			
			#if 0
			// Control: calc energy: 
			double T = 0.5*(m1+m2)*SQR(l1*phip) + 
				m2*( 0.5*SQR(l2*psip) + l1*l2*phip*psip*cos(phi-psi) );
			double U = -g*( (m1+m2)*l1*cos(phi) + m2*l2*cos(psi) );
			double E = T+U;
			
			static int cnt=0;
			if(!(++cnt%100))
			{  fprintf(stderr,"\n%g",E);  }
			#endif
		}
		
		// [overriding virtual]
		void jacobian(double /*x*/,const double *y,
			NUM::SMatrix<double> &df_dy,double *df_dx)
		{
			register double y0=y[0],y1=y[1],y2=y[2],y3=y[3];
			
			for(int i=0; i<4; i++)  df_dx[i]=0.0;
			
			// df_dy[i][j] = d f[i] / d y[j]
			
			// Diff: y_0' = y_1
			df_dy[0][0]=0.0;
			df_dy[0][1]=1.0;
			df_dy[0][2]=0.0;
			df_dy[0][3]=0.0;
			
			// Diff: y_2' = y_3
			df_dy[2][0]=0.0;
			df_dy[2][1]=0.0;
			df_dy[2][2]=0.0;
			df_dy[2][3]=1.0;
			
			double cos02=cos(y0-y2);
			double sin02=sin(y0-y2);
			
			double tmpNa = SQR(l1*l2) * m2 * ((m1+m2) - m2*SQR(cos02));
			double tmpZa = -2.0*SQR(m2*l2)*l1*sin02;
			df_dy[1][1] = tmpZa*l1*cos02*y1 / tmpNa;
			df_dy[1][3] = tmpZa*l2*y3 / tmpNa;
			
			double tmpZb = 2.0*SQR(l1)*sin02*m2*l2;
			df_dy[3][1] = tmpZb*l1*(m1+m2)*y1 / tmpNa;
			df_dy[3][3] = tmpZb*m2*l2*cos02*y3 / tmpNa;
			
			double tmpA = m2*l1*l2*( m2*l1*l2*sin02*SQR(y1) - g*m2*l2*sin(y2) );
			double tmpB0 = m2*l1*l2*SQR(y3);
			double tmpB1 = g*(m1+m2)*l1;
			double tmpNc = SQR(m2*l1*l2/tmpNa)*cos02*sin02;
			double tmpF = ( sin02*tmpB0 + tmpB1*sin(y0) ) * SQR(l2)*m2;
			double tmpRHa = 2.0*( tmpF + cos02*tmpA ) * tmpNc;
			df_dy[1][0] = 
				(	  sin02*tmpA
					- ( cos02*tmpB0 + tmpB1*cos(y0) ) * SQR(l2)*m2
					- SQR(m2*l1*l2*cos02*y1)
				) / tmpNa + tmpRHa;
			
			double tmpC1 = m2*l1*l2;
			double tmpCa = tmpC1*SQR(y1);
			double tmpCc = g*m2*l2;
			df_dy[1][2] = 
				(	  l1*l2*SQR(m2*l2*y3)*cos02
					- ( tmpCa*sin02 - tmpCc*sin(y2) ) * tmpC1*sin02
					+ ( tmpCa*cos02 + tmpCc*cos(y2) ) * tmpC1*cos02
				) / tmpNa - tmpRHa;
			
			double tmpCb = tmpC1*SQR(y3);
			double tmpCd = g*(m1+m2)*l1;
			double tmpDa = m2*l2*SQR(l1)*(m1+m2);
			double tmpDb = m2*l2*sin02*SQR(y3) + g*(m1+m2)*sin(y0);
			double tmpEa = l1*sin02*SQR(y1) - g*sin(y2);
			double tmpRHb = 2.0*( tmpEa * tmpDa + tmpDb * tmpC1*l1*cos02 ) * tmpNc;
			df_dy[3][0] = 
				(	  l1*l2*SQR(l1*y1)*(m1+m2)*m2*cos02
					- ( tmpCb*sin02 + tmpCd*sin(y0) ) * tmpC1*sin02
					+ ( tmpCb*cos02 + tmpCd*cos(y0) ) * tmpC1*cos02
				) / tmpNa - tmpRHb;
			df_dy[3][2] = 
				(	- ( l1*cos02*SQR(y1) + g*cos(y2) ) * tmpDa
					+ tmpDb * tmpC1*l1*sin02
					- SQR(m2*l1*l2*cos02*y3)
				) / tmpNa + tmpRHb;
			
			
			#if 0
			// This is what Maple calculated: 
			
			double df_dyH[4][4];
			
			df_dyH[1][0] = 
				((-m2*l1*l2*cos02*SQR(y3)-g*(m1+m2)*l1*cos(y0))*SQR(l2)*m2+
				m2*l1*l2*sin02*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2))-
				SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02)*SQR(y1))/(SQR(l1)*(m1+m2)*
				SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02))-
				2*((-m2*l1*l2*sin02*SQR(y3)-g*(m1+m2)*l1*sin(y0))*SQR(l2)*m2-
				m2*l1*l2*cos02*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2)))/
				SQR(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*
				SQR(cos02))*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02;
			df_dyH[1][2] = 
				(SQR(m2)*l1*l2*l2*l2*cos02*SQR(y3)-
				m2*l1*l2*sin02*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2))-
				m2*l1*l2*cos02*(-m2*l1*l2*cos02*SQR(y1)-g*m2*l2*cos(y2)))/
				(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02))+
				2*((-m2*l1*l2*sin02*SQR(y3)-g*(m1+m2)*l1*sin(y0))*SQR(l2)*m2-
				m2*l1*l2*cos02*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2)))/
				SQR(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*
				SQR(cos02))*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02;
			
			df_dyH[3][0] = 
				(l1*l1*l1*(m1+m2)*m2*l2*cos02*SQR(y1)+m2*l1*l2*sin02*
				(-m2*l1*l2*sin02*SQR(y3)-g*(m1+m2)*l1*sin(y0))-
				m2*l1*l2*cos02*(-m2*l1*l2*cos02*SQR(y3)-
				g*(m1+m2)*l1*cos(y0)))/(SQR(l1)*(m1+m2)*
				SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02))-
				2*(SQR(l1)*(m1+m2)*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2))-
				m2*l1*l2*cos02*(-m2*l1*l2*sin02*SQR(y3)-g*(m1+m2)*l1*sin(y0)))/
				SQR(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*
				SQR(cos02))*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02;
			df_dyH[3][2] = 
				(SQR(l1)*(m1+m2)*(-m2*l1*l2*cos02*SQR(y1)-g*m2*l2*cos(y2))-
				m2*l1*l2*sin02*(-m2*l1*l2*sin02*SQR(y3)-g*(m1+m2)*l1*sin(y0))-
				SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02)*SQR(y3))/
				(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02))+
				2*(SQR(l1)*(m1+m2)*(m2*l1*l2*sin02*SQR(y1)-g*m2*l2*sin(y2))-
				m2*l1*l2*cos02*(-m2*l1*l2*sin(y0-y2)*SQR(y3)-g*(m1+m2)*l1*
				sin(y0)))/SQR(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*
				SQR(l2)*SQR(cos02))*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02;
			
			df_dyH[1][1] = 
				-2*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02*y1/
				(SQR(l1)*(m1+m2)*SQR(l2)*m2-SQR(m2)*SQR(l1)*SQR(l2)*
				SQR(cos02));
			df_dyH[1][3] = 
				-2*SQR(m2)*l1*l2*l2*l2*sin02*y3/(SQR(l1)*(m1+m2)*SQR(l2)*m2-
				SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02));
			
			df_dyH[3][1] = 
				2*l1*l1*l1*(m1+m2)*m2*l2*sin02*y1/
				(SQR(l1)*(m1+m2)*SQR(l2)*m2-
				SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02));
			df_dyH[3][3] = 
				2*SQR(m2)*SQR(l1)*SQR(l2)*cos02*sin02*y3/
				(SQR(l1)*(m1+m2)*SQR(l2)*m2-
				SQR(m2)*SQR(l1)*SQR(l2)*SQR(cos02));
			
			for(int i=0; i<4; i++)
			{
				if(fabs(df_dyH[1][i]-df_dy[1][i])>1e-14)
				{  fprintf(stderr,"OOPS: [1][%d]\n",i);  }
				if(fabs(df_dyH[3][i]-df_dy[3][i])>1e-14)
				{  fprintf(stderr,"OOPS: [3][%d]\n",i);  }
			}
			#endif
		}
		
		// [overriding virtual]
		void calcyscale(double xn,const double *yn,
			const double *dy_dx,double h,double *yscale)
		{  NUM::ODESolverDriver::DefaultYScale(dim,xn,yn,dy_dx,h,yscale);  }
	public:
		MyODEFunc() : Function_ODE(4) {}
		~MyODEFunc() {}
};

//------------------------------------------------------------------------------

class MainWindow : public QWidget
{
	private:
		XPainter *black;
		XPainter *rgb[3];  // red, green, blue
		
		NUM::ODESolver odesolve[3];
		NUM::Function_ODE *odefunc[3];
		
		int prev_xco[3],prev_yco[3];
		
		int steps_per_timer;
		
		int run[3];
		bool running;
		int iter[3];
		bool automatic;
		
		void _Redraw();
		
		void timerEvent(QTimerEvent *);
		void keyPressEvent(QKeyEvent *);
		void mousePressEvent(QMouseEvent *);
		void paintEvent(QPaintEvent *);
	public:
		MainWindow(QWidget *parent=NULL,const char *name=NULL);
		~MainWindow();
};

void MainWindow::_Redraw()
{
}

void MainWindow::mousePressEvent(QMouseEvent * /*ev*/)
{
	//int x0=ev->x();
	//int x1=ev->y();
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
	switch(ev->key())
	{
		case Key_Return:
		case Key_Enter:  break;
		case Key_Q:  close();  break;
		case Key_H:
			printf(
				"  h  print this HELP\n"
				"  q  QUIT\n"
				"  p  PAUSE\n"
				"  t  print TIME and iterations\n"
				" rgb run RED GREEN BLUE algo (only if not automatic)\n"
				"  a  switch on/off AUTOMATIC mode\n"
				"  s  single STEP\n"
				" 1-9 set number of steps per timer to 2^0..2^8\n");
			break;
		case Key_P:
			if(running)  killTimers();
			else  startTimer(10);
			running=!running;
			break;
		case Key_S:
			timerEvent(NULL);
			break;
		case Key_R:  run[0]=!run[0];  break;
		case Key_G:  run[1]=!run[1];  break;
		case Key_B:  run[2]=!run[2];  break;
		case Key_T:
			for(int c=0; c<3; c++)
			{
				fprintf(stderr,"%s time=%g, iter=%d, h=%.2g, evals=%d <- %s\n",
					c==0 ? "RED:  " : (c==1 ? "GREEN:" : "BLUE: "),
					odesolve[c].xn,
					iter[c],
					odesolve[c].h,
					odesolve[c].GetODEFunc().ncalls,
					odesolve[c].GetSolver()->DrvName().str());
			}
			break;
		case Key_1:  steps_per_timer=1;  break;
		case Key_2:  steps_per_timer=2;  break;
		case Key_3:  steps_per_timer=4;  break;
		case Key_4:  steps_per_timer=8;  break;
		case Key_5:  steps_per_timer=16;  break;
		case Key_6:  steps_per_timer=32;  break;
		case Key_7:  steps_per_timer=64;  break;
		case Key_8:  steps_per_timer=128;  break;
		case Key_9:  steps_per_timer=256;  break;
		case Key_A:  automatic=!automatic;  break;
	}
}

void MainWindow::paintEvent(QPaintEvent *)
{
	_Redraw();
}

void MainWindow::timerEvent(QTimerEvent *)
{
	for(int it=0; it<steps_per_timer*(automatic ? 3 : 1); it++)
	{
		bool do_run[3];
		
		if(automatic)
		{
			double xmin,xmax;
			int cmin=-1,cmax=-1;
			bool first=1;
			for(int c=0; c<3; c++)
			{
				if(!run[c]) continue;
				if(first || xmin>odesolve[c].xn)
				{  xmin=odesolve[c].xn;  cmin=c;  }
				if(first || xmax<odesolve[c].xn)
				{  xmax=odesolve[c].xn;  cmax=c;  }
				first=0;
			}
			for(int c=0; c<3; c++)
			{
				do_run[c]=c==cmin;
			}
		}
		else for(int c=0; c<3; c++)
			do_run[c]=run[c];
		
		int xco[3],yco[3];
		for(int c=0; c<3; c++)
		{
			if(!do_run[c])  continue;
			
			NUM::ODESolver::StepState ss;
			//ss=odesolve[c].Step();
			ss=odesolve[c].Step();
			++iter[c];
			//printf("[%d] ss=%d, %g  (iter=%d)\n",
			//	c,ss,odesolve[c].h,iter[c]);
			assert(ss==NUM::ODESolver::SS_OK);
			
			double phi=odesolve[c].yn[0];
			double psi=odesolve[c].yn[2];
			xco[c]=-int(l1*sin(phi)+l2*sin(psi) +0.5);
			yco[c]=-int(l1*cos(phi)+l2*cos(psi) +0.5);
			
			if(iter[c]>1)
			{  rgb[c]->DrawLine(
				250+prev_xco[c],250-prev_yco[c],
				250+xco[c],250-yco[c]);  }
			
			prev_xco[c]=xco[c];
			prev_yco[c]=yco[c];
			
			/*rgb[c]->FillArc(
				250-40+int(30*cos(2*M_PI*c/3)),
				250-40+int(30*sin(2*M_PI*c/3)),
				80,80);*/
		}
	}
}


MainWindow::MainWindow(QWidget *parent,const char *name) : 
	QWidget(parent,name)
{
	QPainter pnt(this);
	pnt.setBrush(QBrush(Qt::black,Qt::SolidPattern));
	black=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::red,Qt::SolidPattern));
	rgb[0]=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::green,Qt::SolidPattern));
	rgb[1]=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::blue,Qt::SolidPattern));
	rgb[2]=new XPainter(&pnt,true);
	assert(black && rgb[0] && rgb[1] && rgb[2]);
	for(int c=0; c<3; c++)
		rgb[c]->SetFunction(GXand/*Inverted*/);
	
	//odesolve[0].SetSolver(
	//	new NUM::ODESolver_RungeKutta(new NUM::ODE_RK_Stepper_RungeKutta));
	odesolve[0].SetSolver(
		new NUM::ODESolver_RungeKutta_Fehlberg456());
	odesolve[1].SetSolver(
		new NUM::ODESolver_BulirschStoer(
			NUM::ODESolver_BulirschStoer::M_Explicit | 
			NUM::ODESolver_BulirschStoer::S_Deuflhard | 
			NUM::ODESolver_BulirschStoer::E_Rational,
			/*kmax=*/10));
	odesolve[2].SetSolver(
		new NUM::ODESolver_BulirschStoer(
			NUM::ODESolver_BulirschStoer::M_SemiImplicit | 
			NUM::ODESolver_BulirschStoer::E_Polynomial,
			/*kmax=*/8));
	
	// Start conditions: 
	// This about the best we can get. 
	double epsilon[3]={ /*4e-16,*/ 2e-16, 2e-16, 5e-17 };
	for(int c=0; c<3; c++)
	{
		// The different solvers could share one ODE function pointer/ref 
		// but I want them separate for the eval counter. 
		odefunc[c]=new MyODEFunc();
		assert(odefunc[c]);  // ...and all the others; we're sloppy here...
		
		double y0[4]=
		{
			0+0.7*M_PI/1.0/*+c*1e-12*/,
			0,
			0+0.6*M_PI/1.0,
			0
		};
		
		int rv=odesolve[c].Init(*(odefunc[c]),0,y0,0.01,epsilon[c],
			/*need_yscale=*/1);
		assert(!rv);
		
		run[c]=1;
		iter[c]=0;
	}
	
	resize(500,500);
	move(100,100);
	
	setBackgroundColor(Qt::white);
	setCaption("ODE test");
	
	show();
	
	startTimer(10);
	running=1;
	
	steps_per_timer=1;
	automatic=1;
}


MainWindow::~MainWindow()
{
	delete black;
	for(int c=0; c<3; c++)
	{
		delete odefunc[c];
		delete rgb[c];
	}
}


char *prg_name="odetest";

int main(int argc,char **arg)
{
	QApplication qapp(argc,arg);
	qapp.setStyle(new QCDEStyle(TRUE));
	//qapp.setStyle(new QSGIStyle(TRUE));
	
	MainWindow mainwin(NULL);
	
	qapp.setMainWidget(&mainwin);
	
	int r=qapp.exec();
	return(r);
}
