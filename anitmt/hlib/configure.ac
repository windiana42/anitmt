# Process this file with autoconf to produce a configure script.
# If you have trouble, use the latest versions. 
# Things work well with autoconf-2.53 and automake-1.6. 
AC_INIT(hlib,0.9.6,[<see apropriate header file>])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([cpp/cplusplusff.cc])
AM_CONFIG_HEADER(hlib-config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CXX
AC_PROG_RANLIB

AC_DEFINE_UNQUOTED(HLIB_BUILD_SYSTEM,["`uname -a || echo unknown`"],\
	[System hlib was built on])
AC_DEFINE_UNQUOTED(HLIB_BUILD_DATE,["`date || echo unknown`"],\
	[When hlib was built])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h stdarg.h unistd.h])
AC_CHECK_HEADERS([memory.h inttypes.h stdint.h])
AC_CHECK_HEADERS([errno.h fcntl.h limits.h signal.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/poll.h sys/socket.h \
	sys/time.h sys/wait.h sys/resource.h sys/uio.h sys/stat.h])
AC_CHECK_HEADERS([netinet/ip_compat.h])

AC_CHECK_HEADERS([gmp.h], \
	[we_detected_gmp="yes"],\
	[AC_MSG_NOTICE([You are lacking GMP. Some functionality will not be available.])
	 we_detected_gmp="no"])
AM_CONDITIONAL(HAVE_GMP, test x$we_detected_gmp = xyes)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_HEADER_TIOCGWINSZ
AC_HEADER_SYS_WAIT

AC_CHECK_TYPES([u_int8_t,u_int16_t,u_int32_t,u_int64_t,int8_t,int16_t,int32_t,int64_t],,
	[AC_MSG_WARN([Oh... if you're not using gcc, expect trouble.])])

AC_CHECK_TYPES([ssize_t],,
	[AC_MSG_WARN([You don't have ssize_t. Using signed int. (grmbl...)])])

AC_CHECK_TYPES([socklen_t],,
	[AC_MSG_WARN([No socklen_t. Uh...])],
	[#if HAVE_SYS_SOCKET_H
	 # include <sys/socket.h>
	 #endif
	 #if HAVE_SYS_TYPES_H
	 # include <sys/types.h>
	 #endif] )

AC_CHECK_TYPES([siginfo_t],,
	[AC_MSG_WARN([Providing crippled siginfo_t replacement.])],
	[#include <signal.h>
	 #include <sys/signal.h>] )

AC_CHECK_TYPES([struct pollfd],,
	[AC_MSG_WARN([Providing struct pollfd. You seem to lack poll(2)...])],
	[#include <sys/poll.h>] )

## AC_CHECK_MEMBERS([struct stat.st_blksize])

# Check for libraries.

# Check if socket is in -lsocket: 
AC_CHECK_LIB([socket],[socket])

# If needed: Check if sqrt is in -lm: 
#AC_CHECK_LIB([m],[sqrt])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
# AC_FUNC_MALLOC not needed, just checks 
#                If the malloc works correctly (`malloc (0)' returns 
#                a valid pointer)
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_FORK
AC_FUNC_STRTOD
AC_FUNC_GETLOADAVG

AC_CHECK_FUNCS([execve pipe sigaction dup2 gettimeofday kill strerror],,
	[AC_MSG_ERROR([Oh no. Giving up. What crippled architecture are you using?!])])

AC_CHECK_FUNCS([nice setuid setgid setsid chroot],,
	[AC_MSG_ERROR(Sorry, you lack some useful functions.)])

AC_CHECK_FUNCS([malloc_usable_size],,
	[AC_MSG_WARN([Allocation limitation (LMalloc) will not work.])],
	[#include <malloc.h>])

AC_CHECK_FUNCS([poll],,
	[AC_MSG_WARN([Urgs! Your crippled architecture does not have poll(2). 
Hlib will use an emulation using select(2). Note that this emulation is 
slower and cannot provide a poll(2)-replacement. 
NOTE: Applications may not work as expected. 
      Complain at your vendor for not implementing poll(2).])],
	[#include <sys/poll.h>])

# Finnally, check if CLD_EXITED is defined, otherwise all that 
# siginfo_t stuff is useless for ProcessManager. 
AC_MSG_CHECKING([for CLD_EXITED])
  AC_TRY_COMPILE([#include <signal.h>
	#include <sys/signal.h>],
	[siginfo_t x;
	 if(x.si_code==CLD_EXITED);],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_MSG_NOTICE([You are lacking CLD_EXITED. Not using it...])
	 AC_DEFINE(HLIB_PROCMAN_USE_LESS_SIGINFO_T,,[Defined, if CLD_EXITED was not found])])

# Finally write files: 
AC_CONFIG_FILES([include/Makefile \
	include/hlib/Makefile \
	cpp/Makefile \
	fd/Makefile \
	par/Makefile \
	misc/Makefile \
	crypto/Makefile \
	Makefile])
AC_OUTPUT
