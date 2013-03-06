# Process this file with autoconf to produce a configure script.
AC_INIT
AM_INIT_AUTOMAKE(rendview,0.7.2)
AC_CONFIG_SRCDIR([src/rendview.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_RANLIB

# Okay; we need hlib, so check for hlib-config: 
hlib_build_dir="<unset>"
AC_ARG_WITH(hlib,AC_HELP_STRING([--with-hlib],[path to hlib build directory]),
	[if test "$withval" = "no" ; then 
		AC_MSG_ERROR([Cannot switch off use of hlib, RendView depends on it.])
	elif test "$withval" != "yes"; then
		hlib_build_dir="$withval" 
	fi ])
AC_MSG_CHECKING([for hlib])
# anitmt uses a different way to search for a preinstalled hlib library; Consider using the same
# or combining both approaches
hlib_config_script=hlib_config
hlib_LDADD=""
if which hlib_config > /dev/null 2>&1; then
	hlib_LDADD="-lhlib"
else
	checkdirs="./hlib ./hlib-* ../hlib ../hlib-*"
	if test "$hlib_build_dir" != "<unset>" ; then
		checkdirs="$hlib_build_dir $hlib_build_dir/hlib $hlib_build_dir/.."
	fi
	hlibversion="none"
	for i in $checkdirs ; do
	  echo $i/HConfigVars
	  cat $i/HConfigVars
	  test -x "$i/HConfigVars" || continue
	  hlibversion="$($i/HConfigVars VERSION)"
	  case "x$hlibversion" in
	    x1.1.1)  hlibdir="$i" ; break ;;
	  esac
	done
	if test "x$hlibdir" = "x" ; then
		AC_MSG_RESULT([failed])
		AC_MSG_ERROR([Unable to find hlib (need: 1.1.1, found: $hlibversion). Giving up.])
	else
		AC_MSG_RESULT([$hlibdir, version $hlibversion])
	fi
	
	#AC_SUBST(hlibdir)
	if test "$hlibdir" != "./hlib" -a "$hlibdir" != "hlib" ; then
		test -e "hlib" && rm "hlib"
		ln -s "$hlibdir" "hlib"
	fi

	hlib_config_script="$hlibdir/HConfigVars"
	hlib_LDADD="$hlibdir/libhlib.a"
fi

# Checks for header files. HLIB already does a lot of these checks, some 
# are repeated here. 
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h nan.h ieeefp.h])

# Checks for typedefs, structures, and compiler characteristics.
# Most of that stuff is already checked in HLIB and in hlib's config. 
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
# Check for CPU detection using sysconf: 
AC_CHECK_FUNCS([sysconf])
AC_MSG_CHECKING([for _SC_NPROCESSORS_ONLN/CONF])
  AC_TRY_COMPILE([#include <unistd.h>
	#include <stdlib.h>],
	[long n=sysconf(_SC_NPROCESSORS_ONLN);
	n=sysconf(_SC_NPROCESSORS_CONF); ],
	[AC_MSG_RESULT([yes])
	 AC_DEFINE(RENDVIEW_HAVE_SYSCONF_NPROCESSORS,1,\
	 	[Defined, if _SC_NPROCESSORS_ONLN and ~_CONF was found])],
	[AC_MSG_RESULT([no])
	 AC_MSG_NOTICE([It seems you are lacking _SC_NPROCESSORS_ONLN/CONF. Please check that.])
])


# If needed: Check if pow,log are in -lm: 
AC_CHECK_LIB([m],[pow])
AC_CHECK_LIB([m],[log])

# Check if socket is in -lsocket: 
AC_CHECK_LIB([socket],[socket])

# Check for readline: 
AC_CHECK_HEADERS([readline/readline.h readline/history.h],,
	[AC_MSG_ERROR([You are lacking GNU readline devel files. Please install GNU readline devel package.])])

# FrameClockVal: UNUSED -> no isnan(), isinf() needed any longer
#tmp_found_any="no"
#AC_CHECK_FUNCS([isnand isnan fpclass],[tmp_found_any="yes" ; break])
#if test $tmp_found_any = "no" ; then
#	AC_MSG_ERROR([No isnan() found. Giving up.])
#fi
#
#tmp_found_any="no"
#AC_CHECK_FUNCS([isinfd isinf fpclass],[tmp_found_any="yes" ; break])
#if test $tmp_found_any = "no" ; then
#	AC_MSG_ERROR([No isnan() found. Giving up.])
#fi

# FrameClockVal: UNUSED -> no finite() needed any longer
# AC_CHECK_FUNCS([finite],,[AC_MSG_ERROR([No finite() found. Giving up.])])

# FIXME: more checks for lib functions missing

# Set hlib's CFLAGS and CXXFLAGS: 
# This must be done at the end bcause of $(ADDFLAGS). 
CFLAGS="$($hlib_config_script CFLAGS)"
CXXFLAGS="$($hlib_config_script CXXFLAGS)"
CC="$($hlib_config_script CC)"
CXX="$($hlib_config_script CXX)"
export LIBS="$LIBS $hlib_LDADD";
echo "CC = $CC"
echo "CXX = $CXX"
echo "CFLAGS = $CFLAGS"
echo "CXXFLAGS = $CXXFLAGS"
echo "LIBS = $LIBS"

# Finally write files: 
AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/lib/Makefile \
	src/admin/Makefile \
	src/imgfmt/Makefile \
	src/tdriver/Makefile \
	src/tdriver/ldr/Makefile \
	src/tdriver/local/Makefile \
	src/tdriver/render/Makefile src/tdriver/render/driver/Makefile \
	src/tdriver/filter/Makefile src/tdriver/filter/driver/Makefile \
	src/tsource/Makefile \
	src/tsource/local/Makefile src/tsource/ldr/Makefile \
	src/adminshell/Makefile \
	doc/Makefile \
	cfg/Makefile])
AC_OUTPUT
