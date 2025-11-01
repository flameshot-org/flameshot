# Builder image for Flameshot (Ubuntu 24.04)
# Purpose: provide a reproducible build environment with CMake >= 3.29 and Qt6 dev packages.
# Usage: build this image, then mount the repo and run cmake/cmake --build inside a container or use this image in CI.

FROM ubuntu:24.04

ARG DEBIAN_FRONTEND=noninteractive

# Install basic tools and add Kitware APT repo for modern CMake
RUN apt-get update \
    && apt-get install -y --no-install-recommends software-properties-common ca-certificates gnupg curl wget lsb-release apt-transport-https \
    && wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor -o /usr/share/keyrings/kitware-archive-keyring.gpg \
    && echo "deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main" > /etc/apt/sources.list.d/kitware.list \
    && apt-get update

# Install build deps (CMake from Kitware, compilers, Qt dev packages, tools)
RUN apt-get install -y --no-install-recommends \
    cmake \
    build-essential \
    git \
    pkg-config \
    ninja-build \
    ca-certificates \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-svg-dev \
    qt6-tools-dev-tools \
    qtmultimedia5-dev \
    libxcb-xinerama0-dev \
    libxcb1-dev \
    libxcb-util-dev \
    libxcb-render-util0-dev \
    libxcb-render0-dev \
    libx11-dev \
    python3 \
    python3-pip \
    sudo \
    curl \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user for builds
ARG BUILD_USER=builder
RUN useradd -m -s /bin/bash ${BUILD_USER} && \
    echo "${BUILD_USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/${BUILD_USER}

WORKDIR /work

# Default entrypoint is bash so CI/scripts can run cmake inside the container
CMD ["bash"]
