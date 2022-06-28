#!/bin/bash

echo "Change the permission of .dmg file"
hdiutil convert "flameshot.dmg" -format UDRW -o "flameshot_rw.dmg"

echo "Mount it and save the device"
DEVICE=$(hdiutil attach -readwrite -noverify "flameshot_rw.dmg" | grep -E '^/dev/' | sed 1q | awk '{print $1}')
sleep 5

echo "Create the sysmbolic link to application folder"
PATH_AT_VOLUME="/Volumes/flameshot/"
CURRENT_PATH="$(pwd)"
cd "${PATH_AT_VOLUME}"
ln -s /Applications
cd "${CURRENT_PATH}"

# TODO - add background and icon location.
# https://forum.qt.io/topic/94987/how-can-i-add-symbolic-link-application-and-background-image-in-dmg-package/3
#echo "copy the background image in to package"
#mkdir -p "${PATH_AT_VOLUME}".background/
#cp backgroundImage.png "${PATH_AT_VOLUME}".background/
#echo "done"
#
## tell the Finder to resize the window, set the background,
##  change the icon size, place the icons in the right position, etc.
#echo '
#    tell application "Finder"
#    tell disk "/Volumes/src:flameshot"
#        open
#            set current view of container window to icon view
#            set toolbar visible of container window to false
#            set statusbar visible of container window to false
#            set the bounds of container window to {400, 100, 1110, 645}
#            set viewOptions to the icon view options of container window
#            set arrangement of viewOptions to not arranged
#            set icon size of viewOptions to 72
#            set background picture of viewOptions to file ".background:backgroundImage.png"
#            set position of item "flameshot.app" of container window to {160, 325}
#            set position of item "Applications" of container window to {560, 320}
#        close
#        open
#        update without registering applications
#        delay 2
#    end tell
#    end tell
#' | osascript
#
#sync

# unmount it
hdiutil detach "${DEVICE}"
rm -f "flameshot.dmg"

hdiutil convert "flameshot_rw.dmg" -format UDZO -o "flameshot.dmg"
rm -f "flameshot_rw.dmg"
