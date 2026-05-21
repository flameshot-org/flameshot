# Flameshot — Prioritized Backlog (post-v14 beta 2)

**Date:** 2026-05-20 (updated 22:40)
**Source:** [Staff Engineer Panel Review](../reviews/20260520-1620-flameshot-staff-review.md)
**Repo state:** branch `pr1-turn-on-alarms` @ `72fdd9bd`, 5 commits ahead of `master`, working tree clean.

---

## Progress Log

### 2026-05-20 evening — PR-1 "Turn on the alarms" — IN FLIGHT

Branch `pr1-turn-on-alarms` created off `master @ 1ee047f1`. Commits landed:

| Commit | Story | Status |
|---|---|---|
| `b388c66d` | Docs (staff review + backlog) | ✅ committed |
| `10ff5140` | **S1.1** — Pin QHotkey | ✅ committed — fork has no release tags, pinned to current HEAD SHA `d7063877` instead |
| `4090fa11` | **S1.2** — Enable warnings framework (errors OFF) | ✅ committed — `WARNINGS_AS_ERRORS=OFF` forced for initial landing |
| `4e23f9da` | **S1.4** — Add ASAN+UBSAN Linux CI job | ✅ committed — `linux-sanitizers` job in `build_cmake.yml` |
| `72fdd9bd` | **S2.7** — Document manual test scripts | ✅ committed — chose "keep + document" path, added `tests/README.md` |

**Not yet done (pick up next session):**
- ❌ **Local build verification** on macOS — paused before installing `qt@6`/`cmake`/`ninja` via brew. The four changes are mechanical and reviewed, but no compiler has touched them yet. Recommended verification order when resuming: (1) `cmake` configure-only on this Mac (cheap, catches CMake syntax errors and validates the QHotkey pin downloads); (2) full local build (also satisfies the "can we install on the MacBook" question from the same session); (3) push branch to trigger CI matrix (truth source for the sanitizer job and Linux/Windows builds).
- ❌ **PR not opened.** No `gh pr create` yet — branch is local only. Open when verification passes.

**Risk notes:**
- S1.2 is the riskiest commit: uncommenting `set_project_warnings(project_warnings)` will surface every latent warning across 225 files. With `WARNINGS_AS_ERRORS=OFF` it can't break the build, but the noise volume in CI logs may be substantial. Capture the count when first CI run completes — that baseline informs S1.3 sizing.
- S1.4 sanitizer job runs `ctest` which is currently empty. Job will pass trivially today; real coverage starts when S2.1+ land.

**Next session entry point:** `git checkout pr1-turn-on-alarms` then either run the local verify or `git push -u origin pr1-turn-on-alarms` for CI.

---

---

## Executive Summary

Flameshot v14 is in beta 2 with a healthy commit cadence but two structural problems sized as P0 risk: (1) **zero automated test coverage** across 225 source files makes every PR a roll of the dice — visible in the PR #4658 spec-file ping-pong that has flipped merged↔reverted five times; (2) **a 2,085-line `CaptureWidget` god class** is the load-bearing center of the app, and it can't be safely refactored until tests exist around it.

The good news: the codebase has a clean tool-plugin design, a well-thought-through `ConfigHandler` schema, and a complete-but-disabled quality toolchain (warnings framework, ASAN/UBSAN, clang-tidy, sanitizers) — all defined in `cmake/` and never invoked. The cheapest 1-week of work in this repo is just **turning on what already exists**.

This backlog organizes 38 stories across 7 epics, sized for an OSS project with volunteer maintainers. The recommended next-sprint scope (PR-1 of the staff plan) is 8 items that together stand up a real CI safety net for the v14 GA release — none of them require deep code restructuring, none take more than a day each, and together they would have prevented two of the last three reverted PRs.

**No CaptureWidget rewrite is recommended for v14 or v15.** Tests come first, structure later.

---

## Epics

| Epic | Theme | Stories | Priority focus |
|---|---|---|---|
| **E1** | CI / Quality Gates | 6 | P0/P1 — cheapest, highest leverage |
| **E2** | Test Foundation | 7 | P0/P1 — prerequisite for all major refactors |
| **E3** | Cross-Platform Stability | 6 | P1 — addresses the macOS bug-fix surge |
| **E4** | Security & Network Hardening | 5 | P1/P2 — small surface, real issues |
| **E5** | Code-Health Cleanups | 8 | P2/P3 — incremental, contributor-friendly |
| **E6** | Build & Packaging | 3 | P0/P1 — directly fixes #4658 pattern |
| **E7** | Documentation | 3 | P1/P2 — lowers contribution friction |

---

## E1 — CI / Quality Gates

> "Most of the tools to fix this codebase are already in `cmake/`. They're just not turned on."

### S1.1 — Pin QHotkey dependency ✅ DONE (`10ff5140`)
**Problem:** `CMakeLists.txt:144-149` pins QHotkey to branch `master`. Non-reproducible build; vulnerable to upstream changes.
**Acceptance:**
- ✅ `GIT_TAG` references full commit SHA `d7063877c14d5ae2b489dc70bbe02e76a43bf38b` (fork has no release tags)
- ⏳ "Build produces identical output across two consecutive runs" — pending build verification
- ⏳ CMake fetch warning eliminated — pending build run
**Size:** XS (30 min) — actual: 15 min
**Priority:** **P0**
**Deps:** none

### S1.2 — Enable warnings framework (no -Werror) ✅ DONE (`4090fa11`)
**Problem:** `cmake/CompilerWarnings.cmake` is fully defined but never invoked — `set_project_warnings()` is commented out at `CMakeLists.txt:109`.
**Acceptance:**
- ✅ Line 109 replaced with `set_project_warnings(project_warnings)`; `WARNINGS_AS_ERRORS=OFF` forced via CACHE override
- ⏳ "Build still succeeds across all three platforms in CI" — pending push
- ⏳ "Warning count baseline captured in a CI summary" — capture from first CI run; informs S1.3 sizing
**Size:** S (1 hr config + observe) — actual: 20 min config
**Priority:** **P0**
**Deps:** none

### S1.3 — Fix warnings exposed by S1.2 and enable -Werror
**Problem:** Once warnings are visible, the team needs a path to clean them up without halting development.
**Acceptance:**
- New code in CI is gated on zero new warnings (using a diff filter)
- Or: full repo warning-clean and `WARNINGS_AS_ERRORS=TRUE` enabled
- Either path documented in CONTRIBUTING.md
**Size:** M (1-2 days, depends on baseline count)
**Priority:** P1
**Deps:** S1.2

### S1.4 — Enable ASAN+UBSAN on Linux CI ✅ DONE (`4e23f9da`)
**Problem:** `cmake/Sanitizers.cmake` defines all four sanitizers; all default OFF; none invoked in CI.
**Acceptance:**
- ✅ New `linux-sanitizers` job in `build_cmake.yml` runs with `-DENABLE_SANITIZER_ADDRESS=ON -DENABLE_SANITIZER_UNDEFINED_BEHAVIOR=ON`
- ✅ `UBSAN_OPTIONS=halt_on_error=1` to make violations loud; `ASAN_OPTIONS=detect_leaks=0` to silence Qt's noisy at-exit "leaks"
- ⏳ "Job passes on master HEAD" — pending push
- ⏳ Will pass trivially today (empty `ctest`); real coverage starts after S2.1+
**Size:** S (2 hr) — actual: 30 min
**Priority:** **P0**
**Deps:** none

### S1.5 — Enforce clang-tidy on changed lines
**Problem:** `.clang-tidy` is configured (with `WarningsAsErrors: '*'`) but never runs in CI.
**Acceptance:**
- New CI workflow runs `clang-tidy-diff` on PRs, scoped to changed lines only
- Existing violations are grandfathered (no big-bang cleanup required)
- README/CONTRIBUTING explains how to run locally
**Size:** M (1 day)
**Priority:** P1
**Deps:** none

### S1.6 — Delete dead `ctest` invocation or wire to real tests
**Problem:** `build_cmake.yml` runs `ctest` against an empty test suite — hollow gate.
**Acceptance:**
- After S2.1 lands, `ctest` invocation runs real tests
- Or: invocation removed with a one-line comment until E2 lands
**Size:** XS (15 min)
**Priority:** P1
**Deps:** S2.1 (preferred) or none (for the removal path)

---

## E2 — Test Foundation

> "The MVP is not 80% coverage. It's the smallest test suite that prevents the next reverted PR."

### S2.1 — Scaffold `tests/CMakeLists.txt` with Qt Test runner
**Problem:** No `enable_testing()`, no `add_test()`, no test framework wired. Tests/ contains only manual shell scripts.
**Acceptance:**
- `enable_testing()` at top-level CMake
- `tests/CMakeLists.txt` discovers and registers Qt Test executables
- A single trivial test passes (e.g., `2+2==4`) to prove the wiring
- `ctest` from build/ runs the suite
**Size:** S (4 hr)
**Priority:** **P0**
**Deps:** none

### S2.2 — Unit tests for `ConfigHandler::checkForErrors()`
**Problem:** `confighandler.cpp:569-709` contains four validation layers (`checkUnrecognizedSettings`, `checkShortcutConflicts`, `checkSemantics`, orchestrator). Pure logic, no UI deps, ~50 schema options worth of edge cases.
**Acceptance:**
- One test per validation layer (4 minimum)
- One round-trip test (write config → read config → no errors)
- One test per schema-typed option category (Bool, BoundedInt, Color, String, ExistingDir)
- Failure to load a malformed config produces structured `AbstractLogger` errors
**Size:** M (1 day)
**Priority:** **P0**
**Deps:** S2.1

### S2.3 — Unit tests for `FileNameHandler` path templating
**Problem:** Filename generation uses `%TIME`, `%DATE`, and custom tokens. Untested edge cases (special chars, locale, fallbacks) are a frequent source of issues like the May 2026 locale fix at `filenamehandler.cpp:17-24`.
**Acceptance:**
- Round-trip tests for each supported token
- Locale fallback test (catches the existing try/catch path)
- Cross-platform path-separator tests
- Invalid template handling
**Size:** M (1 day)
**Priority:** **P0**
**Deps:** S2.1

### S2.4 — Unit tests for `ValueHandler` type coercion
**Problem:** `valuehandler.cpp` (585 LOC) coerces config strings to typed values. Untested.
**Acceptance:**
- One test per `ValueHandler` subclass (Bool, Int, Color, String, BoundedInt, etc.)
- Boundary conditions (out-of-range, malformed strings)
- Default-value fallback verified
**Size:** M (1 day)
**Priority:** P1
**Deps:** S2.1

