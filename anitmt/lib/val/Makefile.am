INCLUDES = -I$(srcdir)/..
AM_CXXFLAGS = -Wno-non-template-friend #-fmessage-length=$$COLUMNS
noinst_LTLIBRARIES = libval.la
libval_la_SOURCES = internals.hpp \
	imatrix.cpp imatrix.hpp \
	ivector.cpp ivector.hpp \
	flag.hpp flag.cpp \
	neutral.hpp neutral.cpp \
	scalar.hpp \
	vector.cpp vector.hpp \
	matrix.cpp matrix.hpp \
	string.hpp \
	support.cpp support.hpp \
	val.hpp 
check_PROGRAMS = test_val
TESTS = $(check_PROGRAMS)
test_val_SOURCES = test_val.cpp
test_val_LDADD = libval.la 

EXTRA_DIST = README
