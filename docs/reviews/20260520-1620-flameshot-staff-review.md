# Flameshot v14 — Staff Engineer Panel Review

**Date:** 2026-05-20
**Panel:** Tim (SpaceX), Rob (Roblox), Fran (Meta), Al (AWS), Will Larson (Moderator)
**Trigger:** User-requested deep technical dive on the Flameshot codebase ahead of v14.0.0 GA. Output is intended to feed `/pm` directly into a prioritized backlog.
**Repo state at review:** `master` clean, in sync with `origin/master`, HEAD = `1ee047f1 prep for v14 beta 2 (#4692)`.

---

## Problem Statement

**Work type:** Architectural / Enhancement.

Flameshot is a mature C++/Qt6 cross-platform screenshot/annotation tool, ~225 source files, 14 years of organic growth. v14.0.0 is in beta 2. The user asked for "improvements we can make to the code" with a full backlog suitable for incremental adoption by volunteer contributors.

### Quantified state

| Concern | Quantified |
|---|---|
| God object — `CaptureWidget` | **2,085 LOC** in one file, owns event routing + tool lifecycle + undo + panel coordination |
| Top 5 source files (LOC) | `capturewidget.cpp` 2085 / `generalconf.cpp` 991 / `confighandler.cpp` 844 / `screengrabber.cpp` 734 / `main.cpp` 684 |
| Automated test coverage | **0%**. Two shell scripts (`tests/action_options.sh`, `tests/path_option.sh`) require human verification |
| CI test gate | `build_cmake.yml` invokes `ctest` against an empty suite — hollow gate |
| Warnings framework | `set_project_warnings()` is **commented out** at `CMakeLists.txt:109` — defined but not applied |
| Supply chain | QHotkey pinned to `master` branch (`CMakeLists.txt:147`), not a tag or SHA |
| Release-engineering signal | PR **#4658** (openSUSE spec uniformization) has flipped merged↔reverted **5 times** in the past month |
| Recent fix density | **5 of last 11 fixes are `fix(macos):`** — macOS is the most volatile surface |
| Logging discipline | 36 `qDebug/qWarning` calls across 225 files; `AbstractLogger` is well-designed but inconsistently used |
| TODO/FIXME markers | 37 total — low |
| Tooling exists but disabled | clang-tidy config + ASAN/UBSAN/TSAN/MSAN options + warnings framework all defined and unused |

### Root-cause chain

