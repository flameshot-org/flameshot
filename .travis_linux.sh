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

	# Install qt5.9
	sudo add-apt-repository ppa:beineri/opt-qt532-trusty -y
	sudo apt-get update -qq
	sudo apt-get -y install qt53base qt53tools
	source /opt/qt53/bin/qt53-env.sh && qmake --version
	export CC=gcc-4.9 CXX=g++-4.9

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
	mkdir -p ./build-appimage/appdir
	cp $BUILD_DST_PATH/flameshot $APPIMAGE_DST_PATH/appdir
	cp ${project_dir}/docs/desktopEntry/package/* $APPIMAGE_DST_PATH/appdir
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
	./linuxdeployqt $APPIMAGE_DST_PATH/appdir/flameshot -verbose=2 -bundle-non-qt-libs
	ln -sf plugins/platforms/ $APPIMAGE_DST_PATH/appdir/platforms  # An unknown bug
	./linuxdeployqt $APPIMAGE_DST_PATH/appdir/flameshot -verbose=2 -appimage
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