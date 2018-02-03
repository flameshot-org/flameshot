# Flameshot
![image](./img/flameshot.png) 
> Powerful yet simple to use screenshot software.

[![Build Status](https://img.shields.io/travis/lupoDharkael/flameshot.svg)](https://travis-ci.org/lupoDharkael/flameshot)
[![Build status](https://ci.appveyor.com/api/projects/status/github/lupoDharkael/flameshot?svg=true)](https://ci.appveyor.com/project/lupoDharkael/flameshot)
[![License](https://img.shields.io/github/license/lupoDharkael/flameshot.svg)](https://github.com/lupoDharkael/flameshot/blob/master/LICENSE)
[![Release](https://img.shields.io/github/release/lupoDharkael/flameshot.svg)](https://github.com/lupoDharkael/flameshot/releases)

## Usage Preview
![image](./img/appPreview/animatedUsage.gif)

## Index
- [Features](#features)
- [Usage](#usage)
- [Keyboard Shortcuts](#keyboard-shortcuts)
- [Considerations](#considerations)
- [Installation](#installation)
- [Compilation](#compilation)
  - [Debian](#debian)
  - [Fedora](#fedora)
  - [Arch](#arch)
  - [Install](#install)
- [Packaging](#packaging)
- [License](#license)
- [Donations](#donations)

## Features
- Customizable appearance.
- Easy to use.
- In-app screenshot edition.
- DBus interface.
- Upload to Imgur.

## Usage
Example commands:
- capture with GUI:

   `flameshot gui`

- capture with GUI with custom save path:

   `flameshot gui -p ~/myStuff/captures`

- open GUI with a delay of 2 seconds:

   `flameshot gui -d 2000`

- fullscreen capture (asking savepath):

   `flameshot full`

- fullscreen capture with custom save path (no GUI) and delayed:

   `flameshot full -p ~/myStuff/captures -d 5000`

- fullscreen capture with custom save path copying to clipboard:

   `flameshot full -c -p ~/myStuff/captures`

In case of doubt choose the first or the second command as shortcut in your favorite desktop environment.

A systray icon will be in your system's panel while Flameshot is running.
Do a right click on the tray icon and you'll see some menu items to open the configuration window and the information window.
Check out the information window to see all the available shortcuts in the graphical capture mode.

### CLI configuration
You can use the graphical menu to configure Flameshot, but alternatively you can use your terminal or scripts to do so.

- open the confguration menu:

   `flameshot config`

- show the initial help message in the capture mode:

   `flameshot config --showhelp true`

- for more information about the available options use the help flag:

   `flameshot config -h`

## Keyboard shortcuts

### Local

These shortcuts are available in GUI mode:

|  Keys                                                                                         |  Description                         |
|---                                                                                            |---                                   |
| <kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd>                                        | Move selection 1px                   |
| <kbd>Shift</kbd> + <kbd><kbd>←</kbd>, <kbd>↓</kbd>, <kbd>↑</kbd>, <kbd>→</kbd></kbd>          | Resize selection 1px                 |
| <kbd>Esc</kbd>                                                                                | Quit capture                         |
| <kbd>Ctrl</kbd> + <kbd>C</kbd>                                                                | Copy to clipboard                    |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                                                                | Save selection as a file             |
| <kbd>Ctrl</kbd> + <kbd>Z</kbd>                                                                | Undo the last modification           |
| Right Click                                                                                   | Show color picker                    |
| Mouse Wheel                                                                                   | Change the tool's thickness          |

<kbd>Shift</kbd> + drag a handler of the selection area: mirror redimension in the opposite handler.

### Global

If you want use Flameshot as a default screenshot utility, chanses are you want to launch it using the <kbd>Prt Sc</kbd> key. Flameshot doesn't yet offer a fully-automated option to do so, but you can configure your system to do so.

#### On KDE Plasma desktop

To make configuration easier, there's a [file](docs/shortcuts-config/flameshot-shortcuts-kde) in the repository that more or less automates this process. This file will assign the following keys to the following actions by default:

|  Keys                                                           |  Description                                                                                |
|---                                                              |---                                                                                          |
| <kbd>Prt Sc</kbd>                                               | Start the Flameshot screenshot tool and take a screenshot                                   |
| <kbd>Ctrl</kbd> + <kbd>Prt Sc</kbd>                             | Wait for 3 seconds, then start the Flameshot screenshot tool and take a screenshot          |
| <kbd>Shift</kbd> + <kbd>Prt Sc</kbd>                            | Take a full-screen (all monitors) screenshot and save it                                    |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>Prt Sc</kbd>          | Take a full-screen (all monitors) screenshot and copy it to the clipboard                   |

If you don't like the defaults, you can change them manually later.

Steps for using the configuration:

1. The configuration file configures shortcuts so that Flameshot automatically saves (without opening the save dialog) screenshots to _~/Pictures/Screenshots_ folder. Make sure you have that folder by running the following command:
    ```
    mkdir -p ~/Pictures/Screenshots
    ```
   (If you don't like the default location, you can skip this step and configure your preferred directory later.)

2. Download the configuration file:
    ```
    cd ~/Desktop; wget https://raw.githubusercontent.com/lupoDharkael/flameshot/master/docs/shortcuts-config/flameshot-shortcuts-kde
    ```
3. Go to _System Settings_ → _Shortcuts_ → _Custom Shortcuts_.
4. If there's one, you'll need to disable an entry for Spectacle, the default KDE screenshot utility first because its shortcuts might collide with Flameshot's ones; so, just uncheck the _Spectacle_ entry.
5. Click _Edit_ → _Import..._, navigate to the Desktop folder (or wherever you saved the configuration file) and open the configuration file.
6. Now the Flameshot entry should appear in the list. Click _Apply_ to apply the changes.
7. If you want to change the defaults, you can expand the entry, select the appropriate action and modify it as you wish; the process is pretty mush self-explanatory.

## Considerations

- Experimental Gnome Wayland and Plasma Wayland support.

- If you are using Gnome you need to install the [TopIcons](https://extensions.gnome.org/extension/1031/topicons/) extension in order to see the systemtray icon.

- In order to speed up the first launch of Flameshot (DBus init of the app can be slow), consider starting the application automatically on boot.

- Press <kbd>Enter</kbd> or <kbd>Ctrl</kbd> + <kbd>C</kbd> when you are in a capture mode and you don't have an active selection and the whole desktop will be copied to your clipboard! Pressing <kbd>Ctrl</kbd> + <kbd>S</kbd> will save your capture in a file! Check the [Shortcuts](#shortcuts) for more information.

- Execute the command `flameshot` without parameters or use the "Launch Flameshot" desktop entry to launch a running instance of the program without taking actions.

## Installation

There are a packages available for a few distros:
- [Arch](https://aur.archlinux.org/packages/flameshot/)
- [openSUSE](https://software.opensuse.org/package/flameshot)
- [Void Linux](https://github.com/voidlinux/void-packages/tree/master/srcpkgs/flameshot) (`xbps-install flameshot`)
- [Docker](https://github.com/ManuelLR/docker-flameshot)
- Fedora: dnf install flameshot

## Compilation
The compilation requires Qt version 5.3 or higher and GCC 4.9.2 or higher.

### Debian
Compilation Dependencies:
````
apt install git g++ build-essential qt5-qmake qt5-default qttools5-dev-tools
````

Compilation: run `qmake && make` in the main directory.

### Fedora
Compilation Dependencies:
````
dnf install qt5-devel gcc-c++ git qt5-qtbase-devel qt5-linguist
````

Compilation:  run `qmake-qt5 && make` in the main directory.

### Arch
Compilation Dependencies:
````
pacman -S git qt5-base base-devel qt5-tools
````

Compilation:  run `qmake && make` in the main directory.

### Install

Simply use `make install` with privileges.

## Packaging

In order to generate the makefile installing in `/usr` instead of in `/usr/local` you can use the `packaging` option to generate the proper makefile (`qmake CONFIG+=packaging` instead of just `qmake`).

If you want to install in a custom directory you can define the `BASEDIR` variable.

**Example**:
You want to install Flameshot in ~/myBuilds/test. You would execute the following to do so:
`qmake CONFIG+=packaging BASEDIR=~/myBuilds/test && make install`

### Runtime Dependencies

**Debian**:
````
libqt5dbus5, libqt5network5, libqt5core5a, libqt5widgets5, libqt5gui5
````
Optional:
```
openssl, ca-certificates
```

**Fedora**:
````
qt5-qtbase
````
Optional:
```
openssl, ca-certificates
```

**Arch**:
````
qt5-base
````
Optional:
```
openssl, ca-certificates
```

## License
- The main code is licensed under [GPLv3](./LICENSE)
- The logo of Flameshot is licensed under [Free Art License v1.3](./img/flameshotLogoLicense.txt)
- The button icons are licensed under Apache License 2.0. See: https://github.com/google/material-design-icons
- The code at capture/capturewidget.cpp is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.cpp (GPLv2)
- The code at capture/capturewidget.h is based on https://github.com/ckaiser/Lightscreen/blob/master/dialogs/areadialog.h (GPLv2)
- I copied a few lines of code from KSnapshot regiongrabber.cpp revision 796531 (LGPL)
- Qt-Color-Widgets taken and modified from https://github.com/mbasaglia/Qt-Color-Widgets (see their license and exceptions in the project) (LGPL/GPL)

Info: If I take code from your project and that implies a relicense to GPLv3, you can reuse my changes with the original previous license of your project applied.

## Donations
I improve Flameshot in my free time because I want to create something good for everyone to use. 
If you want you can donate some bucks [here](https://www.paypal.me/lupoDharkael).

## Acknowledgment
I really appreciate those who have shown interest in the early development process:
- [Cosmo](https://github.com/philpem)
- [XerTheSquirrel](https://github.com/XerTheSquirrel)
- [The members of Sugus GNU/Linux](https://github.com/SUGUS-GNULinux)
- ismatori
