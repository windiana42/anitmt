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

#include "cspline.h"
#include "lspline.h"
#include "xspline.h"
#include "aspline.h"

// gcc -O2 qt-test.cc test.cc -o ./test -fno-rtti -fno-exceptions -I. -I../.. -I../../../../hlib-build/include/ libqtxlib.a -lqt-mt -lm -L/opt/Qt/lib ../../../../hlib-build/libhlib.a ../../../../anivision-build/src/numerics/libnumerics.a ../../../../anivision-build/src/numerics/spline/libnumerics_spline.a ../../../../anivision-build/src/numerics/diff_int/libnumerics_diff_int.a ../../../../anivision-build/src/numerics/linalg/libnumerics_linalg.a

// In test.cc: 
extern void DOTEST(NUM::SplineBase *spline,QTX::XPainter *pnt);


using namespace QTX;

struct POINT
{
	int x0,x1;
};

class MainWindow : public QWidget
{
	private:
		XPainter *black,*blue,*red,*green;
		NUM::SplineBase *spline;
		
		int npnt,dimpnt;
		POINT *pnt;
		
		int use_tvals;
		int xtplot;
		
		void _Redraw(int update_caption=0);
		void _SwitchDegree(int deg);
		
		void timerEvent(QTimerEvent *);
		void keyPressEvent(QKeyEvent *);
		void mousePressEvent(QMouseEvent *);
		void paintEvent(QPaintEvent *);
	public:
		MainWindow(QWidget *parent=NULL,const char *name=NULL);
		~MainWindow();
};

int my_intmcp(const void *a,const void *b)
{
	return(((POINT*)a)->x0-((POINT*)b)->x0);
}

void MainWindow::_Redraw(int update_caption)
{
	erase();
	int winh=size().height();
	
	for(int i=0; i<npnt; i++)
	{  black->FillRectangle(pnt[i].x0-1,pnt[i].x1-1,3,3);  }
	black->DrawLine(0,winh/2,size().width(),winh/2);
	
	if(update_caption)
	{
		char tmp[64];
		snprintf(tmp,64,"Spline: %s(%d) %s",
			spline->SplineProperties()->name,
			npnt,xtplot ? "X,T" : "X,Y");
		setCaption(tmp);
	}
	
	if(npnt<2)  return;
	
	if(xtplot)
	{
		qsort(pnt,npnt,sizeof(POINT),&my_intmcp);
		NUM::VectorArray<double> yarr(npnt,1);
		double tvals[npnt];
		for(int i=0; i<npnt; i++)
		{
			tvals[i]=pnt[i].x0;
			yarr[i][0]=pnt[i].x1;
		}
		double paramA=-1.1,paramB=0.03;
		
		int rv;
		switch(spline->SplineProperties()->splinetype)
		{
			case NUM::SplineBase::ST_CSpline:
				rv=((NUM::CubicPolySpline*)spline)->Create(yarr,tvals,
					//NUM::SBC_MomentumA|NUM::SBC_MomentumB,&paramA,&paramB,NULL);
					//NUM::SBC_SlopeA|NUM::SBC_SlopeB,&paramA,&paramB,NULL);
					//NUM::SBC_MomentumA|NUM::SBC_SlopeB,&paramA,&paramB,NULL);
					NUM::SBC_SlopeB|NUM::SBC_MomentumB,&paramA,&paramB);
					//--NUM::SBC_Periodic,NULL,NULL);
				break;
			case NUM::SplineBase::ST_LSpline:
				rv=((NUM::LinearSpline*)spline)->Create(yarr,tvals);
				break;
			case NUM::SplineBase::ST_XSpline:
				rv=((NUM::X_Spline*)spline)->Create(yarr,tvals,NULL);
				break;
			case NUM::SplineBase::ST_ASpline:
				rv=((NUM::AkimaSpline*)spline)->Create(yarr,tvals);
				break;
			default: assert(0);
		}
		if(rv)
		{  fprintf(stderr,"spline->Create()=%d\n",rv);  }
		double tmp;
		double start=spline->GetT(0);
		double end=spline->GetT(spline->N());
		for(double t=start; t<end; t+=0.0001*(end-start))
		{
			spline->Eval(t,&tmp);
			int x=int(t+0.5);
			int y=int(tmp+0.5);
			black->DrawPoint(x,y);
			spline->EvalD(t,&tmp);
			y=int(30.0*tmp+0.5)+winh/2;
			red->DrawPoint(x,y);
			spline->EvalDD(t,&tmp);
			y=int(3000*tmp+0.5)+winh/2;
			blue->DrawPoint(x,y);
		}
		{
			double sA,sB,mA,mB;
			spline->EvalD(start,&sA);   spline->EvalD(end,&sB);
			spline->EvalDD(start,&mA);  spline->EvalDD(end,&mB);
			fprintf(stderr,"slope=%g,%g\t\tmomentum=%g,%g\n",sA,sB,mA,mB);
		}	
	}
	else
	{
		int npnt=this->npnt;
		//++npnt;   // <-- For periodic only!
		NUM::VectorArray<double> arr(npnt,2);
		for(int i=0; i<npnt; i++)
		{
			arr[i][0]=pnt[i % this->npnt].x0;
			arr[i][1]=pnt[i % this->npnt].x1;
		}
		double tvals[npnt];
	#if 0
		srand(0);
		tvals[0]=-double(rand()%10000)/1000.0;
		for(int i=1; i<npnt; i++)
		{  tvals[i]=tvals[i-1]+double(rand()%10000)/1000.0+0.5;  }
	#else
		for(int i=0; i<npnt; i++)
		{  tvals[i]=20*(i-10);  }
	#endif
		double slopeA[2]={0,0},slopeB[2]={use_tvals ? 50 : 1000,0};
		int rv;
		switch(spline->SplineProperties()->splinetype)
		{
			case NUM::SplineBase::ST_CSpline:
				rv=((NUM::CubicPolySpline*)spline)->Create(
					arr,use_tvals ? (use_tvals==2 ? tvals : NUM::SplineBase::TVALS_DIST) : NULL,
					NUM::SBC_MomentumA|NUM::SBC_MomentumB,NULL,NULL);
					//NUM::SBC_Hermite,slopeA,slopeB);
					//--NUM::SBC_Periodic,NULL,NULL);
				break;
			case NUM::SplineBase::ST_LSpline:
				rv=((NUM::LinearSpline*)spline)->Create(
					arr,use_tvals ? (use_tvals==2 ? tvals : NUM::SplineBase::TVALS_DIST) : NULL);
				break;
			case NUM::SplineBase::ST_XSpline:
				rv=((NUM::X_Spline*)spline)->Create(
					arr,use_tvals ? (use_tvals==2 ? tvals : NUM::SplineBase::TVALS_DIST) : NULL,
					NULL);
				break;
			case NUM::SplineBase::ST_ASpline:
				rv=((NUM::AkimaSpline*)spline)->Create(
					arr,use_tvals ? (use_tvals==2 ? tvals : NUM::SplineBase::TVALS_DIST) : NULL);
				break;
			default: assert(0);
		}
		if(rv)
		{  fprintf(stderr,"spline->Create()=%d\n",rv);  }

		double tmp[2];
		double start=spline->GetT(0);
		double end=spline->GetT(spline->N());
		for(double t=start; t<end; t+=0.0001*(end-start))
		{
			spline->Eval(t,tmp);
			int x=int(tmp[0]+0.5);
			int y=int(tmp[1]+0.5);
			black->DrawPoint(x,y);
		}
	}
}


