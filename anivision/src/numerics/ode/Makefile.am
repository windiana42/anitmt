INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src
noinst_LIBRARIES = libnumerics_ode.a

EXTRA_DIST = odetest.cc

libnumerics_ode_a_SOURCES = \
	odesolver.h odesolver.cc \
	odedrv_rk.h odedrv_rk.cc oderk_stp.cc \
	odedrv_bs.h odedrv_bs.cc \
	odedrv_rkf456.h odedrv_rkf456.cc
