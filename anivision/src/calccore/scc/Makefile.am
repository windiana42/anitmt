INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include -I$(top_builddir)/src
noinst_LIBRARIES = libccsimple.a

libccsimple_a_SOURCES = \
	curve_function.h curve_function.cc \
	ccif_simple.h basefunc.cc \
	factory.h factory.cc \
	scc_parse.h scc_parse.cc \
	scc_token.h scc_token.cc \
	setting.cc object.cc objexec.cc value.cc

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
