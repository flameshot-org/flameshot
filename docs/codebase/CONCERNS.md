# Codebase Concerns

**Analysis Date:** 2026-07-24

Evidence-based concerns for Flameshot, a cross-platform Qt/C++ screenshot tool
built with CMake. Every item cites concrete repo-relative paths. Current state
only.

## Tech Debt

**Marker comments (TODO/FIXME/HACK/XXX):**
- Issue: 38 marker comments across 20 files in `src/`. Breakdown:
  ~30 `TODO`, ~3 `FIXME` (functional workarounds), 1 `HACK`. Most are
  refactor/cleanup notes; a few flag real workarounds.
- Files (highest concentration and most actionable):
  - `src/widgets/capture/capturewidget.cpp` (9 markers, incl. two
    `// FIXME this is a temporary workaround` at lines 1991 and 2003, and a
    performance TODO at line 1842)
  - `src/tools/imgupload/imguploadermanager.cpp` (lines 6, 17, 24, 39):
    "remove this hard-code and create plugin manager", storage selection is
    hard-wired to a single provider
  - `src/core/globalshortcutfilter.cpp:31`: "temporary workaround; proper
    global [shortcut handling]"
  - `src/utils/abstractlogger.cpp:12,60`: bare `// TODO` placeholders
  - `src/utils/valuehandler.cpp:529`: `#include <QApplication> // TODO remove
    after FIXME (see below)` — include kept only to work around a pending fix
- Impact: accumulated small debt; the capturewidget FIXMEs and the uploader
  hard-coding are the ones most likely to bite feature work.
- Fix approach: address the two capturewidget workarounds when touching capture
  rendering; extract the uploader into the promised plugin/manager abstraction
  before adding new storage backends.

**God object — `capturewidget.cpp`:**
- Issue: 2086 lines, by far the largest translation unit (next largest is
  `src/config/generalconf.cpp` at 991). Owns capture rendering, tool objects,
  input handling, and undo/redo.
- Files: `src/widgets/capture/capturewidget.cpp`,
  `src/widgets/capture/capturewidget.h` (235 lines)
- Impact: high cognitive load; changes ripple across unrelated capture
  concerns; the file itself carries refactor TODOs (lines 1565, 1842, 1862).
- Fix approach: split tool-object lifecycle and event handling into
  collaborators; the existing `src/widgets/capture/capturetoolobjects.cpp`
  is a natural seam.

**Hard-coded single uploader backend:**
- Issue: `imguploadermanager` hard-codes the Imgur backend and URL rather than
  discovering storages via a plugin manager.
- Files: `src/tools/imgupload/imguploadermanager.cpp:32-33`,
  `src/tools/imgupload/imguploadermanager.h:10`
  (`#define IMG_UPLOADER_STORAGE_DEFAULT "imgur"`)
- Impact: adding S3/custom-server uploaders requires editing the manager
  instead of registering a plugin.
- Fix approach: introduce the registry/plugin abstraction the TODOs describe.

## Known Bugs

**Locale-specific strftime names broken on Windows:**
- Symptoms: localized date/time names (e.g. Cyrillic) render incorrectly in the
  filename-pattern chooser on Windows.
- Files: `src/config/strftimechooserwidget.cpp:48,56,64,73` (four
  `// TODO - fix localized names on windows (ex. Cyrillic)` markers)
- Trigger: run on Windows with a non-Latin system locale and open the filename
  pattern editor.
- Workaround: none in code.

**No global shortcuts on Linux (X11/Wayland):**
- Symptoms: global hotkeys are not registered on Linux desktops.
- Files: `src/config/shortcutswidget.cpp:228`
  (`// TODO - Linux doesn't support global shortcuts for (XServer and
  Wayland)`), `src/core/globalshortcutfilter.cpp:31`
- Trigger: attempt to bind a system-wide capture shortcut on Linux.
- Workaround: users configure shortcuts via their desktop environment / WM
  (documented in `docs/UsageX11MinimalWM.md`,
  `docs/UsageHyprlandSwayWlroots.md`).

**UI language change requires restart:**
- Symptoms: switching visuals/UI language does not retranslate live.
- Files: `src/config/visualseditor.cpp:138`
  (`// TODO: Retranslate UI without restart`)
