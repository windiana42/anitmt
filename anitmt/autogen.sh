#!/bin/bash
# Automatically calls automake and autoconf the first time if necessary...
# License: LGPL. USE AT YOUR OWN RISK!!
# Needs the unix tools bash, wc and which.

# check for aclocal, autoconf and automake
NEEDED="aclocal autoconf automake"


echo "You have:"
which $NEEDED

if test `echo $NEEDED|wc -w` != `which $NEEDED|wc -l`; then 
    printf "\nBut you need: $NEEDED.\n";
    echo "Come back later.";
    exit 1;
else
    printf "\nOk. All neccessary tools are available.\n";
fi

# Make aclocal.m4
if test ! -f aclocal.m4; then
    echo "Creating aclocal.m4.";
    aclocal || exit 2;
fi

# Make configure
if test ! -f configure; then
    echo "Creating configure.";
    autoconf || exit 2;
fi

# Make Makefile.in(s)
if test ! -f Makefile.in; then
    echo "Creating Makefile.ins.";
    automake || exit 2;
fi
echo
echo "Now you may run \"./configure\" to configure the source code,"
echo "followed by \"make\" to build it."
echo "If you want to install it, type \"make install\" after all."
