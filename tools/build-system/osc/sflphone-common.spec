#####################################################
# File Name: sflphone-common.spec
#
# Purpose :
#
# Author: Julien Bonjean (julien@bonjean.info) 
#
# Creation Date: 2009-05-27
# Last Modified: 2009-10-07
#####################################################

Name:           sflphone-common
License:        GNU General Public License (GPL)
Group:          System Environment/Daemons
Summary:        SIP and IAX2 compatible softphone - Core
Version:        0.9.6
Release:        opensuse
URL:            http://www.sflphone.org/
Vendor:         Savoir-faire Linux
Packager:	Julien Bonjean <julien.bonjean@savoirfairelinux.com>

BuildRoot:      %{_tmppath}/%{name}
Source0:        sflphone-common-%{version}.tar.gz
BuildRequires:	speex-devel
BuildRequires:	gcc-c++
BuildRequires:	expat
BuildRequires:	alsa-devel
BuildRequires:	dbus-1-devel
BuildRequires:	libzrtpcpp-devel
BuildRequires:	commoncpp2-devel
BuildRequires:	libsamplerate-devel


%if %{defined suse_version}
BuildRequires:	libpulse-devel
BuildRequires:	libccrtp-devel
BuildRequires:	libexpat-devel
BuildRequires:	libgsm-devel
BuildRequires:	libcppunit-devel
BuildRequires:	libuuid-devel
BuildRequires:	libopenssl-devel
BuildRequires:	libexpat0
%endif

%if %{defined fedora_version}
BuildRequires:	pulseaudio-libs-devel
BuildRequires:	openssl-devel
BuildRequires:	openssl
BuildRequires:	expat-devel
BuildRequires:	ccrtp-devel
BuildRequires:	cppunit-devel
BuildRequires:	uuid-devel
BuildRequires:	gsm-devel
%endif

Requires:	libsamplerate
Requires:	libexpat1
Requires:	commoncpp2
Requires:	libgsm1
Requires:	libspeex
Requires:	dbus-1
Requires:	dbus-1-x11
Requires:	xorg-x11
Requires:	libasound2
Requires:	libpulse0
Requires:	libccrtp1
Conflicts:      sflphone
Prefix:		%{_prefix}

%description
SFLphone is meant to be a robust enterprise-class desktop phone.
 SFLphone is released under the GNU General Public License.
 SFLphone is being developed by the global community, and maintained by
 Savoir-faire Linux, a Montreal, Quebec, Canada-based Linux consulting company.

Authors:
--------
    Julien Bonjean <julien.bonjean@savoirfairelinux.com>

%prep
%setup -q

%build
cd libs/pjproject
./autogen.sh
./configure --prefix=%{_prefix}
make dep
make clean
make
cd -
./autogen.sh
./configure --prefix=%{_prefix} 
make -j

%install
make DESTDIR=%{buildroot} install
rm -rf %{buildroot}/%{_prefix}/include

%clean
cd libs/pjproject
make clean
cd -
make clean

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README TODO
%dir %{_prefix}/lib/sflphone
%dir %{_prefix}/lib/sflphone/codecs
%dir %{_prefix}/lib/sflphone/plugins
%dir %{_prefix}/share/sflphone
%dir %{_prefix}/share/sflphone/ringtones
%{_prefix}/lib/libdbus-*
%{_prefix}/lib/sflphone/codecs/*
%{_prefix}/lib/sflphone/plugins/*
%{_prefix}/share/dbus-1/services/org.sflphone.*
%{_prefix}/share/sflphone/ringtones/*
%{_prefix}/lib/sflphone/sflphoned
%doc %{_prefix}/share/man/man1/sflphoned.1.gz

%changelog