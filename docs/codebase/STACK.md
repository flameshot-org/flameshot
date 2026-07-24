# Technology Stack

**Analysis Date:** 2026-07-24

## Languages

**Primary:**
- C++20 - all application code under `src/` (`.cpp`/`.h`). Enforced via
  `target_compile_features(project_options INTERFACE cxx_std_20)` in
  `CMakeLists.txt`.

**Secondary:**
- CMake - build system definitions (`CMakeLists.txt`, `src/CMakeLists.txt`,
  per-module `src/*/CMakeLists.txt`, `cmake/*.cmake`).
- Qt Linguist translation source (`.ts`, XML) - 48 files under
  `data/translations/Internationalization_*.ts`. NOTE: these are Qt `.ts`
  files, not TypeScript. They are compiled to `.qm` at build time and drive
  localization. There is no TypeScript/Node component in this repo.
- Nix - reproducible build/dev environment (`flake.nix`, `default.nix`,
  `shell.nix`, `flake.lock`).
- Shell - test scripts (`tests/*.sh`) and packaging helpers
  (`packaging/macos/create_dmg.sh`, `data/shell-completion/*`).
- Qt Designer UI XML (`.ui`) - a few widgets (e.g.
  `src/widgets/uploadhistory.ui`, `src/cli`/`src/widgets/*.ui`).

## Runtime

**Environment:**
- Native compiled desktop binary (`flameshot`). No managed runtime. Minimum
  Qt runtime 6.2.4; SVG plugin required at runtime.

**Package Managers:**
- CMake FetchContent - C++ dependency acquisition, pinned by git tag/hash; no
  classic lockfile (`CMakeLists.txt`).
- Nix flakes - `flake.lock` pins `nixpkgs`, `flake-parts`, `treefmt-nix`,
  `flake-compat`, `systems`.
- Distro system package managers consume the build (apt/dnf/pacman per
  `README.md`; deb/rpm built in CI).

## Frameworks and Tools

**Application:**
- Qt 6 (Widgets/GUI toolkit) - core framework; modules `Core`, `Gui`,
  `Widgets`, `Network`, `Svg`, `LinguistTools`, and `DBus` on UNIX
  (`src/CMakeLists.txt`).
- Qt-Color-Widgets - color picker UI widgets, fetched from GitLab
  `mattbas/Qt-Color-Widgets` (`CMakeLists.txt`; flake pins rev `3.0.0`).
- KDSingleApplication (KDAB) - single-instance enforcement, `v1.2.1` via
  FetchContent, bundled by default (`CMakeLists.txt`).
- QHotkey - global hotkey support, Windows and macOS only, fetched from
  `flameshot-org/QHotkey` (`CMakeLists.txt`, `src/CMakeLists.txt`).
- KF6 GuiAddons (`KF6::GuiAddons`) - optional Wayland clipboard
  (`KSystemClipboard`), enabled by `USE_WAYLAND_CLIPBOARD`
  (`src/CMakeLists.txt`).

**Build and Dev:**
- CMake >= 3.22 - primary build system (`CMakeLists.txt`).
- CPack - installer/archive generation: WIX + ZIP (Windows), ZIP (macOS),
  TGZ (Linux) (`CMakeLists.txt`).
- ccache - compiler cache wiring (`cmake/Cache.cmake`).
- clang-format - style enforcement, Mozilla-based (`.clang-format`).
- clang-tidy - static analysis, `WarningsAsErrors: '*'` (`.clang-tidy`,
  `cmake/StaticAnalyzers.cmake`).
- cmake-format - CMake formatting (`.cmake-format.yaml`).
- windeployqt / macdeployqt - Qt runtime bundling (`src/CMakeLists.txt`).
- MkDocs + mkdocs-material - developer docs site
  (`.github/workflows/deploy-dev-docs.yml`, `docs/dev/`).

**Testing:**
- Shell-based CLI option tests - `tests/action_options.sh`,
  `tests/path_option.sh`. No C++ unit-test framework detected.

## Key Dependencies

**Critical:**
- Qt 6 (`Core`/`Gui`/`Widgets`/`Network`/`Svg`) - the entire UI, capture,
  networking, and rendering stack (`src/CMakeLists.txt`).
- Qt DBus - Linux daemon IPC, portal screenshots, notifications
  (`src/CMakeLists.txt`, `src/core/`, `src/utils/`).
- QtColorWidgets - annotation color selection UI (`CMakeLists.txt`).
- KDSingleApplication - prevents multiple daemon instances (`CMakeLists.txt`).

**Infrastructure:**
- OpenSSL - required on Windows for HTTPS uploads; DLLs bundled by installer
  (`src/CMakeLists.txt`, `appveyor.yml`; `ENABLE_OPENSSL`).
- QHotkey - global shortcuts on Windows/macOS (`CMakeLists.txt`).
- grim - external CLI used at runtime for wlroots/Wayland capture (added to
  `PATH` by the Nix wrapper, `flake.nix` `postFixup`).
- xdg-desktop-portal (+ backend) - Wayland screenshot capture at runtime
  (`PKGBUILD` optdepends, `src/utils/screengrabber.cpp`).

## Configuration

**Build:**
- `CMakeLists.txt` - root build config, dependency fetching, CPack, version
  `FLAMESHOT_VERSION 14.0.0`.
- `src/CMakeLists.txt` - target, Qt modules, translations, install rules,
  windeployqt/macdeployqt.
- `cmake/` - `StandardProjectSettings.cmake`, `Cache.cmake`,
  `CompilerWarnings.cmake`, `Sanitizers.cmake`, `StaticAnalyzers.cmake`.
- `flake.nix` / `default.nix` / `shell.nix` - Nix build and dev shell.
- `snapcraft.yaml`, `PKGBUILD`, `packaging/` - distro packaging.
- Notable CMake options: `ENABLE_IMGUR` (OFF), `DISABLE_UPDATE_CHECKER`,
  `USE_KDSINGLEAPPLICATION` (ON), `USE_WAYLAND_CLIPBOARD` (OFF),
  `USE_PORTABLE_CONFIG` (ON on Windows), `USE_MONOCHROME_ICON`, `GENERATE_TS`.

**Environment:**
- `FLAMESHOT_PREDEFINED_COLOR_PALETTE_LARGE` - build-time palette selection
  (`src/CMakeLists.txt`).
- `GIT_HASH` - override embedded commit hash (`src/CMakeLists.txt`).
- `QTDIR` / `Qt6_DIR` - Qt install location for the build.
- `OPENSSL_ROOT_DIR` - Windows OpenSSL location (`appveyor.yml`).
- Runtime user config stored via `QSettings` INI (see Platform Requirements),
  not environment variables.

## Platform Requirements

**Development:**
- Qt >= 6.2.4 with development tools (`README.md`).
- GCC >= 11 (or MSVC 2022 on Windows; Clang supported).
- CMake >= 3.22.
- On Linux: `qt6-base`, `qt6-svg`, `qt6-tools`, `kguiaddons`
  (`README.md`, `PKGBUILD`).

**Production or Runtime:**
- Linux: Qt 6 base + SVG; DBus session bus; `xdg-desktop-portal` backend for
  Wayland; `grim` for wlroots compositors; optional `qt6-imageformats` for
  extra export formats. Config in `~/.config/flameshot/flameshot.ini`
  (`QSettings` IniFormat, UserScope, `src/utils/confighandler.cpp`).
- Windows: OpenSSL runtime DLLs (for uploads); portable config stored beside
  the executable (`flameshot.ini`, `USE_PORTABLE_CONFIG`); ships `flameshot`
  GUI + `flameshot-cli` console binary.
- macOS: deployment target 10.15; app bundle `org.flameshot.Flameshot`;
  code-signed (ad hoc or Developer ID) via `codesign`/`macdeployqt`.

---

*Stack analysis: 2026-07-24*
