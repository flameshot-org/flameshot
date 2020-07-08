#!/bin/bash

BASE_VERSION_OLD=0.7.3
BASE_VERSION_NEW=0.7.4

sed -i "s/BASE_VERSION = ${BASE_VERSION_OLD}/BASE_VERSION = ${BASE_VERSION_NEW}/g" flameshot.pro

sed -i "s/AppVersion=${BASE_VERSION_OLD}/AppVersion=${BASE_VERSION_NEW}/g" ./win_setup/flameshot.iss
sed -i "s/VersionInfoVersion=${BASE_VERSION_OLD}/VersionInfoVersion=${BASE_VERSION_NEW}/g" ./win_setup/flameshot.iss

sed -i "s/version: ${BASE_VERSION_OLD}/version: ${BASE_VERSION_NEW}/g" appveyor.yml

sed -i "s/VERSION=${BASE_VERSION_OLD}/VERSION=${BASE_VERSION_NEW}/g" .travis.yml

qmake