void MainWindow::_SwitchDegree(int deg)
{
	switch(deg)
	{
		case 1:
			delete spline;
			spline=new NUM::LinearSpline();
			break;
		case 2:
			delete spline;
			spline=new NUM::X_Spline();
			break;
		case 3:
			delete spline;
			spline=new NUM::CubicPolySpline();
			break;
		case 4:
			delete spline;
			spline=new NUM::AkimaSpline();
			break;
	}
	
	_Redraw(1);
}


void MainWindow::mousePressEvent(QMouseEvent *ev)
{
	int x0=ev->x();
	int x1=ev->y();
	
	if(npnt>=dimpnt)
	{
		dimpnt+=16;
		pnt=(POINT*)realloc(pnt,sizeof(POINT)*dimpnt);
	}
	if(ev->button()==LeftButton)
	{
		pnt[npnt].x0=x0;
		pnt[npnt].x1=x1;
		++npnt;
	}
	else if(ev->button()==RightButton)
	{
		memmove(pnt+1,pnt,npnt*sizeof(POINT));
		pnt[0].x0=x0;
		pnt[0].x1=x1;
		++npnt;
	}
	
	_Redraw(1);
}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
	switch(ev->key())
	{
		case Key_Return:
		case Key_Enter:  break;
		case Key_Q:  close();  break;
		case Key_R:  npnt=0; _Redraw(1);  break;
		case Key_T:
			use_tvals=(use_tvals+1)%3;
			fprintf(stderr,"use_tvals=%s\n",
				use_tvals ? (use_tvals==2 ? "random/const inc" : "distance") : "none");
			_Redraw(1);
			break;
		case Key_X:
			xtplot=!xtplot;
			_Redraw(1);
			break;
		case Key_1:  _SwitchDegree(1);  break;
		case Key_2:  _SwitchDegree(2);  break;
		case Key_3:  _SwitchDegree(3);  break;
		case Key_4:  _SwitchDegree(4);  break;
		case Key_H:
			printf(
				"  h -> help"
				"  q -> quit\n"
				"  r -> reset (clear points)\n"
				"  t -> use t values\n"
				"  x -> toggle X,T <-> X,Y plot\n"
				"1,3 -> switch degree, 2 -> xspline\n"
				"  c -> trigger test function (calc)\n");
			break;
		case Key_C:
			DOTEST(spline,green);
			break;
	}
}

void MainWindow::paintEvent(QPaintEvent *)
{
	_Redraw();
}

void MainWindow::timerEvent(QTimerEvent *)
{
	
}


MainWindow::MainWindow(QWidget *parent,const char *name) : 
	QWidget(parent,name)
{
	npnt=dimpnt=0;
	pnt=NULL;
	
	use_tvals=1;
	xtplot=0;
	
	QPainter pnt(this);
	pnt.setBrush(QBrush(Qt::black,Qt::SolidPattern));
	black=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::blue,Qt::SolidPattern));
	blue=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::red,Qt::SolidPattern));
	red=new XPainter(&pnt,true);
	pnt.setBrush(QBrush(Qt::darkGreen,Qt::SolidPattern));
	green=new XPainter(&pnt,true);
	assert(black && blue && red && green);
	
	spline=new NUM::CubicPolySpline();
	
	resize(500,500);
	move(100,100);
	
	setBackgroundColor(Qt::white);
	setCaption("Spline (c) Wolfgang Wieser");
	
	show();
}


MainWindow::~MainWindow()
{
	delete spline;
	
	delete black;
	delete blue;
	delete red;
	delete green;
	if(pnt)
	{  free(pnt);  }
}


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

