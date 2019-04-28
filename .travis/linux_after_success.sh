#!/bin/bash --

set -e

DIST_PATH=dist

if [[ "${EXTEN}" == "other" ]]; then
	cp "${BUILD_DST_PATH}/flameshot" "${ROOT_PATH}/.travis/services/flameshot_${VERSION}_${ARCH}"
	cd "${ROOT_PATH}/.travis/services"
	TEMP_DOWNLOAD_URL=$(travis_retry bash \
	"${ROOT_PATH}/.travis/services/${UPLOAD_SERVICE}.sh" \
	flameshot_"${VERSION}_${ARCH}")
else
	case "${OS}" in
		"ubuntu"|"debian")
		    cp "${DIST_PATH}/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}" "${ROOT_PATH}/.travis/services/flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}"
			cd "${ROOT_PATH}/.travis/services"
			TEMP_DOWNLOAD_URL=$(travis_retry bash \
				"${ROOT_PATH}/.travis/services/${UPLOAD_SERVICE}.sh" \
				"flameshot_${VERSION}_${DIST}_${ARCH}.${EXTEN}")
			;;
		"fedora")
			cp "${DIST_PATH}/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}" "${ROOT_PATH}/.travis/services/flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}"
			cd "${ROOT_PATH}/.travis/services"
			TEMP_DOWNLOAD_URL=$(travis_retry bash \
				"${ROOT_PATH}/.travis/services/${UPLOAD_SERVICE}.sh" \
				"flameshot_${VERSION}_fedora${DIST}_${ARCH}.${EXTEN}")
			;;
	esac
fi

