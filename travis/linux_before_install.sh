#!/bin/bash --

set -e

if [[ "${DIST}" == "trusty" ]]; then
	sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
	sudo add-apt-repository -y ppa:beineri/opt-qt532-trusty
	sudo apt-get -qq update

	# Get linuxdeployqt tool
	wget \
		-c "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage" \
		-O linuxdeployqt
	chmod +x linuxdeployqt
fi
