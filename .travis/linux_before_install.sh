#!/bin/bash --

set -e

if [[ "${EXTEN}" == "other" ]]; then
	travis_retry sudo apt update
fi
