# Codebase Structure

**Analysis Date:** 2026-07-24

## Directory Layout

```text
flameshot/
|-- src/                    # All C++ application source
|   |-- main.cpp            # Entry point: CLI parse + app bootstrap
|   |-- windows-cli.cpp     # Windows console launcher shim
|   |-- cli/                # Command-line parser, arguments, options
|   |-- core/               # App controller, daemon, capture request, D-Bus
|   |-- widgets/            # GUI: capture editor, tray, dialogs, panels
|   |   |-- capture/        # The full-screen capture/annotation editor
|   |   `-- panel/          # Side/utility panel widgets
|   |-- tools/              # Annotation + action tools (CaptureTool hierarchy)
|   |   |-- <toolname>/     # One dir per tool (arrow, pencil, text, ...)
|   |   `-- imgupload/      # Image upload backends (imgur)
|   |-- config/             # Settings UI: config window, editors, resolver
|   `-- utils/              # Screen grab, config, logging, paths, saving
|-- data/                   # Icons, translations, desktop files, resources
|-- cmake/                  # CMake helper modules
|-- packaging/              # Distro/OS packaging (macos, appimage, etc.)
|-- scripts/                # Build/dev/release helper scripts
|-- tests/                  # Shell-based CLI smoke tests
|-- docs/                   # Documentation (this file lives in docs/codebase/)
|-- CMakeLists.txt          # Top-level build configuration
|-- flameshot.example.ini   # Reference configuration file
`-- README.md               # Project overview
```

## Directory Purposes

**`src/cli/`:**
- Purpose: Parse and validate command-line arguments
- Contains: parser, argument, and option abstractions
- Key files: `src/cli/commandlineparser.cpp`, `src/cli/commandoption.cpp`, `src/cli/commandargument.cpp`

**`src/core/`:**
- Purpose: Application orchestration, daemon lifecycle, IPC, capture request model
- Contains: singletons, value objects, D-Bus adapter, signal handling
- Key files: `src/core/flameshot.cpp`, `src/core/flameshotdaemon.cpp`, `src/core/capturerequest.cpp`, `src/core/flameshotdbusadapter.cpp`, `src/core/signaldaemon.cpp`, `src/core/globalshortcutfilter.cpp`, `src/core/qguiappcurrentscreen.cpp`

**`src/widgets/`:**
- Purpose: All Qt Widgets UI outside the settings screens
- Contains: capture editor (`capture/`), tray icon, upload dialogs/history, notifications, launcher, color picker
- Key files: `src/widgets/capture/capturewidget.cpp` (main editor), `src/widgets/capture/selectionwidget.cpp`, `src/widgets/capture/buttonhandler.cpp`, `src/widgets/capture/capturetoolbutton.cpp`, `src/widgets/trayicon.cpp`, `src/widgets/capturelauncher.cpp`

**`src/tools/`:**
- Purpose: Editor tools; each tool is a `CaptureTool` subclass in its own subdirectory
- Contains: abstract bases, `ToolFactory`, per-tool dirs, image upload subsystem
- Key files: `src/tools/capturetool.h` (base interface), `src/tools/toolfactory.cpp` (registry), `src/tools/abstracttwopointtool.*`, `src/tools/abstractpathtool.*`, `src/tools/abstractactiontool.*`, `src/tools/capturecontext.h`

**`src/tools/imgupload/`:**
- Purpose: Image upload UI and backends
- Contains: `ImgUploaderManager`, `ImgUploaderBase`, `ImgUploaderTool`, and `storages/imgur/`
- Key files: `src/tools/imgupload/imguploadermanager.cpp`, `src/tools/imgupload/storages/imguploaderbase.cpp`, `src/tools/imgupload/storages/imgur/imguruploader.cpp`

**`src/config/`:**
- Purpose: Settings/preferences UI and config-error resolution
- Contains: config window, shortcut widget, color/visuals editors, filename editor
- Key files: `src/config/configwindow.cpp`, `src/config/generalconf.cpp`, `src/config/shortcutswidget.cpp`, `src/config/configresolver.cpp`, `src/config/cacheutils.cpp`

**`src/utils/`:**
- Purpose: Reusable, mostly non-UI helpers
- Contains: screen grabbing, config handler, logging, filename/path handling, history, save-to-file/clipboard, value handling, desktop info
- Key files: `src/utils/screengrabber.cpp`, `src/utils/confighandler.cpp`, `src/utils/abstractlogger.cpp`, `src/utils/valuehandler.cpp`, `src/utils/filenamehandler.cpp`, `src/utils/screenshotsaver.cpp`, `src/utils/history.cpp`, `src/utils/globalvalues.cpp`

**`data/`:**
- Purpose: Non-code resources compiled/installed with the app
- Contains: application icons, tool icons, translations (`Internationalization_*.ts/.qm`), `.desktop` files, Qt resource files

## Key File Locations

**Entry Points:**
- `src/main.cpp`: `main()`, CLI parsing, mode selection, app/daemon bootstrap
- `src/windows-cli.cpp`: Windows console entry shim

