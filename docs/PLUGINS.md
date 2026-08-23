# Flameshot Plugin Architecture

Flameshot features a modular C++ plugin architecture based on Qt's `QPluginLoader`. This allows developers and users to build, install, and load custom capture tools, post-processing utilities, and third-party integrations without modifying or recompiling the core Flameshot application.

---

## 1. Overview

Plugins are compiled as dynamic shared libraries:
- Linux: `.so`
- macOS: `.dylib`
- Windows: `.dll`

When Flameshot starts, the **PluginManager** automatically discovers, verifies, and loads all valid plugins from standard search paths. Custom tools provided by enabled plugins automatically appear in the capture toolbar alongside native tools.

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

### A. Define the Plugin Class
Implement `FlameshotPluginInterface` and provide custom `CaptureTool` instances:

```cpp
#include "plugins/flameshotplugininterface.h"
#include "tools/abstractactiontool.h"

class MyCustomTool : public AbstractActionTool
{
    Q_OBJECT
public:
    explicit MyCustomTool(QObject* parent = nullptr) : AbstractActionTool(parent) {}

    QString name() const override { return "My Tool"; }
    QString description() const override { return "Does awesome custom processing"; }
    QIcon icon(const QColor& background, bool inEditor) const override {
        return QIcon(":/mytool/icon.svg");
    }
    CaptureTool::Type type() const override { return CaptureTool::NONE; }
    CaptureTool* copy(QObject* parent = nullptr) override { return new MyCustomTool(parent); }
    void process(QPainter& painter, const QPixmap& pixmap) override {
        // Custom drawing or image processing
    }
    bool closeOnButtonPressed() const override { return false; }
};

class MyPlugin : public QObject, public FlameshotPluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID FlameshotPluginInterface_iid)
    Q_INTERFACES(FlameshotPluginInterface)

public:
    QString pluginId() const override { return "com.example.myplugin"; }
    QString pluginName() const override { return "My Custom Plugin"; }
    QString pluginVersion() const override { return "1.0.0"; }
    QString pluginAuthor() const override { return "Jane Doe"; }
    QString pluginDescription() const override { return "Provides custom processing tools."; }

    QList<CaptureTool*> createTools(QObject* parent = nullptr) override {
        return { new MyCustomTool(parent) };
    }
};
```

### B. Build with CMake
```cmake
cmake_minimum_required(VERSION 3.16)
project(myplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(myplugin MODULE
    myplugin.h
    myplugin.cpp
)

target_include_directories(myplugin PRIVATE /path/to/flameshot/src)
target_link_libraries(myplugin PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
```

---

## 4. Managing Plugins via GUI

Open Flameshot Settings (`flameshot config`) and navigate to the **Plugins** tab to:
- View all detected plugins and their metadata (version, author, description).
- Enable or disable plugins on the fly with real-time UI updates.
- Open the plugins directory in your file manager.
- Rescan and reload plugins without restarting the application.
