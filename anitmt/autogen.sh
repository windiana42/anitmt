#!/bin/bash
# Automatically calls automake and autoconf the first time if necessary...
# License: LGPL. USE AT YOUR OWN RISK!!
# Copyright: Onno Kortmann
# Needs the unix tools bash, wc and which.
#
# Command line arguments:
# (none) - create files as necessary
# (-f) - recreate all files (a bit like autoreconf, but not really :) 



# check for aclocal, autoconf and automake
NEEDED="aclocal autoconf automake"


echo "You have:"
which $NEEDED

if test `echo $NEEDED|wc -w` != `which $NEEDED|wc -l`; then 
    printf "\nBut you need: $NEEDED.\n";
    echo "Come back later.";
    exit 1;
else
    printf "\nOk. All neccessary tools are available.\n\n";
fi

# FIXME: Correct argument handling... there's only "-f" now, so that's
# no problem yet

topdir="`pwd`"
for i in . hlib ; do 
echo "-----<entering directory $i>-----"
cd $i

# Make aclocal.m4
if test -n "$1" -o ! -f aclocal.m4; then
    echo "Creating aclocal.m4.";
    aclocal || exit 2;
fi

# Make configure
if test -n "$1" -o ! -f configure; then
    echo "Creating configure.";
    autoconf || exit 2;
fi

# Make Makefile.in(s)
if test -n "$1" -o ! -f Makefile.in; then
    echo "Creating Makefile.ins.";
    automake -a || exit 2;
fi

cd "$topdir" 
done

echo
echo "Now you may run \"./configure\" to configure the source code,"
echo "followed by \"make\" to build it."
echo "If you want to install it, type \"make install\" after all."
