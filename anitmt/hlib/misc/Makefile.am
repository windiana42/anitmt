CFLAGS = @CFLAGS@ -W -Wall -D_GNU_SOURCE
CXXFLAGS = @CXXFLAGS@ -W -Wall -D_GNU_SOURCE

INCLUDES = -I../include/
noinst_LIBRARIES = libmisc.a

libmisc_a_SOURCES = \
	checkmalloc.c limitmalloc.c \
		installsighandler.c \
		llongstr.c ullongstr.c \
		memfind.c \
		msecelapsed.c msecelapsedr.c \
		nonblock.c tcpnodelay.c \
		getprgname.c \
	prototypes.h hlib.h 

