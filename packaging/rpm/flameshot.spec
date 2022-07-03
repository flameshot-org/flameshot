#
# spec file for package flameshot on fedora, rehl, opensuse leap 15.x
#

# fedora >= 30, rhel >=7
%define is_rhel_or_fedora (0%{?fedora} && 0%{?fedora} >= 30) || (0%{?rhel} && 0%{?rhel} >= 7)
# openSUSE Leap >= 15.2
%define is_suse_leap (0%{?is_opensuse} && 0%{?sle_version} >= 150200)

Name: flameshot
Version: 12.1.0
%if %{is_rhel_or_fedora}
Release: 1%{?dist}
%endif
%if %{is_suse_leap}
Release: 1
%endif
License: GPLv3+ and ASL 2.0 and GPLv2 and LGPLv3 and Free Art
Summary: Powerful yet simple to use screenshot software
URL: https://github.com/flameshot-org/flameshot
Source0: %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires: cmake >= 3.13.0
BuildRequires: gcc-c++ >= 7
BuildRequires: fdupes
%if %{is_suse_leap}
BuildRequires: update-desktop-files
BuildRequires: appstream-glib
%endif
%if %{is_rhel_or_fedora}
BuildRequires: libappstream-glib
BuildRequires: ninja-build
%endif
BuildRequires: desktop-file-utils

BuildRequires: cmake(Qt5Core) >= 5.9.0
%if %{is_rhel_or_fedora}
BuildRequires: cmake(KF5GuiAddons) >= 5.89.0
%endif
BuildRequires: cmake(Qt5DBus) >= 5.9.0
BuildRequires: cmake(Qt5Gui) >= 5.9.0
BuildRequires: cmake(Qt5LinguistTools) >= 5.9.0
BuildRequires: cmake(Qt5Network) >= 5.9.0
BuildRequires: cmake(Qt5Svg) >= 5.9.0
BuildRequires: cmake(Qt5Widgets) >= 5.9.0


Requires: hicolor-icon-theme
%if %{is_rhel_or_fedora}
Requires: qt5-qtbase >= 5.9.0
Requires: qt5-qttools >= 5.9.0
Requires: qt5-qtsvg%{?_isa} >= 5.9.0
%endif
%if %{is_suse_leap}
Requires: libQt5Core5 >= 5.9.0
Requires: libqt5-qttools >= 5.9.0
Requires: libQt5Svg5 >= 5.9.0
%endif
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
 * Upload to Imgur

%prep
%autosetup -p1

%build
%if %{is_suse_leap}
%cmake -DCMAKE_BUILD_TYPE=Release
%endif
%if %{is_rhel_or_fedora}
	
%cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_WAYLAND_CLIPBOARD:BOOL=ON \
%endif
%cmake_build

%install
%cmake_install
# https://fedoraproject.org/wiki/PackagingDrafts/find_lang
%find_lang Internationalization --with-qt
%if %{is_suse_leap}
%suse_update_desktop_file -r org.flameshot.Flameshot Utility X-SuSE-DesktopUtility
%endif
%fdupes %{buildroot}%{_datadir}/icons

%check
%if %{is_rhel_or_fedora}
appstream-util validate-relax --nonet %{buildroot}%{_metainfodir}/*.metainfo.xml
%endif
%if %{is_suse_leap}
appstream-util validate-relax --nonet %{buildroot}%{_datadir}/metainfo/*.metainfo.xml
%endif
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
%if %{is_suse_leap}
%{_datadir}/metainfo/org.flameshot.Flameshot.metainfo.xml
%endif
%if %{is_rhel_or_fedora}
%{_metainfodir}/org.flameshot.Flameshot.metainfo.xml
%endif
%{_datadir}/bash-completion/completions/%{name}
%{_datadir}/zsh/site-functions/_%{name}
%{_datadir}/fish/vendor_completions.d/%{name}.fish
%{_datadir}/dbus-1/interfaces/org.flameshot.Flameshot.xml
%{_datadir}/dbus-1/services/org.flameshot.Flameshot.service
%{_datadir}/icons/hicolor/*/apps/*.png
%{_datadir}/icons/hicolor/scalable/apps/*.svg
%{_mandir}/man1/%{name}.1*

%changelog
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
