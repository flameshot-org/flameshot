#!/usr/bin/env sh

# Automated test script for drawTextShadow configuration setting
# Arguments:
# 1. path to tested flameshot executable (default: ./build/src/flameshot)

FLAMESHOT="$1"
[ -z "$FLAMESHOT" ] && FLAMESHOT="./build/src/flameshot"

if [ ! -x "$FLAMESHOT" ]; then
    echo "Error: Flameshot executable not found at '$FLAMESHOT'" >&2
    exit 1
fi

CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/flameshot"
CONFIG_FILE="$CONFIG_DIR/flameshot.ini"
BACKUP_FILE="$CONFIG_DIR/flameshot.ini.bak_test"

# Cleanup function to restore original config on exit
cleanup() {
    if [ -f "$BACKUP_FILE" ]; then
        mv "$BACKUP_FILE" "$CONFIG_FILE"
    else
        rm -f "$CONFIG_FILE"
    fi
}
trap cleanup EXIT INT TERM

mkdir -p "$CONFIG_DIR"
if [ -f "$CONFIG_FILE" ]; then
    cp "$CONFIG_FILE" "$BACKUP_FILE"
fi

echo "=== Test 1: Testing valid configuration 'drawTextShadow = true' ==="
cat <<EOF > "$CONFIG_FILE"
[General]
drawTextShadow=true
EOF

OUT=$("$FLAMESHOT" config --check 2>&1)
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    echo "PASS: 'drawTextShadow=true' is valid."
else
    echo "FAIL: 'drawTextShadow=true' check failed with output: $OUT" >&2
    exit 1
fi

echo "=== Test 2: Testing valid configuration 'drawTextShadow = false' ==="
cat <<EOF > "$CONFIG_FILE"
[General]
drawTextShadow=false
EOF

OUT=$("$FLAMESHOT" config --check 2>&1)
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    echo "PASS: 'drawTextShadow=false' is valid."
else
    echo "FAIL: 'drawTextShadow=false' check failed with output: $OUT" >&2
    exit 1
fi

echo "=== Test 3: Testing invalid configuration 'drawTextShadow = invalid_value' ==="
cat <<EOF > "$CONFIG_FILE"
[General]
drawTextShadow=invalid_value
EOF

OUT=$("$FLAMESHOT" config --check 2>&1)
EXIT_CODE=$?
if [ $EXIT_CODE -ne 0 ] || echo "$OUT" | grep -q "error"; then
    echo "PASS: Invalid value was correctly detected."
else
    echo "FAIL: Invalid value was unexpectedly accepted." >&2
    exit 1
fi

echo "All tests passed successfully!"
