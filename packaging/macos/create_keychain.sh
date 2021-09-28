#!/bin/bash
# Inspired by
# https://localazy.com/blog/how-to-automatically-sign-macos-apps-using-github-actions

TEMP_CI_CERT_FILENAME="temp_ci_appleDistribution.p12"

# Get the following variables from MacOS-pack.yaml:
# APP_NAME
# APPLE_DEV_IDENTITY
# APPLE_DEVELOPER_ID_APPLICATION_CERT_PASS
# APPLE_DEVELOPER_ID_APPLICATION_CERT_DATA
# APPLE_TEMP_CI_KEYCHAIN_PASS

# For the Community (if no Apple Developer ID available)
if [[ "${APPLE_DEV_IDENTITY}" == "" ]]; then
  echo "WARNING: No credentials for signing found"
  echo "WARNING: Cannot create keychain for signing"
  echo "WARNING: dmg package won't be signed and notarized"
  exit 0
fi

# create keychain
security create-keychain -p "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain
security default-keychain -s build.keychain
security unlock-keychain -p "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain

# import certificate
[ -r "${TEMP_CI_CERT_FILENAME}" ] && rm "${TEMP_CI_CERT_FILENAME}"
echo "${APPLE_DEVELOPER_ID_APPLICATION_CERT_DATA}" | base64 --decode > "${TEMP_CI_CERT_FILENAME}"
security import "${TEMP_CI_CERT_FILENAME}" -P "${APPLE_DEVELOPER_ID_APPLICATION_CERT_PASS}" -k build.keychain -T /usr/bin/codesign
[ -r "${TEMP_CI_CERT_FILENAME}" ] && rm "${TEMP_CI_CERT_FILENAME}"
security find-identity -v
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "${APPLE_TEMP_CI_KEYCHAIN_PASS}" build.keychain
