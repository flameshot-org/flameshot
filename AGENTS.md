# flameshot

Qt6/C++20 截图 + 标注工具，GPLv3+。

## 构建

```sh
# 依赖: Qt6 (6.2.4+) Core/Gui/Widgets/Network/Svg/DBus(Linux), GCC 11+, CMake 3.22+
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
# 产物在 build/src/flameshot
```

关键 CMake 选项（在 `CMakeLists.txt:79-87` 定义）：

| 选项 | 默认 | 说明 |
|------|------|------|
| `USE_PORTABLE_CONFIG` | OFF (Linux), ON (Win) | 配置存应用目录而非 `~/.config` |
| `DISABLE_UPDATE_CHECKER` | OFF | 禁用版本检查 |
| `ENABLE_IMGUR` | OFF | 启用 Imgur 上传（编译时加入相关源码） |
| `USE_WAYLAND_CLIPBOARD` | OFF | 使用 KF6GuiAddons Wayland 剪贴板 |
| `FLAMESHOT_DEBUG_CAPTURE` | OFF | 调试截图捕获模式 |
| `USE_MONOCHROME_ICON` | OFF | 默认使用单色图标 |
| `USE_KDSINGLEAPPLICATION` | ON | 单实例保护（bundled 或系统） |
| `GENERATE_TS` | OFF | 重新生成翻译 .ts 源文件 |

Nix 开发环境：`nix develop`（或 `nix-shell`）。

## 架构

```
src/
├── main.cpp         # 入口：CLI 解析 → 分发到 gui/full/screen/config/launcher
├── cli/             # 自定义 CLI 解析器（非 QCommandLineParser）
├── core/            # Flameshot 单例、FlameshotDaemon（后台服务+托盘）、DBus 适配器(Linux)
├── config/          # 配置窗口、UI 颜色/快捷键编辑器
├── utils/           # ConfigHandler(QSettings)、屏幕抓取、文件保存、文件名格式化
├── widgets/         # CaptureWidget(截图编辑器核心)、TrayIcon、CaptureLauncher
└── tools/           # 标注工具: arrow/circle/line/pencil/marker/pixelate/text/...
    ├── abstractactiontool.h   # 一次性动作工具基类
    ├── abstractpathtool.h     # 路径绘制工具基类
    └── abstracttwopointtool.h # 两点工具基类
```

## 运行模式

| 命令 | 行为 |
|------|------|
| `flameshot`（无参数） | 启动后台 daemon + 系统托盘 |
| `flameshot gui` | 交互式截图 GUI |
| `flameshot full` | 全屏截图，无 GUI |
| `flameshot screen` | 指定屏幕截图，`-n 0` 指定屏幕 |
| `flameshot config` | 打开配置 GUI（或 `-m mainColor -k contrastColor` CLI 配置） |

`--raw` 使命令阻塞等待截图完成并输出 PNG 数据。`--print-geometry` 输出 `WxH+X+Y` 格式的区域信息。

配置路径：`~/.config/flameshot/flameshot.ini`。
完整示例：`flameshot.example.ini`。

## 代码风格

- `.clang-format`：Mozilla 风格，缩进 4，BraceWrapping 开启 class/struct/enum/function
- `.clang-tidy`：开启全部检查，排除 fuchsia/google/zircon/abseil/llvm 组，warnings as errors
- CI 用 clang-format 11 检查 `src/*.{h,cpp}`
- 格式化命令：`clang-format -i $(git ls-files "*.cpp" "*.h")`

## 测试

**没有自动化单元测试。** `tests/` 下的脚本是手动测试：

- `tests/action_options.sh` — 需要显示器、人工确认通知、在 GUI 中进行选择
- `tests/path_option.sh` — 验证 `-p` 路径选项的各种场景

两者都需要运行中的 flameshot daemon 和图形环境。运行方式：
```sh
bash tests/action_options.sh ./build/src/flameshot
```

CI 中 `ctest` 不运行任何真实测试（无 CTest 注册项）。

## 平台相关

- **Windows**：构建两个可执行文件 — `flameshot.exe`（GUI，`WIN32_EXECUTABLE`，无控制台输出）和 `flameshot-cli.exe`（控制台子系统，用于 `-h` 等有 stdout 的场景）
- **macOS**：Qt6 路径 `-DQt6_DIR="$(brew --prefix qt6)/lib/cmake/Qt6"`，打包用 `make create_dmg`
- **Linux**：通过 DBus 接口（`org.flameshot.Flameshot`）通信；wayland 需参考 `docs/UsageHyprlandSwayWlroots.md`

## 依赖管理

- Qt-Color-Widgets：gitlab.com/mattbas/Qt-Color-Widgets，通过 FetchContent 或 `external/` 本地拷贝
- KDSingleApplication：github.com/KDAB/KDSingleApplication，通过 FetchContent 或 `external/` 本地拷贝
- QHotkey：仅 Windows/macOS 使用
- 所有 dependencies 通过编译 CMake 自动下载，无需手动安装

## 构建产物

- `build/_deps/` — FetchContent 下载的依赖源码
- `build/src/translations/*.qm` — 编译后的翻译文件
- `cmake --build build` 后可执行文件在 `build/src/flameshot`

## 注意事项

- 首次启动较慢（DBus 初始化），可设置开机自启
- 最小化窗口管理器（i3/dwm/xmonad）可能需要启用 `useX11LegacyScreenshot` 配置项，见 `docs/UsageX11MinimalWM.md`
- 修改按钮列表、快捷键绑定等复杂配置建议用 GUI：`flameshot config`
- 翻译在 `data/translations/Internationalization_*.ts`，通过 Weblate 协作
