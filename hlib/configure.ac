# Compiling with make: 
# Frequently used targets: 
#   make                 -> compile
#   make all             -> compile
#   make check           -> compile (& run) check programs
#   make install         -> perform installation
#   make clean           -> delete compiler output
#   make distclean       -> delete everything unnecessary
#   make uninstall       -> remove installed files
#   make dist            -> create dist tarball
#   make distcheck       -> do "make dist", then unpack dist archive in test 
#                           directory, compile it, run "make check" and then 
#                           install it in a temporarary directory. 
# Installation in /tmp/test-inst: 
#   make DESTDIR=/tmp/test-inst install
# Run all the necessary autotools: 
#   autoreconf [-i]

# Process this file with autoconf to produce a configure script.
# If you have trouble, use the latest versions. 
# Things work well with autoconf-2.53 and automake-1.6. 
AC_INIT(hlib,1.1.0,[<see apropriate header file>])
# You can pass options to AM_INIT_AUTOMAKE: 
# AM_INIT_AUTOMAKE([dist-bzip2,...]
#  dist-bzip2 -> create bzip2-compressed dist tarball. 
#  gnu, foreign -> set strictness level
# These options can be passed on a per-Makefile basis by setting the 
# AUTOMAKE_OPTIONS variable in the associated Makefile.am. 

# Make sure that make rules for re-config when changing configure.ac or 
# Makefile.am are not created (and hence that no autotools are run by 
# make) automatically unless --enable-maintainer-mode is passed. 
# AM_MAINTAINER_MODE		

# Make sure people don't use stoneage versions of autoconf: 
#AC_PREREQ(2.53)

# Replacement functions (strncasecmp(),...)
# -> see AC_REPLACE_FUNCS([strncasecmp]), ++ @LIBOBJS@ at proggy_LDADD

# A macro to test if compiler accepts flags: 
# Use: GCC_TRY_CFLAGS(COMPILER, CFLAGS, 
#         [ACTION-IF-SUCCESS], [ACTION-IF-FAILED])
# where COMPILER is "C" or "CPP". 
AC_DEFUN([GCC_TRY_CFLAGS],[
	# Compile with $1 and inspect standard error for given flags.
	cfile="none"
	case "$1" in
		c|C)      cfile="conftest.c" ; crun="$CC" ;;
		CPP|cpp)  cfile="conftest.cpp" ; crun="$CXX" ;;
		*)  AC_MSG_ERROR([Illegal compiler "$1" specified.]) ;;
	esac
	AC_MSG_CHECKING([whether $crun accepts $2])
	cgrepfor=""
	cfirst="yes"
	for i in $2 ; do
		i="${i/=*/}"    # remove rvalue in e.g. -malign-jumps=2
		if test $cfirst = "yes" ; then
			cfirst="no"
			cgrepfor="$i"
		else
			cgrepfor="$cgrepfor|$i"
		fi
	done
	echo 'int main(void) { return 0; }' > "$cfile"
	if test -z "$($crun -c $2 $cfile 2>&1 | egrep -e "$cgrepfor")"; then
		ctry_result="yes"
	else
		ctry_result="no"
	fi
	rm -f "$cfile" conftest.o
	AC_MSG_RESULT([$ctry_result])
	
	if test x"$ctry_result" = x"yes" ; then
		ifelse([$3],[],[:],[$3])
	else
		ifelse([$4],[],[:],[$4])
	fi
])

# USAGE: 
#   WW_CHECK_LIBRARY_HEADER(libname,disable-desc-str,header,
#        [ACTION-IF-SUCCESS], [ACTION-IF-FAILED])
#   Also sets libhdr_$1_detected to "yes" or "no" and defines HAVE_$3. 
# example: WW_CHECK_LIBRARY_HEADER(gmp,[GMP-based routines (RSA)],gmp.h)
AC_DEFUN([WW_CHECK_LIBRARY_HEADER],[
	libhdr_$1_detected="yes"
	AC_ARG_ENABLE($1,AC_HELP_STRING([--disable-$1],
		[Disable $2]),
		[libhdr_$1_detected="$enableval"])
	if test x"$libhdr_$1_detected" = xyes ; then
		AC_CHECK_HEADERS([$3], \
			[libhdr_$1_detected="yes"],\
			[AC_MSG_NOTICE([You are lacking $3. Some functionality ($2) will not be available.])
			 libhdr_$1_detected="no"])
	else
		AC_MSG_CHECKING([for $3])
		AC_MSG_RESULT([disbled])
		libhdr_$1_detected="no"
	fi
	
	if test x"$libhdr_$1_detected" = x"yes" ; then
		# The aproptiate define is included automatically. 
		#wwcktmp="$3"
		#wwcktmp="$(echo "$wwcktmp" | tr "[a-z./]" "[A-Z__]")"
		#wwcktmp="HAVE_$wwcktmp"
		#AC_DEFINE($wwcktmp,,"$3 header")
		ifelse([$4],[],[:],[$4])
	else
		ifelse([$5],[],[:],[$5])
	fi
])

