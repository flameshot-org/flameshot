---
title: Building the Flameshot .deb package
date: 2026-07-24
category: developer-experience
module: packaging/debian
problem_type: developer_experience
component: tooling
severity: low
applies_when:
  - Building a Debian/Ubuntu .deb of Flameshot locally or in CI
  - Reproducing a package build outside the official CI pipeline
  - Debugging why dpkg-buildpackage fails at the final dpkg-deb step
tags: [deb, debian, packaging, dpkg-buildpackage, debhelper, cmake, qt6]
---

# Building the Flameshot .deb package

## Context
Flameshot is a C++/CMake project, not a Node/Electron app, so there is no
`package.json`-driven packaging. The Debian packaging metadata lives under
`packaging/debian/` (`control`, `rules`, `changelog`, `compat`,
`copyright`, `source/format`, `docs`), **not** at the repository root where
`dpkg-buildpackage` expects a `debian/` directory. This mismatch, plus a
build-directory permission trap, makes the first-time build non-obvious.

## Guidance

### 1. Put the packaging metadata where dpkg expects it
`dpkg-buildpackage` reads `debian/` at the tree root. Copy (don't just
rely on the source location) the tracked metadata into place:

```bash
cp -r packaging/debian debian
```

A copy is preferable to a symlink because the build writes its staging tree
(`debian/flameshot/`, `debian/.debhelper/`) back into that directory; keep it
out of the tree afterward (it is untracked).

### 2. Build a binary-only, unsigned package
```bash
DEB_BUILD_OPTIONS="parallel=$(nproc)" dpkg-buildpackage -us -uc -b
```

- `-b` = binary-only (skips the source package; the source format is
  `3.0 (native)`, so a full source build is unnecessary for a local `.deb`).
- `-us -uc` = do not sign the `.changes`/`.dsc` (no GPG key needed).
- `parallel=N` speeds up the C++ compile.

Build dependencies (from `packaging/debian/control`): `cmake (>= 3.22~)`,
`debhelper (>= 12)`, `qt6-base-dev`, `qt6-tools-dev`, `qt6-tools-dev-tools`,
`qt6-svg-dev | libqt6svg6-dev`, `qt6-l10n-tools`, `libgl-dev`.

### 3. Work around the parent-directory write trap
`dpkg-buildpackage` writes the finished `.deb` into the **parent** of the
build tree (`../`). When the tree is at `/workspace`, the parent is `/`,
which is not writable, so the build fails only at the very end:

```
dpkg-deb: error: unable to create '../flameshot_14.0.rc3-1_amd64.deb': Permission denied
dh_builddeb: error: ... dpkg-deb --root-owner-group --build debian/flameshot .. returned exit code 2
```

The compile and install have already fully succeeded at this point — the
staging tree `debian/flameshot/` is complete. Build the `.deb` directly from
that staged tree into a writable location instead of re-running the whole
build:

```bash
dpkg-deb --root-owner-group --build debian/flameshot /workspace/flameshot_14.0.rc3-1_amd64.deb
# optional debug-symbols package:
dpkg-deb --root-owner-group --build debian/.debhelper/flameshot/dbgsym-root \
         /workspace/flameshot-dbgsym_14.0.rc3-1_amd64.deb
```

### 4. Verify the result without installing
```bash
dpkg-deb --info    flameshot_14.0.rc3-1_amd64.deb   # metadata + Depends
dpkg-deb --contents flameshot_14.0.rc3-1_amd64.deb  # payload
# Check dependency satisfaction against the system, install nothing:
apt-get install --simulate --no-install-recommends ./flameshot_14.0.rc3-1_amd64.deb
```
A clean run reports `0 upgraded, 1 newly installed, 0 to remove` when all
hard `Depends` are already present.

## Why This Matters
The final `dpkg-deb` failure is misleading: it appears after a long,
successful compile and reads like a build error, when it is purely a
filesystem-permission issue on the output directory. Knowing that the staged
tree is already complete turns a full rebuild (minutes) into a one-second
`dpkg-deb --build`. Knowing the metadata lives in `packaging/debian/` avoids
the "there is no `debian/` directory" dead end entirely.

## When to Apply
- Any local or scripted `.deb` build of this repo
- When `dpkg-buildpackage` errors at `dh_builddeb`/`dpkg-deb` with
  "Permission denied" on a `../*.deb` path
- When deciding where a packaging change (dependencies, install rules) should
  go — edit `packaging/debian/*`, the tracked source of truth, not a
  throwaway root `debian/`

## Examples

Two Flameshot-specific quirks encoded in `packaging/debian/rules`:

```makefile
override_dh_auto_configure:
	# The existence of an empty .git directory triggers syncqt.
	mkdir .git || true
	dh_auto_configure -- -DFETCHCONTENT_FULLY_DISCONNECTED=OFF

override_dh_auto_install:
	dh_auto_install
	rm -rf debian/flameshot/usr/include   # drop dev headers
	rm -rf debian/flameshot/usr/lib       # drop static libs
```

- The `.git` directory is required so CMake/Qt's `syncqt` runs; in a fresh
  checkout it already exists, and `mkdir .git || true` is a no-op.
- `FETCHCONTENT_FULLY_DISCONNECTED=OFF` lets CMake `FetchContent` pull the
  vendored deps (`qtcolorwidgets`, `kdsingleapplication`) — the build needs
  network access on a clean tree.

Package version comes from the top `packaging/debian/changelog` entry
(currently `14.0.rc3-1`). Note that CMake's own CPack config
(`CMakeLists.txt`, the `CPACK_*` block) uses the `TGZ` generator on Linux, not
`DEB` — CPack produces a tarball here, so the debhelper flow above is the path
to an actual `.deb`.

## Related
- `packaging/debian/` — tracked Debian metadata (source of truth)
- `packaging/rpm/flameshot.spec`, `snapcraft.yaml`, `PKGBUILD`,
  `packaging/flatpak/` — sibling packaging formats
- `CMakeLists.txt` (`CPACK_*` block) — CPack config (TGZ on Linux, WIX on Windows)
