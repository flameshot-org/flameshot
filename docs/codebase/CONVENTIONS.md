# Coding Conventions

**Analysis Date:** 2026-07-24

Flameshot is a cross-platform Qt6 C++ screenshot tool. Source lives in `src/`,
organized by concern: `src/cli`, `src/config`, `src/core`, `src/tools`,
`src/utils`, `src/widgets`. Every tool has its own subdirectory under
`src/tools/<tool>/`.

## Naming Patterns

**Files:**
- All-lowercase, no separators. Header + source pairs share a base name:
  `arrowtool.h` / `arrowtool.cpp`, `filenamehandler.cpp`, `confighandler.cpp`,
  `capturewidget.h`. The file name is the lowercased class name.
- One primary class per file; the file base name matches that class.
- Tool implementations live in `src/tools/<name>/<name>tool.{h,cpp}` (e.g.
  `src/tools/arrow/arrowtool.cpp`).

**Functions and Methods:**
- `camelCase` for methods and free functions: `parseFilename()`,
  `properScreenshotPath()`, `configurationWidget()`, `boundingRect()`.
- Qt override methods keep their Qt spelling (`paintEvent`, `mousePressEvent`).

**Variables:**
- Local variables and parameters: `camelCase` (`configuredArrowStyle`,
  `bottomTranslation`). Short/loop locals may be terse (`i`, `val`, `res`).
- Member variables: `m_` prefix + `camelCase` — `m_arrowPath`, `m_arrowStyle`,
  `m_targets`, `m_defaultChannel`, `m_enableMessageHeader`. This is consistent
  across the codebase; new members MUST use the `m_` prefix.
- File-scope constants in anonymous namespaces use `PascalCase`:
  `ArrowWidth`, `ArrowHeight`, `MinArrowStyle` (see `src/tools/arrow/arrowtool.cpp`).

**Types and Classes:**
- Classes: `PascalCase` — `ArrowTool`, `FileNameHandler`, `AbstractLogger`,
  `CaptureWidget`, `ConfigHandler`.
- Abstract base classes prefixed `Abstract` — `AbstractLogger`,
  `AbstractTwoPointTool` (`src/tools/abstracttwopointtool.h`).
- Enums: `enum class` is rare (1 occurrence, `ArrowStyle`); the codebase mostly
  uses plain C-style `enum` with `PascalCase` enumerators (`Target`, `Channel`
  in `src/utils/abstractlogger.h`). Some legacy enums use `ALL_CAPS` values
  (`TYPE_ARROW`, `CaptureTool::Type`). Prefer `enum class` for new code.

## Code Style

**Formatting:**
- `clang-format`, `BasedOnStyle: Mozilla` with overrides (`.clang-format`):
  `IndentWidth: 4`, `AccessModifierOffset: -4`, and custom brace wrapping —
  braces break onto a new line after `class`, `struct`, `enum`, and function
  definitions, but empty bodies are not split. Control-flow braces stay on the
  same line (`if (...) {`).
- CI enforces formatting via `.github/workflows/clang-format.yml` using
  `DoozyX/clang-format-lint-action@v0.18` with `clangFormatVersion: 11` over
  `./src` (`h,cpp`). Unformatted code fails CI. Pin to clang-format 11 locally
  to match CI exactly.
- CMake files: `cmake-format` per `.cmake-format.yaml` (`line_width: 120`,
  `tab_size: 2`, unix line endings).

**Linting:**
- `.clang-tidy` enables `*` (all checks) minus several families:
  `-fuchsia-*,-google-*,-zircon-*,-abseil-*,-llvm-*,-llvmlibc-*`,
  `-modernize-use-trailing-return-type`, `-performance-no-automatic-move`,
  `-cppcoreguidelines-owning-memory`. `WarningsAsErrors: '*'` — every enabled
  tidy check is an error. Note: clang-tidy is configured but not wired into a
  dedicated CI job (only clang-format runs as a lint gate).
- Additional compiler-warning and static-analyzer toggles live in
  `cmake/CompilerWarnings.cmake` and `cmake/StaticAnalyzers.cmake`.

## Import Organization

**Order** (observed consistently, e.g. `src/tools/arrow/arrowtool.cpp`,
`src/utils/filenamehandler.cpp`, `src/widgets/capture/capturewidget.h`):
1. In a `.cpp`, the matching header first (`#include "arrowtool.h"`).
2. Other project headers in double quotes, using paths relative to `src/`
   (`#include "utils/confighandler.h"`, `#include "tools/capturecontext.h"`).
3. Qt and standard-library headers in angle brackets (`<QComboBox>`,
   `<QPainter>`, `<cmath>`, `<exception>`), grouped after project headers.

**Header guards:**
- `#pragma once` everywhere; no include-guard macros.

**Forward declarations:**
- Headers forward-declare Qt and project classes rather than including them
  where possible (see the block of `class QLabel;`, `class ColorPicker;` in
  `src/widgets/capture/capturewidget.h`) to cut compile-time coupling. Follow
  this in new headers.

**Path Aliases:**
- Not applicable (C++). Include roots are set in CMake; project includes are
  written relative to `src/`.

