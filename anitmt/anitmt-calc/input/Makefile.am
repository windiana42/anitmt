INCLUDES = 		-I$(srcdir)/../../lib/ -I$(srcdir)/../ -I../ \
			-I../functionality -I$(srcdir)/../functionality
SUBDIRS = 		adl2
AM_CXXFLAGS = 		-Wno-non-template-friend
noinst_LTLIBRARIES =	libinput.la
libinput_la_SOURCES =	input.hpp input.cpp
libinput_la_LIBADD =	adl2/libadlparser.la 
