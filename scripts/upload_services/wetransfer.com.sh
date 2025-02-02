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

# check if the wetransfer python script is uptodate
REMOTE_SHA="$(curl --silent --location "https://github.com/iamleot/transferwee/raw/master/transferwee.py" | sha1sum | grep -o -E '^[0-9a-f]+')"
LOCAL_SHA="$(sha1sum 'scripts/upload_services/transferwee.py' | grep -o -E '^[0-9a-f]+')"
if [ "${}" != "${}" ]; then
    echo 'The transferwee.py is not fully uptodate. Consider checking the diff'
fi

FILE="${1}"

if [ ! -f "$FILE" ]; then
    echo "File '${FILE}' not found"
    exit 1
fi


SCRIPTS_PATH=`dirname ${0}`

python3 -m pip install -U -q requests

RESPONSE="$(python3 "${SCRIPTS_PATH}/transferwee.py" upload "${FILE}")"

echo "${RESPONSE}"  # to terminal
