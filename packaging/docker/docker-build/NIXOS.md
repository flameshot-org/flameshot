# NixOS note

NixOS has an uncommon runtime layout and uses immutable package store paths. Building Flameshot for NixOS is best done using `nix-shell` or `nix build` as described in the main `README.md` under the NixOS section.

This repository provides linux distro builder images for more traditional distributions (Ubuntu, Fedora, Arch). NixOS users should prefer the native `nix-shell` workflow instead of these Docker builders.