# Uses GCC_TRY_CFLAGS for C and C++ and adds flags to 
# hlibcfg_final_add_c[xx]flags if successful. 
AC_DEFUN([C_CPP_TRY_FINAL_FLAGS],[
	GCC_TRY_CFLAGS(C,[$1],
		[hlibcfg_final_add_cflags="$1 $hlibcfg_final_add_cflags"])
	GCC_TRY_CFLAGS(CPP,[$1],
		[hlibcfg_final_add_cxxflags="$1 $hlibcfg_final_add_cxxflags"])
])
# The same for C++ only. 
AC_DEFUN([CPP_TRY_FINAL_FLAGS],[
	GCC_TRY_CFLAGS(CPP,[$1],
		[hlibcfg_final_add_cxxflags="$1 $hlibcfg_final_add_cxxflags"])
])


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

# Called after AC_CANONICAL_BUILD to avoid warnings: 
AM_INIT_AUTOMAKE

# Basic setup: 
AC_CONFIG_SRCDIR([cpp/cplusplusff.cc])
AM_CONFIG_HEADER(hlib-config.h)

# Make sure that we get rid of the horrible default "-g -O2":
# You can use ADDCFLAGS / ADDCXXFLAGS to pass additional flags to make. 
# In order to pass your own optimization flags, use CFLAGS="opt> -flags...". 
origCFLAGS="$CFLAGS"
origCXXFLAGS="$CXXFLAGS"
# For AC_PROG_CC and those, we only remove the "opt> " and "zap> " NOW, 
# because we re-compute the flags complete new below. 
case "$CFLAGS" in
	"opt> "*)  CFLAGS="${CFLAGS:5}" ;;
	"zap> "*)  CFLAGS="${CFLAGS:5}" ;;
esac
case "$CXXFLAGS" in
	"opt> "*)  CXXFLAGS="${CXXFLAGS:5}" ;;
	"zap> "*)  CXXFLAGS="${CXXFLAGS:5}" ;;
esac

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

gccpipe_Enabled="yes"
AC_ARG_ENABLE(gccpipe,AC_HELP_STRING([--disable-gccpipe],
	[do not pass -pipe to gcc, even if supported]),
	[gccpipe_Enabled="$enableval"])

# Optimization C / C++ compiler flags: 
AC_SUBST(DBGCFLAGS)
AC_SUBST(DBGCXXFLAGS)

AC_ARG_VAR(DBGCFLAGS,[C compiler flags for debugging target (make debug)])
AC_ARG_VAR(DBGCXXFLAGS,[C++ compiler flags for debugging targer (make debug)])

test "x$DBGCFLAGS" = "x"   && DBGCFLAGS="-g"
test "x$DBGCXXFLAGS" = "x" && DBGCXXFLAGS="-g"

HLIB_CXXCOMPILER_VERSION=""
hlibcfg_final_add_cflags="\$(ADDFLAGS) \$(ADDCFLAGS)"
hlibcfg_final_add_cxxflags="\$(ADDFLAGS) \$(ADDCXXFLAGS)"

# See what flags can be passed: 
if test "x$gccpipe_Enabled" != "xno" ; then
	C_CPP_TRY_FINAL_FLAGS([-pipe])
fi
# -fno-rtti and -fno-exceptions (C++ only)
CPP_TRY_FINAL_FLAGS([-fno-rtti])
CPP_TRY_FINAL_FLAGS([-fno-exceptions])

