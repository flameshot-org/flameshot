#!/bin/bash --

set -e

if [[ "${DIST}" == "trusty" ]]; then
	travis_retry sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	travis_retry sudo add-apt-repository -y ppa:beineri/opt-qt532-trusty
	travis_retry sudo apt-get -qq update

	# Get linuxdeployqt tool
	travis_retry wget \
		-c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
		-O linuxdeployqt
	chmod +x linuxdeployqt
fi
