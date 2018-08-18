#!/bin/bash --

set -e

DIST_PATH=dist

if [[ ! -d "${DIST_PATH}" ]]; then
	mkdir "${DIST_PATH}"
fi

if [[ "${DIST}" == "trusty" ]]; then
	project_dir="$(pwd)"
	BUILD_DST_PATH=build-test
	APPIMAGE_DST_PATH=build-appimage

	#source /opt/qt53/bin/qt53-env.sh
	QT_BASE_DIR=/opt/qt53
	export QTDIR="${QT_BASE_DIR}"
	export PATH="${QT_BASE_DIR}"/bin:"${PATH}"
	export LD_LIBRARY_PATH="${QT_BASE_DIR}"/lib/x86_64-linux-gnu:"${QT_BASE_DIR}"/lib:"${LD_LIBRARY_PATH}"
	export PKG_CONFIG_PATH="${QT_BASE_DIR}"/lib/pkgconfig:"${PKG_CONFIG_PATH}"

	qmake --version
	export CC=gcc-4.9 CXX=g++-4.9

	mkdir "${BUILD_DST_PATH}"
	qmake QMAKE_CXX="${CXX}" QMAKE_CC="${CC}" QMAKE_LINK="${CXX}" DESTDIR="${BUILD_DST_PATH}"
	# Building flameshot
	make -j$(nproc)
	# Running flameshot tests
	make check -j$(nproc)
	ls -alhR

	#
	# Packaging AppImage using linuxdeployqt
	#
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/bin
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/applications
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/dbus-1/interfaces
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/dbus-1/services
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/metainfo
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/bash-completion/completions
	mkdir -p "${APPIMAGE_DST_PATH}"/appdir/usr/share/flameshot/translations
	cp \
		"${BUILD_DST_PATH}"/flameshot \
		"${APPIMAGE_DST_PATH}"/appdir/usr/bin/
	cp \
		"${project_dir}"/dbus/org.dharkael.Flameshot.xml \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/dbus-1/interfaces/
	cp \
		"${project_dir}"/dbus/package/org.dharkael.Flameshot.service \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/dbus-1/services/
	cp \
		"${project_dir}"/docs/appdata/flameshot.appdata.xml \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/metainfo/
	cp \
		"${project_dir}"/docs/bash-completion/flameshot \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/bash-completion/completions/
	cp \
		"${project_dir}"/translations/*.qm \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/flameshot/translations/
	cp \
		"${project_dir}"/docs/desktopEntry/package/* \
		"${APPIMAGE_DST_PATH}"/appdir/usr/share/applications/
	cp \
		"${project_dir}"/img/app/flameshot.png \
		"${APPIMAGE_DST_PATH}"/appdir/
	ls -alhR "${APPIMAGE_DST_PATH}"/appdir

	# Copy other project files
	cp "${project_dir}"/README.md "${APPIMAGE_DST_PATH}"/appdir/README.md
	cp "${project_dir}"/LICENSE "${APPIMAGE_DST_PATH}"/appdir/LICENSE
	echo "${VERSION}" > "${APPIMAGE_DST_PATH}"/appdir/version
	echo "${TRAVIS_COMMIT}" >> "${APPIMAGE_DST_PATH}"/appdir/version

	# Configure env vars
	unset QTDIR
	unset QT_PLUGIN_PATH
	unset LD_LIBRARY_PATH
	tree "${APPIMAGE_DST_PATH}"/appdir

	# Packaging
	# -verbose=2
	./linuxdeployqt "${APPIMAGE_DST_PATH}"/appdir/usr/bin/flameshot -bundle-non-qt-libs

	rm -f "${APPIMAGE_DST_PATH}"/appdir/usr/lib/libatk-1.0.so.0
	cp \
		/usr/lib/x86_64-linux-gnu/qt5/plugins/platforminputcontexts/libfcitxplatforminputcontextplugin.so \
		"${APPIMAGE_DST_PATH}"/appdir/usr/plugins/platforminputcontexts/
	cd "${APPIMAGE_DST_PATH}"/appdir/usr/bin
	ln -sf ../plugins/platforms/ .   # An unknown bug
	ln -sf ../share/flameshot/translations/ . # add translation soft link 
	cd "${project_dir}"

	# -verbose=2
	./linuxdeployqt "${APPIMAGE_DST_PATH}"/appdir/usr/share/applications/flameshot.desktop -appimage

	ls -alhR -- *.AppImage
	cp -- *.AppImage "${APPIMAGE_DST_PATH}"/

	tree "${APPIMAGE_DST_PATH}"/

	ls -l "${APPIMAGE_DST_PATH}"/*.AppImage

	# Rename AppImage and move AppImage to DIST_PATH 
	cd "${APPIMAGE_DST_PATH}"
	mv Flameshot-${VERSION}-${ARCH}.AppImage flameshot_${VERSION}_${ARCH}.AppImage
	cd ..
	cp \
		"${APPIMAGE_DST_PATH}"/flameshot_${VERSION}_${ARCH}.AppImage \
		"${DIST_PATH}"/flameshot_${VERSION}_${ARCH}.${EXTEN}
	pwd
else
	travis_retry git clone https://github.com/flameshotapp/packpack.git
	travis_retry packpack/packpack
	pwd && ls

	case "${OS}" in
		"ubuntu"|"debian")
			# copy deb to dist path for distribution
			cp \
				build/flameshot_*_*.deb \
				"${DIST_PATH}"/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}
			;;
		"fedora")
			cp \
				build/flameshot-${VERSION}-${RELEASE}.*.${ARCH}.rpm \
				"${DIST_PATH}"/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}
			;;
	esac
fi