**Configuration:**
- `CMakeLists.txt` (root) and `src/CMakeLists.txt`: build definition, Qt component discovery
- `src/utils/confighandler.cpp` / `.h`: runtime settings (INI via `QSettings`)
- `flameshot.example.ini`: documented example of all config keys
- `.clang-format`, `.clang-tidy`, `.cmake-format.yaml`: code-style tooling

**Core Logic:**
- `src/core/flameshot.cpp`: capture orchestration + `exportCapture` dispatch
- `src/core/flameshotdaemon.cpp`: tray/clipboard/pin daemon + IPC
- `src/widgets/capture/capturewidget.cpp`: interactive editor
- `src/tools/toolfactory.cpp`: tool registry
- `src/utils/screengrabber.cpp`: platform screen capture

**Testing:**
- `tests/action_options.sh`, `tests/path_option.sh`: shell-based CLI smoke tests (no unit-test framework detected)

## Naming Conventions

**Files:**
- Lowercase, no separators, matching the primary class in `PascalCase`: `capturewidget.cpp` -> `CaptureWidget`, `flameshotdaemon.cpp` -> `FlameshotDaemon`
- Header/source pairs `.h` + `.cpp`; Qt Designer forms as `.ui` (e.g. `infowindow.ui`)
- Tools follow `<name>tool.{h,cpp}` inside `src/tools/<name>/` (e.g. `arrow/arrowtool.cpp`)

**Directories:**
- Lowercase single words; one directory per tool under `src/tools/`
- Layered top-level dirs under `src/` (`cli`, `core`, `widgets`, `tools`, `config`, `utils`)

**Code:**
- Classes `PascalCase`; methods/variables `camelCase`; members prefixed `m_` (e.g. `m_captureWindow`); enums `PascalCase` with `UPPER_SNAKE` values (e.g. `CaptureTool::TYPE_ARROW`)
- Formatting enforced by `.clang-format`; static analysis via `.clang-tidy`

## Where to Add New Code

**New annotation tool:**
- Create `src/tools/<newtool>/<newtool>tool.h` and `.cpp`, subclassing the most appropriate base: `AbstractTwoPointTool` (shapes), `AbstractPathTool` (freehand), or `AbstractActionTool` (non-drawing action)
- Add a new value to the `CaptureTool::Type` enum at the BOTTOM (`src/tools/capturetool.h:25` — comment there warns not to reorder)
- Register it in the `ToolFactory::CreateTool` switch (`src/tools/toolfactory.cpp:41`) with `if_TYPE_return_TOOL(...)`
- Update `CaptureToolButton::iterableButtonTypes` / `buttonTypeOrder` (`src/widgets/capture/capturetoolbutton.cpp`) per the header comment
- Add the source files to `src/tools/CMakeLists.txt`
- Add an icon under `data/` if the tool has a button

**New uploader backend:**
- Add `src/tools/imgupload/storages/<service>/<service>uploader.{h,cpp}` subclassing `ImgUploaderBase` (`src/tools/imgupload/storages/imguploaderbase.h:19`), implementing `upload()` and `deleteImage()`
- Wire selection into `ImgUploaderManager::init()` / `uploader()` (`src/tools/imgupload/imguploadermanager.cpp:22`) — the file has TODO stubs showing the intended `if (plugin == "s3") ...` pattern
- Add the source files to `src/tools/CMakeLists.txt` (inside the `ENABLE_IMGUR` block or a new option)

**New config option:**
- Add a `CONFIG_GETTER_SETTER(getter, setter, Type)` line in `src/utils/confighandler.h` (near `:75`)
- Register the key in `recognizedGeneralOptions` and provide a `ValueHandler` in `src/utils/confighandler.cpp` (see the note at `confighandler.h:73`)
- Surface it in the settings UI under `src/config/generalconf.cpp` if user-facing
- Document it in `flameshot.example.ini`

**New CLI option/mode:**
- Add a `CommandOption`/`CommandArgument` and wire it into the relevant mode block in `src/main.cpp` (`:262`-`:682`); add a checker lambda if the value needs validation

**New window/dialog:**
- Implementation under `src/widgets/` (general UI) or `src/config/` (settings); expose an opener on `Flameshot` (`src/core/flameshot.cpp`) if launched from tray/CLI; add to `src/widgets/CMakeLists.txt`

**Utilities:**
- Shared helpers go in `src/utils/` as a `<name>.{h,cpp}` pair, registered in `src/utils/CMakeLists.txt`

## Special Directories

**`data/`:**
- Purpose: Icons, translations (`.ts`/`.qm`), desktop entries, Qt resources
- Generated: Partly (`.qm` compiled from `.ts`)
- Committed: Yes (`.ts` sources committed; build compiles `.qm`)

**`packaging/`:**
- Purpose: OS/distro packaging metadata and helper assets (macOS `.icns`, AppImage, etc.)
- Generated: No
- Committed: Yes

**`scripts/`:**
- Purpose: Build, translation, and release automation scripts
- Generated: No
- Committed: Yes

**`build/` (if present):**
- Purpose: CMake out-of-source build output
- Generated: Yes
- Committed: No (git-ignored)

---

*Structure analysis: 2026-07-24*
