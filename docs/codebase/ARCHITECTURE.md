# Architecture

**Analysis Date:** 2026-07-24

## System Overview

```text
                              +-----------------------------+
   CLI args / hotkey / tray   |   src/main.cpp (entry)      |
   ------------------------->  |   arg parse + app bootstrap |
                              +--------------+--------------+
                                             |
                          +------------------+-------------------+
                          v                                      v
              +-------------------------+         +------------------------------+
              | Flameshot (singleton)   |         | FlameshotDaemon (singleton)  |
              | src/core/flameshot.cpp  |<------->| src/core/flameshotdaemon.cpp |
              | orchestrates captures   |  D-Bus  | tray, clipboard, pins,       |
              | + exportCapture()       |  /IPC   | update check                 |
              +-----------+-------------+         +---------------+--------------+
                          |                                       |
          +---------------+----------------+          +-----------+-----------+
          v               v                v          v           v           v
  +----------------+ +-----------+ +-------------+ +---------+ +---------+ +----------+
  | CaptureWidget  | | Screen-   | | CaptureReq  | | TrayIcon| | PinWidget| | Config- |
  | (GUI editor)   | | Grabber   | | (value obj) | |         | |         | | Window   |
  | widgets/capture| | utils/    | | core/       | | widgets/| | tools/pin| | config/  |
  +-------+--------+ +-----------+ +-------------+ +---------+ +---------+ +----------+
          |
          | ToolFactory().CreateTool(type)
          v
  +-----------------------------+      +--------------------------------+
  | CaptureTool hierarchy       |      | ImgUploaderManager / Base      |
  | tools/*  (pencil, arrow...) |      | tools/imgupload/*  (imgur)     |
  +-----------------------------+      +--------------------------------+

  Cross-cutting: ConfigHandler (utils/confighandler.cpp, QSettings singleton),
                 AbstractLogger (utils/abstractlogger.cpp), ValueHandler, PathInfo.
```

## Component Responsibilities

| Component | Responsibility | File |
| --- | --- | --- |
| `main()` | Bootstrap Qt app, parse CLI, select capture mode, register D-Bus/single-instance | `src/main.cpp:202` |
| `Flameshot` | Application-level singleton; routes capture requests, owns capture/config/launcher/info windows, dispatches export tasks | `src/core/flameshot.cpp:116` |
| `FlameshotDaemon` | Long-lived singleton hosting tray, clipboard, pinned widgets, update checks; receives IPC calls | `src/core/flameshotdaemon.cpp:66` |
| `CaptureRequest` | Value object describing capture mode, delay, target path, and bitmask of export tasks | `src/core/capturerequest.h:10` |
| `CaptureWidget` | Full-screen interactive editor: selection, tool buttons, drawing, undo/redo, finalize | `src/widgets/capture/capturewidget.cpp:333` |
| `ScreenGrabber` | Platform screen capture (xdg-desktop-portal, X11 legacy, native macOS/Windows) | `src/utils/screengrabber.cpp` |
| `CaptureTool` | Abstract base for every annotation/action tool | `src/tools/capturetool.h:13` |
| `ToolFactory` | Maps `CaptureTool::Type` enum to concrete tool instances | `src/tools/toolfactory.cpp:35` |
| `ImgUploaderManager` | Selects and drives an uploader backend (currently Imgur only) | `src/tools/imgupload/imguploadermanager.cpp:36` |
| `ConfigHandler` | Singleton wrapper over `QSettings` INI; typed getters/setters via macros; validation | `src/utils/confighandler.cpp` / `.h:62` |
| `AbstractLogger` | Multi-target logging (stderr, log file, desktop notification, string) | `src/utils/abstractlogger.cpp` |
| `FlameshotDBusAdapter` | Exposes `org.flameshot.Flameshot` D-Bus interface on Linux | `src/core/flameshotdbusadapter.h:8` |

## Pattern Overview

**Overall:** Event-driven Qt Widgets desktop application with two collaborating
singletons (an application controller and an optional resident daemon), a
factory-instantiated tool plugin hierarchy, and a request/value object that
carries a capture from CLI/hotkey through to export.

**Key Characteristics:**
- Qt signals/slots for all inter-object communication (e.g. `captureTaken`,
  `captureFailed`, `requestAction`).
