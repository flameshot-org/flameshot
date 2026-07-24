# Testing Patterns

**Analysis Date:** 2026-07-24

**Honest summary:** Flameshot has **no automated unit-test suite**. There is no
Qt Test / QtTest, no CTest test registration, and no C++ test framework wired
into the build. The only tests are two **manual, interactive shell scripts** in
`tests/` that a human runs against a built binary and visually verifies. CI
"testing" is effectively build verification plus a formatting lint. Treat test
coverage as a significant gap (see `docs/codebase/CONCERNS.md` if present).

## Test Framework

**Runner:**
- No automated test runner. `grep` for `add_test`, `enable_testing`, `QtTest`,
  `QTest`, `gtest`, `catch` across `CMakeLists.txt`, `src/`, and `cmake/`
  returns nothing — no unit-test target is defined anywhere.
- Note: `.github/workflows/build_cmake.yml` runs `ctest -C $BUILD_TYPE` after
  the Linux build, but because no tests are registered with CMake, CTest finds
  zero tests. It passes vacuously and does not verify behavior.

**Assertion Library:**
- Not detected. The manual scripts rely on human visual inspection and system
  notifications, not assertions.

**Run Commands:**
```bash
# Build first (from repo root)
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build

# Manual interactive tests (require a running desktop session + flameshot daemon)
sh tests/action_options.sh ./build/src/flameshot   # final-action / CLI options walkthrough
sh tests/path_option.sh    ./build/src/flameshot   # --path / -p output-path behavior

# CTest (currently finds no registered tests; runs vacuously)
ctest --test-dir build -C RelWithDebInfo
```

## Test File Organization

**Location:**
- Separate top-level `tests/` directory. Not co-located with `src/`.

**Naming:**
- `<feature>_<subject>.sh` — `tests/action_options.sh`, `tests/path_option.sh`.

**Structure:**
```text
tests/
|-- action_options.sh   # exercises `flameshot full|screen|gui` with
|                        #   --path/--clipboard/--raw/--pin/--print-geometry/
|                        #   --accept-on-select; human confirms notifications & pins
`-- path_option.sh       # exercises `flameshot screen -p <path>` path resolution:
                         #   nonexistent dir errors, relative vs absolute paths,
                         #   redundancy removal, dir vs file, suffix handling,
                         #   _NUM de-duplication of existing files
```

## Test Structure

**Suite Organization:**
```text
Both scripts are POSIX sh (#!/usr/bin/env sh) that:
- accept the flameshot executable path as $1 (default: "flameshot" on PATH)
- run a sequence of real CLI invocations against a live daemon
- print a human-readable description before each command (echo / cmd helper)
- pause for human input between GUI cases (wait_for_key -> read)
- surface results via stdout, the `display` image viewer, and desktop
  notifications (notify-send on Linux, osascript on macOS)
```

**Patterns:**
- Setup: `path_option.sh` creates and `cd`s into a temp dir
  `/tmp/flameshot_path_test`; `action_options.sh` writes output to `/tmp/`.
- A `flameshot()` shell wrapper appends `--raw >/tmp/img.png` as a hack to make
  the subcommand block until the daemon finishes the pending action.
- Assertion: none automated. The operator reads the printed expectation, then
  visually confirms the app behavior / notification / pinned screenshot.
- Teardown: `path_option.sh` removes its temp dir at start of the run
  (`rm -rf /tmp/flameshot_path_test`). No explicit teardown otherwise.

## Mocking

**Framework:** Not detected.

**Patterns:**
```text
No mocking. Tests drive the real application binary and a real desktop
environment (X11/Wayland/macOS), producing real screenshots and notifications.
```

**What to Mock:**
- Not applicable — there is no isolated unit-test layer to mock into.

**What Not to Mock:**
- Not applicable.

## Fixtures and Factories

**Test Data:**
```text
No fixture files. Test "data" is generated at runtime: screenshots captured by
the running binary and written to /tmp (e.g. /tmp/img.png, files under
/tmp/flameshot_path_test).
```

**Location:**
- `tests/` (scripts only); runtime artifacts under `/tmp/`.

## Coverage

**Requirements:** None enforced. No coverage tooling (gcov/lcov/llvm-cov) is
configured in CMake or CI.

**View Coverage:**
```bash
# Not applicable — no coverage instrumentation is configured.
```

## Test Types

**Unit Tests:**
- Not used. No C++ unit tests exist.

**Integration Tests:**
- Manual, end-to-end only: the two `tests/*.sh` scripts exercise the CLI and GUI
  against a live daemon and require human verification.

**E2E Tests:**
- Only the manual shell walkthroughs above. No automated GUI/E2E harness.

## Continuous Integration

CI is defined under `.github/workflows/` and `appveyor.yml`. What actually runs:

- `build_cmake.yml` — builds on Ubuntu 24.04 (Qt6) and Windows (MSVC, Qt 6.9.3);
  Linux job runs `ctest` (finds no tests). This is the primary correctness gate:
  **the code must compile on Linux and Windows.**
- `clang-format.yml` — formatting lint over `./src` with clang-format 11
  (`DoozyX/clang-format-lint-action@v0.18`). Fails on unformatted code.
- `Linux-pack.yml`, `MacOS-pack.yml`, `Windows-pack.yml` — packaging/build of
  distributables per platform.
- `deploy-dev-docs.yml` — documentation deploy.
- `appveyor.yml` — Windows installer (WIX/MSI) + portable exe build & signing.

**Practical guidance for contributors:** since there is no behavioral test net,
verify changes by (1) building on Linux and Windows to satisfy CI, (2) running
`clang-format` (v11) to pass the lint gate, and (3) manually running the
relevant `tests/*.sh` script against your build when touching CLI/path/action
behavior. When adding meaningful logic, consider introducing a real Qt Test /
CTest target under a new test source tree and registering it with `add_test`, as
none exists today.

---

*Testing analysis: 2026-07-24*
