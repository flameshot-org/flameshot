# Flameshot developer docs

Thank you for your interest in developing flameshot. This developer
documentation (hopefully) has an intuitive structure. It tries to describe what
code is run when a user performs an action in Flameshot.

!!! important

    **Please read this entire page. It will make your life a whole lot easier when
    contributing to Flameshot. If you know exactly what you want to work on, you
    should look at [FAQ](./faq) **

## Project structure

Flameshot is built on C++/Qt5 with CMake as its build system. The source code is
located under `src/`. The entrypoint is `src/main.cpp`.

### `main.cpp`

Flameshot provides both a GUI and a CLI (the latter currently works only on
Linux and macOS).

### Build system

The main cmake file is `CMakeLists.txt` in the project root. It `include`s some
files from the `cmake/` directory as well. These files together control some
more general aspects of the build process, like project information, packaging,
caching etc.

There is also the file `src/CMakeLists.txt`. It mostly defines how the source
files are compiled into targets and how the external libraries are linked. It
does some other stuff too. Currently, there isn't a clear separation of concerns
between `CMakeLists.txt` and `src/CMakeLists.txt`. In the future we should
refactor these files to make it more clear why each of them exists.

## What happens when I launch flameshot?
There are two ways to launch flameshot: daemon mode and single-action mode. In
both modes, an instance of [`Flameshot`][Flameshot] is created via
[`Flameshot::start()`][Flameshot::start]. [`Flameshot`][Flameshot] provides the
high level API for interacting with flameshot; and its methods mimic the CLI
subcommands a great deal. This object is a singleton, so it can only be created
once. It is accessed as [`Flameshot::instance()`][Flameshot::instance].

!!! note

    On Windows, only daemon mode is currently supported.

### Single-action mode (via command line interface)
Single-action mode (also called one-off mode) is triggered when flameshot is
launched with a command line argument - for example as `flameshot gui`. As its
name implies, it performs a single action, such as "take a screenshot
interactively by opening a GUI" or "take a screenshot of the entire screen",
etc. Afterwards, Flameshot quits.

### Daemon mode
This mode is triggered when the `flameshot` command is launched. In this mode, a
flameshot process is started in the background. A system tray is displayed if
the user hasn't disabled it in the config. In addition to [`Flameshot::start()`][Flameshot::start],
if the current process is the daemon, it also calls [`FlameshotDaemon::start()`][FlameshotDaemon::start]
during initialization.

The daemon has the following purposes:

- Run in the background, wait for the user to press a hotkey, and perform
  corresponding action.

    This is true for **Windows** and **macOS**, but not for **Linux**. On Linux, hotkeys
    are meant to be handled by the desktop environment or equivalent.

- Provide a system tray that the user can click to initiate actions via context
  menu

- Periodically check for updates and notify the user

- Act as a host for persistent phenomena. Example: On X11 (linux), when a program
  inserts content into the clipboard, it must keep running so the content
  persists in the clipboard.

!!! note

    All of the above are user-configurable.

#### `FlameshotDaemon`
The class [`FlameshotDaemon`][FlameshotDaemon] handles all communication with
the daemon. The class provides public static methods that are designed so that
the caller does not need to know if the current process is a flameshot daemon or
a single-action invocation of Flameshot. If the current process is the daemon,
then the static methods of [`FlameshotDaemon`][FlameshotDaemon] will call the
corresponding instance methods of the singleton. If not, the current process
will communicate with the daemon process via D-Bus. Then, within the daemon
process, those D-Bus calls will be translated into
[`FlameshotDaemon`][FlameshotDaemon] instance method calls.

## Configuration
The configuration is handled by [`ConfigHandler`][ConfigHandler]. It is
decoupled from any user interface, so it serves the configuration for both the
GUI and CLI. All configuration settings recognized by the config files are
defined as getters in this class. There are also setters for each setting, named
as per the usual convention. For example, the setting `savePath` has a getter
named `savePath` and a setter named `setSavePath`. Before working on a new
config setting for flameshot, please read [this FAQ
entry][faq:add-config-setting].

### Interesting notes

- [`ConfigHandler`][ConfigHandler] is based on `QSettings`
- The configuration uses the `ini` format
- The configuration is automatically reloaded when the config file changes

## Conventions

- Always use `&Class::signal` and `&Class::slot` instead of `SIGNAL(signal())`
  and `SLOT(slot())`. This usually provides better code introspection and makes
  refactoring easier and less error-prone.

[Flameshot]: flameshot/classFlameshot
[Flameshot::instance]: flameshot/classFlameshot#function-instance
[Flameshot::start]: flameshot/classFlameshot#function-start
[ConfigHandler]: flameshot/classConfigHandler
[FlameshotDaemon]: flameshot/classFlameshotDaemon
[FlameshotDaemon::start]: flameshot/classFlameshotDaemon#function-start
[confighandler.h]: flameshot/confighandler_8h
[confighandler.cpp]: flameshot/confighandler_8cpp

[faq:add-config-setting]: faq/#how-do-i-add-a-new-config-setting

[matrix-room]: https://matrix.to/#/#flameshot-org:matrix.org
