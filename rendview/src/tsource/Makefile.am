SUBDIRS = local ldr 
INCLUDES = -I$(top_srcdir)/src/ -I$(top_builddir)/hlib/include
noinst_LIBRARIES = libtsource.a
libtsource_a_SOURCES = init.cpp \
	taskfile.hpp tfmanager.cpp \
	tasksource.hpp tasksource.cpp tsconsumer.cpp ctask.cpp \
	tsfactory.hpp tsfactory.cpp
