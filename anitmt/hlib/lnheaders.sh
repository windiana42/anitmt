#!/bin/sh
srcdir="$1"
if test "x$srcdir" = "x" ; then srcdir="." ; fi
cd $srcdir || exit 1

#subdirs="`egrep "^SUBDIRS = " MakefileA | sed "s/SUBDIRS =//"`" && \
#srcdir="`egrep "^srcdir = " MakefileA | sed "s/srcdir =//"`" && \

subdirs="`egrep "^SUBDIRS = " Makefile.am | sed "s/SUBDIRS =//"`" && \

mkdir -p include/hlib || exit 1

if test \! -L "include/hlib/config.h" ; then
	ln -s ../../config.h include/hlib/config.h || exit 1
fi

for i in $subdirs ; do
	headers="`echo $i/*.h`"
	for j in $headers ; do
		dest="include/hlib/`basename $j`"
		if test \! -L "$dest" ; then
			ln -s "../../$j" "$dest" || exit 1
			echo -n "."
		fi
	done
done

exit 0
