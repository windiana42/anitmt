INCLUDES = -I../include/
noinst_LIBRARIES = libfd.a
libfd_a_SOURCES = \
	htime.cc htimeprint.cc htimecalc.cc \
		fdbase.cc fdmanager.cc \
		procmanager.cc intprocbase.cc procstring.cc \
		cpbase.cc cpmanager.cc \
		timeoutmanager.cc \
	htime.h \
		fdmanager.h fdbase.h \
		procmanager.h intprocbase.h procbase.h \
		cpbase.h cpmanager.h \
		timeoutbase.h timeoutmanager.h

EXTRA_DIST = depend.txt

check_PROGRAMS = check_fd
check_fd_SOURCES = check_fd.cc
check_fd_LDADD = ../libhlib.a $(LIBS) 

check: check_fd$(EXEEXT) ../libhlib.a  xFORCE
	./check_fd$(EXEEXT)

xFORCE:

