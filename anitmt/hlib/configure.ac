# Process this file with autoconf to produce a configure script.
AC_INIT
AM_INIT_AUTOMAKE(hlib,0.9.1)
AC_CONFIG_SRCDIR([cpp/cplusplusff.cc])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h errno.h fcntl.h limits.h stddef.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h sys/wait.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_WAIT3
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memmove memset setenv socket sqrt strcasecmp strchr strdup strerror strrchr strtod strtol strtoul])

AC_CONFIG_FILES([include/Makefile \
	include/hlib/Makefile \
	cpp/Makefile \
	fd/Makefile \
	par/Makefile \
	misc/Makefile \
	Makefile])
AC_OUTPUT
