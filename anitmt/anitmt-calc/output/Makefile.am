INCLUDES = 		-I$(srcdir)/../../lib/ -I$(srcdir)/../ -I../ \
			-I../functionality -I$(srcdir)/../functionality
SUBDIRS = 		lib raw pov
noinst_LTLIBRARIES =	liboutput.la
AM_CXXFLAGS = 		-Wno-non-template-friend
liboutput_la_SOURCES =	output.hpp output.cpp oformats.hpp
liboutput_la_LIBADD = 	raw/librawout.la \
			pov/libpovout.la \
			lib/liboutpututl.la 

