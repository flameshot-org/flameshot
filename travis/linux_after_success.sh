#!/bin/bash --

set -e

DIST_PATH=dist

if [[ "${DIST}" == "trusty" ]]; then
	travis_retry curl \
		--upload-file \
		"${DIST_PATH}"/flameshot_${VERSION}_${ARCH}.${EXTEN} \
		"https://transfer.sh/flameshot_${VERSION}_${ARCH}.${EXTEN}"
else
	case "${OS}" in
		"ubuntu"|"debian")
			travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}"
			;;
		"fedora")
			travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}"
			;;
	esac
fi
