INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
noinst_LIBRARIES = libsupport.a
libsupport_a_SOURCES = \
	prototypes.hpp taskmanagement.h \
	error.cpp \
	get_ncpus.cpp get_load.cpp \
	file.cpp \
	misc.cpp mymath.hpp \
	myaddrinfo.hpp myaddrinfo.cpp myaddrinfo2.cpp \
	ldrproto.hpp ldrproto.cpp ldrproto2.cpp \
	netiobase.hpp netiobase.cpp \
	netiobase_ldr.hpp netiobase_ldr.cpp \
	cmdqueue.hpp cmdqueue.cpp \
	sha1hash.cpp sha1hash.hpp \
	statcache.cpp statcache.hpp

check_PROGRAMS = test_ldrproto
test_ldrproto_SOURCES = test_ldrproto.cpp
test_ldrproto_LDADD = \
	$(top_builddir)/hlib/libhlib.a \
	libsupport.a \
	$(top_builddir)/hlib/libhlib.a

check: test_ldrproto$(EXEEXT) libsupport.a xFORCE
	./test_ldrproto$(EXEEXT)

xFORCE:
