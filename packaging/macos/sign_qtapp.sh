#!/bin/bash
# Inspired by
# https://localazy.com/blog/how-to-automatically-sign-macos-apps-using-github-actions
# https://forum.qt.io/topic/96652/how-to-notarize-qt-application-on-macos/18

# Get the following variables from the MacOS-pack.yaml:
# APP_NAME
# APPLE_DEV_IDENTITY
# APPLE_DEV_USER
# APPLE_DEV_PASS

# For the Community (if no Apple Developer ID available)
if [[ "${APPLE_DEV_IDENTITY}" == "" ]]; then
  echo "WARNING: No credentials for signing found"
  echo "WARNING: dmg package won't be signed and notarized"
  echo "--> Start packaging process"
  "$(brew --prefix qt5)/bin/macdeployqt" "${APP_NAME}.app" -dmg
  echo "--> Update dmg package links"
  "./${HELPERS_SCRIPTS_PATH}/update_package.sh"
  exit 0
fi

echo "--> Start application signing process"
codesign --sign "${APPLE_DEV_IDENTITY}" --verbose --deep "${APP_NAME}.app"

echo "--> Start packaging process"
"$(brew --prefix qt5)/bin/macdeployqt" "${APP_NAME}.app" -dmg -sign-for-notarization="${APPLE_DEV_IDENTITY}"

echo "--> Update dmg package links"
"./${HELPERS_SCRIPTS_PATH}/update_package.sh"

echo "--> Start dmg signing process"
codesign --sign "${APPLE_DEV_IDENTITY}" --verbose --deep "${APP_NAME}.dmg"

echo "--> Start Notarization process"
response=$(xcrun altool -t osx -f "${APP_NAME}.dmg" --primary-bundle-id "org.namecheap.${APP_NAME}" --notarize-app -u "${APPLE_DEV_USER}" -p "${APPLE_DEV_PASS}")
requestUUID=$(echo "${response}" | tr ' ' '\n' | tail -1)

for ((ATTEMPT=5; ATTEMPT>=1; ATTEMPT--))
do
  echo "--> Checking notarization status"
  statusCheckResponse=$(xcrun altool --notarization-info "${requestUUID}" -u "${APPLE_DEV_USER}" -p "${APPLE_DEV_PASS}")

  isSuccess=$(echo "${statusCheckResponse}" | grep "success")
  isFailure=$(echo "${statusCheckResponse}" | grep "invalid")

  if [[ "${isSuccess}" != "" ]]; then
    echo "Notarization done!"
    xcrun stapler staple "${APP_NAME}.dmg"
    EXIT_CODE=$?
    if [ ${EXIT_CODE} -ne 0 ]; then
      echo "Stapler failed!"
      exit ${EXIT_CODE}
    fi
    echo "Stapler done!"
    break
  fi
  if [[ "${isFailure}" != "" ]]; then
    echo "${statusCheckResponse}"
    echo "Notarization failed"
    exit 1
  fi

  echo "Notarization not finished yet, sleep 2m then check again..."
  for num in {1..12}
  do
    sleep 10
    echo "Elapsed: ${num}0 sec"
  done
done

if [[ "${ATTEMPT}" == 0 ]]; then
  export NOTARIZATION_CHECK="false"
  echo "::warning Notarization check failed"
else
  export NOTARIZATION_CHECK="true"
fi

echo "--> Start verify signing process"
codesign -dv --verbose=4 "${APP_NAME}.dmg"