- Trigger: change language in settings.
- Workaround: restart the application.

## Security Considerations

**Hard-coded default Imgur Client-ID in source:**
- Risk: `src/utils/confighandler.cpp:134` ships a default value for
  `uploadClientSecret` (an Imgur OAuth Client-ID, a public app identifier). It
  is user-overridable via settings (`src/config/generalconf.cpp:617,627`) and
  sent as an `Authorization: Client-ID <value>` header
  (`src/tools/imgupload/storages/imgur/imguruploader.cpp:93-96`). This is a
  public client identifier by Imgur's design, not a private secret, but its
  presence in a public repo means the shared quota can be exhausted/abused.
- Files: `src/utils/confighandler.cpp:134`,
  `src/tools/imgupload/storages/imgur/imguruploader.cpp:88-98`
- Current mitigation: value is overridable in Settings; label calls it a
  client key, not a secret. No `.env` or secret files are used by the app.
- Recommendation: keep documenting that heavy users should supply their own
  Client-ID; do not treat this as a credential to be hidden (it is public), but
  monitor for abuse of the default quota.

**Image upload over network (Imgur):**
- Risk: screenshots are POSTed to `https://api.imgur.com/3/image`; response and
  delete-token are parsed from JSON.
- Files: `src/tools/imgupload/storages/imgur/imguruploader.cpp:77-99`,
  `src/tools/imgupload/storages/imguploaderbase.cpp`
- Current mitigation: endpoint is HTTPS (TLS handled by Qt Network / system
  libraries via `QNetworkAccessManager`); no custom certificate handling and no
  TLS verification is disabled anywhere in `src/`. The image content-type
  header is malformed (`"application/application/x-www-form-urlencoded"` at
  line 91-92) but Imgur tolerates it.
- Recommendation: verify Qt is not configured to ignore SSL errors (none found
  in `src/`); consider fixing the duplicated content-type string.

**External process launching (app launcher / terminal / macOS autostart):**
- Risk: Flameshot launches user-configured programs and terminals with the
  screenshot path as an argument, and runs `osascript` on macOS for
  startup-item management.
- Files:
  - `src/tools/launcher/applauncherwidget.cpp:144,171` — uses
    `QProcess::splitCommand` + `QProcess::startDetached` (argument vector, not a
    shell string), so no shell-injection surface; on Windows it deliberately
    does not split (line 138-142).
  - `src/tools/launcher/terminallauncher.cpp:45-52` — launches from a fixed
    allow-list of known terminals via `startDetached` with an arg vector.
  - `src/utils/confighandler.cpp:289-303` — runs `osascript` on macOS with
    fixed AppleScript strings (no untrusted interpolation).
- Current mitigation: no `system()`/`popen()`/shell string execution anywhere
  in `src/`; all invocations use `QProcess` argument vectors.
- Recommendation: keep using arg-vector `QProcess` calls; commands are
  user-supplied by design, so treat launcher config as trusted input.

**D-Bus surface (Linux IPC):**
- Risk: the daemon registers `org.flameshot.Flameshot` and exposes methods
  (`gui`, `attachPin`, `attachScreenshotToClipboard`, `attachTextToClipboard`)
  callable by any process on the session bus.
- Files: `src/core/flameshotdaemon.cpp:433-454`,
  `src/core/flameshotdbusadapter.cpp:16-32`,
  `src/core/flameshotdaemon.h`
- Current mitigation: session bus scope (same-user processes only); this is
  expected IPC for the tray/daemon split.
- Recommendation: none required beyond session-bus scoping; be cautious adding
  methods that write files or execute programs from D-Bus input.

**Clipboard / screenshot handling:**
- Risk: screenshots and text are placed on the clipboard and, on Wayland,
  captured via the freedesktop portal.
- Files: `src/utils/screengrabber.cpp:55-119` (portal via `QDBusInterface`),
  `src/core/flameshotdaemon.cpp` (clipboard attach paths)
- Current mitigation: Wayland capture goes through the sanctioned
  `org.freedesktop.portal.Screenshot` interface (respects the Wayland security
  model); "Capture Active Monitor" is explicitly disabled on Wayland
  (`src/utils/screengrabber.cpp:191-192`).
