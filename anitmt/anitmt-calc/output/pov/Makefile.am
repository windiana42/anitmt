INCLUDES = 		-I$(srcdir)/../ -I$(srcdir)/../../ -I../../ \
			-I$(srcdir)/../lib/ -I$(srcdir)/../../../lib/ \
			-I$(builddir)/../../functionality -I$(srcdir)/../../functionality
noinst_LIBRARIES = 	libpovout.a
CXXFLAGS = 		@CXXFLAGS@ -Wno-non-template-friend
libpovout_a_SOURCES = 	pov.hpp pov.cpp \
			fdump.cpp fdump.hpp \
			findstr.cpp findstr.hpp \
			parse.cpp parse.hpp \
			tokid.hpp

