INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src
noinst_LIBRARIES = libobjdesc.a

libobjdesc_a_SOURCES = \
	omatrix.h omatrix.cc \
	curve.h curve.cc \
	move.h move.cc \
	curve_fixed.h curve_fixed.cc \
	curve_splinebase.h curve_splinebase.cc \
	curve_lspline.h curve_lspline.cc \
	curve_cspline.h curve_cspline.cc \
	curvetmap_none.h curvetmap_none.cc \
	curvetmap_linear.h curvetmap_linear.cc \
	frontspec.h frontspec.cc \
	frontspec_speed.h frontspec_speed.cc \
	upspec.h upspec.cc \
	upspec_gravity.h upspec_gravity.cc \
	valhist.h valhist.cc

#check_PROGRAMS = test
#test_SOURCES = test.cc
#test_LDADD = \
#	$(top_builddir)/hlib/libhlib.a $(LIBS) \
#	libpovcore.a \
#	../pov-scan/libpovscanner.a \
#	../ani-parser/libaniparser.a \
#	../core/libavcore.a \
#	../sourcepos/libsourcepos.a \
#	../valtype/libvaltype.a \
#	$(top_builddir)/hlib/libhlib.a \
#	libpovcore.a 
#
#check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
#	./test$(EXEEXT) $(srcdir)/../pov-scan/test.pov

xFORCE:
