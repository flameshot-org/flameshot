#!/bin/bash --

set -e

if [[ "${EXTEN}" == "other" ]]; then
	# Compile-time
	travis_retry sudo apt install -y gcc g++ build-essential qt5-default qt5-qmake qttools5-dev-tools
	# Run-time
	travis_retry sudo apt install -y libqt5dbus5 libqt5network5 libqt5core5a libqt5widgets5 libqt5gui5 libqt5svg5-dev
	# Optional
	travis_retry sudo apt install -y openssl ca-certificates
	# Install fcitx-frontend-qt5
	travis_retry sudo apt install -y fcitx-frontend-qt5

fi