- Two process roles from one binary: transient capture invocations vs. a
  resident daemon, bridged by D-Bus (Linux) or `KDSingleApplication` (macOS/Windows).
- `Flameshot::instance()` and `FlameshotDaemon::instance()` are Meyers/lazy
  singletons; static public methods proxy to the daemon instance or fall back
  to IPC when the current process is not the daemon.
- Annotation tools implement a common `CaptureTool` interface and are created
  by enum via `ToolFactory`.
- Config is a file-backed singleton (`QSettings`), hot-reloaded via
  `QFileSystemWatcher` (`ConfigHandler::fileChanged`).

## Layers

**Entry / CLI:**
- Purpose: Parse arguments, choose capture mode, bootstrap the Qt application object
- Location: `src/main.cpp`, `src/cli/`, `src/windows-cli.cpp`
- Contains: `main()`, `CommandLineParser`, `CommandArgument`, `CommandOption`
- Depends on: `core/`, `utils/confighandler`
- Used by: OS process launch

**Core / Orchestration:**
- Purpose: Application control, capture routing, daemon/IPC, export dispatch
- Location: `src/core/`
- Contains: `Flameshot`, `FlameshotDaemon`, `CaptureRequest`, D-Bus adapter, signal daemon
- Depends on: `utils/`, `widgets/`, `tools/`
- Used by: `main()`, tray icon, D-Bus callers

**Widgets / UI:**
- Purpose: The capture editor and all supporting dialogs/windows
- Location: `src/widgets/` (editor in `src/widgets/capture/`)
- Contains: `CaptureWidget`, `SelectionWidget`, `ButtonHandler`, tray icon, upload dialogs
- Depends on: `tools/`, `config/`, `utils/`, `core/`
- Used by: `Flameshot`

**Tools / Annotation:**
- Purpose: Drawing tools and action tools available in the editor
- Location: `src/tools/`
- Contains: `CaptureTool` hierarchy, `ToolFactory`, image-upload backends
- Depends on: `utils/`, `core/capturerequest`
- Used by: `CaptureWidget` via `CaptureToolButton`

**Config / Settings UI:**
- Purpose: Preference windows and editors
- Location: `src/config/`
- Contains: `ConfigWindow`, shortcut editor, color editors, `ConfigResolver`
- Depends on: `utils/confighandler`
- Used by: `Flameshot::config()`, `resolveAnyConfigErrors()`

**Utils / Cross-cutting:**
- Purpose: Screen grabbing, config, logging, filename/path helpers, history, saving
- Location: `src/utils/`
- Contains: `ScreenGrabber`, `ConfigHandler`, `AbstractLogger`, `ValueHandler`, `FileNameHandler`, `screenshotsaver`
- Depends on: Qt only (plus platform APIs)
- Used by: all other layers

## Data Flow

### Primary Flow

Interactive screenshot (`flameshot gui`) → annotate → export:

1. `main()` parses args, builds a `CaptureRequest(GRAPHICAL_MODE, ...)` and calls `requestCaptureAndWait` (`src/main.cpp:469`, `src/main.cpp:499`, `src/main.cpp:528`).
2. `Flameshot::requestCapture` dispatches on capture mode, deferring by `delay` via `QTimer::singleShot` (`src/core/flameshot.cpp:420`).
3. For `GRAPHICAL_MODE`, `Flameshot::gui` constructs and shows a full-screen `CaptureWidget` (`src/core/flameshot.cpp:122`, `src/core/flameshot.cpp:160`).
4. `CaptureWidget` grabs the screen via `ScreenGrabber` into `CaptureContext`, then `initButtons()` builds tool buttons (`src/widgets/capture/capturewidget.cpp:333`).
5. Each `CaptureToolButton` instantiates its tool through `ToolFactory().CreateTool(type, this)` (`src/widgets/capture/capturetoolbutton.cpp:47`).
6. User drags a selection and picks tools; mouse events call the tool's `drawStart`/`drawMove`/`drawEnd`; drawn objects are stored in `m_captureToolObjects` and rendered by `processPixmapWithTool` / `drawToolsData` (`src/widgets/capture/capturewidget.cpp:1840`, `:1872`); undo/redo backed by `QUndoStack`.
7. On accept, `m_captureDone` is set and the widget's destructor calls `Flameshot::instance()->exportCapture(pixmap(), geometry, request)` (`src/widgets/capture/capturewidget.cpp:326`).
8. `Flameshot::exportCapture` reads the task bitmask and performs each: PRINT_GEOMETRY/PRINT_RAW to stdout, SAVE via `screenshotsaver`, COPY/PIN via `FlameshotDaemon`, UPLOAD via `ImgUploaderManager` (`src/core/flameshot.cpp:449`).
9. Clipboard, pin hosting, and tray live in the daemon; non-daemon processes reach them through static methods that marshal over D-Bus / `KDSingleApplication` (`src/core/flameshotdaemon.cpp:114`, `:137`).
10. `captureTaken`/`captureFailed` signals let the transient CLI process exit with the right code (`src/main.cpp:66`).

