#CFLAGS = @CFLAGS@ -W -Wall -D_GNU_SOURCE
#CXXFLAGS = @CXXFLAGS@ -W -Wall -D_GNU_SOURCE

INCLUDES = -I../include/
noinst_LIBRARIES = libmisc.a

libmisc_a_SOURCES = \
	hlib_id0.c hlib_id1.c \
		checkmalloc.c limitmalloc.c \
		installsighandler.c \
		llongstr.c ullongstr.c \
		memfind.c \
		msecelapsed.c msecelapsedr.c \
		nonblock.c tcpnodelay.c sockerror.c reuseaddr.c \
		getprgname.c \
		gettermsize.c \
		getloadavg.c \
	prototypes.h hlib.h lmalloc.h

check_PROGRAMS = check_misc
check_misc_SOURCES = check_misc.cc
check_misc_LDADD = ../libhlib.a $(LIBS)

check: check_misc$(EXEEXT) ../libhlib.a  xFORCE
	./check_misc$(EXEEXT)

xFORCE:

