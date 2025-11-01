Runtime image for Flameshot

Purpose
- Small runtime image that contains the Flameshot binary and minimal Qt runtime libraries.

Key details
- Base: ubuntu:24.04 (slim-ish runtime layer)
- Expects compiled binary to be available in the builder image at `/src/build/src/flameshot`.
- Copies binary into `/usr/local/bin/flameshot` and runs as non-root `flameshot` user.

Usage (local with manual copy)
- If you built locally using the builder container and produced `/path/to/repo/build/src/flameshot`, you can build the runtime with a local context that places the binary in the right place or use the builder image approach described in the builder's README.

Run examples (GUI)
- GUI mode requires host display socket, DBus and possibly GPU device access. Example (X11):

```bash
xhost +local:docker
docker run --rm -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --device /dev/dri \
  --network=host \
  flameshot:local gui
xhost -local:docker
```

Notes
- For Wayland or safer socket forwarding, see `packaging/docker/readme.md` for more advanced examples.