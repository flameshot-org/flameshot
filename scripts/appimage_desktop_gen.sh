#! /bin/bash
cat >> package/appimage/flameshot.AppDir/flameshot.desktop << EOF
[Desktop Entry]
Name=Flameshot
GenericName=Screen capture tool
GenericName[es]=Herramienta de captura de pantalla
Comment=Powerfull yet simple to use screenshot software.
Comment[es]=Potente pero simple de usar software de capturas.
Exec=flameshot
Icon=flameshot
Terminal=false
Type=Application
Categories=Graphics;
StartupNotify=false
EOF
