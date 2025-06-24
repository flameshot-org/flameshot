#!/bin/bash

# Script to create and optionally sign a DMG file
# Usage: create_dmg.sh <app_path> <dmg_path> <app_sign_identity> <dmg_sign_identity>
# If signing identities are empty, DMG will be created unsigned

APP_PATH="$1"
DMG_PATH="$2"
APP_SIGN_IDENTITY="$3"
DMG_SIGN_IDENTITY="$4"

if [ $# -ne 4 ]; then
    echo "Usage: create_dmg.sh <app_path> <dmg_path> <app_sign_identity> <dmg_sign_identity>"
    echo "Note: Leave signing identities empty to create unsigned DMG"
    exit 1
fi

echo "Creating DMG from: $APP_PATH"
echo "Output DMG: $DMG_PATH"

rm -f "$DMG_PATH"

TEMP_DIR=$(mktemp -d)
echo "Using temp directory: $TEMP_DIR"

cp -R "$APP_PATH" "$TEMP_DIR/"

# Create Applications symlink
ln -s /Applications "$TEMP_DIR/Applications"

# Calculate size needed for DMG (in KB)
SIZE=$(du -sk "$TEMP_DIR" | cut -f1)
SIZE=$((SIZE + 1000))  # Add some padding

echo "Creating DMG with size: ${SIZE}k"

# Create DMG
hdiutil create -srcfolder "$TEMP_DIR" \
    -volname "Flameshot" \
    -fs HFS+ \
    -fsargs "-c c=64,a=16,e=16" \
    -format UDZO \
    -size ${SIZE}k \
    "$DMG_PATH"

if [ $? -ne 0 ]; then
    echo "Failed to create DMG"
    rm -rf "$TEMP_DIR"
    exit 1
fi

echo "DMG created successfully"

# Sign the DMG (either with identity or ad hoc)
if [ -n "$DMG_SIGN_IDENTITY" ] && [ "$DMG_SIGN_IDENTITY" != "" ]; then
    echo "Signing DMG with identity: $DMG_SIGN_IDENTITY"
    codesign --force --sign "$DMG_SIGN_IDENTITY" --timestamp "$DMG_PATH"

    if [ $? -eq 0 ]; then
        echo "DMG signed with Developer ID"
        # Verify signature
        echo "Verifying DMG signature..."
        codesign --verify --verbose "$DMG_PATH"
    else
        echo "Failed to sign DMG with identity"
        rm -rf "$TEMP_DIR"
        exit 1
    fi
else
    echo "Signing DMG with ad hoc signature (no identity required)"
    codesign --force --sign - "$DMG_PATH"
    
    if [ $? -eq 0 ]; then
        echo "DMG signed with ad hoc signature"
    else
        echo "Failed to ad hoc sign DMG"
        rm -rf "$TEMP_DIR"
        exit 1
    fi
fi

# Clean up
rm -rf "$TEMP_DIR"

echo "DMG creation complete: $DMG_PATH"

if [ -z "$DMG_SIGN_IDENTITY" ] || [ "$DMG_SIGN_IDENTITY" = "" ]; then
    echo ""
    echo "NOTE: This DMG uses ad hoc signing (no Developer ID required)."
    echo "Users will see a security warning but can still run the app by:"
    echo "1. Right-clicking the app and selecting 'Open'"
    echo "2. Or going to System Preferences > Security & Privacy and clicking 'Open Anyway'"
    echo "3. The warning only appears on first launch"
    echo ""
    echo "The app and DMG are properly signed for Apple Silicon requirements."
fi
