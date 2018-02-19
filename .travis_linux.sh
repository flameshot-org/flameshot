#!/bin/bash
if [[ "${DIST}" != "trusty" ]]; then 
	git clone https://github.com/packpack/packpack.git packpack ;
	mkdir ./dist ;
	pwd && ls ;
	packpack/packpack ;
	if [ $OS == "ubuntu" ];
	then curl --upload-file build/flameshot_*_*.deb "https://transfer.sh/flameshot_$VERSION-$RELEASE_ubuntu_$ARCH.deb" ;
	# copy deb to dist path for distribution
	cp build/flameshot_*_*.deb dist/flameshot_$VERSION-$RELEASE_ubuntu_$ARCH.deb ;
	elif [ $OS == "debian" ];
	then curl --upload-file build/flameshot_*_*.deb "https://transfer.sh/flameshot_$VERSION-$RELEASE_debian_$ARCH.deb" ;
	cp build/flameshot_*_*.deb dist/flameshot_$VERSION-$RELEASE_debian_$ARCH.deb ;
	elif [ $OS == "fedora" ];
	then curl --upload-file build/flameshot-$VERSION-$RELEASE.*.$ARCH.rpm "https://transfer.sh/flameshot_$VERSION-$RELEASE_fedora_$ARCH.rpm" ;
	cp build/flameshot-$VERSION-$RELEASE.*.$ARCH.rpm dist/flameshot_$VERSION-$RELEASE_fedora_$ARCH.rpm ;
	else echo "";
	fi
	echo -e "\n" ;
	pwd && ls ;
elif [[ "${DIST}" == "trusty" ]]; then
	project_dir=$(pwd)
	DIST_PATH='dist'
	BUILD_DST_PATH='build-test'
	APPIMAGE_DST_PATH='build-appimage'

	# Install qt5.3.2
	sudo add-apt-repository ppa:beineri/opt-qt532-trusty -y
	sudo apt-get update -qq
	sudo apt-get -y install qt53base qt53tools
	source /opt/qt53/bin/qt53-env.sh && qmake --version
	export CC=gcc-4.9 CXX=g++-4.9

	# Install fcitx-frontend-qt5
	sudo apt-get -y install fcitx-frontend-qt5

	mkdir build-test
	qmake QMAKE_CXX=$CXX QMAKE_CC=$CC QMAKE_LINK=$CXX DESTDIR=$BUILD_DST_PATH
	# Building flameshot
	make -j$(nproc)
	# Running flameshot tests
	make check -j$(nproc)
	ls -alhR

	#
	# Packaging AppImage using linuxdeployqt
	#
	mkdir build-appimage
	mkdir -p ./build-appimage/appdir/usr/bin
    mkdir -p ./build-appimage/appdir/usr/share/applications
    mkdir -p ./build-appimage/appdir/usr/share/dbus-1/interfaces
    mkdir -p ./build-appimage/appdir/usr/share/dbus-1/services
    mkdir -p ./build-appimage/appdir/usr/share/metainfo
    mkdir -p ./build-appimage/appdir/usr/share/bash-completion/completions
    mkdir -p ./build-appimage/appdir/usr/share/flameshot/translations
	cp $BUILD_DST_PATH/flameshot $APPIMAGE_DST_PATH/appdir/usr/bin
    cp ${project_dir}/dbus/org.dharkael.Flameshot.xml $APPIMAGE_DST_PATH/appdir/usr/share/dbus-1/interfaces
    cp ${project_dir}/dbus/package/org.dharkael.Flameshot.service $APPIMAGE_DST_PATH/appdir/usr/share/dbus-1/services
    cp ${project_dir}/docs/appdata/flameshot.appdata.xml $APPIMAGE_DST_PATH/appdir/usr/share/metainfo
    cp ${project_dir}/docs/bash-completion/flameshot $APPIMAGE_DST_PATH/appdir/usr/share/bash-completion/completions
    cp ${project_dir}/translations/*.qm $APPIMAGE_DST_PATH/appdir/usr/share/flameshot/translations
	cp ${project_dir}/docs/desktopEntry/package/* $APPIMAGE_DST_PATH/appdir/usr/share/applications
	cp ${project_dir}/img/flameshot.png $APPIMAGE_DST_PATH/appdir
	ls -alhR $APPIMAGE_DST_PATH/appdir

	# Copy other project files
	cp "${project_dir}/README.md" "$APPIMAGE_DST_PATH/appdir/README.md"
	cp "${project_dir}/LICENSE" "$APPIMAGE_DST_PATH/appdir/LICENSE"
	echo ${VERSION} > ./$APPIMAGE_DST_PATH/appdir/version
	echo "${TRAVIS_COMMIT}" >> ./$APPIMAGE_DST_PATH/appdir/version

	# Configure env vars
	unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
	tree $APPIMAGE_DST_PATH/appdir

	# Get linuxdeployqt tool
	wget -c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" -O linuxdeployqt
	chmod +x linuxdeployqt

	# Packaging
    # -verbose=2
	./linuxdeployqt $APPIMAGE_DST_PATH/appdir/usr/bin/flameshot -bundle-non-qt-libs

    rm -f $APPIMAGE_DST_PATH/appdir/usr/lib/libatk-1.0.so.0
    cp /usr/lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts/libfcitxplatforminputcontextplugin.so $APPIMAGE_DST_PATH/appdir/usr/plugins/platforminputcontexts/
    cd $APPIMAGE_DST_PATH/appdir/usr/bin
	ln -sf ../plugins/platforms/ .   # An unknown bug
    cd ${project_dir}

    # -verbose=2
	./linuxdeployqt $APPIMAGE_DST_PATH/appdir/usr/share/applications/flameshot.desktop -appimage

	ls -alhR ./*.AppImage
	cp *.AppImage $APPIMAGE_DST_PATH/

	tree $APPIMAGE_DST_PATH/

	ls -l $APPIMAGE_DST_PATH/*.AppImage

	mkdir dist

	# Rename AppImage and move AppImage to DIST_PATH 
	cd $APPIMAGE_DST_PATH && mv Take_graphical_screenshot-${VERSION}-x86_64.AppImage flameshot_x86_64_${VERSION}.AppImage
	cd .. && cp $APPIMAGE_DST_PATH/flameshot_x86_64_${VERSION}.AppImage $DIST_PATH/flameshot_x86_64_${VERSION}.AppImage

	pwd

    curl --upload-file $DIST_PATH/flameshot_x86_64_${VERSION}.AppImage "https://transfer.sh/flameshot_x86_64_${VERSION}.AppImage" ;
	
	exit 0 ;
else echo "" ;
fi
