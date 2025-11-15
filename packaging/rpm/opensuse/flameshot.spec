#
# spec file for package flameshot on opensuse leap 15.x
#
Name:    flameshot
Version: 13.1.0
Release: 2
License: GPLv3+ and ASL 2.0 and GPLv2 and LGPLv3 and Free Art
Summary: Powerful yet simple to use screenshot software
URL:     https://github.com/flameshot-org/flameshot
Source0: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz
Vendor:  Flameshot

BuildRequires: cmake >= 3.16.0
BuildRequires: gcc-c++ >= 7
BuildRequires: fdupes
BuildRequires: update-desktop-files
BuildRequires: appstream-glib
BuildRequires: desktop-file-utils

BuildRequires: cmake(Qt6Core) >= 6.0.0
BuildRequires: cmake(Qt6DBus) >= 6.0.0
BuildRequires: cmake(Qt6Gui) >= 6.0.0
BuildRequires: cmake(Qt6LinguistTools) >= 6.0.0
BuildRequires: cmake(Qt6Network) >= 6.0.0
BuildRequires: cmake(Qt6Svg) >= 6.0.0
BuildRequires: cmake(Qt6Widgets) >= 6.0.0

Requires: hicolor-icon-theme
Requires: qt6-base >= 6.0.0
Requires: qt6-tools >= 6.0.0
Requires: qt6-svg >= 6.0.0

Recommends: qt6-imageformats
Recommends: xdg-desktop-portal%{?_isa}
Recommends: (xdg-desktop-portal-gnome%{?_isa} if gnome-shell%{?_isa})
Recommends: (xdg-desktop-portal-kde%{?_isa} if plasma-workspace-wayland%{?_isa})
Recommends: (xdg-desktop-portal-wlr%{?_isa} if wlroots%{?_isa})

%description
Powerful and simple to use screenshot software with built-in
editor with advanced features.

Features:

 * Customizable appearance.
 * Easy to use.
 * In-app screenshot edition.
 * DBus interface.

%prep
%autosetup -p1

%build
%cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS:BOOL=OFF

%cmake_build

%install
%cmake_install
rm -rf %{buildroot}%{_includedir}/QtColorWidgets
rm -rf %{buildroot}%{_libdir}/cmake/QtColorWidgets
rm -f %{buildroot}%{_libdir}/libQtColorWidgets.*
rm -f %{buildroot}%{_libdir}/pkgconfig/QtColorWidgets.pc
rm -rf %{buildroot}%{_includedir}/kdsingleapplication-qt6
rm -rf %{buildroot}%{_libdir}/cmake/KDSingleApplication-qt6
rm -f %{buildroot}%{_libdir}/libkdsingleapplication-qt6.*
# https://fedoraproject.org/wiki/PackagingDrafts/find_lang
%find_lang Internationalization --with-qt
%suse_update_desktop_file -r org.flameshot.Flameshot Utility X-SuSE-DesktopUtility
%fdupes %{buildroot}%{_datadir}/icons

%check
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.metainfo.xml
desktop-file-validate %{buildroot}%{_datadir}/applications/*.desktop

%files -f Internationalization.lang
%{_datadir}/%{name}/translations/Internationalization_grc.qm
%doc README.md
%license LICENSE
%dir %{_datadir}/%{name}
%dir %{_datadir}/%{name}/translations
%dir %{_datadir}/bash-completion/completions
%dir %{_datadir}/zsh/site-functions
%{_bindir}/%{name}
%{_datadir}/applications/org.flameshot.Flameshot.desktop
%{_datadir}/metainfo/org.flameshot.Flameshot.metainfo.xml
%{_datadir}/bash-completion/completions/%{name}
%{_datadir}/zsh/site-functions/_%{name}
%{_datadir}/fish/vendor_completions.d/%{name}.fish
%{_datadir}/dbus-1/interfaces/org.flameshot.Flameshot.xml
%{_datadir}/dbus-1/services/org.flameshot.Flameshot.service
%{_datadir}/icons/hicolor/*/apps/*.png
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%{_mandir}/man1/%{name}.1*

%changelog
* Sat Oct 28 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.3.0
- Updated for v13.3.0 release

* Sat Oct 24 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.2.0
- Updated for v13.2.0 release

* Sat Aug 16 2025 Elliott Tallis <tallis.elliott@gmail.com> - 13.1.0-2
- Minor spec file tweaks

* Sun Aug 15 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.1.0
- Update for v13.1.0 release

* Sun Aug 06 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.0.1
- Update for v13.0.1 release

* Sun Aug 03 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.0.0
- Update for v13 release

* Sun Jul 27 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.0.rc2
- Beta for 13 release.

* Sun Jul 12 2025 Jeremy Borgman <borgman.jeremy@pm.me> - 13.0.rc1
- Beta for 13 release.

* Sun Jul 03 2022 Jeremy Borgman <borgman.jeremy@pm.me> - 12.1.0-1
- Update for 12.1 release.

* Wed Jun 21 2022 Jeremy Borgman <borgman.jeremy@pm.me> - 12.0.0-1
- Update for 12.0 release.

* Fri Jan 14 2022 Jeremy Borgman <borgman.jeremy@pm.me> - 11.0.0-1
- Update for 11.0 release.

* Sun Aug 29 2021 Zetao Yang <vitzys@outlook.com> - 0.10.1-2
- Minor SPEC fixes.

* Sun Jul 25 2021 Jeremy Borgman <borgman.jeremy@pm.me> - 0.10.1-1
- Updated for flameshot 0.10.1

* Mon May 17 2021 Jeremy Borgman <borgman.jeremy@pm.me> - 0.10.0-1
- Updated for flameshot 0.10.0

* Sat Feb 27 2021 Jeremy Borgman <borgman.jeremy@pm.me> - 0.9.0-1
- Updated for flameshot 0.9.0

* Wed Oct 14 2020 Jeremy Borgman <borgman.jeremy@pm.me> - 0.8.5-1
- Updated for flameshot 0.8.5

* Sat Oct 10 2020 Jeremy Borgman <borgman.jeremy@pm.me> - 0.8.4-1
- Updated for flameshot 0.8.4

* Sat Sep 19 2020 Jeremy Borgman <borgman.jeremy@pm.me> - 0.8.3-1
- Updated for flameshot 0.8.3

* Mon Sep 07 2020 Zetao Yang <vitzys@outlook.com> - 0.8.0-1
- Updated for flameshot 0.8.0
- More details, please see https://flameshot.org/changelog/#v080

* Sat Aug 18 2018 Zetao Yang <vitzys@outlook.com> - 0.6.0-1
- Updated for flameshot 0.6.0
- More details, please see https://flameshot.org/changelog/#v060

* Tue Jan 09 2018 Zetao Yang <vitzys@outlook.com> - 0.5.0-1
- Initial package for flameshot 0.5.0
- More details, please see https://flameshot.org/changelog/#v050
