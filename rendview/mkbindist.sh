#!/bin/sh
# 
# Make a distribution. Call: 
# mkbindist.sh path-to-rendview-src rendview-version

srcdir="$1"
version="$2"
configguess="$srcdir/config.guess"

# Guess system: 
hwtype="$("$configguess")"
if test "$?" != "0" ; then
	echo "config.guess failed."
	exit 1
fi

hwtype0="$hwtype"
hwtype="$(echo "$hwtype" | sed "s/-/ /g")"
declare -i cnt=0
hwtmpA=""
hwtmpB=""
for i in  $hwtype ; do
	test "$cnt" = "0" && hwtmpA="$i"
	test "$cnt" = "2" && hwtmpB="$i"
	cnt="$cnt"+1
done
if test "$cnt" -lt 3 ; then
	echo "Failure to extract hw type -- $hwtype0"
	exit 1
fi
case "$hwtmpA" in
	i?86) hwtmpA="i386" ;;
esac
hwtype="$hwtmpA-$hwtmpB"

ddirname="rendview-$version-$hwtype"
echo "Building binary dist for $hwtype: $ddirname"

trap 'rm -r "$ddirname"' INT TERM EXIT

# Temporary directory which will get tar'ed for distro: 
mkdir $ddirname || exit 1

# Copy the files: 
cp -a "$srcdir/README" "$ddirname/" || exit 1
cp -a "$srcdir/INSTALL" "$ddirname/" || exit 1

cp -a src/rendview "$ddirname/" || exit 1
strip "$ddirname/rendview"

cp -a src/adminshell/rvadminshell "$ddirname/" || exit 1
strip "$ddirname/rvadminshell"

mkdir "$ddirname/doc" || exit 1
for i in admin.html database.html examples.html index.html quickstart.html \
	taskmanager.html tasksource.html terminal.html ; do
cp -a "$srcdir/doc/$i" "$ddirname/doc/" || exit 1
done

mkdir "$ddirname/cfg" || exit 1
cp -a "$srcdir/cfg/renderers.par" "$srcdir/cfg/filters.par" "$ddirname/cfg/" || exit 1

# Archive...
tar -c "$ddirname" | gzip --best > "$ddirname.tar.gz"

echo "Complete: $(ls -l "$ddirname.tar.gz")"
exit 0
