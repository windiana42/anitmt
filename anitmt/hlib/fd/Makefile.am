INCLUDES = -I../include/
noinst_LIBRARIES = libfd.a
libfd_a_SOURCES = \
	htime.cc htimeprint.cc htimecalc.cc \
		fdbase.cc fdmanager.cc \
		procmanager.cc intprocbase.cc \
	htime.h \
		fdmanager.h fdbase.h \
		procmanager.h intprocbase.h procbase.h \
		cpbase.h cpmanager.h
#		cpbase.cc cpmanager.cc 

check_PROGRAMS = check_fd
check_fd_SOURCES = check_fd.cc
check_fd_LDADD = ../libhlib.a $(LIBS) -lm

check: check_fd ../libhlib.a  xFORCE
	./check_fd

xFORCE:

