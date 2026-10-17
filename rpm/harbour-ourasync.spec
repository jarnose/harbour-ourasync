Name:       harbour-ourasync

Summary:    Oura Sync
Version:    1.4.0
Release:    1
License:    MIT
URL:        https://github.com/jarnose/harbour-ourasync
Source0:    %{name}-%{version}.tar.bz2
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(sailfishsecrets)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Concurrent)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(zlib)
# No pkgconfig(Qt5Bluetooth): Sailfish OS's Qt5 has no QtConnectivity. The ring
# is reached through BlueZ's D-Bus API with Qt5DBus instead.
# No pkgconfig(Qt5LinguistTools) virtual exists; CMake finds lrelease through
# the CMake package config that this plain package provides.
BuildRequires:  qt5-qttools-linguist
BuildRequires:  desktop-file-utils
BuildRequires:  cmake

%description
Reads the data of an Oura ring (Gen 3, 4 and 5) over Bluetooth LE, straight
from the ring and without an Oura account or subscription, and shows it:
sleep stages, heart rate, HRV, skin temperature, SpO2 and activity.

Everything stays on the phone. Reading the ring needs no network. History from
before the ring was synced here can be imported from Oura's own data export (a
file) or, optionally, from the Oura Cloud API. The ring's authentication key and
the cloud sign-in are kept in the Sailfish Secrets vault.

The ring talks to one app at a time, and a ring that the Oura app has set up
cannot be shared: this app sets up a factory-reset ring itself, after which the
Oura app can no longer use it.

%prep
%setup -q -n %{name}-%{version}

%build

%cmake
%make_build

%install
%make_install

desktop-file-install --delete-original         --dir %{buildroot}%{_datadir}/applications                %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
