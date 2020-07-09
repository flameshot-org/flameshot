#!/bin/bash

BASE_VERSION_NEW="0.7.3"

BASE_VERSION_CUR_EXT=$(cat flameshot.pro |grep "[0-9]\+\.[0-9]\+\.[0-9]\+-[0-9a-e]*" |awk "{print \$3}")
BASE_VERSION_CUR=$(echo "${BASE_VERSION_CUR_EXT}" |cut -d '-' -f1)
BASE_VERSION_NEW_EXT="${BASE_VERSION_NEW}-$(git rev-parse --short HEAD)"

if [ "${BASE_VERSION_CUR_EXT}" == "${BASE_VERSION_NEW_EXT}" ]; then
  echo "Current and new versions are the same, no action required"
  exit 1
fi


TAG_EXISTS=$(git ls-remote --tags origin |grep "refs/tags/v${BASE_VERSION_NEW_EXT}")
if [ "" != "${TAG_EXISTS}" ]; then
  echo "Tag already exists: ${TAG_EXISTS}"
  echo "Please update to another version or remove git tag"
  exit 1
fi

# update version to a new one
sed -i "s/BASE_VERSION = ${BASE_VERSION_CUR}/BASE_VERSION = ${BASE_VERSION_NEW}/g" flameshot.pro
sed -i "s/AppVersion=${BASE_VERSION_CUR_EXT}/AppVersion=${BASE_VERSION_NEW_EXT}/g" ./win_setup/flameshot.iss
sed -i "s/VersionInfoVersion=${BASE_VERSION_CUR}/VersionInfoVersion=${BASE_VERSION_NEW}/g" ./win_setup/flameshot.iss
sed -i "s/version: ${BASE_VERSION_CUR}/version: ${BASE_VERSION_NEW}/g" appveyor.yml
sed -i "s/VERSION=${BASE_VERSION_CUR}/VERSION=${BASE_VERSION_NEW}/g" .travis.yml

qmake

# push current release
git add flameshot.pro ./win_setup/flameshot.iss appveyor.yml .travis.yml
git commit -m "Update version to ${BASE_VERSION_NEW}"
git push

# add new release tag
git tag "v${BASE_VERSION_NEW}"
git push origin --tags

echo "New Flameshot version is: ${BASE_VERSION_NEW}"
