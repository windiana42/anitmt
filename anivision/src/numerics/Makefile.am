INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src
SUBDIRS = linalg diff_int interpolate spline ode fit

noinst_LIBRARIES = libnumerics.a

libnumerics_a_SOURCES = config.h \
	num_math.h misc.cc \
	la_basics.h la_basics.cc \
	function.h function.cc

check_PROGRAMS = test
test_SOURCES = test.cc
test_LDADD = \
	$(top_builddir)/hlib/libhlib.a $(LIBS) \
	libnumerics.a \
	$(top_builddir)/hlib/libhlib.a

check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	./test$(EXEEXT)

xFORCE:
