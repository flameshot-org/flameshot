---
title: Building the .deb package with Google Drive uploader support
date: 2026-07-24
category: developer-experience
module: packaging
problem_type: developer_experience
component: tooling
severity: medium
applies_when:
  - Building a Debian .deb package that must include Google Drive upload support
  - A CMake feature is gated behind an option that defaults OFF
  - A stale build directory may hold a previous cache value for the option
  - Verifying that an optional feature was actually compiled into the binary
tags:
  - flameshot
  - debian-packaging
  - cmake
  - google-drive
  - deb
  - qt6
  - build-options
related_components:
  - documentation
---

# Building the .deb package with Google Drive uploader support

## Context

Google Drive support in Flameshot is not a runtime setting — it is a compile-time
CMake option that is **OFF by default**:

- `CMakeLists.txt:88` — `option(ENABLE_GDRIVE "Enable Google Drive Uploader" OFF)`
- `CMakeLists.txt:94-96` — `if(ENABLE_GDRIVE)` adds the `ENABLE_GDRIVE` compile definition.

Because the default is OFF, the standard `.deb` build procedure (see
[building-the-deb-package.md](building-the-deb-package.md)) produces a binary with
**no Drive support at all**. The build succeeds, the package installs cleanly, and
nothing warns you — the Drive code paths simply were never compiled in. This is the
non-obvious part: a correct-looking `.deb` can silently ship without the feature you
were packaging it for.

The Debian packaging manifests never set an upload-backend flag. During the Drive
feature work, a grep for `ENABLE_IMGUR` across `packaging/`, `snapcraft.yaml`,
`PKGBUILD`, and CI files found **zero references** — the manifests build with CMake
defaults, so the flag was deliberately left unwired (there was nothing to wire it
into for Imgur either). The consequence: a `.deb` from the stock
`packaging/debian/rules` gets **neither Imgur nor Drive**, and the earlier packaged
`.deb` documented in the base doc predates the Drive feature entirely. Enabling Drive
in a package is therefore a deliberate, separate step. (session history)

There is a second implication worth knowing up front. Enabling Drive also turns on the
shared, backend-neutral upload infrastructure:

- `CMakeLists.txt:101-103` — `if(ENABLE_IMGUR OR ENABLE_GDRIVE)` adds `ENABLE_UPLOADER`,
  which gates the upload tool type, its keyboard shortcuts, the upload dialogs, and
  upload history.

So `-DENABLE_GDRIVE=ON` is not a narrow flag — it lights up the whole uploader
UI/plumbing that Drive rides on, even if Imgur stays off.

## Guidance

Build the `.deb` exactly as the base doc describes, with one flag added and one cache
precaution. The delta from the base flow is small; do not re-run the whole base
procedure from memory — follow the base doc and apply the changes below.

1. **Add `-DENABLE_GDRIVE=ON` to the CMake configure step.** The packaging rules
   already override `dh_auto_configure`. In `packaging/debian/rules`:

   ```makefile
   override_dh_auto_configure:
   	# The existence of an empty .git directory triggers syncqt.
   	mkdir .git || true
   	# This is required to use Cmake FetchContent
   	dh_auto_configure -- \
   		-DFETCHCONTENT_FULLY_DISCONNECTED=OFF \
   		-DENABLE_GDRIVE=ON
   ```

   Note the trailing backslash added to the `-DFETCHCONTENT...` line so the new flag
   joins the same `dh_auto_configure --` invocation.

   - For a **permanent** packaging change, edit the tracked `packaging/debian/rules`.
   - For a **one-off local build**, edit the throwaway root `debian/rules` (the copy
     `dpkg-buildpackage` actually reads after you copy `packaging/debian` → `debian/`).

2. **Clear stale build state before rebuilding.** If a prior default build ran, its
   CMake cache and staging tree persist and can defeat the new flag. As a safety measure
   to force a fresh reconfigure, remove them first:

   ```bash
   rm -rf obj-x86_64-linux-gnu debian/flameshot debian/.debhelper
   ```

3. **Build**, following the base doc: copy `packaging/debian` → `debian/`, then run
   `DEB_BUILD_OPTIONS="parallel=$(nproc)" dpkg-buildpackage -us -uc -b`.

4. **Confirm the flag actually took** after the configure step:

   ```bash
   grep ENABLE_GDRIVE obj-x86_64-linux-gnu/CMakeCache.txt
   # expect: ENABLE_GDRIVE:BOOL=ON
   ```

5. **Assemble the package from the staged tree.** As documented in the base doc, the
   final `dpkg-deb` write to `../*.deb` fails because the parent of `/workspace` (`/`)
   is not writable, but `debian/flameshot/` is already complete. Build directly from it:

   ```bash
   dpkg-deb --root-owner-group --build debian/flameshot /workspace/flameshot_14.0.rc3-1_amd64.deb
   ```

