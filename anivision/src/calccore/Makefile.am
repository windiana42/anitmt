INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src \
	-I$(top_builddir)/anitmt-src/lib \
	-I$(top_builddir)/anitmt-src/anitmt-calc/functionality \
	-I$(top_builddir)/anitmt-build/anitmt-calc/functionality \
	-I$(top_builddir)/anitmt-src/anitmt-calc \
	-I$(top_builddir)/anitmt-build/anitmt-calc

noinst_LIBRARIES = libcalccore.a

SUBDIRS = scc anitmt

libcalccore_a_SOURCES = \
	ccif.h ccif.cc factory.cc \
	sctparse.h sctparse.cc \
	adtree.h adtree.cc adteval.cc

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
