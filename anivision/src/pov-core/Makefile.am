INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src
noinst_LIBRARIES = libpovcore.a

libpovcore_a_SOURCES = \
	povtree.h povtree.cc \
	povparser.h povparser.cc \
	objectspec.h objectspec.cc \
	povwriter.h povwriter.cc

check_PROGRAMS = test
test_SOURCES = test.cc
test_LDADD = \
	$(top_builddir)/hlib/libhlib.a $(LIBS) \
	libpovcore.a \
	../pov-scan/libpovscanner.a \
	../ani-parser/libaniparser.a \
	../core/libavcore.a \
	../calccore/libcalccore.a \
	../sourcepos/libsourcepos.a \
	../valtype/libvaltype.a \
	$(top_builddir)/hlib/libhlib.a \
	libpovcore.a 

check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	./test$(EXEEXT) $(srcdir)/../pov-scan/test.pov

xFORCE:
