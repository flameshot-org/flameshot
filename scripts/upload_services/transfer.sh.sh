#!/bin/sh

URL="https://transfer.sh"

if [ $# -eq 0 ]; then
    echo "Usage: transfer.sh FILE\n"
    exit 1
fi

FILE=$1

if [ ! -f "$FILE" ]; then
    echo "File ${FILE} not found"
    exit 1
fi

RESPONSE=$(curl -# -F "file=@${FILE}" "${URL}")

echo "${RESPONSE}"  # to terminal