- Recommendation: no change; portal usage is the correct approach.

Project security posture (from `SECURITY.md`): only the latest stable release
and `master` HEAD are supported; vulnerabilities go to
`admin@flameshot.org` / the GitHub security tab, never public issues; reports
must include a CVSS vector, a working PoC, and reproduction steps. Explicitly
out of scope: issues requiring prior root/compromised OS, and raw scanner
output without a verified PoC. The project is volunteer-maintained FLOSS, so
triage timelines are best-effort.

## Performance Bottlenecks

**Capture object redraw on every update:**
- Problem: all capture tool objects are re-rendered on updates rather than only
  the changed region.
- Files: `src/widgets/capture/capturewidget.cpp:1842`
  (`// TODO refactor this for performance. The objects should not all be
  updated`), `src/widgets/capture/capturetoolobjects.cpp:93` (cache "is not
  optimal and cache will be used just after first tool")
- Cause: no dirty-region tracking; full-scene invalidation.
- Improvement path: track dirty rectangles / cache rendered tool objects and
  repaint only changed regions.

## Fragile Areas

**Cross-platform screen grabbing:**
- Files: `src/utils/screengrabber.cpp` (740 lines, 13 platform `#ifdef`
  branches; distinct code paths for `Q_OS_MACOS`, `Q_OS_WIN`, and
  `Q_OS_UNIX && !Q_OS_MACOS` with a Wayland portal sub-path at lines 55-119,
  247-275, 296-332, 353+)
- Why fragile: each of macOS, Windows, X11, and Wayland-portal is a separate
  branch that cannot be exercised on a single machine; a change compiled and
  tested on one OS may silently break another.
- Safe modification: change one `#ifdef` branch at a time; build and manually
  test on the affected OS; keep the portal path aligned with the Wayland
  security-model constraints already encoded (e.g. active-monitor capture
  disabled on Wayland).
- Test coverage: none automated (see Test Coverage Gaps).

**Platform conditional compilation, project-wide:**
- Files: 168 platform `#ifdef` occurrences across `src/`; densest in
  `src/core/flameshot.cpp` (16), `src/utils/confighandler.cpp` (14),
  `src/utils/screengrabber.cpp` (13), `src/core/flameshotdaemon.cpp` (10),
  `src/config/generalconf.cpp` (10), `src/main.cpp` (9),
  `src/widgets/trayicon.cpp` (8)
- Why fragile: behavior diverges per OS; contributors on Linux rarely exercise
  the macOS/Windows branches. macOS notably has no D-Bus, so each Flameshot
  instance is its own daemon (`src/core/flameshotdaemon.cpp:275`) — a genuinely
  different runtime model.
- Safe modification: compile-test all three OSes via the CI packaging
  workflows before merging changes to these files.

**Global singletons / shared mutable state:**
- Files: `src/core/flameshot.cpp:116` (`Flameshot::instance()`),
  `src/core/flameshotdaemon.cpp:273` (`FlameshotDaemon::instance()`),
  `src/utils/confighandler.cpp` (settings accessed via freshly constructed
  `ConfigHandler()` wrapping a shared `QSettings`)
- Why fragile: two app-wide singletons are reached from many call sites
  (`src/main.cpp`, `src/core/globalshortcutfilter.cpp`,
  `src/core/flameshotdbusadapter.cpp`, tray/daemon code). Ordering matters —
  e.g. `src/core/flameshotdaemon.cpp:108` notes the tray icon needs
  `FlameshotDaemon::instance()` to be non-null.
- Safe modification: preserve initialization order in `src/main.cpp`; do not
  assume a singleton exists in a given run mode (daemon vs one-shot vs macOS
  per-instance).
- Test coverage: none.

## Scaling Limits

**Single-image, interactive workflow:**
- Current capacity: Not measured. Flameshot is an interactive desktop tool —
  one capture/edit/upload at a time; there is no batch or headless-throughput
  path beyond the CLI single-shot mode (`src/cli/`, `src/main.cpp`).
- Limit: not a server workload; scaling limits are effectively per-image
  rendering performance (see Performance Bottlenecks), not concurrency.
- Scaling path: N/A for the product shape.

## Dependencies at Risk

**Qt major version (now Qt6-default, Qt5 legacy):**
- Risk: `CMakeLists.txt:20` sets `QT_VERSION_MAJOR 6` by default; the codebase
  parameterizes linkage as `Qt${QT_VERSION_MAJOR}::…`
  (`src/CMakeLists.txt:187-195`). Some code still carries Qt5-vs-Qt6 timing
  workarounds (e.g. `src/widgets/capture/capturewidget.cpp:242`
  "In Qt6 some timing related [behavior changed]").
- Impact: distros still building against Qt5 rely on the parameterized path;
  Qt6-specific behavior differences (timing, APIs) can surface as subtle
  regressions.
- Migration plan: treat Qt6 as primary; keep the Qt5 build path working only as
  long as target distros need it, then drop the version parameterization.

**C++20 toolchain requirement:**
- Risk: `CMakeLists.txt:100` requires `cxx_std_20`.
- Impact: older distro toolchains cannot build; constrains the packaging matrix.
- Migration plan: none needed; document minimum compiler versions in packaging.

**Bundled QtColorWidgets:**
- Risk: a third-party widget lib is vendored and shares `QT_VERSION_MAJOR`
  (`CMakeLists.txt:19`).
- Impact: upstream fixes must be manually synced.
- Migration plan: track upstream; prefer system package where available.

## Missing Critical Features

**Pluggable upload backends:**
- Problem: only Imgur is wired in; the manager is hard-coded
  (`src/tools/imgupload/imguploadermanager.cpp:6-39`).
- Blocks: adding S3, custom-server, or self-hosted upload targets without
  editing core manager code.

**Live UI retranslation:**
- Problem: language changes need an app restart
  (`src/config/visualseditor.cpp:138`).
- Blocks: seamless in-session language switching.

## Test Coverage Gaps

**Near-total absence of automated tests:**
- What is not tested: essentially the entire codebase. The only artifacts in
  `tests/` are two shell scripts —
  `tests/action_options.sh` and `tests/path_option.sh` — that are manual/
  interactive (they print notifications for a human to eyeball; require
  ImageMagick's `display`). There is no unit-test target: no `enable_testing`,
  `add_test`, or `ctest` in `CMakeLists.txt` or `src/CMakeLists.txt`, and the
  CI build workflow (`.github/workflows/build_cmake.yml`) has no test/coverage
  step.
- Files: `tests/action_options.sh`, `tests/path_option.sh`; no coverage for
  `src/utils/screengrabber.cpp`, `src/widgets/capture/capturewidget.cpp`,
  `src/tools/imgupload/**`, `src/utils/confighandler.cpp`, `src/cli/**`.
- Risk: platform-specific regressions, capture/rendering bugs, config parsing
  errors, and upload failures ship undetected; refactoring the large fragile
  files (capturewidget, screengrabber) is high-risk with no safety net.
- Priority: High. Start with pure-logic units that need no display:
  `src/cli/commandlineparser.cpp`, `src/utils/confighandler.cpp` (option
  parsing/defaults), `src/utils/filenamehandler` (pattern parsing), and the
  imgur JSON reply parsing in
  `src/tools/imgupload/storages/imgur/imguruploader.cpp:32-73`. Add a `ctest`
  target and a CI test step.

**Note on the "TypeScript component":** the ~48 `.ts` files under
`data/translations/` (`Internationalization_*.ts`) are **Qt Linguist
translation sources (XML)**, not TypeScript. There is no TypeScript component in
this repo (`Not detected`). No separate test story applies to them beyond Qt's
`lupdate`/`lrelease` tooling.

## Static Analysis Posture (context)

- `.clang-tidy` enables nearly all checks (`Checks: '*'` minus a few families)
  with `WarningsAsErrors: '*'`, and `.clang-format` / `.cmake-format.yaml` are
  present; `.github/workflows/clang-format.yml` enforces formatting in CI.
- This partly compensates for the lack of unit tests (catches many correctness
  and style issues at build time) but does not exercise runtime behavior.

---

*Concerns audit: 2026-07-24*
