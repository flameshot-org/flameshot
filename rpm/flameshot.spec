Name: flameshot
Version: 0.5.0
Release: 1%{?dist}
Summary: Powerful yet simple to use screenshot software
Summary(eu-ES): Potente pero simple de usar software de capturas

%global sourcename flameshot

Group: Application
License: GPLv3
URL: https://github.com/lupoDharkael/%{sourcename}
Source0: https://github.com/lupoDharkael/%{sourcename}/archive/v%{version}.tar.gz

#%%define _binaries_in_noarch_packages_terminate_build   0
#BuildArch: noarch

BuildRequires: gcc-c++  >= 4.9.2  
BuildRequires: pkgconfig(Qt5Core) >= 5.3.0
BuildRequires: pkgconfig(Qt5Gui)  >= 5.3.0
BuildRequires: pkgconfig(Qt5Widgets)  >= 5.3.0
BuildRequires: qt5-qttools-devel
BuildRequires: qt5-linguist
BuildRequires: git

Requires: qt5-qtbase >= 5.3.0
Requires: qt5-qttools

%description
Flameshot is a screenshot software, it's
powerful yet simple to use for GNU/Linux

%prep
%setup -q -n v%{version}

%build
#%%qmake_qt5 PREFIX=%{_prefix}
%qmake_qt5 CONFIG+=packaging CONFIG-=debug CONFIG+=release
make %{?_smp_mflags}

%install
%make_install INSTALL_ROOT=%{buildroot}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_datadir}/dbus-1/interfaces/org.dharkael.Flameshot.xml
%{_datadir}/metainfo/flameshot.appdata.xml
%{_datadir}/dbus-1/services/org.dharkael.Flameshot.service
%{_datadir}/flameshot/translations/Internationalization_*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/applications/%{name}-init.desktop
%{_datadir}/applications/%{name}-config.desktop
%{_datadir}/bash-completion/completions/%{name}
%{_datadir}/icons/%{name}.png
#%%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
#%%{_datadir}/icons/flameshot/apps/scalable/%{name}.svg

%changelog
* Tue Jan 09 2018 Zetao Yang <yangzetao2015@outlook.com> - 0.5.0-1
- Updated for flameshot 0.5.0
- Catalan translation.
- Debian package configuration.
- Add --raw flag: prints the raw bytes of the png after the capture.
- Bash completion.
- Blur tool.
- Preview draw size on mouse pointer after tool selection.
- App Launcher tool: choose an app to open the capture.
- Travis integration
- Configuration import, export and reset.
- Experimental Wayland support (Plasma & Gnome)
* Tue Jan 09 2018 Zetao Yang <yangzetao2015@outlook.com> - 0.5.0-1
- Initial package