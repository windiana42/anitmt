%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define  debug_package %{nil}

Name: anitmt
Version: 0.2.4
Release: %rel

Summary: AniTMT is an Animation System for generating films with povray
License: GPL-2.0-only
Group: Productivity/Graphics/Visualization/Raytracers
Vendor: Martin Trautmann <martintrautmann@gmx.de>
Packager: Martin Trautmann <martintrautmann@gmx.de>
Url: http://sourceforge.net/project/anitmt

Source: https://master.dl.sourceforge.net/project/anitmt/AniTMT/%version/%name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %_tmppath/%name-%version-root

BuildRequires: gcc-c++ bison flex

%description
AniTMT is an Animation System for generating films with povray

%prep
%setup -q
#perl -pi -e 's|\${prefix}|%prefix|' README
#perl -pi -e 's|PREFIX|%prefix|' doc/FAQ

%build

if [ ! -x configure ]; then
	CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%prefix
fi
CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --enable-debug="no" $ARCH_FLAGS --prefix=%prefix --libdir=$RPM_BUILD_ROOT%{_libdir}

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
%doc AUTHORS COPYING COPYING.GPL COPYING.LGPL README NEWS ChangeLog TODO FAQ
%{_bindir}/*
%{_libdir}/*
%{_datadir}/anitmt/example/

#%changelog
#* first version
