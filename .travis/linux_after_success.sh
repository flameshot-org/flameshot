#!/bin/bash --

set -e

DIST_PATH=dist

if [[ "${EXTEN}" == "other" ]]; then
	TEMP_DOWNLOAD_URL=$(travis_retry curl \
		--upload-file \
		"${BUILD_DST_PATH}"/flameshot \
		"https://transfer.sh/flameshot_${VERSION}_${ARCH}")
else
	case "${OS}" in
		"ubuntu"|"debian")
			TEMP_DOWNLOAD_URL=$(travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}")
			;;
		"fedora")
			TEMP_DOWNLOAD_URL=$(travis_retry curl \
				--upload-file \
				"${DIST_PATH}"/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN} \
				"https://transfer.sh/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}")
			;;
	esac
fi
