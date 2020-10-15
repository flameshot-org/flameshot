#!/bin/sh
#=========================================================================================================================
# WeTransfer is a service to send big or small files from A to B. 
# It can transfer any type of file - such as presentations, photos, videos, music or documents - to friends and colleagues.
# You can send files up to 2 GB and they will be available for 7 days, with no registration.

# API doc: https://developers.wetransfer.com/documentation
# Using transferwee.py: https://github.com/iamleot/transferwee
#=========================================================================================================================

if [ $# -eq 0 ]; then
    echo "Usage: python3 transferwee.py FILE\n"
    exit 1
fi

FILE=$1

if [ ! -f "$FILE" ]; then
    echo "File ${FILE} not found"
    exit 1
fi


scripts_path=`dirname $0`

python3 -m pip install -U -q requests

RESPONSE=$(python3 ${scripts_path}/transferwee.py upload "${FILE}")

echo "${RESPONSE}"  # to terminal
