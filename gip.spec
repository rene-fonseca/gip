# ***************************************************************************
#    Generic Image Processing (GIP) Framework
#    A framework for developing image processing applications
#
#    See COPYRIGHT.txt for details.
#
#    This framework is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#    For the licensing terms refer to the file 'LICENSE'.
# ***************************************************************************

Name: gip-framework
Version: 0.1.2
Release: 1
License: GPL
Source: %{name}-%{version}.tar.bz2
Url: https://dev.azure.com/renefonseca/gip
Vendor: Rene Moeller Fonseca
Packager: Rene Moeller Fonseca
Distribution: The GIP Framework
Prefix: /usr
BuildRoot: %{_builddir}/%{name}-%{version}-%{release}-root



Summary: Runtime support provided by the GIP framework
Group: System Environment/Libraries
Requires: base-framework, libpng, libjpeg
%description
This particular package provides the runtime support needed by applications
which are rely on the Generic Image Processing (GIP) framework. If you want to
develop your own applications using this framework you need the dedicated
development package which should accompany this package.



%package devel
Summary: Development support for the GIP framework
Group: Development/Libraries
Requires: %{name} = %{version}, base-framework-devel, libpng-devel, libjpeg-devel

%description devel
This package provides the files required for developing applications using the
Generic Image Processing (GIP) framework.



%prep
%setup
CFLAGS="${RPM_OPT_FLAGS}" CXXFLAGS="${RPM_OPT_FLAGS}" ./configure \
  --target=%{_target} \
  --prefix=%{_prefix}



%build
processors=`egrep -c ^cpu[0-9]+ /proc/stat || :`
if [ "${processors}" -gt "1" ]; then
  make -j${processors} "MAKE=make -j${processors}"
else
  make
fi



%install
make DESTDIR=%{buildroot} install-strip



# the main package
%files
%attr(-,root,root) %{prefix}/lib/libgip.so*



# development package
%files devel
# package all header files
%attr(-,root,root) %{prefix}/include
# package all static libraries
%attr(-,root,root) %{prefix}/lib/libgip.a
# libtool libraries
%attr(-,root,root) %{prefix}/lib/libgip.la



%clean
# clean up root (may be overwritten by rpm configuration file!)
[ -n "%{buildroot}" -a "%{buildroot}" != "/" ] && rm -rf %{buildroot}

# clean up source tree
rm -rf %{_builddir}/%{name}
