#!/bin/bash
# Automatically calls automake and autoconf the first time if necessary...
# License: LGPL. USE AT YOUR OWN RISK!!
# Copyright: Onno Kortmann
# Needs the unix tools bash, wc and which.
#
# Command line arguments:
# (none) - create files as necessary
# (-f|--force) - recreate all files (a bit like autoreconf, but not really :) 
# (-r|--fix-makefile-rpath) - fix problem in aclocal/automake to allow disabling
#                             rpath in binaries/libraries (needed by non-root rpmbuild)

FORCE=false
RPATH=""

for i in "$@"
do
case $i in
    -f|--force)
    FORCE=true
    ;;
    -r=*|--fix-automake-rpath=*)
    RPATH="${i#*=}"
    shift # past argument=value
    ;;
    *)
    # unknown option
    echo "### WARNING ### $0: unrecognized argument: $i"
    ;;
esac
done

# check for aclocal, autoconf and automake
NEEDED="aclocal autoconf autoheader automake"


echo "You have:"
which $NEEDED

if test `echo $NEEDED|wc -w` != `which $NEEDED|wc -l`; then 
    printf "\nBut you need: $NEEDED.\n";
    echo "Come back later.";
    exit 1;
else
    printf "\nOk. All neccessary tools are available.\n\n";
fi

topdir="`pwd`"
for i in . ; do 
test -d $i || continue
echo "-----<entering directory $i>-----"
cd $i

# Make aclocal.m4
if $FORCE || ! test -f aclocal.m4; then
    echo "Creating aclocal.m4.";
    aclocal || exit 2;
    if test -n "$RPATH"; then
	echo "Fixing rpath issue in aclocal.m4"
	sed -i 's/_LT_TAGVAR(hardcode_libdir_flag_spec, $1)='\''$wl-rpath $wl$libdir/_LT_TAGVAR(hardcode_libdir_flag_spec, $1)='\''/' aclocal.m4
    fi
fi

# Make configure
if $FORCE || ! test -f configure; then
    echo "Creating configure.";
    autoconf || exit 2;
fi

# support for libtool
libtoolize --force --copy || exit 2

# Make config.h.in
if $FORCE || ! test -f config.h.in; then
    echo "Creating config.h.in.";
    autoheader || exit 2
fi

# Make Makefile.in(s)
if $FORCE || ! test -f Makefile.in; then
    echo "Creating Makefile.ins.";
    automake -a || exit 2;
    if test -n "$RPATH"; then
        echo "Fixing rpath issue in Makefile.in files"
        find -name Makefile.in -exec sed -i 's| -rpath $(libdir)| -rpath '"$RPATH"'|g' {} \;  
    fi
fi

cd "$topdir" 
echo
done

echo "Now you may run \"./configure\" to configure the source code,"
echo "followed by \"make\" to build it."
echo "Finally, if you want to install it, type \"make install\"."
