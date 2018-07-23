#!/bin/bash --

set -e

if [[ "${DIST}" == "trusty" ]]; then
	sudo apt-get install -qq build-essential git

	sudo -E apt-get -yq \
		--no-install-suggests --no-install-recommends --force-yes \
		install openssl libssl-dev

	#sudo -E apt-get -yq \
	#	--no-install-suggests --no-install-recommends --force-yes \
	#	install libgl1-mesa-dev

	sudo -E apt-get -yq \
		--no-install-suggests --no-install-recommends --force-yes \
		install tree

	sudo apt-get install -qq gcc-4.9 g++-4.9

	# Install qt5.3.2
	sudo apt-get -y install qt53base qt53tools qt53svg

	# Install fcitx-frontend-qt5
	sudo apt-get -y install fcitx-frontend-qt5
fi
