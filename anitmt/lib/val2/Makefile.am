INCLUDES = -I$(srcdir)/..
# CXX = gcc -V3.0 -I/usr/include/g++-3/ 
CXXFLAGS = @CXXFLAGS@ -Wno-non-template-friend #-fmessage-length=$$COLUMNS
noinst_LIBRARIES = libval.a
libval_a_SOURCES = internals.hpp \
	imatrix.cpp imatrix.hpp \
	ivector.cpp ivector.hpp \
	flag.hpp \
	scalar.hpp \
	vector.cpp vector.hpp \
	matrix.cpp matrix.hpp \
	string.hpp \
	support.cpp support.hpp \
	val.hpp 

check_PROGRAMS = test_val
test_val_SOURCES = test_val.cpp
test_val_LDADD = libval.a -lstdc++ -lm

EXTRA_DIST = README
