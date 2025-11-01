#!/usr/bin/env bash
# Simple wrapper to run flameshot inside the runtime container with common host mounts
set -euo pipefail

# Usage: run-flameshot.sh [--gui|--full|other args]
# This script assumes it's executed inside the container. For host-side helper, see packaging/docker/readme.md

if [[ -z "${DISPLAY:-}" && -z "${WAYLAND_DISPLAY:-}" ]]; then
  echo "No DISPLAY or WAYLAND_DISPLAY set; running in CLI mode"
  exec /usr/local/bin/flameshot "$@"
else
  exec /usr/local/bin/flameshot "$@"
fi
