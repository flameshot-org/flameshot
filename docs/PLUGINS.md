# Flameshot Plugin Architecture

Flameshot features a modular C++ plugin architecture based on Qt's `QPluginLoader`. This allows developers and users to build, install, and load custom capture tools, post-processing utilities, and third-party integrations without modifying or recompiling the core Flameshot application.

---

## 1. Overview

Plugins are compiled as dynamic shared libraries:
- Linux: `.so`
- macOS: `.dylib`
- Windows: `.dll`

When Flameshot starts, the **PluginManager** automatically discovers, verifies, and loads all valid plugins from standard search paths. Custom tools provided by enabled plugins automatically appear in the capture toolbar alongside native tools.

A reference implementation can be found under [`examples/sample-plugin/`](../examples/sample-plugin/).

---

## 2. Plugin Search Locations

Flameshot searches for plugins in the following locations (in order of priority):

1. **`FLAMESHOT_PLUGIN_PATH` environment variable**: A colon-separated (or semicolon-separated on Windows) list of directory paths.
2. **User Data Directory**:
   - Linux: `~/.local/share/flameshot/plugins/`
   - Windows: `%APPDATA%/flameshot/plugins/`
   - macOS: `~/Library/Application Support/flameshot/plugins/`
3. **System Library Directory**: `/usr/lib/flameshot/plugins/`
4. **Application Binary Directory**: `<executable_dir>/plugins/`

You can also open the user plugins directory directly from **Flameshot Configuration → Plugins tab → "Open Plugins Folder"**.

---

## 3. Creating a Custom Plugin

### A. Define the Tool and Plugin Classes

Implement `FlameshotPluginInterface` and provide one or more `CaptureTool` subclasses.
The example below shows a **Highlight tool** that draws a yellow semi-transparent rectangle
over the current selection — a concrete starting point you can adapt.

`highlighttool.h`:
```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "plugins/flameshotplugininterface.h"
#include "tools/abstractactiontool.h"

/**
 * @brief HighlightTool draws a semi-transparent yellow overlay on the
 *        captured region when activated from the toolbar.
 */
class HighlightTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit HighlightTool(QObject* parent = nullptr);

    QString name() const override;
    QString description() const override;
    QIcon icon(const QColor& background, bool inEditor) const override;
    CaptureTool::Type type() const override;
    CaptureTool* copy(QObject* parent = nullptr) override;
    void process(QPainter& painter, const QPixmap& pixmap) override;
    bool closeOnButtonPressed() const override;
};

class HighlightPlugin : public QObject, public FlameshotPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FlameshotPluginInterface_iid)
    Q_INTERFACES(FlameshotPluginInterface)

public:
    explicit HighlightPlugin(QObject* parent = nullptr) : QObject(parent) {}

    QString pluginId()          const override { return QStringLiteral("org.flameshot.highlight"); }
    QString pluginName()        const override { return QStringLiteral("Highlight Tool"); }
    QString pluginVersion()     const override { return QStringLiteral("1.0.0"); }
    QString pluginAuthor()      const override { return QStringLiteral("Your Name"); }
    QString pluginDescription() const override {
        return QStringLiteral("Draws a semi-transparent yellow highlight over the selection.");
    }

    QList<CaptureTool*> createTools(QObject* parent = nullptr) override {
        return { new HighlightTool(parent) };
    }
};
```

`highlighttool.cpp`:
```cpp
// SPDX-License-Identifier: GPL-3.0-or-later
#include "highlighttool.h"
#include <QPainter>

HighlightTool::HighlightTool(QObject* parent)
    : AbstractActionTool(parent) {}

QString HighlightTool::name() const        { return QStringLiteral("Highlight"); }
QString HighlightTool::description() const { return QStringLiteral("Yellow highlight overlay"); }
CaptureTool::Type HighlightTool::type() const { return CaptureTool::NONE; }
bool HighlightTool::closeOnButtonPressed() const { return false; }

QIcon HighlightTool::icon(const QColor& /*background*/, bool /*inEditor*/) const
{
    // Replace with your own bundled SVG icon
    return QIcon::fromTheme(QStringLiteral("format-text-color"));
}

CaptureTool* HighlightTool::copy(QObject* parent)
{
    return new HighlightTool(parent);
}

void HighlightTool::process(QPainter& painter, const QPixmap& /*pixmap*/)
{
    painter.save();
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.fillRect(painter.viewport(), QColor(255, 255, 0, 80));
    painter.restore();
}
```

### B. Build with CMake

Place your plugin sources in a directory and point `target_include_directories` at the
Flameshot source tree (the path you cloned / checked out):

```cmake
cmake_minimum_required(VERSION 3.16)
project(highlightplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(highlightplugin MODULE
    highlighttool.h
    highlighttool.cpp
)

# Point this at the src/ directory of your Flameshot source checkout
target_include_directories(highlightplugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src  # adjust as needed
)

target_link_libraries(highlightplugin PRIVATE
    Qt6::Core Qt6::Gui Qt6::Widgets
)
```

Build and install:
```bash
cmake -B build -S .
cmake --build build
cp build/libhighlightplugin.so ~/.local/share/flameshot/plugins/
```

> **Note:** Plugin IDs use reverse-domain notation (`org.flameshot.highlight`). Use your own
> domain or a unique prefix to avoid ID collisions with other community plugins.

---

## 4. Managing Plugins via GUI

Open Flameshot Settings (`flameshot config`) and navigate to the **Plugins** tab to:
- View all detected plugins and their metadata (version, author, description).
- Enable or disable plugins on the fly with real-time UI updates.
- Open the plugins directory in your file manager.
- Rescan and reload plugins without restarting the application.
- Install a plugin directly from a `.so` / `.dll` file via the **"Install Plugin…"** button.

---

## 5. Managing Plugins via CLI

```bash
# List all discovered plugins and their enabled/disabled status
flameshot --list-plugins

# Install a plugin from a compiled library file
flameshot --install-plugin /path/to/libhighlightplugin.so

# Enable or disable a plugin by its reverse-domain ID
flameshot --enable-plugin  org.flameshot.highlight
flameshot --disable-plugin org.flameshot.highlight

# Move a plugin to the disabled directory (stops it from loading)
flameshot --remove-plugin  org.flameshot.highlight

# Open the Plugins configuration tab directly
flameshot plugins
```

---

## 6. Plugin Identifier Conventions

| Field | Convention | Example |
|:------|:-----------|:--------|
| `pluginId` | Reverse-domain, lowercase, dotted | `org.flameshot.highlight` |
| `pluginName` | Short human-readable title | `Highlight Tool` |
| Library name | `lib<projectname>.so` | `libhighlightplugin.so` |
| Build target | Same as project name | `highlightplugin` |

Plugins that ship with Flameshot use the `org.flameshot.plugin.*` namespace. Community
plugins should use their own domain prefix.

---

## 7. Reference Implementations

| Plugin | Location | Description |
|:-------|:---------|:------------|
| Sample Plugin | `examples/sample-plugin/` | Minimal skeleton — start here |
| OCR Plugin | `plugins/ocr-plugin/` | Tesseract-based text extraction with interactive canvas |
| QR Scanner | `plugins/qr-plugin/` | QR code and barcode detector with clipboard and URL support |
