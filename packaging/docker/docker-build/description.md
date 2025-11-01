Builder image for Flameshot

Purpose
- Provides a reproducible environment to build Flameshot from source.
- Targets Ubuntu 24.04 and installs CMake (from Kitware APT), gcc/g++, Qt6 development packages, and common build tools.

Key details
- Base: ubuntu:24.04
- Installs: cmake, build-essential, git, qt6-base-dev, qt6-tools-dev, qt6-svg-dev, ninja-build, python3
- Creates user `builder` (uid 1000) for non-root builds

Usage (local)
1. Build the image from `packaging/docker/docker-build`:

   ```bash
   cd packaging/docker/docker-build
   docker build -t flameshot-builder:local .
   ```

2. Run a container to build the project (bind-mount your repo root into /src):

   ```bash
   REPO_ROOT=$(cd "$(dirname "$0")/../../.." && pwd)
   docker run --rm -it -v "$REPO_ROOT":/src -w /src flameshot-builder:local \
     bash -lc 'cmake -S . -B build -DQT_VERSION_MAJOR=6 -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)'
   ```

Notes for CI
- In CI, build the builder image first, run the same build step inside the builder image, and then copy artifacts into the runtime image (multi-stage or by exporting files).