#! /bin/bash
cp ../build/flameshot ./deb/opt/flameshot/flameshot
cd ./deb/opt/flameshot
wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
chmod +x linuxdeployqt & ls
pwd & ./linuxdeployqt ./flameshot
rm ./linuxdeployqt ./AppRun
rm -rf ./doc
rm -rf .gitkeep
tar -zcvf build.tar.gz .
#upload .tar.gz file
curl --upload-file ./build.tar.gz https://transfer.sh/flameshot_$(date +%Y%m%d)_amd64.tar.gz
rm -rf ./build.tar.gz
cd ..
cd ..
cd ..
ls
dpkg -b deb flameshot_amd64.deb
curl --upload-file ./flameshot_amd64.deb https://transfer.sh/flameshot_$(date +%Y%m%d)_amd64.deb


