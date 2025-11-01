Docker packaging for Flameshot

This folder contains Docker scaffolding for building and running Flameshot.

Structure

- docker-build/: builder image and helper scripts
- docker-run/: runtime image and run helper

Quickstart (local)

Build builder image and run a build:

```bash
cd packaging/docker/docker-build
./build.sh
```

Build runtime image (example using the builder image name):

```bash
cd packaging/docker/docker-run
# Option A: runtime Dockerfile expects a builder image named flameshot-builder:local
docker build --build-arg BUILDER_IMAGE=flameshot-builder:local -t flameshot:local .

# Option B: copy binary from host into a build context and build
# (not shown here)
```

Run locally (CLI):

```bash
docker run --rm -v "$PWD":/work -w /work flameshot:local --help
```

Run GUI (X11 example):

```bash
xhost +local:docker
docker run --rm -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri --network=host flameshot:local gui
xhost -local:docker
```

See `docker-build/description.md` and `docker-run/description.md` for more details and CI notes.