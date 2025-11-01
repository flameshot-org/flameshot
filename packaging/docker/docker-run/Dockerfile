# Runtime image for Flameshot
# This Dockerfile expects a builder image (produced earlier) that contains the built binary at /src/build/src/flameshot

ARG BUILDER_IMAGE=flameshot-builder:local
FROM ubuntu:24.04 AS runtime-base

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6network6 \
    libqt6svg6 \
    libqt6dbus6 \
    libxcb1 \
    libxcb-render0 \
    libxcb-shm0 \
    libx11-6 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Copy binary from builder image. This uses an external image named by BUILDER_IMAGE.
# CI should ensure the builder image includes the compiled binary at /src/build/src/flameshot
FROM ${BUILDER_IMAGE} as builder

FROM runtime-base
COPY --from=builder /src/build/src/flameshot /usr/local/bin/flameshot
RUN chmod +x /usr/local/bin/flameshot

# Create non-root user
RUN useradd -m flameshot && mkdir -p /home/flameshot/.cache
USER flameshot
WORKDIR /home/flameshot

ENTRYPOINT ["/usr/local/bin/flameshot"]
CMD ["--help"]