1. No automated tests →
2. No mechanism to detect regression →
3. Maintainers can't tell which PRs are safe →
4. Process becomes "merge → observe in production → revert if broken" (the #4658 ping-pong) →
5. Refactor work piles up because nobody can verify it →
6. CaptureWidget becomes unmaintainable god class

### Constraints

- Open-source, volunteer maintainers, no paid team
- v14 is in beta — narrow window for small wins, no window for big refactors
- The 2,085-line `CaptureWidget` is load-bearing

---

## Panel Analysis

### Tim — Staff Engineer, SpaceX

**Risk assessment:** The single failure mode that scares me is **regression silence**. You have a 2,085-line god class that paints, routes events, owns tool state, and manages undo — and zero automated tests. Every PR that touches `capturewidget.cpp` is rolling dice. The #4658 five-cycle merge/revert isn't a code smell, it's a delivery-system failure.

**Risk matrix:**

| Risk | P (1-5) | C (1-5) | Score |
|---|---|---|---|
| Regression in CaptureWidget on next refactor | 5 | 5 | **25** |
| QHotkey `master` branch hijack or breaking-change cascade | 2 | 5 | 10 |
| #4658-style merge/revert ping-pong continues | 5 | 3 | 15 |
| Silent error swallow ships a bad upload to user | 3 | 3 | 9 |
| macOS Sonoma+ permission API drift breaks capture again | 4 | 4 | 16 |

**Key quote:** *"You don't have a tech-debt problem. You have a 'can't verify anything is safe' problem. Fix that first; the rest follows."*

**Unique contribution:** The 5x ping-pong on #4658 is not a "fix the spec file" problem — it's evidence the team can't tell when a packaging change is safe. The fix isn't more careful reviewing, it's a packaging-smoke-test job that builds the .deb/.rpm/.dmg/.exe on every PR.

### Rob — Staff Engineer, Roblox

**Risk assessment:** CaptureWidget makes **83+ direct calls into child panels**. Those are 83 things that break silently when a child widget changes its API. The plugin model (`src/tools/`) is actually well-designed (23 `emit requestAction(Request)` calls, zero downcasts, complete i18n), but the capture widget is the festering hotspot.

The latent bug nobody's talking about: memory-model inconsistency in `capturewidget.h:187-206`. Some members are `QPointer` (auto-null on delete), some are raw pointers with manual `delete` calls (lines 572, 600, 606, 1468, 2022). Adding a new feature in 18 months that calls into one of the raw-pointer members after deletion = intermittent crash, nightmare to repro.

**Key quote:** *"The plugin model is good — leave it alone. The capture widget is bad — but you can't fix it without tests. Pick the boring path."*

**Unique contribution:** `screengrabber.cpp:85-112` spins a synchronous `QEventLoop` with a 30-second timeout during monitor selection. On a slow Wayland portal response, that **freezes the entire app for 30 seconds**. Real reported-but-unfiled bug waiting to happen.

### Fran — Staff Engineer, Meta

**Risk assessment:** Stop thinking "improve the code" and start bucketing:

**Bucket A — Fix yesterday (CI gates):**
- Warnings framework off (`CMakeLists.txt:109`) — turn on
- QHotkey on `master` — pin
- clang-tidy config exists but doesn't run — wire it
- CI calls `ctest` against empty suite — add a real test

**Bucket B — Compounding (test foundation):**
- Smoke tests for CLI entry points
- Unit tests for `ConfigHandler::checkForErrors()` (already isolated, `confighandler.cpp:569-709`)
- Unit tests for `FileNameHandler`
- One per-platform headless capture smoke

**Bucket C — Don't care (yet):** Decomposing CaptureWidget. Without tests, decomposing it makes things worse. Revisit v15.

**Key quote:** *"Sort everything into 'fix yesterday', 'compound', and 'don't care'. Most projects die because they confuse the buckets."*

**Unique contribution:** The #4658 ping-pong is a **process gap**, not a code problem. There's no .deb/.rpm/.dmg build that runs on every PR (only on master). Move packaging to PR-blocking.

### Al — Staff Engineer, AWS (Qt/Desktop platform analog)

**Risk assessment:** You have **three different cross-platform problems pretending to be one:**

1. **Screen capture** — fundamentally different APIs per platform. Currently `#if defined(Q_OS_*)` jungle in `screengrabber.cpp`. Tolerable at 3 branches; unmaintainable when each branch has sub-cases (GNOME 45 vs GNOME 46 — exactly the bug #4664 fixed).
2. **Global hotkey** — delegated to QHotkey dep. **Right call. Don't bring it in-tree.**
3. **Single-instance / IPC** — D-Bus vs KDSingleApplication split via `#ifdef`. Each path is small. OK.

The actual platform problem is #1. Extract `IScreenGrabber` → `X11ScreenGrabber`, `WaylandPortalScreenGrabber`, `MacOSScreenGrabber`, `WindowsScreenGrabber`. Then #4664-style bugs are "diff one file, write one test."

On macOS bug surge: Apple's privacy-permission cadence will keep producing these. Write **one integration test per platform** that catches "did capture return non-null pixels" so regressions are caught in CI.

**Key quote:** *"The macOS bug rate isn't because the code is bad — it's because Apple shipped two privacy-policy changes in 18 months. Build a test that runs after every OS release, not patches reactive to each issue."*

**Unique contribution:** Runtime vs compile-time platform detection is mixed. `QGuiApplication::platformName()` at `screengrabber.cpp:126` runs alongside `#if defined(Q_OS_MACOS)` in nearby code. Pick one model.

---

## Consensus Matrix

| Question | Tim | Rob | Fran | Al |
|---|---|---|---|---|
| Refactor CaptureWidget now? | No | No | No | No |
| Enable warnings framework (1-line)? | **Yes** | Yes | Yes | Yes |
| Pin QHotkey to tag/SHA? | **Yes** | Yes | Yes | Yes |
| Unit-test ConfigHandler/FileNameHandler/ValueHandler first? | Yes | Yes | **Yes** | Yes |
| Per-PR packaging build matrix? | Yes | Yes | **Yes** | Yes |
| Per-platform capture smoke test? | Yes | Yes | Yes | **Yes** |
| Standardize CaptureWidget pointer ownership? | Yes | **Yes** | Defer | Yes |
| Convert `SLOT()` strings to PMF? | Yes | **Yes** | Yes | Yes |
| Extract portal code from screengrabber? | Defer | Yes | Defer | **Yes** |
| Pull QHotkey in-tree? | No | No | No | No |
| Big-bang Qt6 modernization? | No | No | No | No |
| Move clang-tidy from available → enforced? | Yes | Yes | **Yes** | Yes |

**Unanimous (8):** Don't refactor CaptureWidget · Enable warnings · Pin QHotkey · Test isolated logic first · PR packaging matrix · Per-platform capture smoke · Convert SLOT() strings · Enforce clang-tidy on diffs

**Majority (3-of-4):** Extract portal code · Standardize pointer ownership

---

## Will's Clarifying Questions (with verified answers)

| Question | Answer | Impact |
|---|---|---|
| Does CI actually run `ctest`? | Yes — `build_cmake.yml` runs it against an empty suite | Adding ONE real test immediately makes existing CI infra useful |
| Current C++ standard? | **C++20** via `cxx_std_20` in `cmake/StandardProjectSettings.cmake:18` | No urgent "modernize" epic |
| `compile_commands.json` exported? | Yes (`StandardProjectSettings.cmake:18`) | clang-tidy can run today with no CMake changes |
| Sanitizers framework exist but disabled? | Yes — all four sanitizers defined, all default OFF | Easy win: enable ASAN+UBSAN on Linux CI |
| Contributor / testing docs? | README has compile/install per-distro; no testing guide because there are no tests | New "How to write a test" doc must accompany test foundation work |
| QHotkey has a Qt6-compatible tagged release? | Repo is `flameshot-org/QHotkey` fork; upstream last release was `v1.5.0` (2024) | Pin to `v1.5.0` or fork HEAD SHA |
| Anybody run the shell scripts? | Manual-only, no CI hook | Effectively dead — revive (port to real runner) or delete |
| Is uncommenting warnings safe? | Risk: `WARNINGS_AS_ERRORS` defaults TRUE — might break first build | Update plan: land warnings ON + errors OFF first, fix, then flip |

### Reassessment

- **Warnings framework:** Two-step rollout. (1) Enable warnings, errors OFF — 1 hr. (2) Fix the warnings — 1-2 days. (3) Flip errors ON — 5 min.
- **Sanitizers:** Added to the recommendation list — ASAN+UBSAN on the Linux CI job. ~2 hr.

---

## Will Larson's Decision — Top 13 Ranked Improvements

| # | What | Why now | File:Line | Effort |
|---|---|---|---|---|
| 1 | Pin QHotkey to a release tag or SHA | Supply-chain risk; non-reproducible builds | `CMakeLists.txt:144-149` | **S** (30 min) |
| 2 | Enable warnings framework (no -Werror first) | One-line uncomment exposes years of latent issues | `CMakeLists.txt:109` | **S** (1 hr) |
| 3 | Wire `enable_testing()` + Qt Test runner | Foundation for everything else; replaces hollow `ctest` | new `tests/CMakeLists.txt` | **S** (4 hr) |
| 4 | Unit tests for `ConfigHandler` validation | Already isolated logic; catches config-schema bugs forever | `confighandler.cpp:569-709` | **M** (1 day) |
| 5 | Unit tests for `FileNameHandler` + `ValueHandler` | Pure logic, no UI deps | `filenamehandler.cpp`, `valuehandler.cpp` | **M** (1 day) |
| 6 | Add packaging matrix as PR-blocking job | Exactly what #4658's ping-pong needed | new `.github/workflows/packaging-pr.yml` | **M** (1 day) |
| 7 | Add ASAN+UBSAN to Linux CI build | Sanitizers framework exists, just flip flags | `cmake/Sanitizers.cmake`, `build_cmake.yml` | **S** (2 hr) |
| 8 | Per-platform capture smoke test (asserts non-null pixmap) | Catches recurring macOS-permission and X11-portal regressions | new `tests/integration/` | **M** (2 days) |
| 9 | Convert 18 SLOT() strings to PMF in `initShortcuts` | Compile-time safety; mechanical | `capturewidget.cpp:1650-1718` | **S** (2 hr) |
| 10 | Fix screengrabber synchronous `QEventLoop` freeze | Real UX bug — 30s app freeze on slow portal | `screengrabber.cpp:85-112` | **M** (1 day) |
| 11 | Standardize `CaptureWidget` pointer ownership | Prevent slow-motion crash bug | `capturewidget.h:187-206` | **M** (1 day) |
| 12 | Extract xdg-desktop-portal block into `PortalScreenGrabber` class | First testable platform-impl boundary; isolates #4664-style hotspot | `screengrabber.cpp:52-160` | **M** (1 day) |
| 13 | Enable clang-tidy on diff'd lines in CI | Per-PR quality ratchet | `.clang-tidy` + new workflow | **M** (1 day) |

## The 3 Biggest Architectural Risks if Unaddressed

1. **CaptureWidget will eventually require a rewrite no one can ship.** At 2,085 LOC with zero tests, every quarter that passes makes the rewrite cost grow super-linearly while willingness shrinks. *Not urgent in v14, critical for v15 planning.* Mitigation: write tests around it first (items 4-8), so a future rewrite can be verified incrementally.

2. **The platform-conditional jungle in `screengrabber.cpp` will turn each new macOS/GNOME/Wayland release into a fire drill.** Apple's privacy-permission cadence and Wayland portal spec evolution will keep producing #4664-style bugs. Cost-per-bug grows because each fix touches the same big function. Mitigation: portal extraction (item 12) — once one platform is a separate class, the rest follow.

3. **No mechanism exists to detect that a packaging change is safe to ship.** #4658 ping-pong is the symptom; the disease is that maintainers can't tell which downstream distros a spec-file change broke. Mitigation: PR-blocking packaging matrix (item 6) — single highest-leverage process change in this entire review.

## Recommended Test Strategy (from zero)

The pragmatic MVP — **not** chasing coverage percentages.

**Tier 1 — Compile/static safety (ship in days, no test code):**
- Warnings on (item 2) → fix easy ones → flip to -Werror
- clang-tidy on changed lines (item 13)
- ASAN+UBSAN on Linux CI (item 7)

**Tier 2 — Pure-logic unit tests (ship in 1-2 weeks):**
- `ConfigHandler` validation (~50 option schemas, error paths) — item 4
- `FileNameHandler` (path templating, edge cases on `%TIME`, `%DATE`, special chars) — item 5
- `ValueHandler` (type coercion, validation) — item 5

These three give the most coverage per hour: ~2,200 LOC of validation/parsing already isolated with no Qt-UI dependencies.

**Tier 3 — One smoke per platform (ship in 2-3 weeks):**
- Linux: `xvfb-run flameshot full --raw > /tmp/x.png; verify non-empty PNG`
- macOS: same via GUI runner in `macos-15` GH runner
- Windows: same via PowerShell

Just one per platform. Goal isn't testing capture quality — it's catching "capture returns null after the OS API changes."

**Tier 4 (deferred to v15+):** Tool-by-tool unit tests using the `CaptureTool` fixture (plugin model is clean enough to support this).

**What NOT to do:** Don't try to test `CaptureWidget` directly. It's untestable in its current shape; testing-first blocks on refactoring-first which the panel unanimously says don't do yet.

## Quick-Wins List (≤1 day each, no dependencies)

| # | Action | Effort | File:Line |
|---|---|---|---|
| Q1 | Pin QHotkey | 30 min | `CMakeLists.txt:147` |
| Q2 | Replace `add_definitions()` with `target_compile_definitions()` | 30 min | `src/CMakeLists.txt:64-68` |
| Q3 | Convert 18 SLOT() strings to PMF | 2 hr | `capturewidget.cpp:1650-1718` |
| Q4 | Enable warnings (without -Werror) | 1 hr | `CMakeLists.txt:109` |
| Q5 | Enable ASAN+UBSAN on Linux CI | 2 hr | `cmake/Sanitizers.cmake`, `build_cmake.yml` |
| Q6 | Delete or revive `action_options.sh` / `path_option.sh` | 1 hr | `tests/` |
| Q7 | Add `enable_testing()` and tests/CMakeLists.txt scaffold | 1 hr | new `tests/CMakeLists.txt` |
| Q8 | Add `.editorconfig` | 15 min | new `.editorconfig` |
| Q9 | Document the test-writing workflow in CONTRIBUTING.md | 4 hr | `docs/CONTRIBUTING.md` |
| Q10 | Replace silent `return nullptr` with `AbstractLogger::error()` log | 30 min | `screengrabber.cpp:381` |

A volunteer with one weekend can land Q1-Q5 + Q7-Q8 — eight PRs that materially improve the project.

## What's Explicitly Deferred

| Item | Rationale | Revisit When |
|---|---|---|
| `CaptureWidget` decomposition | Untestable currently; refactor without tests is net-negative | After Tier 2+3 tests land |
| Subclass-per-platform `ScreenGrabber` | Right destination, wrong order | After portal extraction (item 12) proves the pattern |
| `qt_add_executable` / `qt_finalize_target` migration | Mechanical, low value | When upgrading to Qt 6.7+ |
| Pulling QHotkey in-tree | Maintenance cost > supply-chain benefit if pinned | Only if upstream goes abandoned |
| Async-everywhere capture pipeline refactor | Big swing, not justified by current bug rate (except 30s freeze) | After v15 |

## Key Takeaways

> *"The 5x merge/revert on #4658 is the loudest signal in this repo. It's not a code problem — it's a 'we cannot verify a change before shipping it' problem."* — Tim

> *"The tool plugin model is good. The capture widget is bad. Don't refactor the bad without tests, and don't touch the good at all."* — Rob

> *"Bucket A, B, C. Projects die because they confuse the buckets."* — Fran

> *"The macOS bug rate is Apple's fault, not yours. The fix is a per-OS-release test, not chasing each issue."* — Al

**Generalizable insights:**
- A 2,000-line god class with zero tests is locked in place — tests are the prerequisite to any future cleanup.
- "Pin your deps" and "actually run the warnings you've already configured" are the cheapest wins almost any project has lying around.
- Process gaps (packaging not gated) masquerade as code problems (spec file keeps breaking).
- For a volunteer OSS project, the question isn't "what's optimal" — it's "what's the smallest set of changes that compounds."

## Files Referenced

| File | Role |
|---|---|
| `CMakeLists.txt` | Root build, FetchContent pinning, warnings opt-in |
| `cmake/CompilerWarnings.cmake` | Detailed but un-invoked warning framework |
| `cmake/Sanitizers.cmake` | ASAN/UBSAN/TSAN/MSAN options, all default OFF |
| `cmake/StaticAnalyzers.cmake` | clang-tidy + cppcheck, opt-in only |
| `cmake/StandardProjectSettings.cmake` | C++20 standard, compile_commands.json |
| `src/CMakeLists.txt` | Deprecated `add_definitions` at :64-68 |
| `src/widgets/capture/capturewidget.{h,cpp}` | 2085 LOC god class; mixed pointer ownership |
| `src/widgets/capture/selectionwidget.cpp` | 569 LOC, 150-line event filter |
| `src/widgets/capture/buttonhandler.cpp` | 395 LOC |
| `src/tools/capturetool.{h,cpp}` | Plugin base class — clean signal-bus design |
| `src/tools/toolfactory.cpp` | Hard-coded 24-case factory; closed to extension |
| `src/tools/text/texttool.cpp` | 357 LOC — outlier among tools |
| `src/tools/imgupload/storages/imgur/imguruploader.cpp` | Each uploader owns its own NAM |
| `src/utils/confighandler.{h,cpp}` | Good schema design at :74-154; validation at :569-709 |
| `src/utils/screengrabber.cpp` | 734 LOC; portal block :52-160; synchronous QEventLoop :85-112 |
| `src/utils/abstractlogger.h` | Well-designed but inconsistently used |
| `src/core/flameshot.{h,cpp}` | 535 LOC coordinator; singleton |
| `src/core/flameshotdaemon.{h,cpp}` | 517 LOC IPC + tray |
| `src/core/flameshotdbusadapter.cpp` | Session-bus only — safe |
| `.github/workflows/build_cmake.yml` | Runs `ctest` against empty suite |
| `.github/workflows/MacOS-pack.yml` | dmg builds on `macos-15` (arm64 + intel) |
| `tests/action_options.sh`, `tests/path_option.sh` | Manual GUI test scripts; not in CI |

## Findings to Fix (consolidated)

| # | File | Lines | Description | Fix |
|---|---|---|---|---|
| F1 | `CMakeLists.txt` | 144-149 | QHotkey pinned to `master` branch | Pin to `v1.5.0` or commit SHA |
| F2 | `CMakeLists.txt` | 109 | `set_project_warnings()` commented out | Uncomment with `WARNINGS_AS_ERRORS=FALSE` initially |
| F3 | `src/CMakeLists.txt` | 64-68 | Deprecated `add_definitions()` for Windows version | Replace with `target_compile_definitions()` |
| F4 | `src/CMakeLists.txt` | 104-105 | Direct `CMAKE_CXX_FLAGS` manipulation for MSVC `/MP` | Use `target_compile_options()` |
| F5 | `src/widgets/capture/capturewidget.h` | 187-206 | Mixed QPointer + raw pointers in member declarations | Standardize on QPointer for all dynamically-deleted members |
| F6 | `src/widgets/capture/capturewidget.cpp` | 1650-1718 | 18 old-style SLOT() string connections | Convert to `&Class::method` PMF form |
| F7 | `src/widgets/capture/capturewidget.cpp` | 572, 600, 606, 1468, 2022 | Manual `delete` calls on parented widgets | Audit each; remove or document why manual |
| F8 | `src/widgets/capture/capturewidget.cpp` | 701, 1091, 1157, 1194 | Magic numbers in painting/event logic (`10, 12`, `200`, `QPoint(1000, 200)`) | Promote to named constants |
| F9 | `src/utils/screengrabber.cpp` | 85-112 | Synchronous QEventLoop blocks UI for up to 30s | Convert to async pattern with QFutureWatcher / signal callback |
| F10 | `src/utils/screengrabber.cpp` | 52-160 | xdg-desktop-portal logic interleaved with X11/macOS/Win | Extract to `PortalScreenGrabber` class |
| F11 | `src/utils/screengrabber.cpp` | 381 | Silent `return nullptr` on monitor preview failure | Add `AbstractLogger::error()` log |
| F12 | `src/utils/screenshotsaver.cpp` | 78 | Silent `return false` on save failure | Add `AbstractLogger::error()` log |
| F13 | `src/tools/launcher/terminallauncher.cpp` | 48 | User-supplied command string passed to terminal — shell metacharacters interpreted | Document or sandbox; at minimum, warn in UI |
| F14 | `src/utils/confighandler.cpp` | 134 | Default Imgur Client ID hardcoded as fallback (`313baf0c7b4d3ff`) | Move to build-time `-D` define so distros can override |
| F15 | `tests/action_options.sh`, `tests/path_option.sh` | — | Manual-only, never run in CI | Either port to a real test runner or delete |
| F16 | `.github/workflows/build_cmake.yml` | — | Calls `ctest` against empty suite | Add at least one real test or remove the call |
| F17 | `src/core/flameshot.cpp` | header | `QThread::msleep(delay)` blocks main thread | Replace with `QTimer::singleShot` |
| F18 | `src/tools/toolfactory.cpp` | 35-71 | Hard-coded 24-case factory; closed to extension | Move to self-registering pattern (compile-time list via X-macro or static registrar) |
| F19 | `src/tools/imgupload/storages/imgur/imguruploader.h` | 29 | Each uploader creates own QNetworkAccessManager | Share a single NAM at app or daemon level |
| F20 | `src/utils/screengrabber.cpp` | 126 vs nearby `#ifdef` | Mixed runtime/compile-time platform detection | Pick one model; runtime preferred for Linux WSI variants |

## Implementation Plan (sequence of 4 staged PRs)

**PR 1 — "Turn on the alarms" (½ day):**
- F1 Pin QHotkey
- F2 Warnings on (errors off)
- F16 + F15 Either delete or revive the test scripts
- Enable ASAN+UBSAN on Linux CI

**PR 2 — "Build the test foundation" (3 days):**
- Add `tests/CMakeLists.txt` with `enable_testing()` and Qt Test
- Unit tests for `ConfigHandler::checkForErrors()` (F-coverage of 50 schema options)
- Unit tests for `FileNameHandler` (path templating edge cases)
- Unit tests for `ValueHandler` (type coercion)
- Documentation in `docs/CONTRIBUTING.md` on how to write a Flameshot test

**PR 3 — "Mechanical cleanups" (2 days, mostly auto-able):**
- F3, F4 Modernize CMake idioms
- F6 SLOT() → PMF
- F8 Extract magic numbers
- F11, F12, F17 Logging consistency + fix `QThread::msleep`
- Fix the warnings exposed in PR 1; flip to `-Werror`

**PR 4 — "Process gate + portal extraction" (3 days):**
- F6-process: PR-blocking packaging matrix (`.deb`, `.rpm`, `.dmg`, `.exe` all build on every PR)
- F10 Extract `PortalScreenGrabber` class
- F9 Async monitor selection
- Per-platform capture smoke test in CI

**Deferred to v15 RFC:**
- `CaptureWidget` decomposition
- Full subclass-per-platform `ScreenGrabber`
- F18 Self-registering tool factory
- F19 Shared QNetworkAccessManager
