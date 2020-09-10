#!/bin/sh
#===============================================================
# File URLs are valid for at least 30 days and up to a year (see below).
# Shortened URLs do not expire.
# Maximum file size: 512.0 MiB
# Blocked file types: application/x-dosexec, application/x-executable
#===============================================================

URL="https://0x0.st"

if [ $# -eq 0 ]; then
    echo "Usage: 0x0.st FILE\n"
    exit 1
fi

FILE=$1

if [ ! -f "$FILE" ]; then
    echo "File ${FILE} not found"
    exit 1
fi

RESPONSE=$(curl -# -F "file=@${FILE}" "${URL}")

echo "${RESPONSE}"  # to terminal
