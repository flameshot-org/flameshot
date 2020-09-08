Name: flameshot
Version: 0.8.0
Release: 1%{?dist}
Summary: Powerful yet simple to use screenshot software
Summary(eu-ES): Potente pero simple de usar software de capturas

%global sourcename flameshot

Group: Application
License: GPLv3
URL: https://github.com/flameshot-org/%{sourcename}
Source0: https://github.com/flameshot-org/%{sourcename}/archive/v%{version}.tar.gz

#%%define _binaries_in_noarch_packages_terminate_build   0
#BuildArch: noarch

%if 0%{?is_opensuse}
%if 0%{?suse_version} >= 1500
BuildRequires: gcc-c++ >= 4.9.2
BuildRequires: update-desktop-files 
%else
BuildRequires: gcc7
BuildRequires: gcc7-c++
%endif
BuildRequires: libqt5-qttools-devel
BuildRequires: libqt5-linguist
%else
BuildRequires: gcc-c++  >= 4.9.2  
%endif 

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
BuildRequires: qt5-qttools-devel
BuildRequires: qt5-linguist
%endif

BuildRequires: cmake  >= 3.13.0
BuildRequires: pkgconfig
BuildRequires: pkgconfig(Qt5Core) >= 5.3.0
BuildRequires: pkgconfig(Qt5Gui)  >= 5.3.0
BuildRequires: pkgconfig(Qt5DBus) >= 5.3.0
BuildRequires: pkgconfig(Qt5Network) >= 5.3.0
BuildRequires: pkgconfig(Qt5Widgets)  >= 5.3.0
BuildRequires: pkgconfig(Qt5Svg)  >= 5.3.0


%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version}
Requires: qt5-qtbase >= 5.3.0
Requires: qt5-qttools
Requires: qt5-qtsvg
%endif
%if 0%{?is_opensuse}
Requires: libQt5Core5 >= 5.3.0
Requires: libqt5-qttools
Requires: libQt5Svg5
%endif
Requires: hicolor-icon-theme

%description
Flameshot is a screenshot software, it's
powerful yet simple to use for GNU/Linux

%prep
%setup -q -n v%{version}

%build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX=/usr
make %{?_smp_mflags}

%install
%make_install INSTALL_ROOT=%{buildroot}

%if 0%{?is_opensuse}
%if 0%{?suse_version} >= 1500
%suse_update_desktop_file %{name} Graphics
%endif
%endif

%if 0%{?fedora}
%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig
%endif

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/dbus-1/interfaces/org.flameshot.Flameshot.xml
%{_datadir}/dbus-1/services/org.flameshot.Flameshot.service
%{_datadir}/metainfo/flameshot.metainfo.xml
%{_datadir}/flameshot/translations
%{_datadir}/applications/%{name}.desktop
%{_datadir}/bash-completions/completions/%{name}
%{_datadir}/icons/hicolor

%changelog
* Mon Sep 07 2020 Zetao Yang <vitzys@outlook.com> - 0.8.0-1
- Updated for flameshot 0.8.0
* Sat Aug 18 2018 Zetao Yang <vitzys@outlook.com> - 0.6.0-1
- Updated for flameshot 0.6.0
- More details, please see https://flameshot.js.org/#/changelog?id=v060
* Tue Jan 09 2018 Zetao Yang <vitzys@outlook.com> - 0.5.0-1
- Initial package for flameshot 0.5.0
- More details, please see https://flameshot.js.org/#/changelog?id=v051