# Check gcc version if gcc: 
# GCC = "yes" for GNU C; GXX = "yes" for GNU C++
if test "$GXX" = "yes" ; then
	HLIB_CXXCOMPILER_VERSION="GCC:"
	
	# We use gcc and NOT g++: 
	CXX="${CXX/"g++"/gcc}"
	
	# Make sure that we get rid of the horrible default "-g -O2":
	# See above for more info...
	CFLAGS=""
	CXXFLAGS=""
	
	# We're actually using gcc. 
	if test "x$gccpipe_Enabled" != "xno" ; then
		AC_MSG_NOTICE([Passing -pipe to compiler; use --disable-gccpipe to disable.])
		hlibcfg_final_add_cflags="-pipe $hlibcfg_final_add_cflags"
		hlibcfg_final_add_cxxflags="-pipe $hlibcfg_final_add_cxxflags"
	fi
	
	# Set warning flags: 
	WARNCFLAGS="-W -Wall -Wformat -Winline"
	WARNCXXFLAGS="$WARNCFLAGS -Wreorder -Woverloaded-virtual"
	
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
		if test "${gccvers:0:3}" = "3.0" ; then 
			gcc_optflg_prefix="-m"
		fi
	fi
	
	# On GNU systems...
	case "$target" in
		*-gnu)
			CFLAGS="$CFLAGS -D_GNU_SOURCE"
			CXXFLAGS="$CXXFLAGS -D_GNU_SOURCE"
			DBGCFLAGS="$DBGCFLAGS -D_GNU_SOURCE"
			DBGCXXFLAGS="$DBGCXXFLAGS -D_GNU_SOURCE"
		;;
	esac
	
	# Generic optimization flags: 
	myOPTFLAGS="-Os -fomit-frame-pointer -DHLIB_SIZE_OPT=1"
	GCC_TRY_CFLAGS(C,"-funit-at-a-time",[myOPTFLAGS="$myOPTFLAGS -funit-at-a-time"])
	
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
	
	# Okay, finally set the vars: 
	DBGCFLAGS="$DBGCFLAGS $WARNCFLAGS"
	DBGCXXFLAGS="$DBGCXXFLAGS $WARNCXXFLAGS"
	
	case "$origCFLAGS" in
		"opt> "*) CFLAGS="${origCFLAGS:5} $CFLAGS $WARNCFLAGS" ;;
		"zap> "*) CFLAGS="${origCFLAGS:5}" ;;
		*) CFLAGS="$myOPTCFLAGS $CFLAGS $WARNCFLAGS" ;;
	esac
	case "$origCXXFLAGS" in
		"opt> "*) CXXFLAGS="${origCXXFLAGS:5} $CXXFLAGS $WARNCXXFLAGS" ;;
		"zap> "*) CXXFLAGS="${origCXXFLAGS:5}" ;;
		*) CXXFLAGS="$myOPTCXXFLAGS $CXXFLAGS $WARNCXXFLAGS" ;;
	esac
	
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
	AC_MSG_NOTICE([Not using gcc; not touching any compiler args...])
fi

# Rest of the program checks: 
AC_PROG_LN_S
AC_PROG_INSTALL
AC_PROG_RANLIB

# AC_DEFINE(HAVE_XYZ,[value],[description])

# Create symbol in config.h: 
#   AC_DEFINE_UNQUOTED -> eval second arg in shell
#   AC_DEFINE -> take second arg literally

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

# Set language for subsequent tests. 
# See also: AC_LANG_PUSH(), AC_LANG_POP()
AC_LANG(C++)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stddef.h stdlib.h string.h stdarg.h unistd.h])
AC_CHECK_HEADERS([memory.h inttypes.h stdint.h])
AC_CHECK_HEADERS([errno.h fcntl.h limits.h signal.h])
AC_CHECK_HEADERS([arpa/inet.h])
AC_CHECK_HEADERS([sys/ioctl.h sys/poll.h sys/socket.h \
	sys/time.h sys/wait.h sys/resource.h sys/uio.h sys/stat.h])
AC_CHECK_HEADERS([netinet/ip_compat.h])

WW_CHECK_LIBRARY_HEADER(gmp,[GMP-based routines (RSA crypto)],gmp.h)
WW_CHECK_LIBRARY_HEADER(z,[zlib-based routines (compression)],zlib.h)
WW_CHECK_LIBRARY_HEADER(bz2,[bzlib-based routines (compression)],bzlib.h)
WW_CHECK_LIBRARY_HEADER(png,[libPNG-based routines (PNG image IO)],png.h)


AC_CHECK_HEADERS([mcheck.h])

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

# See if the compiler accepts __attribute__((__const__)) for functions: 
AC_MSG_CHECKING([for attribute:const])
  AC_TRY_COMPILE([],
	[class X {
		static inline void foo() __attribute__((__const__)) {}
	 };],
	[AC_MSG_RESULT([yes])],
	[AC_MSG_RESULT([no])
	 AC_DEFINE(HLIB_ATTRIBUTE_CONST_MISSING,,[Defined, if __attribute__((__const__)) not supported])])

## AC_CHECK_MEMBERS([struct stat.st_blksize])

# Check for libraries.
# AC_CHECK_LIB(lib,function,[action-if-found],[action-if-not-found].[other-libs])

