# Process this file with autoconf to produce a configure script.
AC_INIT
AM_INIT_AUTOMAKE(rendview,0.3)
AC_CONFIG_SRCDIR([src/rendview.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
# FIXME: more missing

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
# FIXME: missing

AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/lib/Makefile \
	src/imgfmt/Makefile \
	src/render/Makefile src/render/driver/Makefile \
	src/filter/Makefile src/filter/driver/Makefile \
	src/tsource/Makefile src/tsource/local/Makefile src/tsource/nrp/Makefile \
	cfg/Makefile])
AC_OUTPUT
