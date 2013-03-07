# Process this file with autoconf to produce a configure script.
AC_INIT(anitmt, 0.2.3, anitmt@theofel.de)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([anitmt-calc/anitmt-calc.cpp])
AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
#AC_PROG_RANLIB obsolete because of PROG_LIBTOOL
AC_PROG_YACC
AM_PROG_LIBTOOL
#AC_PROG_LEX
AM_PROG_LEX	# replaced for automake

# Check flex version if flex is needed. 
need_flex="n"
for i in anitmt-calc/input/adl2/scanner anitmt-calc/funcgen/scanner ; do
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
      2.5.[[2-9]][[0-9]])  fstatus="y" ;;
      2.5.17|2.5.18|2.5.19)  fstatus="y" ;;
    esac
    if test "$fstatus" = "n" ; then
      AC_MSG_ERROR([Your version of flex ($flexvers) is outdated.
You need at least flex-2.5.17. Grab it at ftp://ftp.uncg.edu/people/wlestes.])
      exit 1
    fi
    unset fstatus flexvers
  else
    AC_MSG_WARN([*** You are not using flex but "$LEX". Expect trouble. ***])
  fi
fi
unset need_flex

# Check for gcc version less than 3.0: 
GCC_LESS_THAN_3="n/a"
gccvers="$($CXX -v 2>&1 | tail -1)"
if test "${gccvers/" *"/}" = "gcc" ; then
  # We're actually using gcc, so check version
  # this test is not perfect as it can be tricked using "gcc -Vxyz"
  GCC_LESS_THAN_3="no"
  echo -n "checking $CXX version... "
  gccvers="${gccvers/"*version "/}"
  echo "$gccvers"
  if test "${gccvers:0:1}" -lt 3 ; then 
    AC_MSG_NOTICE([You are using a gcc version less than \
3.0 which has some bugs with templates.])
    AC_MSG_NOTICE([** Activated Workaround **])
	AC_DEFINE(GCC_HACK,,[Workaround for busgs in gcc<3.0])
	GCC_LESS_THAN_3="yes"
  fi
fi
unset gccvers

echo CXXFLAGS = $CXXFLAGS
echo debug_Enabled = "$debug_Enabled"

# Debugging?
yydebug_Enabled="yes"
debug_Enabled="noopt" # default option
AC_ARG_ENABLE(debug,AC_HELP_STRING([--enable-debug=VAL],
	[VAL=noopt: build with debugging info and don't pass optimization flags (default); 
	 VAL=yes: build with debugging info (slower and larger); 
	 VAL=no: no debugging info; 
	 VAL=fast: additionally do not pass optimization flags]),
	[debug_Enabled="$enableval"])
if test "$debug_Enabled" = "noopt" ; then
	# don't optimize when debugging is enabled
	CXXFLAGS=" $CXXFLAGS "
	CXXFLAGS="${CXXFLAGS// -O? / }"
	CXXFLAGS="${CXXFLAGS// -O / }"
	case " $CXXFLAGS " in
		*" -g "*) ;;
		*) CXXFLAGS="$CXXFLAGS -g" ;;
	esac
elif test "$debug_Enabled" = "yes" ; then
	case " $CXXFLAGS " in
		*" -g "*) ;;
		*) CXXFLAGS="$CXXFLAGS -g" ;;
	esac
elif test "$debug_Enabled" = "no"; then
	CXXFLAGS=" $CXXFLAGS "
	CXXFLAGS="${CXXFLAGS// -g / }"
	yydebug_Enabled="no"
elif test "$debug_Enabled" = "fast" ; then
	CXXFLAGS=" $CXXFLAGS "
	CXXFLAGS="${CXXFLAGS// -g / }"
	CXXFLAGS="${CXXFLAGS// -O / }"
	CXXFLAGS="${CXXFLAGS// -O? / }"
	yydebug_Enabled="no"
fi

echo CXXFLAGS = $CXXFLAGS
echo debug_Enabled = "$debug_Enabled"

# YYDEBUG switch
AC_ARG_ENABLE(yydebug,AC_HELP_STRING([--enable-yydebug],
	[build bison parsers with YYDEBUG turned on (default=yes)]),
	[yydebug_Enabled="$enableval"])
if test "$yydebug_Enabled" = "no" ; then
	yydebug_Enabled="0"
else 
	yydebug_Enabled="1"
fi
AC_DEFINE_UNQUOTED(YYDEBUG,[$yydebug_Enabled],[Bison parser debugging])

# See if -lm is needed: 
AC_CHECK_LIB([m],[sqrt])

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lefence':
#AC_CHECK_LIB([efence], [main])
# FIXME: Replace `main' with a function in `-lstdc':
#AC_CHECK_LIB([stdc], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h limits.h malloc.h memory.h stddef.h stdlib.h stdarg.h string.h])
AC_CHECK_HEADERS([strings.h sys/time.h unistd.h inttypes.h stdint.h sys/stat.h])
AC_CHECK_HEADERS([memory.h inttypes.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_HEADER_TIME

AC_CHECK_TYPES([ssize_t],,\
	[AC_MSG_WARN([You don't have ssize_t. Using signed int. (grmbl...)])])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STAT
#AC_FUNC_STRTOD
AC_CHECK_FUNCS([dup2 gettimeofday stat])
AC_CHECK_FUNCS([memmove memset strcasecmp strchr strerror strrchr strtol strtoul])
AC_CHECK_FUNCS([setenv])

AC_CONFIG_FILES([ Makefile
	lib/Makefile
	lib/flex/Makefile
	lib/message/Makefile
	lib/par/Makefile
	lib/param/Makefile
	lib/proptree/Makefile
	lib/solve/Makefile
	lib/utl/Makefile
	lib/val/Makefile
	lib/hlib/Makefile
	lib/hlib/par/Makefile
	lib/hlib/cpp/Makefile
	lib/hlib/misc/Makefile
	anitmt-calc/Makefile
	anitmt-calc/funcgen/Makefile
	anitmt-calc/functionality/Makefile
	anitmt-calc/input/Makefile
	anitmt-calc/input/adl2/Makefile
	anitmt-calc/output/Makefile
	anitmt-calc/output/lib/Makefile
	anitmt-calc/output/pov/Makefile
	anitmt-calc/output/raw/Makefile
	utils/Makefile 
	utils/mkavi/Makefile 
	utils/tga2avi/Makefile
	example/Makefile
	example/test/Makefile
	example/test/test_ani/Makefile
	example/myrobot/Makefile
	example/myrobot/ani/Makefile
	example/circles/Makefile 
	example/circles/ani/Makefile ])
AC_OUTPUT

if test "$GCC_LESS_THAN_3" = "yes" ; then
	case " $CXXFLAGS " in
		*" -O"?" "*|*" -O "*)
			echo 
			echo "You are using a 2.x version of gcc and your CXXFLAGS"
			echo "contain optimization flags (-O2 etc.)."
			echo "BEWARE: Builting anitmt takes _ages_. Better upgrade to gcc-3.x or"
			echo "        do not pass optimization flags (try --enable-debug=fast)."
			echo "If unsure what to do simply re-configure using --enable-debug=fast."
			echo 
			;;
	esac
fi