# Check if socket is in -lsocket: 
AC_CHECK_LIB([socket],[socket])

# If needed: Check if sqrt is in -lm: 
#AC_CHECK_LIB([m],[sqrt])

# Check for the GMP library. 
AC_SUBST(HL_GMP_LDFLAGS)
if test "x$libhdr_gmp_detected" = xyes ; then
	AC_MSG_CHECKING([for -lgmp])
	SAVE_LIBS="$LIBS"
	HL_GMP_LDFLAGS="-lgmp"
	LIBS="$LIBS $HL_GMP_LDFLAGS"
	AC_TRY_LINK([#include <gmp.h>],
		[mpz_ptr x;mpz_init(x);],
		[AC_MSG_RESULT([yes ($HL_GMP_LDFLAGS)])],
		[AC_MSG_RESULT([no])
		 AC_MSG_WARN([gmp.h found but -lgmp cannot be linked. Disabling.])
		 HL_GMP_LDFLAGS="" ; libhdr_gmp_detected="no" ])
	LIBS="$SAVE_LIBS"
else
	HL_GMP_LDFLAGS=""
fi
AM_CONDITIONAL(HAVE_GMP, test "x$libhdr_gmp_detected" = xyes)

# Check for libz: 
# No need to check for the library flags yet; header checked above. 
AM_CONDITIONAL(HAVE_LIBZ, test "x$libhdr_z_detected" = "xyes")

# Check for libbz2: 
# No need to check for the library flags yet; header checked above. 
AM_CONDITIONAL(HAVE_LIBBZ2, test "x$libhdr_bz2_detected" = "xyes")

# Check for libpng: 
# No need to check for the library flags yet; header checked above. 
AM_CONDITIONAL(HAVE_LIBPNG, test "x$libhdr_png_detected" = "xyes")

# Checks for library functions.
AC_LANG_PUSH(C)   # <-- Needed because fork and strtod tests will fail with C++. 
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
AC_CHECK_FUNCS([getloadavg])
AC_LANG_POP()    # <-- Switch back to C++ language. 

AC_CHECK_FUNCS([execve pipe sigaction dup2 gettimeofday kill strerror],,
	[AC_MSG_ERROR([Oh no. Giving up. What crippled architecture are you using?!])])

AC_CHECK_FUNCS([nice setuid setgid setsid chroot],,
	[AC_MSG_ERROR(Sorry, you lack some useful functions.)])

AC_CHECK_FUNCS([malloc_usable_size],,
	[AC_MSG_WARN([Allocation limitation (LMalloc) will not work.])],
	[#include <malloc.h>])

AC_CHECK_FUNCS([mcheck mprobe])

AC_CHECK_FUNCS([poll],,
	[AC_MSG_WARN([Urgs! Your crippled architecture does not have poll(2). 
Hlib will use an emulation using select(2). Note that this emulation is 
slower and cannot provide a full poll(2)-replacement. 
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

# Last flags processing: 
CFLAGS="$CFLAGS $hlibcfg_final_add_cflags"
CXXFLAGS="$CXXFLAGS $hlibcfg_final_add_cxxflags"
DBGCFLAGS="$DBGCFLAGS $hlibcfg_final_add_cflags"
DBGCXXFLAGS="$DBGCXXFLAGS $hlibcfg_final_add_cxxflags"
# Also, remove any debugging from CFLAGS/CXXFLAGS
CFLAGS=" $CFLAGS " ; CFLAGS="${CFLAGS// -g / }"
CXXFLAGS=" $CXXFLAGS " ; CXXFLAGS="${CXXFLAGS// -g / }"

AC_MSG_NOTICE([CFLAGS = $CFLAGS])
AC_MSG_NOTICE([CXXFLAGS = $CXXFLAGS])
AC_MSG_NOTICE([DBGCFLAGS = $DBGCFLAGS])
AC_MSG_NOTICE([DBGCXXFLAGS = $DBGCXXFLAGS])


# Finally write files: 
AC_CONFIG_FILES([ \
	include/Makefile \
	include/hlib/Makefile \
	cpp/Makefile \
	fd/Makefile \
	par/Makefile \
	misc/Makefile \
	crypto/Makefile \
	compress/Makefile \
	Makefile])
#AC_CONFIG_FILES([$srcdir/HConfigVars],[chmod ug+x "$srcdir/HConfigVars"])
AC_CONFIG_FILES([HConfigVars],[chmod ug+x "HConfigVars"])

# Finally do it: 
AC_OUTPUT

# AutoHeader stuff to append: 
# AH_BOTTOM([#include xyz])
