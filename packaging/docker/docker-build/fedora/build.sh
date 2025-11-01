#!/usr/bin/env bash
set -euo pipefail

# Build helper for Fedora builder image
IMAGE_NAME=flameshot-builder-fedora:local
HERE=$(cd "$(dirname "$0")" && pwd)
# script is located at packaging/docker/docker-build/fedora
# go up 4 levels to reach repo root
REPO_ROOT=$(cd "$HERE/../../../.." && pwd)

echo "Building builder image: $IMAGE_NAME"
docker build -t "$IMAGE_NAME" -f "$HERE/Dockerfile" "$HERE"

echo "Running build inside container (output will be written to $REPO_ROOT/build)"
mkdir -p "$REPO_ROOT/build"
# capture host uid/gid to restore ownership after the container run
HOST_UID=$(id -u)
HOST_GID=$(id -g)

# Run cmake inside container with repo mounted at /src
docker run --rm -it --user "$HOST_UID:$HOST_GID" -v "$REPO_ROOT":/src -w /src "$IMAGE_NAME" \
  bash -lc "rm -rf build && cmake -S . -B build -DQT_VERSION_MAJOR=6 -DCMAKE_BUILD_TYPE=Release && cmake --build build -j\$(nproc)"

echo "Build finished. Artifacts in $REPO_ROOT/build (created as UID:GID $HOST_UID:$HOST_GID)"
