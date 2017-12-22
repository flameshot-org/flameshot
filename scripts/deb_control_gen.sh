#! /bin/bash
cat >> package/deb/DEBIAN/control << EOF
Package: flameshot
Version: 0.5.0
Architecture: amd64
Maintainer: Ahmed Zetao Yang <vitzys@outlook.com>
Installed-Size: 54400
Depends: libstdc++6, libc6 (>= 2.14), libgcc1, libglib2.0-0, 	libglib2.0-0, libx11-6, libpng12-0, libharfbuzz0b, zlib1g, libgl1-mesa-glx | libgl1, libdbus-1-3, libproxy1v5, libpcre16-3, libffi6, libpcre3, libxcb1, libfreetype6, libgraphite2-3, libxext6, libxau6, libxdmcp6, libselinux1, liblzma5, libgcrypt20, libgpg-error0, libsystemd0, libxcb-xinerama0
Recommends: openssl, ca-certificates
Section: qt
Priority: optional
Homepage: https://github.com/lupoDharkael/flameshot
Description: Screenshot software
 Powerful yet simple to use screenshot software for GNU/Linux 
EOF
