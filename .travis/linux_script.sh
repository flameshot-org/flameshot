#!/bin/bash --

set -e

DIST_PATH=dist

if [[ ! -d "${DIST_PATH}" ]]; then
	mkdir "${DIST_PATH}"
fi

if [[ "${EXTEN}" == "other" ]]; then
	project_dir="$(pwd)"
	BUILD_DST_PATH=build-test

	qmake --version
	mkdir "${BUILD_DST_PATH}"
	qmake -makefile DESTDIR="${BUILD_DST_PATH}" "${project_dir}"/flameshot.pro
	# Building flameshot
	make -j$(nproc)
	# Running flameshot tests
	make check -j$(nproc)
	ls -alhR

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