### S2.5 — Per-platform capture smoke test
**Problem:** Apple has shipped two privacy-permission changes in 18 months. Wayland portal spec evolves. Each change means a manual-detected regression (see #4664 — broke daemon-mode capture for all GNOME Wayland users until fixed reactively).
**Acceptance:**
- New CI workflow per platform (Linux xvfb, macOS-15, Windows-2025)
- Each invokes `flameshot full --raw > capture.png`
- Asserts non-empty, non-zero-pixel PNG output
- Runs on every PR that touches `src/utils/screengrabber.*`
- Runs nightly on `master`
**Size:** M (2 days, mostly CI plumbing)
**Priority:** P1
**Deps:** none

### S2.6 — CLI smoke tests
**Problem:** `flameshot --help`, `flameshot config`, `flameshot gui`, etc., have no automated regression test.
**Acceptance:**
- Headless test invokes 8-10 CLI entry points
- Asserts exit codes and stderr/stdout shape
- Catches the `commandlineparser.cpp` (409 LOC) class of regressions early
**Size:** S (4 hr)
**Priority:** P1
**Deps:** S2.1

### S2.7 — Decide fate of `tests/action_options.sh` and `tests/path_option.sh` ✅ DONE (`72fdd9bd`)
**Problem:** Two manual shell scripts that nobody runs. Either revive or delete.
**Acceptance:**
- ✅ Decision: **keep + document**. They are real, usable manual smoke recipes — just undiscoverable.
- ✅ Added `tests/README.md` describing what each covers, how to run, dependencies, and why they're not in CI
- ⏳ Future port to Qt Test runner deferred to when S2.1 lands
**Size:** XS-S (1 hr if delete; 4 hr if port) — actual: 25 min for the README path
**Priority:** P2
**Deps:** S2.1 (only if porting; the document-only decision unblocks)

---

## E3 — Cross-Platform Stability

> "The macOS bug rate isn't because the code is bad — it's because Apple shipped two privacy-policy changes in 18 months."

### S3.1 — Fix synchronous QEventLoop freeze in monitor selection
**Problem:** `screengrabber.cpp:85-112` runs a synchronous `QEventLoop` with a 30-second timeout. On slow Wayland portal responses, the entire app freezes for up to 30s.
**Acceptance:**
- Replaced with async pattern (`QFutureWatcher` or signal callback)
- UI remains responsive during portal wait
- Existing 30s timeout preserved (now as a non-blocking timer)
- Regression test added (S2.5 can verify)
**Size:** M (1 day)
**Priority:** P1
**Deps:** S2.5 helpful but not blocking

### S3.2 — Extract `PortalScreenGrabber` class
**Problem:** xdg-desktop-portal code (lines 52-160 of `screengrabber.cpp`) is interleaved with X11/macOS/Win code in a 734-line file. Bug #4664 was in this block; isolating it is the first step toward testability.
**Acceptance:**
- New `src/utils/portalscreengrabber.{h,cpp}` (or similar)
- `ScreenGrabber::grabScreen()` delegates to platform-specific class
- All existing capture flows continue to work (verified by S2.5)
- No behavior change — pure restructuring
**Size:** M (1 day)
**Priority:** P1
**Deps:** S2.5 strongly recommended

### S3.3 — Normalize runtime-vs-compile-time platform detection
**Problem:** `screengrabber.cpp:126` uses `QGuiApplication::platformName()` (runtime) alongside `#if defined(Q_OS_MACOS)` (compile-time) for the same purpose in nearby code.
**Acceptance:**
- One detection model used consistently (runtime preferred for Linux WSI variants)
- `DesktopInfo` (already exists in `src/utils/desktopinfo.h`) used as the single source
- Migration documented in code comments where compile-time `#ifdef` is necessary (e.g., macOS Carbon API calls)
**Size:** M (1 day)
**Priority:** P2
**Deps:** S3.2 (extraction makes this cleaner)

### S3.4 — RFC: Subclass-per-platform `IScreenGrabber` interface
**Problem:** Long-term architectural target — replace the `#ifdef` jungle with `X11ScreenGrabber`, `WaylandPortalScreenGrabber`, `MacOSScreenGrabber`, `WindowsScreenGrabber` as compile-conditional implementations.
**Acceptance:**
- RFC document in `docs/RFC/` with interface design, migration plan, testing approach
- Reviewed by maintainers, decision recorded
- Implementation deferred to v15 (do not build in v14)
**Size:** M (1 day for RFC; XL if implemented)
**Priority:** P2 (the RFC); P3 (the implementation)
**Deps:** S3.2 should land first to inform the design

### S3.5 — Group recent macOS fixes into a regression-prevention test set
**Problem:** Five `fix(macos):` commits in the last 11 fixes (clipboard, fullscreen overlay, privacy API, config tab rendering, etc.). Each was reactive.
**Acceptance:**
- Each of the 5 fixes has a corresponding regression test (or test ticket if not currently testable)
- Tests live in `tests/integration/macos/`
- CI runs on macOS-15 arm64 + intel
**Size:** L (3 days — depends on what's reachable from integration tests)
**Priority:** P2
**Deps:** S2.5

### S3.6 — Investigate Windows screen-scaling pin position (related to #4614)
**Problem:** Pin position on Windows for scaled screens was a recent fix; the root cause may have other manifestations.
**Acceptance:**
- Code audit of all `devicePixelRatio()` / scaling-related usage in `src/widgets/`
- Regression test for pinning on a scaled display
- Any additional issues filed as separate tickets
**Size:** M (1 day audit + tests)
**Priority:** P2
**Deps:** S2.5

---

## E4 — Security & Network Hardening

> "Five real findings. None are critical, but the QHotkey unpinning is the biggest open door."

### S4.1 — Move Imgur Client ID to build-time define
**Problem:** Default Imgur Client ID `313baf0c7b4d3ff` is hardcoded at `confighandler.cpp:134`. Distros currently can't easily override.
**Acceptance:**
- `-DFLAMESHOT_IMGUR_CLIENT_ID=...` CMake option
- Default preserved for source builds
- Documented in README "for packagers"
**Size:** S (2 hr)
**Priority:** P2
**Deps:** none

### S4.2 — Document or sandbox terminal launcher command injection vector
**Problem:** `terminallauncher.cpp:48` passes a user-supplied string to a terminal emulator. Shell metacharacters will be interpreted. Not a vulnerability per se (user explicitly opted in), but a sharp edge.
**Acceptance:**
- Code comment at the call site explains the trust boundary
- UI shows a one-time warning when adding a command containing shell metacharacters
- OR: documented as a "by design" choice in `docs/SECURITY.md`
**Size:** S (2 hr)
**Priority:** P2
**Deps:** none

### S4.3 — Share `QNetworkAccessManager` across uploaders
**Problem:** `imguruploader.h:29` creates a fresh NAM per uploader. No connection pooling; potential for cookie/cache divergence if more storage backends are added.
**Acceptance:**
- Singleton NAM owned at app or daemon level
- Each uploader requests handles via a factory
- No behavior change for the user
**Size:** S (4 hr)
**Priority:** P3
**Deps:** none

### S4.4 — Audit and document the D-Bus interface surface
**Problem:** D-Bus interface is currently safe (session bus only, no shell command parameters, only byte arrays and text) but undocumented.
**Acceptance:**
- New `docs/dbus-interface.md` documenting every exposed method, parameter types, trust model
- Linked from `docs/SECURITY.md`
**Size:** S (3 hr)
**Priority:** P2
**Deps:** none

### S4.5 — Add basic security policy and reporting
**Problem:** No `SECURITY.md` at repo root. Researchers have no clear disclosure path.
**Acceptance:**
- `SECURITY.md` at repo root with supported versions, reporting email/process, expected response time
- Linked from README
**Size:** XS (1 hr)
**Priority:** P1
**Deps:** none

---

## E5 — Code-Health Cleanups

> "Mechanical work that lowers contribution friction. Volunteers can pick these up between bigger tasks."

### S5.1 — Convert 18 SLOT() strings to PMF connect form
**Problem:** `capturewidget.cpp:1650-1718` uses string-based `SLOT()` connections that fail at runtime, not compile time.
**Acceptance:**
- All 18 connections use `&Class::method` form
- Build passes; clang-tidy `modernize-use-default-member-init` happy
**Size:** S (2 hr)
**Priority:** P1
**Deps:** none

### S5.2 — Standardize `CaptureWidget` pointer ownership
**Problem:** `capturewidget.h:187-206` mixes `QPointer<>` (auto-null on delete) with raw pointers manually deleted at `:572, 600, 606, 1468, 2022`. Latent crash risk for future contributors.
**Acceptance:**
- One ownership model used throughout (`QPointer` for all dynamically-deleted members recommended)
- Code comment at the top of the member-declarations block explains the chosen rule
- No behavior change
**Size:** M (1 day)
**Priority:** P1
**Deps:** none

### S5.3 — Replace `add_definitions()` with target-scoped calls
**Problem:** `src/CMakeLists.txt:64-68` uses 5 deprecated `add_definitions()` calls for Windows version macros.
**Acceptance:**
- All 5 converted to `target_compile_definitions()` with explicit scope
- Also fix `:104-105` (direct `CMAKE_CXX_FLAGS` manipulation for MSVC `/MP`)
- Windows build still produces a working .exe
**Size:** S (30 min)
**Priority:** P2
**Deps:** none

### S5.4 — Promote magic numbers to named constants
**Problem:** `capturewidget.cpp:701, 1091, 1157, 1194` contain unmarked pixel literals (`(0, 0, 10, 12)`, `200`, `QPoint(1000, 200)`).
**Acceptance:**
- Each magic number named (`const int XYWH_BOX_PADDING_X = 10;`) or moved to a theme/config struct
- No behavior change; visual diff = zero pixels
**Size:** S (2 hr)
**Priority:** P3
**Deps:** none

### S5.5 — Consistent `AbstractLogger` use; remove silent error swallowing
**Problem:** `screengrabber.cpp:381` returns `nullptr` silently; `screenshotsaver.cpp:78` returns `false` silently on some paths. 36 direct `qDebug`/`qWarning` calls bypass the well-designed `AbstractLogger` system.
**Acceptance:**
- Audit pass: every silent `return nullptr/false` either logs via `AbstractLogger` or has a code comment explaining why
- New code follows the convention (documented in CONTRIBUTING.md)
**Size:** M (1 day)
**Priority:** P2
**Deps:** none

### S5.6 — Replace `QThread::msleep(delay)` in flameshot.cpp
**Problem:** Blocks main thread for a synthetic delay. Should be a `QTimer::singleShot`.
**Acceptance:**
- `QThread::msleep` removed
- Equivalent async behavior via `QTimer`
- Tests (when they exist) prove the delay still applies
**Size:** XS (30 min)
**Priority:** P2
**Deps:** none

### S5.7 — Split `TextTool` configuration into its own helper class
**Problem:** `texttool.cpp` is 357 LOC — 7x its peer tools. It directly inherits `CaptureTool` instead of using an abstract base, and embeds widget + config + font management.
**Acceptance:**
- Font / config logic moved to a helper class
- TextTool LOC reduced to <150
- Behavior unchanged (verified by tool unit tests when E2 progresses)
**Size:** M (1 day)
**Priority:** P3
**Deps:** S2.1 helpful

### S5.8 — Self-registering tool factory
**Problem:** `toolfactory.cpp:35-71` is a hard-coded 24-case switch. Adding a tool requires touching 3+ central files. Closed to extension.
**Acceptance:**
- Tools register themselves at static-init time (X-macro list, or `REGISTER_TOOL()` macro)
- Adding a new tool requires only adding files under `src/tools/<newname>/`
- All 24 existing tools migrated; behavior unchanged
**Size:** L (2 days)
**Priority:** P3
**Deps:** S2.1 (need to verify tool behavior preserved)

---

## E6 — Build & Packaging

> "The #4658 ping-pong is the loudest signal in the repo. It's a process gap, not a code gap."

### S6.1 — PR-blocking packaging matrix
**Problem:** Packaging (.deb / .rpm / .dmg / .exe) only runs on `master` after merge, not on PRs. PR #4658 was merged → reverted → re-merged five times because packaging breakage wasn't visible pre-merge.
**Acceptance:**
- New `.github/workflows/packaging-pr.yml` builds all four package types on every PR
- Failures block merge
- Matrix kept lean: one representative distro per package format (Debian 12, Fedora latest, macOS-15, Windows-2025)
- Build time documented; if >20min wall clock, consider matrix-skip rules
**Size:** M (1 day)
**Priority:** **P0**
**Deps:** none

### S6.2 — Pin all FetchContent dependencies
**Problem:** Beyond QHotkey (S1.1), audit every other `FetchContent_Declare` for unpinned versions.
**Acceptance:**
- Spreadsheet in PR description: dep / current pin / desired pin / decision
- All deps pinned to tag or SHA
- Renovate-style note in CONTRIBUTING about when to update
**Size:** S (2 hr)
**Priority:** P1
**Deps:** S1.1 sets the precedent

### S6.3 — Reproducible build documentation
**Problem:** README has install instructions per distro, but no "build the same binary twice and verify identical output" recipe.
**Acceptance:**
- New section in CONTRIBUTING.md
- Step-by-step Docker recipe for reproducing the Linux build
- Optional: bit-identical verification command
**Size:** S (4 hr)
**Priority:** P3
**Deps:** S1.1, S6.2

---

## E7 — Documentation

> "Lower the bar to contribution, especially for the first patch."

### S7.1 — CONTRIBUTING.md test-writing section
**Problem:** Currently no guidance on how to add a test. Without this, the test suite (E2) won't grow with the project.
**Acceptance:**
- "How to add a test" section in `docs/CONTRIBUTING.md`
- One worked example (e.g., adding a `ConfigHandler` test)
- Links to the Qt Test docs
**Size:** S (4 hr)
**Priority:** P1
**Deps:** S2.1, S2.2

### S7.2 — Architecture overview doc
**Problem:** A new contributor reading `capturewidget.cpp` for the first time has no map. The tools/ plugin contract is good but undocumented.
**Acceptance:**
- New `docs/dev/architecture.md` with: app entry points, daemon lifecycle, capture pipeline, tool plugin contract, config layer
- Diagrams optional but encouraged
- Links to key files with line numbers
**Size:** M (1 day)
**Priority:** P2
**Deps:** none

### S7.3 — macOS-specific build guide
**Problem:** README's macOS install section covers homebrew/macports but not building from source on Apple Silicon. With 5 of last 11 fixes being macOS-specific, more macOS contributors would help.
**Acceptance:**
- New section in README or `docs/dev/build-macos.md`
- Tested recipe for both Apple Silicon and Intel
- Includes `xattr` quarantine workaround
- Mentions known limitations (no global hotkey, no D-Bus)
**Size:** S (3 hr)
**Priority:** P2
**Deps:** none

---

## Priority Distribution

| Tier | Count | Definition |
|---|---|---|
| **P0 (must-do for v14 GA)** | 6 | S1.1, S1.2, S1.4, S2.1, S2.2, S2.3, S6.1 |
| **P1 (next release)** | 12 | S1.3, S1.5, S1.6, S2.4, S2.5, S2.6, S3.1, S3.2, S4.5, S5.1, S5.2, S6.2, S7.1 |
| **P2 (nice-to-have)** | 13 | S2.7, S3.3, S3.4-RFC, S3.5, S3.6, S4.1, S4.2, S4.4, S5.3, S5.5, S5.6, S7.2, S7.3 |
| **P3 (long-term)** | 6 | S3.4-impl, S4.3, S5.4, S5.7, S5.8, S6.3 |

*P0 ≠ "all in v14 GA." It means "if we ship v14 without these, we ship the same delivery risk we have today." The realistic v14-GA scope is S1.1, S1.2, S1.4, S6.1 (the CI gates), plus S2.1 to unblock testing for v14.1.*

---

## Recommended Next Sprint (10 items, ~2 weeks volunteer effort)

| # | Story | Size | Status | Why this together |
|---|---|---|---|---|
| 1 | **S1.1** — Pin QHotkey | XS | ✅ committed (`10ff5140`) | Single highest-risk single-line fix in the repo |
| 2 | **S1.2** — Enable warnings (no -Werror) | S | ✅ committed (`4090fa11`) | Sets up S1.3 follow-up; surfaces the warning baseline |
| 3 | **S1.4** — ASAN+UBSAN on Linux CI | S | ✅ committed (`4e23f9da`) | Cheapest memory-safety win; existing framework |
| 4 | **S2.1** — `tests/CMakeLists.txt` scaffold | S | ⏸ next | Prerequisite for every test going forward |
| 5 | **S2.2** — `ConfigHandler` unit tests | M | ⏸ next | First real test; immediately makes existing `ctest` invocation useful |
| 6 | **S2.3** — `FileNameHandler` unit tests | M | ⏸ next | Highest bug-density area; pays back fast |
| 7 | **S5.1** — SLOT() → PMF | S | ⏸ next | Mechanical, satisfying win for new contributors |
| 8 | **S6.1** — PR-blocking packaging matrix | M | ⏸ next | Directly prevents the #4658 ping-pong from recurring |
| 9 | **S4.5** — `SECURITY.md` | XS | ⏸ next | 1-hour fix; closes a basic OSS-hygiene gap |
| 10 | **S7.1** — CONTRIBUTING.md testing section | S | ⏸ next | Compounds the value of #4-6 by inviting contributors |

Plus side win this evening: **S2.7** (document the manual test scripts) committed as `72fdd9bd`. Not on the original next-sprint list but it slotted in naturally with the PR-1 work.

**Sprint progress: 4/10 stories committed + 1 bonus** (S1.1, S1.2, S1.4, S2.7 — call it 4½). PR not yet opened pending build verification. The remaining six stories (#4-#10) are the natural pickup for the next working session.

**Rationale:** This sprint stands up the CI safety net before v14 GA. Together these items would have caught two of the last three reverted PRs (#4658 spec uniformization via #8; the empty-parent-window portal bug #4664 via #5+#6 if a capture smoke test had existed — close enough to motivate adding it next sprint as S2.5). None require touching CaptureWidget. Eight of the ten are sized XS-S; the two M-sized items (S2.2, S6.1) are independent and can be parallelized. A reasonable volunteer pace ships items 1, 4, 9 in week 1 (low-touch infrastructure) and 2, 3, 5, 6, 7, 8, 10 in week 2.

---

## Known Issues Cross-Reference

| GitHub Issue / PR | Story Coverage |
|---|---|
| #3177 (fixed in 95032bd2) | None needed — fixed |
| #4664 (X11 portal parent_window) | S2.5 (regression test); S3.2 (portal extraction) |
| #4658 (spec uniformization ping-pong) | **S6.1** — the direct fix |
| #4676, #4675, #4658 (revert cycles) | Same — S6.1 |
| #4680 (fix for #3177) | Already merged |
| #4692 (v14 beta 2 prep) | Current state |
| #4622, #4629, #4617, #4627, #4614 (recent macOS fixes) | S3.5 (regression test set); S2.5 (per-platform smoke) |
| #4637 (KDE Plasma keyboard shortcut config) | No direct story; consider as ad-hoc fix |
| #4596 (cosmic full-screen) | S3.3 (runtime platform detection improvement) |

---

## Notes on Effort Sizing

This backlog uses S/M/L sizing calibrated for volunteer OSS contributors working evenings/weekends:

- **XS** = under 1 hour, often a single-line change
- **S** = 1-4 hours, single sitting
- **M** = 1 day, plus review cycle
- **L** = 2-3 days, multi-session work
- **XL** = a week+, architectural change

The implicit overhead per story (review, CI cycles, addressing feedback) is roughly 1× the build estimate for an OSS project. Plan sprint capacity accordingly.

## What This Backlog Does NOT Recommend

- **A v14 CaptureWidget rewrite.** Untestable currently; refactor without tests is net-negative. (See staff review for unanimous panel agreement.)
- **A full Qt6 modernization pass** (`qt_add_executable` etc.). Mechanical, low value, easy to do later.
- **Pulling QHotkey in-tree.** Maintenance cost > supply-chain benefit if it's pinned (S1.1).
- **Coverage targets.** A volunteer project chasing 80% coverage is a dead project. The aim is "tests for the things most likely to break," not numerical thresholds.
- **A test-driven rewrite of any kind in v14.** v14 is in beta. New features, new structure, and new abstractions all wait for v15.
