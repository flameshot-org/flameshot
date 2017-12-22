#! /bin/bash
cat >> package/deb/usr/share/applications/flameshot.desktop << EOF
[Desktop Entry]
Encoding=UTF-8
Name=Flameshot
Name=Take graphical screenshot
Name[es]=Tomar captura gráfica
GenericName=Screen capture tool
GenericName[es]=Herramienta de captura de pantalla
Comment=Powerfull yet simple to use screenshot software.
Comment[es]=Potente pero simple de usar software de capturas.
Exec=/opt/flameshot/flameshot gui
TryExec=/opt/flameshot/flameshot 
Icon=flameshot
Terminal=false
Type=Application
Categories=Graphics;
StartupNotify=false
EOF
