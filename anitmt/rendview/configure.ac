# Process this file with autoconf to produce a configure script.
AC_INIT
AM_INIT_AUTOMAKE(rendview,0.3.8)
AC_CONFIG_SRCDIR([src/rendview.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for header files. HLIB already does a lot of these checks, some 
# are repeated here. 
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h])

# Checks for typedefs, structures, and compiler characteristics.
# Most of that stuff is already checked in HLIB and in hlib's config. 
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
# FIXME: missing
AC_CHECK_FUNCS([sysconf])


# Check if socket is in -lsocket: 
AC_CHECK_LIB([socket],[socket])

# Finally write files: 
AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/lib/Makefile \
	src/imgfmt/Makefile \
	src/tdriver/Makefile \
	src/tdriver/ldr/Makefile \
	src/tdriver/local/Makefile \
	src/tdriver/render/Makefile src/tdriver/render/driver/Makefile \
	src/tdriver/filter/Makefile src/tdriver/filter/driver/Makefile \
	src/tsource/Makefile \
	src/tsource/local/Makefile src/tsource/ldr/Makefile src/tsource/nrp/Makefile \
	cfg/Makefile])
AC_OUTPUT
