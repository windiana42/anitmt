INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
noinst_LIBRARIES = libadminshell.a
bin_PROGRAMS = rvadminshell
LIBS = -lm -lreadline

libadminshell_a_SOURCES = \
	hreadline.h hreadline.cc \
	adminshell.hpp adminshell.cpp

rvadminshell_SOURCES = main.cpp

rvadminshell_LDADD = \
	$(top_builddir)/hlib/libhlib.a \
	../lib/libsupport.a \
	libadminshell.a \
	../admin/libadmin.a \
	../lib/libsupport.a \
	$(top_builddir)/hlib/libhlib.a 	
