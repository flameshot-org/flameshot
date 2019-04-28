#!/bin/sh

#==========================================
# 100 uploads per day, 5GB file size limit for FREE plan.
#==========================================

URL="https://file.io"
DEFAULT_EXPIRE="14d" # Default to 14 days

if [ $# -eq 0 ]; then
    echo "Usage: file.io FILE [DURATION]\n"
    echo "Example: file.io path/to/my/file 1w\n"
    exit 1
fi

FILE=$1
EXPIRE=${2:-$DEFAULT_EXPIRE}

if [ ! -f "$FILE" ]; then
    echo "File ${FILE} not found"
    exit 1
fi

RESPONSE=$(curl -# -F "file=@${FILE}" "${URL}/?expires=${EXPIRE}")

echo "${RESPONSE}" # to terminal
