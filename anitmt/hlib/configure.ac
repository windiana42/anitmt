# Process this file with autoconf to produce a configure script.
# If you have trouble, use the latest versions. 
# Things work well with autoconf-2.53 and automake-1.6. 
AC_INIT(hlib,0.9.9,[<see apropriate header file>])

# Make sure people don't use stoneage versions of autoconf: 
#AC_PREREQ(2.53)

# Check system: 
# Compute the canonical build-system type variable, $build, 
# and its three individual parts $build_cpu, $build_vendor, and $build_os. 
AC_CANONICAL_BUILD
# Compute the canonical host-system type variable, $host, 
# and its three individual parts $host_cpu, $host_vendor, and $host_os. 
AC_CANONICAL_HOST
# Compute the canonical target-system type variable, $target, 
# and its three individual parts $target_cpu, $target_vendor, and $target_os. 
AC_CANONICAL_TARGET

# Basic setup: 
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([cpp/cplusplusff.cc])
AM_CONFIG_HEADER(hlib-config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX

# For --enable and --with options: 
#AC_ARG_ENABLE(feature,AC_HELP_STRING([--enable-feature],[help text]),
#              action-if-given,action-if-not-given)
#AC_ARG_WITH(package,AC_HELP_STRING([--with-package],[help text]),
#            action-if-given,action-if-not-given)
# Where $enableval contains value in --enable-feature=VAL which is "yes" 
# for --enable-feature and "no" for --disable-feature. 

# Optimization C / C++ compiler flags: 
AC_SUBST(OPTCFLAGS)
AC_SUBST(OPTCXXFLAGS)

AC_ARG_VAR(OPTCFLAGS,[Optimization C compiler flags])
AC_ARG_VAR(OPTCXXFLAGS,[Optimization C++ compiler flags])

opt_cflags_were_set="y"
test "x$OPTCFLAGS" = "x" && opt_cflags_were_set="n"
opt_cxxflags_were_set="y"
test "x$OPTCXXFLAGS" = "x" && opt_cxxflags_were_set="n"

HLIB_CXXCOMPILER_VERSION=""
hlibcfg_final_add_cflags=""

# Check gcc version if gcc: 
dnl check gcc version:
# GCC = "yes" for GNU C; GXX = "yes" for GNU C++
if test "$GXX" = "yes" ; then
	HLIB_CXXCOMPILER_VERSION="GCC:"
	
	# We're actually using gcc. 
	if test "x$HLIB_CONFIGURE_NO_PIPE" != "xno" ; then
		AC_MSG_NOTICE([Passing -pipe to compiler.])
		AC_MSG_NOTICE([Use HLIB_CONFIGURE_NO_PIPE=no env var to disable.])
		CC="$CC -pipe"
		CXX="$CXX -pipe"
	fi
	
	# Add warning flags: 
	c_warnflags="-W -Wall -Wformat -Winline"
	cxx_warnflags="$c_warnflags -Wreorder -Woverloaded-virtual"
	CFLAGS="$CFLAGS $c_warnflags"
	OPTCFLAGS="$OPTCFLAGS $c_warnflags"
	CXXFLAGS="$CXXFLAGS $cxx_warnflags"
	OPTCXXFLAGS="$OPTCXXFLAGS $cxx_warnflags"
	
	AC_MSG_CHECKING([checking GNU C++ version])
	#gccvers="$($CXX -v 2>&1 | fgrep "version")"
	#gccvers="${gccvers/"*version "/}"
	gccvers="$($CXX -v 2>&1 | fgrep "version" | fgrep -v "onfigure" | sed -e "s/.*version //")"
	AC_MSG_RESULT([$gccvers])
	
	HLIB_CXXCOMPILER_VERSION="$HLIB_CXXCOMPILER_VERSION $gccvers"
	
	# gcc_optflg_prefix prefix for -malign-jumps <-> -falign-jumps
	#                               (gcc < 3.1)       (gcc >= 3.1)
	gcc_optflg_prefix="-f"
	if test "${gccvers:0:1}" -lt 3 ; then 
		gcc_optflg_prefix="-m"
	else
		# gcc 3.0 or above
		# Pass message length. 
		# NOTE: This works if you "export COLUMNS" in the shell before 
		#       running configure and make. 
		if test "x$COLUMNS" != "x" ; then
			hlibcfg_final_add_cflags="-fmessage-length=\$\$COLUMNS"
		else
			AC_MSG_NOTICE([You may export COLUMNS and re-run configure if you wish that])
			AC_MSG_NOTICE([-fmessage-length= is passed to the compiler.])
		fi
		
		if test "${gccvers:0:3}" = "3.0" ; then 
			gcc_optflg_prefix="-m"
		fi
	fi
	
	# On GNU systems...
	case "$target" in
		*-gnu)
			CFLAGS="$CFLAGS -D_GNU_SOURCE"
			OPTCFLAGS="$OPTCFLAGS -D_GNU_SOURCE"
			CXXFLAGS="$CXXFLAGS -D_GNU_SOURCE"
			OPTCXXFLAGS="$OPTCXXFLAGS -D_GNU_SOURCE"
		;;
	esac
	
	# Generic optimization flags: 
	myOPTFLAGS="-Os -fomit-frame-pointer -DHLIB_SIZE_OPT=1"
	# Additional flags for C / C++ only: 
	myOPTCFLAGS=""
	myOPTCXXFLAGS="-fno-rtti -fno-exceptions"
	
	# Special optimization flags per target type: 
	case "$target" in
		i[[3456]]86-*-linux* | i[[3456]]86-*-freebsd* | i[[3456]]86-*-cygwin*)
			for i in align-jumps=2 align-loops=2 align-functions=2 ; do
				myOPTFLAGS="$myOPTFLAGS ""$gcc_optflg_prefix""$i"
			done
			myOPTFLAGS="$myOPTFLAGS -march=$target_cpu"
			;;
		alpha-*-linux*)
			;;
		ppc-*-linux*)
			;;
		sparc-*-sunos*)
			;;
		*)
			AC_MSG_NOTICE([Happy porting... :)])
			;;
	esac
	
	myOPTCFLAGS="$myOPTFLAGS $myOPTCFLAGS"
	myOPTCXXFLAGS="$myOPTFLAGS $myOPTCXXFLAGS"
	
	AC_MSG_NOTICE([Autodetect optimization: C: $myOPTCFLAGS])
	AC_MSG_NOTICE([Autodetect optimization: C++: $myOPTCXXFLAGS])
	
	# Note: OPTCFLAGS/OPTCXXFLAGS currently contain the warnings, even 
	#       if not set. 
	test "$opt_cflags_were_set" = "n" && OPTCFLAGS="$OPTCFLAGS $myOPTCFLAGS"
	test "$opt_cxxflags_were_set" = "n" && OPTCXXFLAGS="$OPTCXXFLAGS $myOPTCXXFLAGS"
	
	#echo "--------------"
	#echo "$OPTCFLAGS"
	#echo "$OPTCXXFLAGS"
	#echo "--------------"
	
	unset gcc_optflg_prefix
	unset gccvers
