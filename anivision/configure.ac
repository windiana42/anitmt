# Process this file with autoconf to produce a configure script.
AC_INIT
AM_INIT_AUTOMAKE(anivision,0.3.5a)
AC_CONFIG_SRCDIR([src/valtype/valtypes.h])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_LEX
AC_PROG_YACC

# Check flex: 
need_flex="n"
for i in src/ani-parser/ani_scanner src/pov-scan/pov_scanner ; do
  if test -f "$srcdir/$i.ll" -a ! -f "$srcdir/$i.cc" ; then
    need_flex="y"
    break
  fi
done
if test "$need_flex" = "y" ; then
  if test "x$LEX" = "xflex" ; then
    AC_MSG_CHECKING([$LEX version])
    flexvers="$($LEX --version)"
    flexvers="${flexvers/flex /}"
    flexvers="${flexvers/version /}"
    AC_MSG_RESULT([$flexvers])
    fstatus="n"
    case "$flexvers" in
      [[3-9]].*)  fstatus="y" ;;
      2.[[1-4]].*)  fstatus="n" ;;
      2.5.[[4-9]]*)  fstatus="y" ;;
      2.5.3[[1-9]])  fstatus="y" ;;
    esac
    if test "$fstatus" = "n" ; then
      AC_MSG_ERROR([Your version of flex ($flexvers) is outdated.
You need at least flex-2.5.31. Grab it at ftp://ftp.uncg.edu/people/wlestes.])
      exit 1
    fi
    unset fstatus flexvers
  else
    AC_MSG_ERROR([*** You need flex and not "$LEX". Get version flex-2.5.31 or higher. ***])
  fi
fi
unset need_flex

# Check bison:
need_bison="n"
if test -f "$srcdir/src/ani-parser/grammar.yy" ; then
  if test ! -f "$srcdir/src/ani-parser/grammar.cc" -a ! -f "$srcdir/src/ani-parser/grammar.hh" ; then
    need_bison="y"
  fi
fi
if test "$need_bison" = "y" ; then
  case "$YACC" in
    *"bison"*" -y")  YACC=${YACC/ -y/} ;;
  esac
  case "$YACC" in
    *"bison"*) ;;
    *) AC_MSG_ERROR([*** You need bison, not $YACC. ***]) ;;
  esac
fi
unset need_bison

# Okay; we need hlib, so check for hlib-config: 
hlib_build_dir="<unset>"
AC_ARG_WITH(hlib,AC_HELP_STRING([--with-hlib],[path to hlib build directory]),
	[if test "$withval" = "no" ; then 
		AC_MSG_ERROR([Cannot switch off use of hlib, AniVision depends on it.])
	elif test "$withval" != "yes"; then
		hlib_build_dir="$withval" 
	fi ])
AC_MSG_CHECKING([for hlib])
hlibdir=""
checkdirs="./hlib ./hlib-* ../hlib ../hlib-*"
if test "$hlib_build_dir" != "<unset>" ; then
	checkdirs="$hlib_build_dir $hlib_build_dir/hlib $hlib_build_dir/.."
fi
hlibversion="none"
for i in $checkdirs ; do
  test -x "$i/HConfigVars" || continue
  hlibversion="$($i/HConfigVars VERSION)"
  case "x$hlibversion" in
    x1.0.0)  hlibdir="$i" ; break ;;
  esac
done
if test "x$hlibdir" = "x" ; then
	AC_MSG_RESULT([failed])
	AC_MSG_ERROR([Unable to find hlib (need: 1.0.0, found: $hlibversion). Giving up.])
else
	AC_MSG_RESULT([$hlibdir, version $hlibversion])
fi

if test "$hlibdir" != "./hlib" -a "$hlibdir" != "hlib" ; then
	test -e "hlib" && rm "hlib"
	ln -s "$hlibdir" "hlib"
fi

# Set hlib's compiler: 
CC="$($hlibdir/HConfigVars CC)"
CXX="$($hlibdir/HConfigVars CXX)"

# Checks for header files. HLIB already does a lot of these checks, some 
# are repeated here. 
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
# Most of that stuff is already checked in HLIB and in hlib's config. 
AC_C_CONST
AC_C_INLINE

# If needed: Check if pow,log are in -lm: 
AC_CHECK_LIB([m],[pow])
AC_CHECK_LIB([m],[log])




# Set hlib's CFLAGS and CXXFLAGS: 
# This must be done at the end bcause of $(ADDFLAGS). 
CFLAGS="$($hlibdir/HConfigVars CFLAGS)"
CXXFLAGS="$($hlibdir/HConfigVars CXXFLAGS)"
echo "CFLAGS = $CFLAGS"
echo "CXXFLAGS = $CXXFLAGS"

# Finally write files: 
AC_CONFIG_FILES([ \
	Makefile \
	src/Makefile \
	src/core/Makefile \
	src/numerics/Makefile \
	src/numerics/linalg/Makefile \
	src/numerics/diff_int/Makefile \
	src/numerics/interpolate/Makefile \
	src/numerics/spline/Makefile \
	src/numerics/ode/Makefile \
	src/numerics/fit/Makefile \
	src/objdesc/Makefile \
	src/depgraph/Makefile \
	src/calccore/Makefile \
	src/calccore/scc/Makefile \
	src/calccore/anitmt/Makefile \
	src/ani-parser/Makefile \
	src/pov-scan/Makefile \
	src/pov-core/Makefile \
	src/sourcepos/Makefile \
	src/valtype/Makefile])
AC_OUTPUT
