# Process this file with autoconf to produce a configure script.
AC_INIT(anitmt, 0.2.0, anitmt@theofel.de)
AC_CONFIG_SRCDIR([anitmt-calc/anitmt-calc.cpp])

AM_INIT_AUTOMAKE(anitmt, 0.2.0)
AM_CONFIG_HEADER(config.h)	# replaced for automake

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
#AC_PROG_LEX
AM_PROG_LEX	# replaced for automake
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_PROG_YACC

# check for gcc version less than 3.0
dnl check gcc version:
gccvers="`$CXX -v 2>&1 | tail -1`"
if test "${gccvers/" *"/}" = "gcc" ; then
  # we're actually using gcc, so check version
  # this test is not perfect as it can be tricked using "gcc -Vxyz"
  echo -n "checking $CXX version... "
  gccvers="${gccvers/"*version "/}"
  echo "$gccvers"
  if test "${gccvers:0:1}" -lt 3 ; then 
    echo You are using a gcc version less than 3.0 which has some bugs with templates
    echo "** Activated Workaround **"
    export CXXFLAGS="$CXXFLAGS -fno-access-control -DGCC_HACK"
  fi
fi
unset gccvers

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lefence':
AC_CHECK_LIB([efence], [main])
# FIXME: Replace `main' with a function in `-lm':
#AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lstdc':
#AC_CHECK_LIB([stdc], [main])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([alloca.h errno.h fcntl.h limits.h malloc.h memory.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_WAIT3
AC_CHECK_FUNCS([dup2 gettimeofday memmove memset pow setenv sqrt strcasecmp strchr strerror strrchr strtol strtoul])

AC_CONFIG_SUBDIRS(hlib)
AC_CONFIG_FILES([Makefile
                 anitmt-calc/Makefile
                 anitmt-calc/funcgen/Makefile
                 anitmt-calc/funcgen/test/Makefile
                 anitmt-calc/functionality/Makefile
                 anitmt-calc/input/Makefile
                 anitmt-calc/input/adl2/Makefile
                 anitmt-calc/output/Makefile
                 anitmt-calc/output/lib/Makefile
                 anitmt-calc/output/pov/Makefile
                 anitmt-calc/output/raw/Makefile
                 anitmt-view/Makefile
                 anitmt-view/parser/Makefile
                 lib/Makefile
                 lib/flex/Makefile
                 lib/message/Makefile
                 lib/par/Makefile
                 lib/param/Makefile
                 lib/proptree/Makefile
                 lib/solve/Makefile
                 lib/utl/Makefile
                 lib/val/Makefile
                 lib/val2/Makefile
                 mkavi/Makefile])
AC_OUTPUT