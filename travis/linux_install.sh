#!/bin/bash --

set -e

if [[ "${DIST}" == "trusty" ]]; then
	travis_retry sudo apt-get install -qq build-essential git

	travis_retry sudo -E apt-get -yq \
		--no-install-suggests --no-install-recommends --force-yes \
		install openssl libssl-dev

	#travis_retry sudo -E apt-get -yq \
	#	--no-install-suggests --no-install-recommends --force-yes \
	#	install libgl1-mesa-dev

	travis_retry sudo -E apt-get -yq \
		--no-install-suggests --no-install-recommends --force-yes \
		install tree

	travis_retry sudo apt-get install -qq gcc-4.9 g++-4.9

	# Install qt5.3.2
	travis_retry sudo apt-get -y install qt53base qt53tools qt53svg

	# Install fcitx-frontend-qt5
	travis_retry sudo apt-get -y install fcitx-frontend-qt5
fi