Non-interactive modes (`full`, `screen`) skip the editor: `Flameshot::full`/`Flameshot::screen` grab directly and call `exportCapture` (`src/core/flameshot.cpp:183`, `:237`).

**State Management:**
- Transient per-capture state lives in `CaptureContext` (`src/tools/capturecontext.h:13`) and the drawn `CaptureToolObjects` inside `CaptureWidget`.
- Persistent app state lives in the `ConfigHandler` INI file and the resident daemon (clipboard ownership, pin widgets, tray).
- Last-used selection region is cached via `getLastRegion`/`setLastRegion` (`src/config/cacheutils.*`).

## Key Abstractions

**CaptureTool:**
- Purpose: Uniform interface for every editor tool (drawing and actions)
- Examples: `src/tools/capturetool.h:13`, concrete tools under `src/tools/<name>/`
- Pattern: Abstract base `QObject` with a `Type` enum (`src/tools/capturetool.h:25`) and a `Request` enum for callbacks to the widget (`src/tools/capturetool.h:58`); three intermediate bases specialize behavior:
  - `AbstractTwoPointTool` (`src/tools/abstracttwopointtool.h`) — shape tools (arrow, line, rectangle, circle, marker, pixelate, selection, invert, circlecount)
  - `AbstractPathTool` (`src/tools/abstractpathtool.h`) — freehand (pencil)
  - `AbstractActionTool` (`src/tools/abstractactiontool.h`) — non-drawing actions (copy, save, exit, undo, redo, pin, move, upload, size +/-, app launcher, accept)

**ToolFactory:**
- Purpose: Central enum-to-class instantiation for tools
- Examples: `src/tools/toolfactory.cpp:35`
- Pattern: `switch` over `CaptureTool::Type` with a `if_TYPE_return_TOOL` macro; guarded by `#ifdef ENABLE_IMGUR` and platform macros

**CaptureRequest / ExportTask:**
- Purpose: Describe what to capture and what to do with the result
- Examples: `src/core/capturerequest.h:10`, task bitmask enum at `:20`
- Pattern: Value object with a bitwise-OR task flag set; free `operator|/&/|=` overloads at `src/core/capturerequest.h:71`

**ImgUploaderBase:**
- Purpose: Backend-agnostic image upload widget interface
- Examples: `src/tools/imgupload/storages/imguploaderbase.h:19`, Imgur impl `src/tools/imgupload/storages/imgur/imguruploader.cpp`
- Pattern: Abstract `QWidget` with `upload()`/`deleteImage()` pure virtuals; `ImgUploaderManager` chooses the concrete class (currently hard-coded to Imgur — see TODOs in `imguploadermanager.cpp:22`)

## Entry Points

**GUI daemon (no args):**
- Location: `src/main.cpp:216`
- Trigger: `flameshot` with no arguments
- Responsibilities: Create `QApplication`, single-instance guard (`KDSingleApplication`), start `FlameshotDaemon`, register D-Bus service, run event loop

**CLI subcommands:**
- Location: `src/main.cpp:262`-`:682` (`gui`, `screen`, `full`, `launcher`, `config`)
- Trigger: `flameshot <subcommand> [options]`
- Responsibilities: Build a `CaptureRequest`, `reinitializeAsQApplication`, run capture, exit with code