## Error Handling

**Patterns:**
- No project-wide exception strategy; C++ exceptions are used narrowly. Example:
  `src/utils/filenamehandler.cpp` wraps `std::locale::global()` in
  `try { ... } catch (std::exception&)` and falls back to a default locale,
  reporting via the logger.
- Validation is defensive and inline: clamp/validate then continue with a
  sensible default rather than throwing (e.g. `isValidArrowStyle()` guards in
  `src/tools/arrow/arrowtool.cpp`; empty/degenerate geometry returns `{}`).
- User-facing errors are surfaced through `AbstractLogger::error(...)` (which
  can raise a desktop notification), not exceptions — see `src/main.cpp` and
  `src/core/flameshotdaemon.cpp`.
- Unused parameters are marked with `Q_UNUSED(x)`.

## Logging

**Framework:**
- Project logger `AbstractLogger` (`src/utils/abstractlogger.{h,cpp}`) is the
  primary mechanism. It is a stream-style logger with `Info`/`Warning`/`Error`
  channels and bitmask `Target`s (`Notification`, `Stderr`, `LogFile`, `String`,
  `Stdout`; `Default = Notification | LogFile | Stderr`).
- Raw Qt `qWarning()`/`qDebug()` are used sparingly, mostly in `src/main.cpp`
  and `src/core/flameshotdaemon.cpp` for early-startup / DBus diagnostics.

**Patterns:**
- Construct and stream: `AbstractLogger::error() << tr("Unable to connect via DBus");`
  (`src/core/flameshotdaemon.cpp`), `AbstractLogger::info() << "Screenshot aborted.";`
  (`src/main.cpp`).
- Choose an explicit target when the default set is wrong, e.g.
  `AbstractLogger::error(AbstractLogger::Stderr)`
  (`src/utils/filenamehandler.cpp`), or disable the header with
  `.enableMessageHeader(false)` (`src/cli/commandlineparser.cpp`).
- Prefer `AbstractLogger` over raw `qWarning`/`qDebug` for anything the user
  should see, because it can route to notifications and the log file.

## Comments and Documentation

**When to Comment:**
- Comments explain non-obvious geometry math and intent, not mechanics — see the
  inline explanations in `getArrowHead`/`getCurvedArrowShaft`
  (`src/tools/arrow/arrowtool.cpp`). Acknowledged hacks are labeled ("this is
  hack, but looks not very bad").
- Every source and header begins with SPDX headers:
  `// SPDX-License-Identifier: GPL-3.0-or-later` and
  `// SPDX-FileCopyrightText: ...`. New files MUST include these.
- Files derived from other projects add attribution comments (see the header of
  `src/widgets/capture/capturewidget.h`).

**Doc Comments:**
- Doxygen-style `/** ... */` with `@brief`, `@param`, `@note` is used for a
  minority of public APIs (~13 files), e.g. the block above
  `FileNameHandler::properScreenshotPath` in `src/utils/filenamehandler.cpp` and
  the class doc on `AbstractLogger`. Not universal — most methods are
  undocumented. Add Doxygen comments for non-trivial public functions.

## Function and Module Design

**Size:** Functions are generally small-to-medium and single-purpose;
complex logic (geometry, path handling) is decomposed into free helper
functions in anonymous namespaces rather than large methods.

**Parameters:** Pass Qt/STL objects by `const&` (`const QColor&`,
`const QString&`, `const QPixmap&`). Small value types (`QPoint`, `int`) are
passed by value. Out/mutation params like `QPainter&` are passed by non-const
reference.

**Return Values:** Return by value; use brace-init `return {};` for empty/default
objects (`QRect`, `QPainterPath`). `nullptr` default arguments for optional
`QObject* parent`.

**Exports:** Not applicable (C++). Visibility is expressed via class access
sections. Internal helpers are placed in an unnamed `namespace { ... }` at the
top of the `.cpp` to give them internal linkage (see `arrowtool.cpp`).

**Barrel Files:** Not applicable.

## Qt-Specific Idioms

- `Q_OBJECT` in every QObject-derived class; MOC is enabled via
  `set(CMAKE_AUTOMOC ON)` in `src/CMakeLists.txt` (also `AUTORCC`, `AUTOUIC`).
- Access sections use Qt keywords in a consistent order:
  `public:` → `public slots:` → `signals:` → `protected:` →
  `private slots:` → `private:` (see `src/widgets/capture/capturewidget.h`).
- Constructors take `QObject* parent = nullptr` (or a Qt widget parent) and
  forward to the base initializer list.
- Virtual overrides are explicitly marked `override` (343 occurrences).
- Signal/slot connections use the type-safe pointer-to-member syntax with
  `qOverload<...>()` for overloaded signals (see the `QComboBox` connect in
  `ArrowTool::configurationWidget`).
- User-facing strings are wrapped in `tr("...")` for translation; string
  literals otherwise use `QStringLiteral(...)` / `QLatin1String(...)` rather
  than bare `"..."`.
- Heap widgets are created with `new` and parented (Qt ownership), assigned to
  `auto*` locals (`auto* widget = new QWidget();`).

---

*Convention analysis: 2026-07-24*
