#! /bin/bash
cat >> package/appimage/flameshot.AppDir/flameshot.desktop << EOF
[Desktop Entry]
Name=Flameshot
Name=Take graphical screenshot
Name[es]=Tomar captura gráfica
GenericName[es]=Herramienta de captura de pantalla
Comment=Powerfull yet simple to use screenshot software.
Comment[es]=Potente pero simple de usar software de capturas.
Exec=flameshot
Icon=flameshot
Terminal=false
Type=Application
Categories=Graphics
StartupNotify=false
EOF
