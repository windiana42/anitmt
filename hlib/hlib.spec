%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Name: hlib
Version: 1.0.0
Release: %rel

Summary: hlib is a general purpose Library of Wolfgang Wieser 
Copyright: GPL
Group: System/Libraries/
Vendor: Martin Trautmann <martintrautmann@gmx.de>
Packager: Martin Trautmann <martintrautmann@gmx.de>
Url: http://www.cip.physik.uni-muenchen.de/~wwieser/ 

Source: ftp://download.sourceforge.net/pub/sourceforge/anitmt/%name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %_tmppath/%name-%version-root

%description
HLIB is a general-purpose library for POSIX and POSIX-like operating systems
providing convenient classes which help you to deal in a sophisticated (as
I hope...) way with following standard issues:

  - catching signals (synchroniously)
  - timers and quasi-parallel execution (single threaded)
  - surveilling FDs (read / write / exception)
  - launching processes (including command line args and environment
     parameters, IO redirection, nice value, working directory)
  - and staying informed about the launched process (termination and
     termination reason, runtime)

For this purpose, HLIB implements a nice select(2) model (well, actually
it is using poll(2)). HLIB provides base classes for the mentioned tasks
and other useful functions (e.g. for making FDs non-blocking) and the
following addons:

  - command line parameter handling
  - strong cryptography and hashing

%prep
%setup -q
#perl -pi -e 's|\${prefix}|%prefix|' README
#perl -pi -e 's|PREFIX|%prefix|' doc/FAQ

%build

if [ ! -x configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%prefix
fi
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --enable-debug="no" $ARCH_FLAGS --prefix=%prefix

if [ -n "$SMP" ]; then
	make -j$SMP "MAKE=make -j$SMP"
else
	make
fi

%install
rm -rf $RPM_BUILD_ROOT
#mkdir -p -m 755 $RPM_BUILD_ROOT%prefix/{{include,lib}/%lib_name}
make install prefix=$RPM_BUILD_ROOT%prefix

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,755)
%doc AUTHORS COPYING COPYING.GPL README NEWS ChangeLog BUILD
%prefix/lib/*

#%changelog
#* first version
