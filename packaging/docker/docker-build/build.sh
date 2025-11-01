#!/usr/bin/env bash
# Convenience script: build the builder image and run a build inside it, producing host-mounted artifacts.
set -euo pipefail

HERE=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$HERE/../../.." && pwd)
IMAGE_NAME=flameshot-builder:local

echo "Building builder image: $IMAGE_NAME"
docker build -t "$IMAGE_NAME" "$HERE"

echo "Running build inside container (output will be written to $REPO_ROOT/build)"
# Run cmake inside container with repo mounted at /src
docker run --rm -it -v "$REPO_ROOT":/src -w /src "$IMAGE_NAME" \
  bash -lc "rm -rf build && cmake -S . -B build -DQT_VERSION_MAJOR=6 -DCMAKE_BUILD_TYPE=Release && cmake --build build -j\$(nproc)"

echo "Build finished. Artifacts in $REPO_ROOT/build" 
