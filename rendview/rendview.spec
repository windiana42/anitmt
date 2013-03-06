%define  RELEASE 1
%define  rel     %{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}

Name: rendview
Version: 0.7.2
Release: %rel

Summary: RendView is a utility to have films rendered.
License: GPL
Group: Productivity/Graphics/Visualization/Raytracers
Vendor: Wolfgang Wieser <wwieser AT gmx DOT de>
Packager: Martin Trautmann <martintrautmann@gmx.de>
Url: http://www.cip.physik.uni-muenchen.de/~wwieser/ 

Source: http://heanet.dl.sourceforge.net/project/anitmt/%name/%version/%name-%version.tar.gz

Prefix: %_prefix
BuildRoot: %_tmppath/%name-%version-root

%description
RendView is a utility to have films rendered.

Simply spoken it does the following: You give it a number of files to
be rendered (i.e. f000000.pov .. f000123.pov, scene.pov, colors.inc)
and then call RendView to have that done, either locally or in a LAN.
RendView will then call the renderer (currently, only POVRay is supported)
and (if told so) also a filter on the rendered frame.

Of course, that's not all. Here is a list of features; please check the
docu for more complete information and how to launch RendView.

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
%doc AUTHORS COPYING README NEWS ChangeLog BUILD mkbindist.sh
%prefix/bin/*
%prefix/share/rendview/

#%changelog
#* first version
