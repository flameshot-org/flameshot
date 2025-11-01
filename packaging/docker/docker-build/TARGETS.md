# Available builder targets

This folder contains per-OS builder images that aim to reproduce the native development environment for the target OS/distro. Use the matching builder to produce a binary that behaves like a native build on that OS.

Available targets:

- `ubuntu24` — Ubuntu 24.04 based builder 
- `fedora` — Fedora 40 based builder
- `arch` — Arch Linux based builder

Notes:
- These builders produce Linux ELF binaries. They are intended to match the target distro's toolchain and runtime packages. They do not produce macOS or Windows binaries.
- NixOS users should use `nix-shell` or refer to `NIXOS.md` for guidance.

Usage example (Fedora):

```bash
cd packaging/docker/docker-build/fedora
./build.sh
# artifacts will be placed in the repo's build/ directory
```
