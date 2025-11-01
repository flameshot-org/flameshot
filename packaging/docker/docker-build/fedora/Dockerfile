# Builder image for Flameshot (Fedora)
FROM registry.fedoraproject.org/fedora:40

ARG DEBIAN_FRONTEND=noninteractive

RUN dnf -y update && \
    dnf -y install \
        cmake \
        gcc-c++ \
        make \
        git \
        pkgconfig \
        ninja-build \
        qt6-qtbase-devel \
        qt6-qtsvg-devel \
        qt6-qttools-devel \
        qt6-qttools-static \
        kf6-kguiaddons-devel \
        python3 \
        sudo \
        which && \
    dnf clean all

RUN useradd -m -s /bin/bash builder && \
    echo "builder ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder

WORKDIR /work

CMD ["bash"]
