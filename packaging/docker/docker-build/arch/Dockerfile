# Builder image for Flameshot (Arch Linux)
FROM archlinux:latest

ARG DEBIAN_FRONTEND=noninteractive

RUN pacman -Syu --noconfirm \
    && pacman -S --noconfirm base-devel cmake git qt6-base qt6-tools qt6-svg ninja python sudo \
    && pacman -Scc --noconfirm

RUN useradd -m -s /bin/bash builder && \
    echo "builder ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/builder

WORKDIR /work

CMD ["bash"]
