#!/bin/bash --

set -e

DIST_PATH=dist

if [[ "${DIST}" == "trusty" ]]; then
	travis_retry curl \
		--upload-file \
		"${DIST_PATH}"/flameshot_${ARCH}_${VERSION}.${EXTEN} \
		"https://transfer.sh/flameshot_${ARCH}_${VERSION}.${EXTEN}"
else
	case "${OS}" in
		"ubuntu"|"debian")
			travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}-${DIST}-${ARCH}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}-${DIST}-${ARCH}_${ARCH}.${EXTEN}"
			;;
		"fedora")
			travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}-fedora${DIST}-${ARCH}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}-fedora${DIST}-${ARCH}_${ARCH}.${EXTEN}"
			;;
	esac
fi