6. **Verify Drive is compiled into the binary** — no install needed, inspect the staged
   binary:

   ```bash
   strings -a debian/flameshot/usr/bin/flameshot | grep -iE "drive|oauth"
   ```

   Expect symbols/strings such as `GDriveUploader`, `GDriveOAuth`, and
   `"Could not obtain a Google Drive access token."`

Runtime `Depends` are unchanged versus a non-Drive build — Drive uses `libqt6network6`,
which is already a dependency (the OAuth loopback listener runs on the already-linked
`Qt6::Network`, so no new packaging dependency is pulled in). Package version is
`14.0.rc3-1`, arch `amd64`; the Drive-enabled package is ~848 KB versus ~805 KB without it.

## Why This Matters

- **Silent feature omission.** The default-OFF flag means the normal build path gives
  you a package that looks finished — compiles, links, installs, runs — but is missing
  the entire Drive feature. There is no error to catch; only inspecting the binary (or
  noticing the feature is gone at runtime) reveals it. If you are packaging *for* Drive,
  forgetting the flag ships the wrong artifact.
- **The cache trap makes a "fixed" build lie.** After you add `-DENABLE_GDRIVE=ON`, a
  leftover `obj-x86_64-linux-gnu/CMakeCache.txt` from an earlier default build still
  holds the OFF value. Without a clean reconfigure, the build can proceed on the stale
  cache and produce another Drive-less package — so the fix appears applied but the
  output is unchanged. The `rm -rf` of the build/staging dirs and the
  `grep ...CMakeCache.txt` check exist to close exactly this gap: force the reconfigure,
  then prove it flipped.
- **One flag, wider surface.** Because `ENABLE_GDRIVE` also implies `ENABLE_UPLOADER`
  (`CMakeLists.txt:101-103`), turning it on changes more than the Drive backend; the
  shared upload UI, shortcuts, dialogs, and history come along too. Reviewers and testers
  should expect that broader change, not just a Drive button.
- **Packaging does not track feature flags for you.** The manifests build with CMake
  defaults and never referenced `ENABLE_IMGUR`, so there is no precedent to copy and no
  automatic wiring — enabling Drive in a package is a conscious edit to `rules`, not a
  side effect of the feature landing in the source tree. (session history)

## When to Apply

- Building or packaging a `.deb` — or running any CMake configure of this project —
  that must include Google Drive support. The flag is mandatory; nothing else enables Drive.
- Deciding *where* the flag belongs:
  - **Tracked** (`packaging/debian/rules`) when Drive should be part of the official
    package going forward.
  - **One-off** (root `debian/rules`, the copy dpkg reads) for a throwaway local build
    you don't intend to commit.
- Any time you rebuild after a previous default build in the same tree — apply the
  stale-cache clean and the `CMakeCache.txt` check before trusting the output.

## Examples

**`packaging/debian/rules` — before / after:**

```diff
 override_dh_auto_configure:
 	mkdir .git || true
 	dh_auto_configure -- \
-		-DFETCHCONTENT_FULLY_DISCONNECTED=OFF
+		-DFETCHCONTENT_FULLY_DISCONNECTED=OFF \
+		-DENABLE_GDRIVE=ON
```

**Force a clean reconfigure, then confirm the flag flipped:**

```bash
rm -rf obj-x86_64-linux-gnu debian/flameshot debian/.debhelper
# ... run dpkg-buildpackage ...
grep ENABLE_GDRIVE obj-x86_64-linux-gnu/CMakeCache.txt
# ENABLE_GDRIVE:BOOL=ON
```

**Verify Drive is actually in the staged binary:**

```bash
strings -a debian/flameshot/usr/bin/flameshot | grep -iE "drive|oauth"
# GDriveUploader
# GDriveOAuth
# Could not obtain a Google Drive access token.
```

## Related

- [building-the-deb-package.md](building-the-deb-package.md) — the base `.deb` build
  procedure. This doc is the Drive-enabling delta on top of it: same
  `packaging/debian/rules` configure step, same parent-directory write trap, same
  staged-tree `dpkg-deb` assembly.
- [../security-issues/qsettings-deferred-write-defeats-permission-chmod.md](../security-issues/qsettings-deferred-write-defeats-permission-chmod.md)
  — tangential: OAuth refresh-token storage for the Drive uploader that `ENABLE_GDRIVE`
  turns on (credential handling, not the build flag).
- `CMakeLists.txt` (`ENABLE_GDRIVE`, `ENABLE_IMGUR`, `ENABLE_UPLOADER` options) — the
  source of truth for what each flag compiles in.
- [../logic-errors/gdrive-visibility-ui-missing-on-drive-only-builds.md](../logic-errors/gdrive-visibility-ui-missing-on-drive-only-builds.md)
  — a complementary failure mode: this doc is about Drive not being compiled in at
  all; that one is about Drive being compiled in and working, but its sharing UI
  hiding itself anyway due to a backend-resolution mismatch between `ImgUploadDialog`
  and `ImgUploaderManager`.
