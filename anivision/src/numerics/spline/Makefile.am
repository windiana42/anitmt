INCLUDES = -I$(top_srcdir)/src/ -I$(top_srcdir)/src/numerics \
	-I$(top_builddir)/hlib/include -I..
noinst_LIBRARIES = libnumerics_spline.a

libnumerics_spline_a_SOURCES = \
	sl_base.cc sl_base.h \
	cspline.cc cspline.h \
	lspline.cc lspline.h \
	xspline.cc xspline.h \
	aspline.cc aspline.h

check_PROGRAMS = qt-test
qt_test_SOURCES = qt-test.cc test.cc
qt_test_LDADD = \
	$(top_builddir)/hlib/libhlib.a $(LIBS) \
	$(srcdir)/libqtxlib.a -lqt-mt -L/opt/Qt/lib \
	libnumerics_spline.a \
	../libnumerics.a \
	../diff_int/libnumerics_diff_int.a \
	../linalg/libnumerics_linalg.a \
	$(top_builddir)/hlib/libhlib.a

check: qt-test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	./qt-test$(EXEEXT)

xFORCE:

