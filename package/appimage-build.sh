#! /bin/bash
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
chmod +x linuxdeployqt & ls
pwd & ./linuxdeployqt ./flameshot.AppDir/flameshot
rm ./linuxdeployqt
rm -rf ./flameshot.AppDir/doc
#libQt5XcbQpa.so.5(QT5.9.3) need libxcb-xinerama
cp /usr/lib/x86_64-linux-gnu/libxcb-xinerama.so.0.0.0 ./flameshot.AppDir/lib/libxcb-xinerama.so.0
pwd & ls
wget -c "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" -O appimagetool
chmod +x appimagetool & ls
./appimagetool flameshot.AppDir
ls
curl --upload-file ./flameshot-x86_64.AppImage https://transfer.sh/flameshot-x86_64_$(date +%Y%m%d)_.AppImage