else  # not GCC
	HLIB_CXXCOMPILER_VERSION="GCC: no"
	
	if test "x$OPTCFLAGS" = "x" ; then
		for i in $CFLAGS ; do 
			test "$i" != "-g" && OPTCFLAGS="$OPTCFLAGS $i"
		done
	fi
	if test "x$OPTCXXFLAGS" = "x" ; then
		for i in $CXXFLAGS ; do 
			test "$i" != "-g" && OPTCXXFLAGS="$OPTCXXFLAGS $i"
		done
	fi
fi

AC_MSG_NOTICE([OPTCFLAGS = $OPTCFLAGS])
AC_MSG_NOTICE([OPTCXXFLAGS = $OPTCXXFLAGS])

# Rest of the program checks: 
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_RANLIB

# AC_DEFINE(HAVE_XYZ,[value],[description])

# Some information compiled into the library: 
AC_DEFINE_UNQUOTED(HLIB_BUILD_SYSTEM,["$build"],
	[canonical build-system type])
AC_DEFINE_UNQUOTED(HLIB_HOST_SYSTEM,["$host"],
	[canonical host-system type])
AC_DEFINE_UNQUOTED(HLIB_TARGET_SYSTEM,["$target"],
	[canonical target-system type])
AC_DEFINE_UNQUOTED(HLIB_BUILD_UNAME,["`uname -a || echo unknown`"],
	[uname -a of system hlib was built on])
AC_DEFINE_UNQUOTED(HLIB_BUILD_DATE,["`date || echo unknown`"],
	[When hlib was built])
AC_DEFINE_UNQUOTED(HLIB_CXXCOMPILER_VERSION,["$HLIB_CXXCOMPILER_VERSION"],
	[C++ compiler hlib was built with])

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

# AC_CHECK_SIZEOF(...)  AC_C_BIGENDIAN()  

AC_CHECK_TYPES([u_int8_t,u_int16_t,u_int32_t,u_int64_t,int8_t,int16_t,int32_t,int64_t],,
	[AC_MSG_WARN([Oh... if you're not using gcc, expect trouble.])])

AC_CHECK_TYPES([ssize_t],,
	[AC_MSG_WARN([You don't have ssize_t. Using signed int. (grmbl...)])])

AC_CHECK_TYPES([socklen_t],,
	[AC_MSG_WARN([No socklen_t. Uh...])],
	[ #if HAVE_SYS_TYPES_H
	 # include <sys/types.h>
	 #endif
	 #if HAVE_SYS_SOCKET_H
	 # include <sys/socket.h>
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
CC="$CC $hlibcfg_final_add_cflags"
CXX="$CXX $hlibcfg_final_add_cflags"

AC_CONFIG_FILES([include/Makefile \
	include/hlib/Makefile \
	cpp/Makefile \
	fd/Makefile \
	par/Makefile \
	misc/Makefile \
	crypto/Makefile \
	Makefile])

# Finally do it: 
AC_OUTPUT

# AutoHeader stuff to append: 
# AH_BOTTOM([#include xyz])