**D-Bus interface:**
- Location: `src/core/flameshotdbusadapter.h:8`, registered at `src/main.cpp:250`
- Trigger: External D-Bus calls to `org.flameshot.Flameshot` (`captureScreen`, `attachPin`, `attachScreenshotToClipboard`, `attachTextToClipboard`)
- Responsibilities: Forward IPC calls into the daemon singleton

**Global hotkeys / tray:**
- Location: `QHotkey` wiring in `src/core/flameshot.cpp:99` (macOS/Windows); `TrayIcon` in `src/widgets/trayicon.cpp`; `GlobalShortcutFilter` on Windows
- Trigger: Configured hotkey or tray menu
- Responsibilities: Invoke `gui()`/`history()`

## Architectural Constraints

- **Concurrency:** Single-threaded Qt event loop (`qApp->exec()`). No worker threads for capture; delays and periodic work use `QTimer::singleShot` (`src/core/flameshot.cpp:428`, update poll at `src/core/flameshotdaemon.cpp:227`). Network work (uploads, update check) is asynchronous via `QNetworkAccessManager` signals. IPC across processes is via D-Bus (Linux) or `KDSingleApplication` message passing (macOS/Windows). On Unix, POSIX signals are bridged to Qt through `SignalDaemon` (`src/core/signaldaemon.cpp`).
- **Global state:** `Flameshot::instance()` (`src/core/flameshot.cpp:116`) and `FlameshotDaemon::m_instance` (`src/core/flameshotdaemon.cpp:517`) singletons; `ConfigHandler` singleton via `getInstance()` (`src/utils/confighandler.h:69`); static `Flameshot::m_origin` (`src/core/flameshot.cpp:535`); process-wide `QSharedMemory` GUI mutex (`src/main.cpp:95`).
- **Boundaries:** The daemon vs. transient-process split is the main boundary — anything requiring persistence (clipboard on X11, pins, tray) must go through `FlameshotDaemon` static methods, which internally decide between direct call and IPC (`src/core/flameshotdaemon.cpp:114`). Heavy platform `#ifdef` boundaries separate Linux (D-Bus, portal), macOS (Objective-C runtime, native fullscreen), and Windows (native event filter).

## Error Handling

**Strategy:** No C++ exceptions for control flow. Functions signal failure via
`bool` out-params (e.g. `ScreenGrabber::grab*(bool& ok)`), Qt signals
(`captureFailed`), and process exit codes; user-facing errors go through
`AbstractLogger`.

**Patterns:**
- `bool ok` out-parameter for grab operations (`src/core/flameshot.cpp:189`, `src/utils/screengrabber.cpp`)
- `captureFailed`/`captureTaken` signals decide CLI exit code (`src/main.cpp:70`, `:83`)
- Named exit codes `E_OK`, `E_ABORTED`, `E_DBUSCONN`, etc. (`src/utils/globalvalues.h`, used at `src/main.cpp:90`)
- Config validation surfaces through `ConfigHandler::checkForErrors` / `checkSemantics` and the interactive `ConfigResolver` dialog (`src/core/flameshot.cpp:391`)
- CLI option validators (`addChecker`) with per-option error strings (`src/main.cpp:355`-`:414`)

## Cross-Cutting Concerns

**Logging:** `AbstractLogger` (`src/utils/abstractlogger.h:10`) — stream-style
API with selectable targets (Stderr, LogFile, desktop Notification, String,
Stdout). `AbstractLogger::info/warning/error()` are the standard entry points;
raw `qWarning()` is used in a few low-level spots.

**Validation:** CLI values validated by lambda checkers in `main()`; config
values validated/typed through `ValueHandler` (`src/utils/valuehandler.cpp`) and
`ConfigHandler` semantic checks; `Region` value handler parses geometry strings
(`src/main.cpp:379`).

**Authentication:** Not applicable for the app itself. Imgur uploads use a
client-id / anonymous token flow inside `imguruploader.cpp`; no user auth system.

**Configuration:** `ConfigHandler` over `QSettings` INI, with typed accessors
generated by `CONFIG_GETTER_SETTER` macros (`src/utils/confighandler.h:29`-`:60`)
and live reload via `QFileSystemWatcher` emitting `fileChanged`
(`src/core/flameshotdaemon.cpp:88`). Example config: `flameshot.example.ini`.

---

*Architecture analysis: 2026-07-24*
