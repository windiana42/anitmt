INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
noinst_LIBRARIES = libvaltype.a
libvaltype_a_SOURCES = \
	tostring.cc	valalloc.cc	valtype.cc valtypes.h \
	vtinteger.h \
	vtmatrix.cc vtmatrix.h \
	vtrange.h vtrange.cc \
	vtscalar.h \
	vtstring.cc vtstring.h \
	vtvector.h

check_PROGRAMS = test
test_SOURCES = test.cc
test_LDADD = libvaltype.a $(top_builddir)/hlib/libhlib.a $(LIBS) 

check: test$(EXEEXT) $(top_builddir)/hlib/libhlib.a xFORCE
	./test$(EXEEXT)

xFORCE:
